#include "EthernetAdapter.hpp"
#include "esp_log.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_eth.h"
#include "esp_eth_com.h"
#include "esp_eth_phy.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_check.h"
#include "sdkconfig.h"
#include "config.h"  // for W5500_* defines
#include <cstring>    // for memset

static const char* TAG = "EthernetAdapter";

EthernetAdapter::EthernetAdapter()
    : eth_netif(nullptr), connected(false) {
    ESP_LOGI(TAG, "EthernetAdapter constructor called");
}

void EthernetAdapter::init() {
    ESP_LOGI(TAG, "Initializing Ethernet (W5500)...");

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&netif_cfg);
    assert(eth_netif);
    ESP_LOGI(TAG, "esp_netif created");
    ESP_ERROR_CHECK(esp_netif_set_default_netif(eth_netif));
    ESP_LOGI(TAG, "esp_netif set as default");

    // Configure SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = W5500_MOSI,
        .miso_io_num = W5500_MISO,
        .sclk_io_num = W5500_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_DISABLED));
    ESP_LOGI(TAG, "SPI bus initialized");

    // SPI device config passed to MAC creation (do NOT manually add device)
    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 128,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = 10 * 1000 * 1000,
        .input_delay_ns = 0,
        .spics_io_num = W5500_CS,
        .flags = 0,
        .queue_size = 20,
        .pre_cb = nullptr,
        .post_cb = nullptr,
    };

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = -1;
    phy_config.reset_gpio_num = -1;

    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(SPI2_HOST, &devcfg);
    w5500_config.int_gpio_num = -1;           // Not using INT pin
    w5500_config.poll_period_ms = 1000;       // Polling mode

    esp_eth_mac_t* mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    esp_eth_phy_t* phy = esp_eth_phy_new_w5500(&phy_config);

    if (!mac || !phy) {
        ESP_LOGE(TAG, "Failed to create MAC or PHY driver");
        abort();
    }
    ESP_LOGI(TAG, "MAC and PHY drivers created");

    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = nullptr;

    ESP_ERROR_CHECK(gpio_install_isr_service(0));  // Required even if int_gpio = -1
    ESP_LOGI(TAG, "ISR service installed");
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
    ESP_LOGI(TAG, "Ethernet driver installed");
    ESP_LOGI(TAG, "Preparing to attach netif");
    void* glue = esp_eth_new_netif_glue(eth_handle);
    if (!glue) {
        ESP_LOGE(TAG, "Failed to create netif glue");
        return;
    }
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, glue));
    ESP_LOGI(TAG, "Netif attached to Ethernet driver");

    ESP_LOGI(TAG, "Checking eth_handle before starting Ethernet");
    if (!eth_handle) {
        ESP_LOGE(TAG, "eth_handle is null before esp_eth_start");
        return;
    }
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));

#if USE_STATIC_IP
    ESP_LOGI(TAG, "Using static IP");
    esp_netif_dhcpc_stop(eth_netif);
    esp_netif_ip_info_t ip_info;
    ip4addr_aton(STATIC_IP_ADDR, &ip_info.ip);
    ip4addr_aton(STATIC_GW_ADDR, &ip_info.gw);
    ip4addr_aton(STATIC_NETMASK, &ip_info.netmask);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(eth_netif, &ip_info));
#else
    ESP_LOGI(TAG, "Starting DHCP client");
    ESP_ERROR_CHECK(esp_netif_dhcpc_start(eth_netif));
#endif

    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    ESP_LOGI(TAG, "Ethernet initialized successfully");
}

void EthernetAdapter::loop() {
    connected = eth_netif && esp_netif_is_netif_up(eth_netif);
    ESP_LOGD(TAG, "Ethernet loop: connected = %s", connected ? "true" : "false");
}

bool EthernetAdapter::isConnected() const {
    ESP_LOGI(TAG, "Checking connection status: %s", connected ? "connected" : "disconnected");
    return connected;
}

std::string EthernetAdapter::getIPAddress() const {
    if (!connected || !eth_netif) {
        ESP_LOGW(TAG, "Not connected or netif not available when getting IP address");
        return "0.0.0.0";
    }

    esp_netif_ip_info_t ip;
    esp_netif_get_ip_info(eth_netif, &ip);
    char buf[16];
    snprintf(buf, sizeof(buf), IPSTR, IP2STR(&ip.ip));
    ESP_LOGI(TAG, "Current IP address: %s", buf);
    return std::string(buf);
}

esp_netif_t* EthernetAdapter::getNetif() const {
    ESP_LOGI(TAG, "Returning eth_netif pointer");
    return eth_netif;
}