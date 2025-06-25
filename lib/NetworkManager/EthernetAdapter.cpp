#include "EthernetAdapter.hpp"
#include "esp_log.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_eth.h"
#include "esp_eth_com.h"
#include "esp_eth_phy.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_check.h"
#include "sdkconfig.h"
#include "config.h"  // for W5500_* defines

static const char* TAG = "EthernetAdapter";

EthernetAdapter::EthernetAdapter()
    : eth_netif(nullptr), connected(false) {}

void EthernetAdapter::init() {
    ESP_LOGI(TAG, "Initializing Ethernet (W5500)...");

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&netif_cfg);
    assert(eth_netif);

    ESP_ERROR_CHECK(esp_netif_set_default_netif(eth_netif));

    // Configure SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = W5500_MOSI,
        .miso_io_num = W5500_MISO,
        .sclk_io_num = W5500_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // SPI device config
    spi_device_interface_config_t devcfg = {
        .command_bits = 16,
        .address_bits = 8,
        .dummy_bits = 0,
        .mode = 0,
        .clock_speed_hz = 10 * 1000 * 1000,
        .spics_io_num = W5500_CS,
        .queue_size = 20,
    };

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

    phy_config.phy_addr = -1;  // not used by W5500
    phy_config.reset_gpio_num = W5500_RST;

    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(SPI2_HOST, &devcfg);
    esp_eth_mac_t* mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    esp_eth_phy_t* phy = esp_eth_phy_new_ksz80xx(&phy_config);

    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = nullptr;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));

#if USE_STATIC_IP
    esp_netif_dhcpc_stop(eth_netif);
    esp_netif_ip_info_t ip_info;
    ip4addr_aton(STATIC_IP_ADDR, &ip_info.ip);
    ip4addr_aton(STATIC_GW_ADDR, &ip_info.gw);
    ip4addr_aton(STATIC_NETMASK, &ip_info.netmask);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(eth_netif, &ip_info));
#else
    ESP_ERROR_CHECK(esp_netif_dhcpc_start(eth_netif));
#endif

    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    ESP_LOGI(TAG, "Ethernet initialized.");
}

void EthernetAdapter::loop() {
    connected = eth_netif && esp_netif_is_netif_up(eth_netif);
}

bool EthernetAdapter::isConnected() const {
    return connected;
}

std::string EthernetAdapter::getIPAddress() const {
    if (!connected || !eth_netif) return "0.0.0.0";
    esp_netif_ip_info_t ip;
    esp_netif_get_ip_info(eth_netif, &ip);
    char buf[16];
    snprintf(buf, sizeof(buf), IPSTR, IP2STR(&ip.ip));
    return std::string(buf);
}

esp_netif_t* EthernetAdapter::getNetif() const {
    return eth_netif;
}
