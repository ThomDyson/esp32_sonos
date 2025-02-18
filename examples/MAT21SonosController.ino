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

#include "touch.h"  // https://github.com/Makerfabs/MaTouch-ESP32-S3-Rotary-IPS-Display-with-Touch-2.1-ST7701/blob/main/lib/TouchLib.zip
// LVGLV 9
#include <lvgl.h>
// Adruino GFX 1.5.x - requires ESP32 3.1.x board definitions
#include <Arduino_GFX_Library.h>
#include <esp32_sonos.h>
#include <WiFi.h>


/************************************************
*  BEGIN local installatin elements
**************************************************/
const char *ssid = "XXXXX";                        // Change this to your WiFi SSID
const char *WiFipassword = XXXXXXX";  // Change this to your WiFi password

bool isRoundDisplay = true;  // round displays use an arc for volume and a narrower display space
uint32_t screenWidth = 480;
uint32_t screenHeight = 480;
/************************************************
*  END local installatin elements
**************************************************/


/************************************************
*  START DISPLAY HARDWARE specific elements
**************************************************/

#define I2C_SDA_PIN 17
#define I2C_SCL_PIN 18

#define BACKLIGHTPIN 38  // backlight pin

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
  screenWidth /* width */, screenHeight /* height */, rgbpanel, 0 /* rotation */,
  true /* auto_flush */, SPIbus, GFX_NOT_DEFINED /* RST */,
  st7701_type5_init_operations, sizeof(st7701_type5_init_operations));

bool GFXinit() {
  Serial.println("GFX init...");

  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
    return false;
  }

  gfx->fillScreen(0x003030);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(4);
  gfx->setCursor(250, 200);
  gfx->println("Hello world");

  return true;
}


// rotary dial elements
#define ROTARY_BUTTON_PIN 14
#define ENCODER_CLK 13  // CLK
#define ENCODER_DT 10   // DT

int rotary_press_flag = 0;
unsigned long button_time = 0;
unsigned long last_button_time = 0;
void BLset(byte state) {
  digitalWrite(BACKLIGHTPIN, HIGH);
}
/************************************************
*  END DISPLAY HARDWARE specific elements
**************************************************/

/*******************************************************************************
 * END LVGL declarations
 ******************************************************************************/
lv_display_t *disp;
lv_color_t *disp_draw_buf;
uint32_t bufSize;

/*******************************************************************************
 * END LVGL declarations
 ******************************************************************************/


/*
int sonosVolume = 5;
bool volume_change_flag = false;
int State;


const long debounceDelay = 750;  // Debounce delay in milliseconds
int loopTimer = 0;
TaskHandle_t Task1;
SemaphoreHandle_t ethMutex;
*/

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
  pinMode(BACKLIGHTPIN, OUTPUT);
  digitalWrite(BACKLIGHTPIN, HIGH);  // back light on for screen
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ROTARY_BUTTON_PIN, INPUT_PULLUP);
  // old_State = digitalRead(ENCODER_CLK);
  attachInterrupt(ENCODER_CLK, encoder_irq, FALLING);
}

void read_rotary_press() {
  if (digitalRead(ROTARY_BUTTON_PIN) == 0) {
    if (rotary_press_flag != 1) {
      rotary_press_flag = 1;
      if (xSemaphoreTake(ethMutex, pdMS_TO_TICKS(100))) {  // Wait max 1 seconds
        sonosClient.toggleMute(currentSonosIP);
        xSemaphoreGive(ethMutex);
        playerIsMute = !playerIsMute;
        // lv_style_set_arc_color(&VolumeArcStyle, lv_palette_main(playerIsMute ? LV_PALETTE_RED : LV_PALETTE_BLUE));
      }
    }
  } else {
    if (rotary_press_flag != 0) {
      rotary_press_flag = 0;
    }
  }
}

uint32_t millis_cb(void) {
  return millis();
}

void LVGLInit() {
  lv_init();
  lv_tick_set_cb(millis_cb);
  Serial.println("Arduino_GFX LVGL_Arduino_v9 example ");
  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);

  bufSize = screenWidth * screenHeight;
  disp_draw_buf =
    (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_SPIRAM);
  disp = lv_display_create(screenWidth, screenHeight);

  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize * 2, LV_DISPLAY_RENDER_MODE_DIRECT);

  /*Initialize the screen input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);
  setup_controller_screen_lvgl_elements();
  setup_startup_screen_lvgl_elements();
}



void setup() {
  Serial.begin(115200);
  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  dial_init();
  WiFi.begin(ssid, WiFipassword);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_WHITE);

  SonosInit();
  LVGLInit();
  lv_scr_load(startupScreen);
}

void loop() {

  lv_task_handler(); /* let the GUI do its work */
  gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
  vTaskDelay(pdMS_TO_TICKS(50));
  if (lv_scr_act() == controllerScreen) {
    if (!volume_change_flag) {
      updateDisplay();
    }
    read_rotary_press();
    if (volume_change_flag) {
      setSonosVolume();
    }
  }
}
