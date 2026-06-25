#include <M5Cardputer.h>
#include <AudioFileSourceICYStream.h>
#include <AudioFileSourceBuffer.h>
#include <AudioFileSourceSD.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include <SD.h>
#include "app_config.h"
#include "stations.h"
#include "wifi_utils.h"
#include "ui_utils.h"
#include "sd_utils.h"
#include "password_utils.h"
#include "audio_player.h"

static constexpr int SD_CS_PIN   = 12;
static constexpr int SD_MOSI_PIN = 14;
static constexpr int SD_MISO_PIN = 39;
static constexpr int SD_SCK_PIN  = 40;

unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_DELAY = 200;

void setup() {
  auto cfg = M5.config();
  auto spk_cfg = M5Cardputer.Speaker.config();
    spk_cfg.sample_rate = 44100;
    spk_cfg.task_pinned_core = APP_CPU_NUM;
    M5Cardputer.Speaker.config(spk_cfg);
  
  M5Cardputer.begin(cfg, true);

  M5Cardputer.Speaker.begin();
  M5Cardputer.Speaker.setVolume(255);

  M5Cardputer.Display.setBrightness(brightnessLevels[currentBrightnessIndex]);

  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setFont(&fonts::FreeMonoOblique9pt7b);

 pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);

  SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  SD.begin(SD_CS_PIN, SPI, 25000000);

  if (!SD.begin(SD_CS_PIN, SPI, 25000000)) {
      M5Cardputer.Display.drawString("SD.begin failed", 10, 20);
      return;
  }

  M5Cardputer.Display.drawString("SD.begin OK", 10, 20);

  if (SD.exists("/web_radio_config")) {
      M5Cardputer.Display.drawString("folder ok", 10, 40);
  } else {
      M5Cardputer.Display.drawString("folder missing", 10, 40);
  }

  if (SD.exists("/web_radio_config/station_list.txt")) {
      M5Cardputer.Display.drawString("file ok", 10, 60);
  } else {
      M5Cardputer.Display.drawString("file missing", 10, 60);
  }

 
  connectToWiFi();
  
  setupAudio();
  stopAudio();
  
  M5Cardputer.Display.fillScreen(BLACK);  
  
  stopAudio();
  mergeRadioStations();
  Playfile();
  toggleFFT();
  redrawUI();
}

void loop() {
  loopAudio();
  M5Cardputer.update();

  if (stationMenuActive) {
    delay(1);
    if (M5Cardputer.Keyboard.isChange() &&
      millis() - lastButtonPress > DEBOUNCE_DELAY) {

      if (M5Cardputer.Keyboard.isKeyPressed('.')) {
        menuIndex = (menuIndex + 1) % numStations;
        drawStationMenu();
      }
      else if (M5Cardputer.Keyboard.isKeyPressed(';')) {
        menuIndex = (menuIndex - 1 + numStations) % numStations;
        drawStationMenu();
      }
      else if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
        curStation = menuIndex;

        currentStreamTitle[0] = '\0';
        streamTitleChanged = true;

        stopAudio();
        Playfile();

        stationMenuActive = false;
        fft_enabled = fftWasEnabled;
        fftSimON = true;

        M5Cardputer.Display.fillRect(0, 0, 240, 14, TFT_BLACK);

        if (!fft_enabled) {
          M5Cardputer.Display.fillRect(
            0,
            header_height,
            240,
            M5Cardputer.Display.height() - header_height - FOOTER_HEIGHT,
            TFT_BLACK
          );
        }

        redrawUI();

      }

      else if (M5Cardputer.Keyboard.isKeyPressed('l')) {
        stationMenuActive = false;
        fft_enabled = fftWasEnabled;
        fftSimON = true;

        M5Cardputer.Display.fillRect(0, 0, 240, 14, TFT_BLACK);

        if (!fft_enabled) {
          M5Cardputer.Display.fillRect(
            0,
            header_height,
            240,
            M5Cardputer.Display.height() - header_height - FOOTER_HEIGHT,
            TFT_BLACK
          );
        }

        redrawUI();


      }

      lastButtonPress = millis();

    }

    return;
  }

  updateBatteryDisplay(5000);

  if (M5Cardputer.Keyboard.isChange() && (millis() - lastButtonPress > DEBOUNCE_DELAY)) {

    if (M5Cardputer.Keyboard.isKeyPressed(';')) volumeUp();
    else if (M5Cardputer.Keyboard.isKeyPressed('.')) volumeDown();
    else if (M5Cardputer.Keyboard.isKeyPressed('m')) volumeMute();
    else if (M5Cardputer.Keyboard.isKeyPressed('/')) stationUp();
    else if (M5Cardputer.Keyboard.isKeyPressed(',')) stationDown();
    else if (M5Cardputer.Keyboard.isKeyPressed('r')) {
      stopAudio();
      playStream(stations[curStation].url);
    }
    else if (M5Cardputer.Keyboard.isKeyPressed('f')) {
      toggleFFT();
    }
    else if (M5Cardputer.Keyboard.isKeyPressed('b')) {
      toggleBrightness();
    }
    else if (M5Cardputer.Keyboard.isKeyPressed('l')) {
      openStationMenu();
    }

    
    lastButtonPress = millis();

   }
  
  if (fft_enabled) {
    updateFFT();
  }
  
  drawStreamTitle();

  delay(1);
}




