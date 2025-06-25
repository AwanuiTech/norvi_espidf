#ifndef ETHERNET_ADAPTER_HPP
#define ETHERNET_ADAPTER_HPP

#include "esp_netif.h"
#include <string>

class EthernetAdapter {
public:
    EthernetAdapter();

    void init();
    void loop();
    bool isConnected() const;
    std::string getIPAddress() const;

    esp_netif_t* getNetif() const;

private:
    esp_netif_t* eth_netif;
    bool connected;
};

#endif // ETHERNET_ADAPTER_HPP

