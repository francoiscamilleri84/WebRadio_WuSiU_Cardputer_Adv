#pragma once
#include <M5Cardputer.h>
#include "app_config.h"

struct RadioStation {
    char name[MAX_NAME_LENGTH];
    char url[MAX_URL_LENGTH];
};

extern RadioStation stations[MAX_STATIONS];
extern size_t numStations;
extern size_t curStation;

void loadDefaultStations();
void mergeRadioStations();