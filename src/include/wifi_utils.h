#pragma once

#include <M5Cardputer.h>
#include <WiFi.h>
#include <vector>
#include <algorithm>
#include "app_config.h"

struct WiFiNetwork {
    String ssid;
    int32_t rssi;
    wifi_auth_mode_t encryption;
};

extern std::vector<WiFiNetwork> networks;
extern int scrollX;

String inputText(const String& prompt, int x, int y, bool isPassword = false);
String scanAndDisplayNetworks();
void displayWiFiInfo();

bool fastConnect(const String& ssid, const String& pass);
void connectToWiFi();

bool ensureWiFiConfigFolder();
bool saveWiFiConfig(const String& ssid, const String& pass);
bool loadWiFiConfig(String& ssid, String& pass);
void resetWiFiConfig();
void resetWiFiSettings();