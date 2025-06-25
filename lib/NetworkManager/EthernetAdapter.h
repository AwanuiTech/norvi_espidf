#ifndef ETHERNET_ADAPTER_H
#define ETHERNET_ADAPTER_H

#include "esp_netif.h"
#include "NetworkManager.h"
#include <string>
#include <cstring>
#include <functional>

class EthernetAdapter : public Adapter {
public:
    //Constructor
    EthernetAdapter();

    //Inherited methods
    void init() override;
    void loop() override;
    bool isConnected() const override;
    esp_netif_t* getNetif() const override;

    //Get methods
    std::string getIPAddress() const;
    std::string getMACAddress() const;

    //Connection event callback
    void onConnectionChange(std::function<void(bool)> cb);
    void reconnect();

private:
    esp_netif_t* eth_netif;
    bool connected;
};

#endif // ETHERNET_ADAPTER_HPP

