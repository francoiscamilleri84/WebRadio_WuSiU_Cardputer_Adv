#include "wifi_utils.h"
#include <esp_wifi.h>
#include <SD.h>

static const char* WIFI_DIR  = "/web_radio_config";
static const char* WIFI_FILE = "/web_radio_config/wifi.txt";

std::vector<WiFiNetwork> networks;
int scrollX = 0;

bool ensureWiFiConfigFolder() {
    if (SD.exists(WIFI_DIR)) return true;
    return SD.mkdir(WIFI_DIR);
}

bool saveWiFiConfig(const String& ssid, const String& pass) {
    if (!ensureWiFiConfigFolder()) return false;

    if (SD.exists(WIFI_FILE)) {
        SD.remove(WIFI_FILE);
    }

    File f = SD.open(WIFI_FILE, FILE_WRITE);
    if (!f) return false;

    f.println(ssid);
    f.println(pass);
    f.close();
    return true;
}

bool loadWiFiConfig(String& ssid, String& pass) {
    if (!SD.exists(WIFI_FILE)) return false;

    File f = SD.open(WIFI_FILE, FILE_READ);
    if (!f) return false;

    ssid = f.readStringUntil('\n');
    ssid.trim();

    pass = f.readStringUntil('\n');
    pass.trim();

    f.close();

    return !ssid.isEmpty();
}

void resetWiFiConfig() {
    if (SD.exists(WIFI_FILE)) {
        SD.remove(WIFI_FILE);
    }
}

bool fastConnect(const String& ssid, const String& pass) {
    M5Cardputer.Display.clear();
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = millis();
    unsigned long lastDot = 0;
    int dots = 0;

    while (millis() - start < WIFI_TIMEOUT) {
        M5Cardputer.update();

        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }

        if (millis() - lastDot > 350) {
            lastDot = millis();
            dots = (dots + 1) % 4;

            M5Cardputer.Display.fillRect(20, 60, 200, 16, BLACK);

            String line = "Connecting";
            for (int i = 0; i < dots; i++) line += ".";

            M5Cardputer.Display.drawString(line, 20, 60);
        }

        vTaskDelay(1);
    }

    WiFi.disconnect(true, true);
    delay(150);

    return false;
}

String inputText(const String& prompt, int x, int y, bool isPassword) {
    String data;
    data.reserve(64);

    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextScroll(true);
    M5Cardputer.Display.drawString(prompt, x, y);

    while (true) {
        M5Cardputer.update();

        if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isPressed()) {
                Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

                for (auto i : status.word) {
                    if (data.length() < 63) data += i;
                }

                if (status.del && data.length() > 0) {
                    data.remove(data.length() - 1);
                }

                if (status.enter) {
                    return data;
                }

                M5Cardputer.Display.fillRect(0, y - 4, M5Cardputer.Display.width(), 25, BLACK);

                String display;
                display.reserve(66);
                display = "> ";
                display += data;
                M5Cardputer.Display.drawString(display, 4, y);
            }
        }

        delay(10);
    }
}

void displayWiFiInfo() {
    M5Cardputer.Display.fillRect(0, 20, 240, 135, BLACK);
    M5Cardputer.Display.setCursor(1, 1);
    M5Cardputer.Display.drawString("Connected to Wi-Fi", 1, 1);
    M5Cardputer.Display.drawString("SSID: " + WiFi.SSID(), 1, 18);
    M5Cardputer.Display.drawString("IP: " + WiFi.localIP().toString(), 1, 33);
    int8_t rssi = WiFi.RSSI();
    M5Cardputer.Display.drawString("RSSI: " + String(rssi) + " dBm", 1, 48);
    delay(2000);
    M5Cardputer.Display.fillRect(0, 0, 240, 135, BLACK);
}

String scanAndDisplayNetworks() {
    scrollX = 0;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true, true);
    delay(200);

    WiFi.scanDelete();
    WiFi.scanNetworks(true, true);

    M5Cardputer.Display.clear();
    M5Cardputer.Display.drawString("Wi-Fi scanning", 1, 1);

    int16_t scanResult;
    do {
        scanResult = WiFi.scanComplete();
        delay(100);
    } while (scanResult == WIFI_SCAN_RUNNING);

    if (scanResult == 0) {
        M5Cardputer.Display.drawString("No networks found.", 1, 15);
        delay(2000);
        return "";
    }

    networks.clear();
    networks.reserve(MAX_NETWORKS);

    for (int i = 0; i < scanResult && i < MAX_NETWORKS; i++) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0 || ssid.length() > 32) continue;

        networks.push_back({
            ssid,
            WiFi.RSSI(i),
            WiFi.encryptionType(i)
        });
    }

    if (networks.empty()) {
        M5Cardputer.Display.drawString("No usable networks", 1, 20);
        delay(2000);
        return "";
    }

    std::sort(networks.begin(), networks.end(),
        [](const WiFiNetwork& a, const WiFiNetwork& b) {
            return a.rssi > b.rssi;
        });

    M5Cardputer.Display.clear();
    M5Cardputer.Display.drawString("Available networks:", 1, 1);

    int selectedNetwork = 0;
    int lastSelectedLocal = -1;

    while (true) {
        if (selectedNetwork != lastSelectedLocal) {
            scrollX = 0;
            lastSelectedLocal = selectedNetwork;
        }

        for (size_t i = 0; i < networks.size(); i++) {
            int y = 18 + i * 18;
            bool selected = (i == selectedNetwork);

            M5Cardputer.Display.fillRect(0, y, 240, 18, BLACK);

            String line;
            line.reserve(96);
            line = selected ? "-> " : "   ";
            line += networks[i].ssid;
            line += " (";
            line += networks[i].rssi;
            line += "dBm)";
            if (networks[i].encryption != WIFI_AUTH_OPEN) {
                line += " *";
            }

            int textWidth = M5Cardputer.Display.textWidth(line);

            if (selected && textWidth > 230) {
                scrollX++;
                if (scrollX > textWidth) scrollX = 0;
                M5Cardputer.Display.drawString(line, 1 - scrollX, y);
            } else {
                M5Cardputer.Display.drawString(line, 1, y);
            }
        }

        M5Cardputer.Display.drawString("Select ENTER: OK", 1, 108);
        M5Cardputer.update();

        if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isPressed()) {
                if (M5Cardputer.Keyboard.isKeyPressed(';') && selectedNetwork > 0) {
                    selectedNetwork--;
                }

                if (M5Cardputer.Keyboard.isKeyPressed('.') &&
                    selectedNetwork < (int)networks.size() - 1) {
                    selectedNetwork++;
                }

                if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                    scrollX = 0;
                    return networks[selectedNetwork].ssid;
                }
            }
        }

        delay(10);
    }
}

void resetWiFiSettings() {
    M5Cardputer.Display.clear();
    M5Cardputer.Display.drawString("Reset WiFi settings", 20, 50);
    M5Cardputer.Display.drawString("Please wait...", 20, 70);

    resetWiFiConfig();

    WiFi.disconnect(true, true);
    delay(200);
    esp_restart();
}

void connectToWiFi() {
    unsigned long resetStart = millis();
    bool resetShown = false;

    while (millis() - resetStart < 1000) {
        M5Cardputer.update();

        if (!resetShown) {
            M5Cardputer.Display.clear();
            M5Cardputer.Display.drawString("Hold BtnG0 to", 40, 50);
            M5Cardputer.Display.drawString("reset WiFi", 50, 70);
            resetShown = true;
        }

        if (M5Cardputer.BtnA.isPressed()) {
            resetWiFiSettings();
            return;
        }

        delay(10);
    }

    WiFi.mode(WIFI_STA);
    esp_wifi_set_ps(WIFI_PS_NONE);
    WiFi.setSleep(false);

    String ssid, pass;

    if (loadWiFiConfig(ssid, pass)) {
        if (fastConnect(ssid, pass)) {
            displayWiFiInfo();
            return;
        }
    }

    M5Cardputer.Display.clear();

    ssid = scanAndDisplayNetworks();
    if (ssid.isEmpty()) return;

    M5Cardputer.Display.clear();
    M5Cardputer.Display.drawString("Password:", 2, 40);
    pass = inputText("> ", 4, 110, true);
    M5Cardputer.Display.clear();

    if (fastConnect(ssid, pass)) {
        saveWiFiConfig(ssid, pass);
        displayWiFiInfo();
    }
}