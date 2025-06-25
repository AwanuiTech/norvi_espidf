#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <string>
#include <assert.h>

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