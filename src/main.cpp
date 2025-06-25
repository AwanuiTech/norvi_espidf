#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "NetworkManager.hpp"

NetworkManager net;

extern "C" void app_main() {
    net.init();

    while (true) {
        net.loop();

        if (net.isConnected()) {
            printf("[Ethernet] Connected! IP: %s\n", net.getIPAddress().c_str());
        } else {
            printf("[Ethernet] Not connected\n");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}