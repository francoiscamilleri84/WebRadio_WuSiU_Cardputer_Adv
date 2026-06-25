#pragma once
#include <M5Cardputer.h>

#define MAX_STATIONS 20
#define MAX_NAME_LENGTH 30
#define MAX_URL_LENGTH 100
#define MAX_NETWORKS 10
#define MAX_SAVED_WIFI 10
#define WIFI_TIMEOUT 9000
#define MIN_WIFI_RSSI -80

#define I2S_BCK 41
#define I2S_WS 43
#define I2S_DOUT 42

#define VOLUME_STEP 20
#define FOOTER_HEIGHT 16


extern uint8_t brightnessLevels[5];
extern uint8_t currentBrightnessIndex;