#include "audio_player.h"
#include "app_config.h"
#include <M5Cardputer.h>
#include <SD.h>
#include "stations.h"
#include "ui_utils.h"

AudioGeneratorMP3 *mp3 = nullptr;
AudioFileSourceICYStream *stream = nullptr;
AudioFileSourceBuffer *buff = nullptr;
AudioFileSourceSD *file = nullptr;
AudioOutputI2S *out = nullptr;
uint16_t curVolume = 115;
bool isMuted = false;
uint16_t prevVolume = 0;
char currentStreamTitle[128] = "";
bool streamTitleChanged = false;

static void applyVolume() {
    if (out) {
        float gain = (float)curVolume / 255.0f;
        out->SetGain(gain);
    }
}

void setupAudio() {
    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCK, I2S_WS, I2S_DOUT);
    applyVolume();
}

void stopAudio() {
    if (mp3) {
        mp3->stop();
        delete mp3;
        mp3 = nullptr;
    }
    if (buff) {
        delete buff;
        buff = nullptr;
    }
    if (stream) {
        delete stream;
        stream = nullptr;
    }
    if (file) {
        delete file;
        file = nullptr;
    }
}

void loopAudio() {
    if (mp3) {
        if (!mp3->loop()) {
            stopAudio();
        }
    }
}

void Playfile() {
    stopAudio();

    currentStreamTitle[0] = '\0';
    streamTitleChanged = true;

    const char* url = stations[curStation].url;

    if (strstr(url, "http")) {
        stream = new AudioFileSourceICYStream(url);
        buff = new AudioFileSourceBuffer(stream, 8192);
        mp3 = new AudioGeneratorMP3();
        mp3->begin(buff, out);
    } else {
        file = new AudioFileSourceSD(url);
        buff = new AudioFileSourceBuffer(file, 8192);
        mp3 = new AudioGeneratorMP3();
        mp3->begin(buff, out);
    }

    showStation();
}

void volumeUp() {
    if (curVolume + VOLUME_STEP > 255) curVolume = 255;
    else curVolume += VOLUME_STEP;
    applyVolume();
    showVolume();
}

void volumeDown() {
    if (curVolume <= VOLUME_STEP) curVolume = 0;
    else curVolume -= VOLUME_STEP;
    applyVolume();
    showVolume();
}

void volumeMute() {
    if (!isMuted) {
        prevVolume = curVolume;
        curVolume = 0;
        isMuted = true;
    } else {
        curVolume = prevVolume;
        isMuted = false;
    }
    applyVolume();
    showVolume();
}

void stationUp() {
    if (numStations > 0) {
        curStation = (curStation + 1) % numStations;
        Playfile();
        showStation();
    }
    showVolume();
}

void stationDown() {
    if (numStations > 0) {
        curStation = (curStation - 1 + numStations) % numStations;
        Playfile();
        showStation();
    }
    showVolume();
}

void playStream(const char* url) {
    stopAudio();
    stream = new AudioFileSourceICYStream(url);
    buff   = new AudioFileSourceBuffer(stream, 8192);
    mp3    = new AudioGeneratorMP3();
    mp3->begin(buff, out);
}