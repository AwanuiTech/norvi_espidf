#include "NetworkManager.hpp"
#include "EthernetAdapter.hpp"
#include "esp_log.h"

static const char* TAG = "NetworkManager";
static EthernetAdapter ethernet;

void NetworkManager::loop() {
    ethernet.loop();
    checkConnection();
}

bool NetworkManager::isConnected() {
    return ethernet.isConnected();
}

std::string NetworkManager::getIPAddress() {
    return ethernet.getIPAddress();
}

void NetworkManager::checkConnection() {
    ethernet_connected = ethernet.isConnected();
}

void NetworkManager::init() {
    ESP_LOGI(TAG, "Initializing NetworkManager...");
    ethernet.init();
    ESP_LOGI(TAG, "NetworkManager ready.");
}