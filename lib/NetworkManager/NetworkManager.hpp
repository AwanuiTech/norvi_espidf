#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include <string>

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