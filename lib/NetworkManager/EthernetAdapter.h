#ifndef ETHERNET_ADAPTER_H
#define ETHERNET_ADAPTER_H

#include "esp_netif.h"
#include <string>
#include <cstring>
#include <functional>

class EthernetAdapter {
public:
    //Constructor
    EthernetAdapter();

    //Inherited methods
    void init();
    void loop();
    bool isConnected() const;

    //Get methods
    std::string getIPAddress() const;
    esp_netif_t* getNetif() const;
    std::string getMACAddress() const;

    //Connection event callback
    void onConnectionChange(std::function<void(bool)> cb);
    void reconnect();

private:
    esp_netif_t* eth_netif;
    bool connected;
};

#endif // ETHERNET_ADAPTER_HPP

