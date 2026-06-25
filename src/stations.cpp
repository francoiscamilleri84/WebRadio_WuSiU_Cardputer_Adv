#include "stations.h"
#include <SD.h>

RadioStation stations[MAX_STATIONS];
size_t numStations = 0;
size_t curStation = 0;

const PROGMEM RadioStation defaultStations[] = {
  {"RMF FM", "http://rmfstream1.interia.pl:80/rmf_fm"},
};
void mergeRadioStations() {
    M5Cardputer.Display.fillScreen(BLACK);

    if (!SD.begin()) {
        M5Cardputer.Display.drawString("SD.begin() failed", 20, 20);
        delay(4000);
        loadDefaultStations();
        return;
    }

    if (!SD.exists("/web_radio_config")) {
        M5Cardputer.Display.drawString("Folder missing:", 20, 20);
        M5Cardputer.Display.drawString("/web_radio_config", 20, 40);
        delay(4000);
        loadDefaultStations();
        return;
    }

    if (!SD.exists("/web_radio_config/station_list.txt")) {
        M5Cardputer.Display.drawString("File missing:", 20, 20);
        M5Cardputer.Display.drawString("/web_radio_config/station_list.txt", 20, 40);
        delay(4000);
        loadDefaultStations();
        return;
    }

    File file = SD.open("/web_radio_config/station_list.txt", FILE_READ);
    if (!file) {
        M5Cardputer.Display.drawString("Open failed:", 20, 20);
        M5Cardputer.Display.drawString("/web_radio_config/station_list.txt", 20, 40);
        delay(4000);
        loadDefaultStations();
        return;
    }

    numStations = 0;

    while (file.available() && numStations < MAX_STATIONS) {
        String line = file.readStringUntil('\n');
        int commaIndex = line.indexOf(',');

        if (commaIndex > 0) {
            String name = line.substring(0, commaIndex);
            String url = line.substring(commaIndex + 1);

            name.trim();
            url.trim();

            if (name.length() > 0 && url.length() > 0) {
                strncpy(stations[numStations].name, name.c_str(), MAX_NAME_LENGTH - 1);
                strncpy(stations[numStations].url, url.c_str(), MAX_URL_LENGTH - 1);
                stations[numStations].name[MAX_NAME_LENGTH - 1] = '\0';
                stations[numStations].url[MAX_URL_LENGTH - 1] = '\0';
                numStations++;
            }
        }
    }

    file.close();

    if (numStations == 0) {
        loadDefaultStations();
    }
}

void loadDefaultStations() {
  numStations = std::min(sizeof(defaultStations)/sizeof(defaultStations[0]), static_cast<size_t>(MAX_STATIONS));
  memcpy(stations, defaultStations, sizeof(RadioStation) * numStations);
}



