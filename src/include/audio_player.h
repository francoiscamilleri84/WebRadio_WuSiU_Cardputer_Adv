#pragma once

#include <AudioFileSourceICYStream.h>
#include <AudioFileSourceBuffer.h>
#include <AudioFileSourceSD.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>

extern AudioGeneratorMP3 *mp3;
extern AudioFileSourceICYStream *stream;
extern AudioFileSourceBuffer *buff;
extern AudioFileSourceSD *file;
extern AudioOutputI2S *out;

extern uint16_t curVolume;
extern bool isMuted;
extern char currentStreamTitle[128];
extern bool streamTitleChanged;

void setupAudio();
void loopAudio();
void stopAudio();
void Playfile();
void volumeUp();
void volumeDown();
void volumeMute();
void stationUp();
void stationDown();
void audio_id3data(const char* info);
void audio_showstation(const char* showstation);
void audio_showstreamtitle(const char* info);
void drawStationMenu();
void openStationMenu();
void showStation();
void drawStreamTitle();
void showVolume();
void playStream(const char* url);
