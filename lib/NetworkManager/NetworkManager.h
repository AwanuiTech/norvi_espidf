#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <string>
#include <assert.h>
#include "esp_netif.h"

class Adapter {
    virtual void init() = 0;
    virtual void loop() = 0;
    virtual bool isConnected() const = 0;
    virtual esp_netif_t* getNetif() const = 0;
};

class NetworkManager {
    public:
        void init();
        void loop();

        bool isConnected();
        std::string getIPAddress();

    private:
        void checkConnection();
        bool ethernet_connected = false;
};

#endif // NETWORK_MANAGER_HPP