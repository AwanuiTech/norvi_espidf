#include "EthernetAdapter.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_eth_driver.h"
#include "config.h"
#include <string>

static const char *TAG = "ETH_W5500";

/// Constructor /////////////////////////////////////////////////////////////////////////////////

EthernetAdapter::EthernetAdapter()
    : eth_netif(nullptr), connected(false) {
    ESP_LOGI(TAG, "EthernetAdapter constructor called");
}

/// Initialise Ethernet /////////////////////////////////////////////////////////////////////////

// @extends adapter
void EthernetAdapter::init() {
    
    ESP_LOGI(TAG, "Initializing W5500 Ethernet...");

    //#############################################//

    ESP_LOGI(TAG, "Step 1: esp_netif_init");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_LOGI(TAG, "Step 1 done");

    //#############################################//

    ESP_LOGI(TAG, "Step 2: esp_event_loop_create_default");
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_LOGI(TAG, "Step 2 done");

    //#############################################//

    ESP_LOGI(TAG, "Step 3: gpio_install_isr_service");
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_LOGI(TAG, "Step 3 done");

    //#############################################//

    ESP_LOGI(TAG, "Step 4: esp_netif_new");
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);
    ESP_LOGI(TAG, "Step 4 done");

    //#############################################//
    ESP_LOGI(TAG, "Step 5: spi_bus_initialize");
    spi_bus_config_t buscfg = {};
        buscfg.miso_io_num = W5500_MISO;
        buscfg.mosi_io_num = W5500_MOSI;
        buscfg.sclk_io_num = W5500_SCK;
        buscfg.quadwp_io_num = -1;
        buscfg.quadhd_io_num = -1;
        buscfg.max_transfer_sz = 2044;
    
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_DISABLED));
    ESP_LOGI(TAG, "Step 5 done");

    //#############################################//

    ESP_LOGI(TAG, "Step 6: spi_bus_add_device");
    // Changed to same as SPI test code that worked.
    spi_device_interface_config_t devcfg = {};
        devcfg.clock_speed_hz = 1 * 1000 * 1000; // Lowered for debug
        devcfg.mode = 0;
        devcfg.spics_io_num = W5500_CS;
        devcfg.command_bits = 0;
        devcfg.queue_size = 1;
        devcfg.address_bits = 0;

    spi_device_handle_t spi_handle;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle));
    vTaskDelay(pdMS_TO_TICKS(250));
    ESP_LOGI(TAG, "Step 6 done");

    //#############################################//

    ESP_LOGI(TAG, "Step 7: ETH_W5500_DEFAULT_CONFIG");
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.reset_gpio_num = -1;
    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(SPI2_HOST, &devcfg);
    ESP_LOGI(TAG, "Step 7 done");

    //#############################################//

    ESP_LOGI(TAG, "Step 8: esp_eth_mac_new_w5500");
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);

    // Set a default MAC address
    uint8_t default_mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01}; // Example MAC address
    if (mac && mac->set_addr) {
        ESP_ERROR_CHECK(mac->set_addr(mac, default_mac));
        ESP_LOGI(TAG, "Default MAC address set: %02X:%02X:%02X:%02X:%02X:%02X", 
                 default_mac[0], default_mac[1], default_mac[2], 
                 default_mac[3], default_mac[4], default_mac[5]);
    }

    // Retrieve and log the MAC address to confirm it was set
    uint8_t mac_addr[6];
    if (mac && mac->get_addr && mac->get_addr(mac, mac_addr) == ESP_OK) {
        ESP_LOGI(TAG, "MAC address in use: %02X:%02X:%02X:%02X:%02X:%02X", 
                 mac_addr[0], mac_addr[1], mac_addr[2], 
                 mac_addr[3], mac_addr[4], mac_addr[5]);
    } else {
        ESP_LOGE(TAG, "Failed to get MAC address");
    }
    ESP_LOGI(TAG, "Step 8 done");

    //#############################################//

    ESP_LOGI(TAG, "Step 9: esp_eth_phy_new_w5500");
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);
    ESP_LOGI(TAG, "Step 9 done");

    //#############################################//

    ESP_LOGI(TAG, "Step 10: esp_eth_driver_install");
    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &eth_handle));
    ESP_LOGI(TAG, "Step 10 done");

    //#############################################//

    ESP_LOGI(TAG, "Step 11: esp_netif_attach");
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    ESP_LOGI(TAG, "Step 11 done");

    //#############################################//

    ESP_LOGI(TAG, "Step 12: esp_eth_start");
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    ESP_LOGI(TAG, "Step 12 done");

    //#############################################//
    ESP_LOGI(TAG, "W5500 Ethernet started.");
}

/// Runtime Loop /////////////////////////////////////////////////////////////////////////

// @extends adapter
void EthernetAdapter::loop() {
    connected = eth_netif && esp_netif_is_netif_up(eth_netif);
    ESP_LOGD(TAG, "Ethernet loop: connected = %s", connected ? "true" : "false");
}

// @extends adapter
bool EthernetAdapter::isConnected() const { return connected; }

// @extends adapter
esp_netif_t* EthernetAdapter::getNetif() const { return eth_netif; }

/// Get Functions /////////////////////////////////////////////////////////////////////////

std::string EthernetAdapter::getIPAddress() const {
    if (!connected || !eth_netif) {
        ESP_LOGW(TAG, "Not connected or netif not available when getting IP address");
        return "0.0.0.0";
    }
    esp_netif_ip_info_t ip;
    esp_netif_get_ip_info(eth_netif, &ip);
    char buf[16];
    snprintf(buf, sizeof(buf), IPSTR, IP2STR(&ip.ip));
    return std::string(buf);
}

std::string EthernetAdapter::getMACAddress() const {
    uint8_t mac[6];
    if (eth_netif && esp_netif_get_mac(eth_netif, mac) == ESP_OK) {
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return std::string(macStr);
    }
    return "00:00:00:00:00:00";
}