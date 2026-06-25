#include "ui_utils.h"
#include "audio_player.h"
#include "stations.h"
#include "app_config.h"
#include <M5Cardputer.h>

#define FFT_SIZE 256
#define WAVE_SIZE 320
#define FOOTER_HEIGHT 16

class fft_t {
public:
  fft_t() {
    for (int i = 0; i < FFT_SIZE; i++) _data[i] = 0;
  }

  void exec(const int16_t* in) {
    if (fftSimON) {
      for (int i = 0; i < FFT_SIZE; i++) _data[i] = abs(in[i]);
    }
  }

  uint32_t get(size_t index) {
    if (index < FFT_SIZE) return _data[index];
    return 0;
  }

private:
  uint32_t _data[FFT_SIZE];
};

bool stationMenuActive = false;
bool fftWasEnabled = false;
const int MENU_VISIBLE = 6;
const int MENU_TOP = 15;
int menuIndex = 0;
int MENU_LINE_H = 0;
int header_height = 50;
bool fft_enabled = false;
bool fftSimON = true;
uint8_t lastVolumeDrawn = 255;

static uint16_t prev_y[(FFT_SIZE / 2) + 1];
static uint16_t peak_y[(FFT_SIZE / 2) + 1];
static fft_t fft;
static int16_t raw_data[WAVE_SIZE * 2];

static uint32_t bgcolor(int y) {
  auto h = M5Cardputer.Display.height() - FOOTER_HEIGHT;
  auto dh = h - header_height;

  int v = ((h - y) << 5) / dh;
  if (dh > header_height) {
    int v2 = ((h - y - 1) << 5) / dh;
    if ((v >> 2) != (v2 >> 2)) return 0x666666u;
  }
  return M5Cardputer.Display.color888(v + 2, v, v + 6);
}


void showStation() {
  M5Cardputer.Display.fillRect(0, 15, 240, 35, TFT_BLACK);
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.drawString(stations[curStation].name, 0, 15);

  M5Cardputer.Display.fillRect(0, 33, 240, 15, TFT_BLACK);
  
  if (currentStreamTitle[0] != '\0') {
    M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5Cardputer.Display.drawString(currentStreamTitle, 0, 33);
  }

  showVolume();
}
void showVolume() {
  if (curVolume == lastVolumeDrawn) return;
  lastVolumeDrawn = curVolume;

  const int rightReserved = 80;
  const int barHeight = 4;
  const int barY = 6;

  int maxBarWidth = M5Cardputer.Display.width() - rightReserved;
  if (maxBarWidth < 10) return;

  M5Cardputer.Display.fillRect(0, barY, maxBarWidth, barHeight + 2, TFT_BLACK);

  int barWidth = map(curVolume, 0, 255, 0, maxBarWidth);

  if (barWidth > 0) {
    M5Cardputer.Display.fillRect(0, barY, barWidth, barHeight, 0xAAFFAA);
  }
}

void drawStreamTitle() {

  if (stationMenuActive) return;

  static int lastX = -10000;
  static int xOffset = 0;
  static bool scrolling = false;
  static bool scrollDone = false;
  static unsigned long lastUpdate = 0;

  const int screenW = M5Cardputer.Display.width();
  const int startScrollX = 80;
  const int yText = 33;
  const int hText = 15;
  const int scrollSpeed = 20;

  M5Cardputer.Display.fillRect(0, 50, screenW, 1, TFT_RED);

  if (!currentStreamTitle[0]) {
    M5Cardputer.Display.fillRect(0, yText, screenW, hText, TFT_BLACK);
    return;
  }

  int textWidth = M5Cardputer.Display.textWidth(currentStreamTitle);

  if (streamTitleChanged) {
    xOffset = startScrollX;
    scrolling = false;
    scrollDone = false;
    streamTitleChanged = false;
    lastX = -10000;

    M5Cardputer.Display.fillRect(0, yText, screenW, hText, TFT_BLACK);
  }

  if (textWidth <= screenW) {
    if (lastX != 0) {
      lastX = 0;
      M5Cardputer.Display.fillRect(0, yText, screenW, hText, TFT_BLACK);
      M5Cardputer.Display.drawString(currentStreamTitle, 0, yText);
    }
    return;
  }

  if (scrollDone) {
    if (lastX != 0) {
      lastX = 0;
      M5Cardputer.Display.fillRect(0, yText, screenW, hText, TFT_BLACK);
      M5Cardputer.Display.drawString(currentStreamTitle, 0, yText);
    }
    return;
  }

  if (!scrolling) {
    scrolling = true;
    xOffset = startScrollX;
    lastUpdate = millis();
  }

  if (millis() - lastUpdate < scrollSpeed) return;
  lastUpdate = millis();

  xOffset--;

  if (xOffset < -textWidth) {
    scrollDone = true;
    scrolling = false;
    return;
  }

  if (xOffset != lastX) {
    lastX = xOffset;
    M5Cardputer.Display.fillRect(0, yText, screenW, hText, TFT_BLACK);
    M5Cardputer.Display.drawString(currentStreamTitle, xOffset, yText);
  }

}


void drawStationMenu() {
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);

  M5Cardputer.Display.drawCentreString("Select station:", 120, 0);
  M5Cardputer.Display.fillRect(0, 13, 240, 1, TFT_RED);

  int start = menuIndex - MENU_VISIBLE / 2;
  if (start < 0) start = 0;
  if (start > (int)numStations - MENU_VISIBLE)
    start = max(0, (int)numStations - MENU_VISIBLE);

  const int listTop = MENU_TOP;
  const int listHeight = MENU_VISIBLE * MENU_LINE_H + 30;
  M5Cardputer.Display.fillRect(0, listTop, 240, listHeight, TFT_BLACK);

  for (int i = 0; i < MENU_VISIBLE; i++) {
    int idx = start + i;
    if (idx >= numStations) break;

    int y = MENU_TOP + i * MENU_LINE_H + 8;

    if (idx == menuIndex) {
      M5Cardputer.Display.fillRect(0, y, 240, MENU_LINE_H, TFT_WHITE);
      M5Cardputer.Display.setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
      M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    }

    int textY = y + (MENU_LINE_H - M5Cardputer.Display.fontHeight()) / 2;
    M5Cardputer.Display.drawString(stations[idx].name, 4, textY);
  }
}

void openStationMenu() {
  stationMenuActive = true;
  menuIndex = curStation;

  fftWasEnabled = fft_enabled;
  fft_enabled = false;
  fftSimON = false;

  M5Cardputer.Display.fillRect(0, 0, 240, 135, TFT_BLACK);

  drawStationMenu();
}



void drawFooter() {
  int y = M5Cardputer.Display.height() - FOOTER_HEIGHT;

  M5Cardputer.Display.fillRect(0, y, 240, FOOTER_HEIGHT, TFT_BLACK);
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.drawCentreString("@WuSiU", 120, y + 1);
}
void toggleBrightness() {
  currentBrightnessIndex++;
  if (currentBrightnessIndex >= 5) {
    currentBrightnessIndex = 0;
  }

  uint8_t brightness = brightnessLevels[currentBrightnessIndex];
  M5Cardputer.Display.setBrightness(brightness);

}

void redrawUI() {
  fftSimON = true;

  showStation();
  lastVolumeDrawn = 255;
  showVolume();
  updateBatteryDisplay(0);
  drawFooter();

  if (fft_enabled) {
    setupFFT();
  }
}

void setupFFT() {
  if (!fft_enabled) return;
  
  for (int x = 0; x < (FFT_SIZE / 2) + 1; ++x) {
    prev_y[x] = INT16_MAX;
    peak_y[x] = INT16_MAX;
  }

  int display_height = M5Cardputer.Display.height() - FOOTER_HEIGHT;
  for (int y = header_height; y < display_height; ++y) {
    M5Cardputer.Display.drawFastHLine(0, y, M5Cardputer.Display.width(), bgcolor(y));
  }
}
void updateFFT() {
  if (!fft_enabled) return;

  static unsigned long lastFFTUpdate = 0;
  if (millis() - lastFFTUpdate < 50) return;
  lastFFTUpdate = millis();

  for (int i = 0; i < WAVE_SIZE * 2; i++) {
    raw_data[i] = random(-32000, 32000);
  }

  fft.exec(raw_data);

  size_t bw = M5Cardputer.Display.width() / 30;
  if (bw < 3) bw = 3;
  int32_t dsp_height = M5Cardputer.Display.height() - FOOTER_HEIGHT;
  int32_t fft_height = dsp_height - header_height - 1;
  size_t xe = M5Cardputer.Display.width() / bw;
  if (xe > (FFT_SIZE / 2)) xe = (FFT_SIZE / 2);

  uint32_t bar_color[2] = {0x000033u, 0x99AAFFu};

  M5Cardputer.Display.startWrite();
  
  int32_t bottom = M5Cardputer.Display.height() - FOOTER_HEIGHT - 1;

  for (size_t bx = 0; bx <= xe; ++bx) {
    size_t x = bx * bw;
    int32_t f = fft.get(bx) * 3;
    int32_t y = (f * fft_height) >> 17;
    if (y > fft_height) y = fft_height;
    y = dsp_height - y;
    if (y > bottom) y = bottom;
    int32_t py = prev_y[bx];

    if (py > bottom) py = bottom;

    if (y != py) {
      M5Cardputer.Display.fillRect(x, y, bw - 1, py - y, bar_color[(y < py)]);
      prev_y[bx] = y;
    }

    py = peak_y[bx] + ((peak_y[bx] - y) > 5 ? 2 : 1);
    if (py < y) {
      M5Cardputer.Display.writeFastHLine(x, py - 1, bw - 1, bgcolor(py - 1));
    } else {
      py = y - 1;
    }
    if (peak_y[bx] != py) {
      peak_y[bx] = py;
      M5Cardputer.Display.writeFastHLine(x, py, bw - 1, TFT_WHITE);
    }
  }
  
  M5Cardputer.Display.endWrite();
}

void toggleFFT() {
  fft_enabled = !fft_enabled;

  M5Cardputer.Display.fillRect(
    0,
    header_height,
    M5Cardputer.Display.width(),
    M5Cardputer.Display.height() - header_height - FOOTER_HEIGHT,
    TFT_BLACK
  );

  if (fft_enabled) {
    setupFFT();
  }

  drawFooter();
}

void updateBatteryDisplay(unsigned long updateInterval) {
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate < updateInterval) return;
  lastUpdate = millis();

  int batteryLevel = M5.Power.getBatteryLevel();
  const int batteryY = 0;

  uint16_t batteryColor = batteryLevel < 30 ? TFT_RED : TFT_GREEN;

  char percentStr[6];
  snprintf(percentStr, sizeof(percentStr), "%d%%", batteryLevel);

  const int battW = 20;
  const int battH = 10;
  const int tipW  = 3;
  const int gap   = 4;

  const int percentMaxW = M5Cardputer.Display.textWidth("100%");

  int percentX = 240 - percentMaxW - 2;
  int battX    = percentX - gap - battW;

  M5Cardputer.Display.fillRect(
    battX - 4,
    batteryY,
    percentMaxW + battW + gap + 10,
    14,
    TFT_BLACK
  );

  M5Cardputer.Display.fillRect(battX, batteryY + 2, battW, battH, TFT_DARKGREY);
  M5Cardputer.Display.fillRect(battX + battW, batteryY + 4, tipW, 6, TFT_DARKGREY);
  M5Cardputer.Display.fillRect(
    battX + 2,
    batteryY + 4,
    (batteryLevel * (battW - 4)) / 100,
    6,
    batteryColor
);

  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.drawString(percentStr, percentX, batteryY);

}
