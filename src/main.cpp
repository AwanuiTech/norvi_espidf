#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"

extern "C" void app_main() {
    // TODO: Initialize components here
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
