#ifndef CONFIG_H
#define CONFIG_H

/// RELAYS ////////////////////////////////////////////

#define RELAY_PIN_1  12
#define RELAY_PIN_2  13
#define RELAY_PIN_3  14
#define RELAY_PIN_4  15

/// NETWORK ////////////////////////////////////////////

// --- SPI Pins for W5500 ---
#define W5500_MOSI 23
#define W5500_MISO 19
#define W5500_SCK  18
#define W5500_CS   5
#define W5500_RST  33

// --- Ethernet Config ---
#define USE_STATIC_IP false
#define STATIC_IP_ADDR "192.168.1.150"
#define STATIC_GW_ADDR "192.168.1.1"
#define STATIC_NETMASK "255.255.255.0"
#define STATIC_DNS1 "8.8.8.8"

///////////////////////////////////////////////////////

#define OLED_SDA     21
#define OLED_SCL     22

#define RS485_RX     16
#define RS485_TX     17

#endif
