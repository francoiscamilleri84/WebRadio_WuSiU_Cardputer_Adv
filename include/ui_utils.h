#pragma once
#include <M5Cardputer.h>

extern bool stationMenuActive;
extern bool fftWasEnabled;
extern int menuIndex;
extern const int MENU_TOP;
extern const int MENU_VISIBLE;
extern int MENU_LINE_H;
extern int header_height;
extern bool fft_enabled;
extern bool fftSimON;
extern uint8_t lastVolumeDrawn;

void showStation();
void showVolume();
void drawStreamTitle();
void drawFooter();
void toggleBrightness();
void drawStationMenu();
void openStationMenu();
void redrawUI();
void setupFFT();
void updateFFT();
void toggleFFT();
void updateBatteryDisplay(unsigned long updateInterval);