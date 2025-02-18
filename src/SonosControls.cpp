#include "SonosUPnP.h"
#include <WiFi.h>
#include "SonosControls.h"

void ethConnectError() {
  Serial.println("Wifi died.");
}

// Define global variables
char currentSonosPlayerName[40] = "";
IPAddress currentSonosIP;
int roomSelector = 0;
#define MAXSONOS 10
int totalSonosCount = 0;
IPAddress ACTIVE_sonosIP;
IPAddress sonosIPList[MAXSONOS];
SonosDevice sonosDevices[MAXSONOS];  // Array to store Sonos devices

WiFiClient client;
SonosUPnP sonosClient = SonosUPnP(client, ethConnectError);
SemaphoreHandle_t ethMutex;
TaskHandle_t Task1;
bool playerIsPlaying = false;
bool playerIsMute;
char sonosPlayState[50] = "";
char sonosSongTitle[100] = "";
char sonosArtist[100] = "";
char trackDuration[10] = "";
char trackPosition[10] = "";
int trackPositionSeconds = 1;
int trackDurationSeconds = 1;
int sonosVolume = 5;
bool volume_change_flag = false;

void clearTrackValues() {
  strncpy(trackPosition, "0:00:00", sizeof(trackPosition) - 1);
  strncpy(trackDuration, "0:00:00", sizeof(trackDuration) - 1);
  trackPositionSeconds = 1;
  trackDurationSeconds = 1;
  strcpy(sonosSongTitle, "-----");
  strcpy(sonosArtist, "-----");
}

void sonosPlayStateButton_event_cb(lv_event_t *ev) {
  //Serial.println("Play state button CB");
  if (lv_event_get_code(ev) == LV_EVENT_CLICKED) {
    if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(2500))) {  // Wait max 2.5 seconds
      if (playerIsPlaying) {
        sonosClient.pause(currentSonosIP);
        lv_label_set_text(playStateLabel, LV_SYMBOL_PLAY);
        lv_obj_set_style_bg_color(sonosPlayStateButton, lv_palette_main(LV_PALETTE_GREEN), 0);  // Set background color (e.g., orange)
      } else {
        sonosClient.play(currentSonosIP);
        lv_label_set_text(playStateLabel, LV_SYMBOL_PAUSE);
        lv_obj_set_style_bg_color(sonosPlayStateButton, lv_palette_main(LV_PALETTE_RED), 0);  // Set background color (e.g., orange)
      }
      xSemaphoreGive(ethMutex);
      playerIsPlaying = !playerIsPlaying;
    }
  }
}

void RoomSelectorLabel_event_cb(lv_event_t *ev) {
  clearTrackValues();
  screenTimeout = millis();  //reset the backlight timer
  roomSelector++;
  Serial.print(" room selector counter ");
  Serial.println(roomSelector);

  if (roomSelector == totalSonosCount) {
    roomSelector = 0;
    Serial.print(" reset room selector counter ");
    Serial.println(roomSelector);
  }
  strncpy(currentSonosPlayerName, sonosDevices[roomSelector].title, sizeof(currentSonosPlayerName) - 1);
  currentSonosIP = sonosDevices[roomSelector].ip;
   Serial.print("Device ");
    Serial.print(currentSonosPlayerName);
    Serial.print(": IP ");
    Serial.println(currentSonosIP.toString());
}

void sonosPlayNextButton_event_cb(lv_event_t *ev) {
  if (lv_event_get_code(ev) == LV_EVENT_CLICKED) {
    if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(2500))) {  // Wait max 2.5 seconds
      sonosClient.skip(currentSonosIP, SONOS_DIRECTION_FORWARD);
      xSemaphoreGive(ethMutex);
      clearTrackValues();
    }
  }
}

void sonosPlayPreviousButton_event_cb(lv_event_t *ev) {
  if (lv_event_get_code(ev) == LV_EVENT_CLICKED) {
    if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(2500))) {  // Wait max 2.5 seconds
      sonosClient.skip(currentSonosIP, SONOS_DIRECTION_BACKWARD);
      xSemaphoreGive(ethMutex);
      clearTrackValues();
    }
  }
}

void volumeAdjustment_value_changed_event_cb(lv_event_t *e) {
  Serial.println("Volume adj call back");
  if (isRoundDisplay) {
    Serial.println(lv_arc_get_value(volumeAdjustment));
    sonosVolume = lv_arc_get_value(volumeAdjustment);
  } else {
    sonosVolume = (int)lv_slider_get_value(volumeAdjustment);
  }
  volume_change_flag = true;
  setSonosVolume();
}


void setSonosVolume() {
  constrain(sonosVolume, 1, 100);
  if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(1000))) {  // Wait max 1 seconds
    sonosClient.setVolume(currentSonosIP, sonosVolume);
    sonosClient.setMute(currentSonosIP, false);  // unmute for good measure
    volume_change_flag = false;
    xSemaphoreGive(ethMutex);
  }
}

void readSonosSoundInfo() {
  if (!volume_change_flag) {
    //Serial.println("readSonosSoundInfo start");
    if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(1000))) {  // Wait max 1 seconds
         //   Serial.println("Reading sonos sound info");
      playerIsMute = sonosClient.getMute(currentSonosIP);
      sonosVolume = sonosClient.getVolume(currentSonosIP);
      constrain(sonosVolume, 1, 100);
      xSemaphoreGive(ethMutex);
    }
  }
}

void readSonosTrackInfo() {
  // Serial.println("readSonosTrackInfo start");
  FullTrackInfo thisFullTrackInfo;
  if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(1000))) {  // Wait max 1 seconds
    thisFullTrackInfo = sonosClient.getFullTrackInfo(currentSonosIP);
    xSemaphoreGive(ethMutex);
    // vTaskDelay(pdMS_TO_TICKS(50));
    if ((strlen(thisFullTrackInfo.title) > 0) || (!playerIsPlaying)) {
      strncpy(sonosSongTitle, thisFullTrackInfo.title,
              sizeof(sonosSongTitle) - 1);
    }
    strncpy(trackPosition, thisFullTrackInfo.position, sizeof(trackPosition) - 1);
    strncpy(trackDuration, thisFullTrackInfo.duration, sizeof(trackDuration) - 1);
    trackPositionSeconds = thisFullTrackInfo.positionSeconds;
    trackDurationSeconds = thisFullTrackInfo.durationSeconds;
    constrain(trackPositionSeconds, 1, 999);
    constrain(trackDurationSeconds, 1, 999);

    if ((strlen(thisFullTrackInfo.creator) > 0) || (!playerIsPlaying)) {
      strncpy(sonosArtist, thisFullTrackInfo.creator, sizeof(sonosArtist) - 1);
    }
  }
}

void readSonosPlayerInfo() {
  static int playerState = 0;
  if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(1000))) {  // Wait max 1 seconds
      // Serial.println("Reading sonos player info");
    playerState = sonosClient.getState(currentSonosIP);
    if ((playerState == SONOS_STATE_PLAYING) || (playerState == SONOS_STATE_TRANSISTION)) {
      playerIsPlaying = true;
    } else {
      playerIsPlaying = false;
    }
    xSemaphoreGive(ethMutex);
  } else {
    Serial.println("NOT Reading sonos player info");
  }
}

void RefreshSonosInfo(void *parameter) {
  static int loopTimmer = 0;
  if (currentSonosIP != IPAddress(0, 0, 0, 0)) {
    while (1) {
      readSonosSoundInfo();
      if ((millis() - loopTimmer) > 2000) {
        readSonosPlayerInfo();
        readSonosTrackInfo();
        loopTimmer = millis();
      }
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
}

void populateSonosDevices() {
  bool gotSonos;
  for (int i = 0; i < MAXSONOS; i++) {
    if (sonosIPList[i] != IPAddress(0, 0, 0, 0)) {  // Skip empty IPs
      sonosDevices[i].ip = sonosIPList[i];
      // Retrieve the zone name and store it in the SonosDevice struct
      gotSonos = sonosClient.getZone(sonosIPList[i], sonosDevices[i].title);
      Serial.println(sonosDevices[i].ip.toString());
      Serial.println(sonosDevices[i].title);
      totalSonosCount++;
    }
  }
}

void SonosInit() {
  ethMutex = xSemaphoreCreateMutex();
  uint8_t myresults;
  myresults = sonosClient.CheckUPnP(sonosIPList, MAXSONOS);
  populateSonosDevices();
  strcpy(currentSonosPlayerName, sonosDevices[roomSelector].title);
  currentSonosIP = sonosDevices[roomSelector].ip;
  Serial.println(currentSonosIP);

  xTaskCreatePinnedToCore(RefreshSonosInfo,  //
                          "Task1",           // name of task.
                          8192,              // Stack size of task
                          NULL,              // parameter of the task
                          1,                 // priority of the task
                          &Task1,            // Task handle to keep track of created task
                          1);                // pin task to core 1
  if (currentSonosIP != IPAddress(0, 0, 0, 0)) {
    readSonosPlayerInfo();
    readSonosSoundInfo();
    readSonosTrackInfo();
  }
}
