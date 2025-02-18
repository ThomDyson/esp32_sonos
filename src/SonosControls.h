#pragma once
// LVGLV 9
#include <lvgl.h>
#include <WiFi.h>
#include "SonosDisplay.h"
#include "SonosUPnP.h"

void ethConnectError();  // Function prototype
void sonosPlayStateButton_event_cb(lv_event_t *ev);
void RoomSelectorLabel_event_cb(lv_event_t *ev);
void sonosPlayNextButton_event_cb(lv_event_t *ev);
void sonosPlayPreviousButton_event_cb(lv_event_t *ev);
void volumeAdjustment_value_changed_event_cb(lv_event_t *e);
void setSonosVolume();
void readSonosSoundInfo();
void readSonosTrackInfo();
void readSonosPlayerInfo();
void RefreshSonosInfo();
void SonosInit();
void clearTrackValues();
void populateSonosDevices();

extern SemaphoreHandle_t ethMutex;
extern TaskHandle_t Task1;
extern int totalSonosCount;

extern char currentSonosPlayerName[40];
extern IPAddress currentSonosIP;
extern int roomSelector;

extern WiFiClient client;
extern SonosUPnP sonosClient;
extern bool playerIsPlaying;
extern bool playerIsMute;
extern char sonosPlayState[50];
extern char sonosSongTitle[100];
extern char sonosArtist[100];
extern char trackDuration[10];
extern char trackPosition[10];
extern int trackPositionSeconds;
extern int trackDurationSeconds;
extern int sonosVolume;
extern bool volume_change_flag;

// Sonos player definitions
struct SonosDevice {
  IPAddress ip;
  char title[40];  // Fixed-size character array for the title
};

extern SonosDevice sonosDevices[];