/*

THIS IS A Sonos Controller  GFX Arduino AND MATOUCH 2.1
In Arduino IDE:
Bard: ESP32S3 Dev Module
Flash Size: 16MB
Partition Scheme: 16MB Flash (3MB APP/9.9MB FATFS)  ** - optional, this demo
runs in 4MB with SPIFFS PSRAM: OPI PSRAM JTAG Adapter: Integrated USB JTAG Flash
Mode: QIO 80Mhz

*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here:
https://docs.lvgl.io/master/get-started/platforms/arduino.html

 based on code from teh GFX example for LVGL9 and arduino
https://github.com/moononournation/Arduino_GFX/blob/master/examples/LVGL/LVGL_Arduino_v9/LVGL_Arduino_v9.ino
 v 1.5.1 of gfx lib
 v 3x of esp32 board manager by espressif

*/

#include "MicroXPath_P.h"
#include "SonosUPnP.h"
#include "touch.h"
#include <WiFi.h>
#include <lvgl.h>

const char *ssid         = "My Home Wifi";               // Change this to your WiFi SSID
const char *WiFipassword = "MySuperSecretPassword";      // Change this to your WiFi password

// Sonos player definitions
struct SonosDevice {
  IPAddress ip;
  char title[40];  // Fixed-size character array for the title
};
/********* ADD your devices here ****** */
SonosDevice SonosPlayers[] = { { IPAddress(192, 168, 0, 123), "Craft Room Sonos" },
                               { IPAddress(192, 168, 0, 163), "Downstairs" },
                               { IPAddress(192, 168, 0, 160), "Gym" } };

#define PLAYER_ARRAY_SIZE 3  // Define the size of the SonosDevices array

#define I2C_SDA_PIN 17
#define I2C_SCL_PIN 18

#include <Arduino_GFX_Library.h>
#define GFX_BL 38  // backlight pin

Arduino_DataBus *SPIbus =
  new Arduino_SWSPI(GFX_NOT_DEFINED /* DC */, 1 /* CS */, 46 /* SCK */,
                    0 /* MOSI */, GFX_NOT_DEFINED /* MISO */);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
  2 /* DE */, 42 /* VSYNC */, 3 /* HSYNC */, 45 /* PCLK */, 4 /* R0 */,
  41 /* R1 */, 5 /* R2 */, 40 /* R3 */, 6 /* R4 */, 39 /* G0/P22 */,
  7 /* G1/P23 */, 47 /* G2/P24 */, 8 /* G3/P25 */, 48 /* G4/P26 */,
  9 /* G5 */, 11 /* B0 */, 15 /* B1 */, 12 /* B2 */, 16 /* B3 */, 21 /* B4 */,
  1 /* hsync_polarity */, 10 /* hsync_front_porch */,
  8 /* hsync_pulse_width */, 50 /* hsync_back_porch */,
  1 /* vsync_polarity */, 10 /* vsync_front_porch */,
  8 /* vsync_pulse_width */, 20 /* vsync_back_porch */);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
  480 /* width */, 480 /* height */, rgbpanel, 0 /* rotation */,
  true /* auto_flush */, SPIbus, GFX_NOT_DEFINED /* RST */,
  st7701_type5_init_operations, sizeof(st7701_type5_init_operations));

uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;
lv_display_t *disp;
lv_color_t *disp_draw_buf1;
lv_obj_t *RoomSelectorLabel;
lv_obj_t *SongTitleLabel;
lv_obj_t *ArtistLabel;
lv_obj_t *volumeArc;
lv_obj_t *sonosPlayStateButton;
lv_obj_t *sonosPlayNextButton;
lv_obj_t *sonosPlayPreviousButton;
lv_obj_t *roomSelectorButton;
lv_style_t TitleLabelStyle;
lv_style_t ButtonLabelStyle;
lv_style_t RoomSelectorLabelStyle;
lv_obj_t *playStateLabel;
lv_obj_t *playNextLabel;
lv_obj_t *playPreviousLabel;

static lv_style_t style_transparent;


lv_style_t VolumeArcStyle;

// rotary dial elements
#define ROTARY_BUTTON_PIN 14
#define ENCODER_CLK 13  // CLK
#define ENCODER_DT 10   // DT

int sonosVolume = 5;
bool volume_change_flag = false;
int State;
int rotary_press_flag = 0;
unsigned long button_time = 0;
unsigned long last_button_time = 0;

const long debounceDelay = 750;  // Debounce delay in milliseconds
int loopTimer = 0;
TaskHandle_t Task1;
SemaphoreHandle_t ethMutex;

/***  SONOS Setup  ***************************/
void ethConnectError() {
  Serial.println("Wifi died.");
}

char currentSonosPlayerName[40];
IPAddress currentSonosPlayer;
int roomSelector = 0;


// SonosInfo infoX;
WiFiClient client;  // setup wifiClient for Sonos Control
SonosUPnP g_sonos = SonosUPnP(client, ethConnectError);
bool playerIsPlaying = false;
bool playerIsMute;
char sonosPlayState[50] = "";
char sonosSongTitle[100] = "";
char sonosArtist[100] = "";

/* LVGL calls it when a rendered image needs to copied to the display*/
// nothing to do here because we refresh in the main loop
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  lv_disp_flush_ready(disp);  // Call it to tell LVGL you are ready
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  static int32_t last_x = -1, last_y = -1;
  int32_t current_x = 0;  // Store the last known X position
  int32_t current_y = 0;  // Store the last known Y position
  int touch_state = read_touch(&current_x, &current_y);

  if (touch_state == 1 && (current_x != last_x || current_y != last_y)) {  // Touch detected
    data->point.x = current_x;                                             // Set touch X coordinate
    data->point.y = current_y;                                             // Set touch Y coordinate
    data->state = LV_INDEV_STATE_PRESSED;                                  // Pressed state
    last_x = current_x;
    last_y = current_y;
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Touched at: X=%d, Y=%d", last_x, last_y);
    Serial.println(buffer);
    lv_label_set_text(RoomSelectorLabel, buffer);
  } else {
    data->state = LV_INDEV_STATE_RELEASED;  // Released state
  }
}

void IRAM_ATTR encoder_irq() {
  static unsigned long lastDebounceTime;
  if ((millis() - lastDebounceTime) > 100) {
    if (digitalRead(ENCODER_DT) == 1) {
      sonosVolume = sonosVolume - 3;
    } else {
      sonosVolume = sonosVolume + 3;
    }
    volume_change_flag = true;
    lastDebounceTime = millis();
  }
}

void dial_init() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);  // back light on for screen
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ROTARY_BUTTON_PIN, INPUT_PULLUP);
  // old_State = digitalRead(ENCODER_CLK);
  attachInterrupt(ENCODER_CLK, encoder_irq, FALLING);
}

lv_obj_t *create_button(lv_obj_t *parent,
                        lv_event_cb_t event_cb,
                        lv_color_t color,
                        int x_offset,
                        int y_offset) {
  lv_obj_t *button = lv_btn_create(parent);
  lv_obj_set_size(button, 100, 100);
  lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
  lv_obj_align(button, LV_ALIGN_CENTER, x_offset, y_offset);
  lv_obj_set_style_bg_color(button, color, 0);
  lv_obj_add_event_cb(button, event_cb, LV_EVENT_CLICKED, NULL);
  return button;
}

lv_obj_t *create_button_label(lv_obj_t *button, const char *labelText) {
  lv_obj_t *thisLabel = lv_label_create(button);
  lv_label_set_text(thisLabel, labelText);                      // Set label text
  lv_obj_set_style_text_color(thisLabel, lv_color_white(), 0);  // Set text color to white
  lv_obj_add_style(thisLabel, &ButtonLabelStyle, 0);
  lv_obj_center(thisLabel);  // Center the label in the button
  return thisLabel;
}


void read_rotary_press() {
  if (digitalRead(ROTARY_BUTTON_PIN) == 0) {
    if (rotary_press_flag != 1) {
      rotary_press_flag = 1;
      if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(100))) {  // Wait max 1 seconds
        g_sonos.toggleMute(currentSonosPlayer);
        xSemaphoreGive(ethMutex);
        playerIsMute = !playerIsMute;
        lv_style_set_arc_color(&VolumeArcStyle, lv_palette_main(playerIsMute ? LV_PALETTE_RED : LV_PALETTE_BLUE));
      }
    }
  } else {
    if (rotary_press_flag != 0) {
      rotary_press_flag = 0;
    }
  }
}

void sonosPlayStateButton_event_cb(lv_event_t *ev) {
  Serial.println("Play state button CB");
  if (lv_event_get_code(ev) == LV_EVENT_CLICKED) {
    if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(2500))) {  // Wait max 2.5 seconds
      if (playerIsPlaying) {
        g_sonos.pause(currentSonosPlayer);
        lv_label_set_text(playStateLabel, LV_SYMBOL_PLAY);
        lv_obj_set_style_bg_color(sonosPlayStateButton, lv_palette_main(LV_PALETTE_GREEN), 0);  // Set background color (e.g., orange)
      } else {
        g_sonos.play(currentSonosPlayer);
        lv_label_set_text(playStateLabel, LV_SYMBOL_PAUSE);
        lv_obj_set_style_bg_color(sonosPlayStateButton, lv_palette_main(LV_PALETTE_RED), 0);  // Set background color (e.g., orange)
      }
      xSemaphoreGive(ethMutex);
      playerIsPlaying = !playerIsPlaying;
    }
  }
}

void RoomSelectorLabel_event_cb(lv_event_t *ev) {
  roomSelector++;
  Serial.print(" room selector counter ");
  Serial.println(roomSelector);
  strcpy(sonosSongTitle, "-----");
  strcpy(sonosArtist, "-----");
  
  if (roomSelector == PLAYER_ARRAY_SIZE) {
    roomSelector = 0;
  }
}

void sonosPlayNextButton_event_cb(lv_event_t *ev) {
  if (lv_event_get_code(ev) == LV_EVENT_CLICKED) {
    if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(2500))) {  // Wait max 2.5 seconds
      g_sonos.skip(currentSonosPlayer, SONOS_DIRECTION_FORWARD);
      xSemaphoreGive(ethMutex);
    }
  }
}

void sonosPlayPreviousButton_event_cb(lv_event_t *ev) {
  if (lv_event_get_code(ev) == LV_EVENT_CLICKED) {
    if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(2500))) {  // Wait max 2.5 seconds
      g_sonos.skip(currentSonosPlayer, SONOS_DIRECTION_BACKWARD);
      xSemaphoreGive(ethMutex);
    }
  }
}

void volumeArc_value_changed_event_cb(lv_event_t *e) {
  Serial.println("Volume arc call back");
  Serial.println(lv_arc_get_value(volumeArc));
  sonosVolume = lv_arc_get_value(volumeArc);
  volume_change_flag = true;
  setSonosVolume();
}

uint32_t millis_cb(void) {
  return millis();
}

void setup_lvgl_display() {
  lv_style_init(&TitleLabelStyle);
  lv_style_set_text_font(&TitleLabelStyle, &lv_font_montserrat_38);

  lv_style_init(&ButtonLabelStyle);
  lv_style_set_text_font(&ButtonLabelStyle, &lv_font_montserrat_28);

  lv_style_init(&RoomSelectorLabelStyle);
  lv_style_set_text_font(&RoomSelectorLabelStyle, &lv_font_montserrat_28);

  screenWidth = gfx->width();
  screenHeight = gfx->height();
  bufSize = screenWidth * screenHeight;
  disp_draw_buf1 =
    (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_SPIRAM);
  disp = lv_display_create(screenWidth, screenHeight);

  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, disp_draw_buf1, NULL, bufSize * 2,
                         LV_DISPLAY_RENDER_MODE_DIRECT);

  /*Initialize the screen input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(
    indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);

  lv_obj_t *screen1 = lv_scr_act();
  lv_obj_set_style_bg_color(screen1, lv_color_hex(0xdddddd), LV_PART_MAIN);

  SongTitleLabel = lv_label_create(screen1);
  lv_label_set_text(
    SongTitleLabel,
    "Hello Arduino, I'm LVGL!(V" GFX_STR(LVGL_VERSION_MAJOR) "." GFX_STR(
      LVGL_VERSION_MINOR) "." GFX_STR(LVGL_VERSION_PATCH) ")");
  lv_obj_set_content_width(SongTitleLabel, round(screenWidth * .65));
  lv_obj_set_content_height(SongTitleLabel, 45);
  lv_obj_align(SongTitleLabel, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_scrollbar_mode(SongTitleLabel, LV_SCROLLBAR_MODE_OFF);
  // lv_obj_set_style_text_align(SongTitleLabel , LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(SongTitleLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_style_anim_duration(SongTitleLabel, 10000, LV_PART_MAIN);
  lv_obj_add_style(SongTitleLabel, &TitleLabelStyle, 0);

  ArtistLabel = lv_label_create(screen1);
  lv_label_set_text(
    ArtistLabel,
    "Hello Arduino, I'm LVGL!(V" GFX_STR(LVGL_VERSION_MAJOR) "." GFX_STR(
      LVGL_VERSION_MINOR) "." GFX_STR(LVGL_VERSION_PATCH) ")");
  lv_obj_set_content_width(ArtistLabel, round(screenWidth * .5));

  lv_obj_align(ArtistLabel, LV_ALIGN_CENTER, 0, -100);
  lv_obj_set_content_height(ArtistLabel, 45);
  lv_obj_set_scrollbar_mode(ArtistLabel, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_text_align(ArtistLabel, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(ArtistLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_add_style(ArtistLabel, &TitleLabelStyle, 0);
  lv_obj_set_style_anim_duration(ArtistLabel, 10000, LV_PART_MAIN);

lv_style_init(&style_transparent);
lv_style_set_bg_opa(&style_transparent, LV_OPA_TRANSP);  // Make background fully transparent
lv_style_set_border_opa(&style_transparent, LV_OPA_TRANSP); // Make border transparent
lv_style_set_outline_opa(&style_transparent, LV_OPA_TRANSP); // Make outline transparent

  roomSelectorButton = lv_button_create(screen1);            /*Add a button the current screen*/
  lv_obj_align(roomSelectorButton, LV_ALIGN_CENTER, 0, 175); /*Set its position*/
  lv_obj_set_size(roomSelectorButton, 300, 50);              /*Set its size*/
  lv_obj_add_event_cb(roomSelectorButton, RoomSelectorLabel_event_cb, LV_EVENT_CLICKED, NULL);
  //lv_obj_set_style_bg_color(roomSelectorButton, lv_color_white(), 0);
  lv_obj_add_style(roomSelectorButton, &style_transparent, 0);

  RoomSelectorLabel = lv_label_create(roomSelectorButton); /*Add a label to the button*/
  lv_obj_center(RoomSelectorLabel);
  lv_obj_set_style_text_color(RoomSelectorLabel, lv_color_black(), 0);  // Set text color to white
  lv_obj_set_content_height(RoomSelectorLabel, 35);
  lv_label_set_text(RoomSelectorLabel, currentSonosPlayerName);
    lv_label_set_long_mode(RoomSelectorLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_add_style(RoomSelectorLabel, &RoomSelectorLabelStyle, 0);


  sonosPlayStateButton = create_button(screen1, sonosPlayStateButton_event_cb, lv_color_hex(0xFF5733), 0, 90);
  sonosPlayNextButton = create_button(screen1, sonosPlayNextButton_event_cb, lv_color_hex(0x33A1FF), 110, 90);
  sonosPlayPreviousButton = create_button(screen1, sonosPlayPreviousButton_event_cb, lv_color_hex(0x33A1FF), -110, 90);
  playStateLabel = create_button_label(sonosPlayStateButton, LV_SYMBOL_PLAY);
  playNextLabel = create_button_label(sonosPlayNextButton, LV_SYMBOL_NEXT);
  playPreviousLabel = create_button_label(sonosPlayPreviousButton, LV_SYMBOL_PREV);


  /*Create an Arc*/
  volumeArc = lv_arc_create(screen1);
  lv_obj_set_size(volumeArc, screenWidth * .9, screenHeight * .9);
  lv_arc_set_rotation(volumeArc, 180);
  lv_arc_set_bg_angles(volumeArc, 0, 180);
  lv_arc_set_value(volumeArc, 10);
  lv_obj_center(volumeArc);
  lv_obj_set_style_arc_width(volumeArc, 35, LV_PART_INDICATOR);
  lv_obj_add_event_cb(volumeArc, volumeArc_value_changed_event_cb,
                      LV_EVENT_VALUE_CHANGED,
                      NULL);  // not touch enabled, no need for a call back.
  lv_style_init(&VolumeArcStyle);
  lv_style_set_arc_color(&VolumeArcStyle, lv_palette_main(LV_PALETTE_RED));
  lv_obj_add_style(volumeArc, &VolumeArcStyle, LV_PART_INDICATOR);



  lv_obj_move_foreground(sonosPlayPreviousButton);
  lv_obj_move_foreground(sonosPlayNextButton);
  lv_obj_move_foreground(sonosPlayStateButton);
  lv_obj_move_foreground(roomSelectorButton);
}

void setSonosVolume() {
  constrain(sonosVolume, 1, 100);
  if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(1000))) {  // Wait max 1 seconds
    g_sonos.setVolume(currentSonosPlayer, sonosVolume);
    g_sonos.setMute(currentSonosPlayer, false);  // unmute for good measure
    volume_change_flag = false;
    xSemaphoreGive(ethMutex);
  }
}

void readSonosSoundInfo() {
  // Serial.println("readSonosSoundInfo start");
  if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(1000))) {  // Wait max 1 seconds
    playerIsMute = g_sonos.getMute(currentSonosPlayer);
    if (!volume_change_flag) {
      sonosVolume = g_sonos.getVolume(currentSonosPlayer);
    }
    constrain(sonosVolume, 1, 100);
    xSemaphoreGive(ethMutex);
  }
}

void readSonosTrackInfo() {
  // Serial.println("readSonosTrackInfo start");
  FullTrackInfo thisFullTrackInfo;
  if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(1000))) {  // Wait max 1 seconds
    thisFullTrackInfo = g_sonos.getFullTrackInfo(currentSonosPlayer);
    xSemaphoreGive(ethMutex);

    // vTaskDelay(pdMS_TO_TICKS(50));
    if ((strlen(thisFullTrackInfo.title) > 0) || (!playerIsPlaying)) {
      strncpy(sonosSongTitle, thisFullTrackInfo.title,
              sizeof(sonosSongTitle) - 1);
    }
    if ((strlen(thisFullTrackInfo.creator) > 0) || (!playerIsPlaying)) {
      strncpy(sonosArtist, thisFullTrackInfo.creator, sizeof(sonosArtist) - 1);
    }
  }
}

void readSonosPlayerInfo() {
  static int playerState = 0;
  if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(1000))) {  // Wait max 1 seconds
    playerState = g_sonos.getState(currentSonosPlayer);
    if ((playerState == SONOS_STATE_PLAYING) || (playerState == SONOS_STATE_TRANSISTION)) {
      playerIsPlaying = true;
    } else {
      playerIsPlaying = false;
    }
    xSemaphoreGive(ethMutex);
  }
}

void updateDisplay() {

  constrain(sonosVolume, 1, 100);
  if (lv_arc_get_value(volumeArc) != sonosVolume) {
    lv_arc_set_value(volumeArc, sonosVolume);
  }
  if (strcmp(lv_label_get_text(SongTitleLabel), sonosSongTitle) != 0) {
    lv_label_set_text(SongTitleLabel, sonosSongTitle);
    Serial.print("Setting song title to ");
    Serial.println(sonosSongTitle);
  }
  if (strcmp(lv_label_get_text(ArtistLabel), sonosArtist) != 0) {
    lv_label_set_text(ArtistLabel, sonosArtist);
    Serial.print("Setting artist to ");
    Serial.println(sonosArtist);
  }

  if (playerIsPlaying) {
    lv_label_set_text(playStateLabel, LV_SYMBOL_PAUSE);
    lv_obj_set_style_bg_color(sonosPlayStateButton, lv_palette_main(LV_PALETTE_RED), 0);  // Set background color (e.g., orange)
  } else {
    lv_label_set_text(playStateLabel, LV_SYMBOL_PLAY);
    lv_obj_set_style_bg_color(sonosPlayStateButton, lv_palette_main(LV_PALETTE_GREEN), 0);  // Set background color (e.g., orange)
  }

  if (playerIsMute) {
    lv_style_set_arc_color(&VolumeArcStyle, lv_palette_main(LV_PALETTE_RED));
  } else {
    lv_style_set_arc_color(&VolumeArcStyle, lv_palette_main(LV_PALETTE_BLUE));
  }


  lv_obj_invalidate(volumeArc);
  lv_label_set_text(RoomSelectorLabel, SonosPlayers[roomSelector].title);
}

void RefreshSonosInfo(void *parameter) {
  static int loopTimmer = 0;
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

void setup() {
  ethMutex = xSemaphoreCreateMutex();
  Serial.begin(115200);
  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  strcpy(currentSonosPlayerName, SonosPlayers[roomSelector].title);
  currentSonosPlayer = SonosPlayers[roomSelector].ip;
  Serial.println(currentSonosPlayer);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  dial_init();
  WiFi.begin(ssid, WiFipassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("wifi on line");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_WHITE);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif
  readSonosPlayerInfo();
  readSonosSoundInfo();
  readSonosTrackInfo();
  lv_init();
  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb(millis_cb);
  setup_lvgl_display();

  // all the sonos http calls are moved to core 1 so lvgl can scroll smoothly
  xTaskCreatePinnedToCore(RefreshSonosInfo,  //
                          "Task1",           // name of task.
                          8192,              // Stack size of task
                          NULL,              // parameter of the task
                          1,                 // priority of the task
                          &Task1,            // Task handle to keep track of created task
                          1);                // pin task to core 1
}

void loop() {

  lv_task_handler(); /* let the GUI do its work */
  gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf1, screenWidth,
                          screenHeight);
  vTaskDelay(pdMS_TO_TICKS(50));
  updateDisplay();
  read_rotary_press();
  if (volume_change_flag) {
    setSonosVolume();
  }
  strcpy(currentSonosPlayerName, SonosPlayers[roomSelector].title);
  currentSonosPlayer = SonosPlayers[roomSelector].ip;
}
