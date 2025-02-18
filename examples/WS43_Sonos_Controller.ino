/************************
* This exampleis for teh Waveshare 4.3 inch 480x800 display
https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3
ESP32S3 DEV Module
Flash Size 8MB(64Mb)
PSRAM OPI PSRAM
Partition Scheme - anything with 3MB for Apps


*********************************************/



/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/9.2/integration/framework/arduino.html  */

// LVGLV 9
#include <lvgl.h>
// Adruino GFX 1.5.x - requires ESP32 3.1.x board definitions
#include <Arduino_GFX_Library.h>
#include <esp32_sonos.h>
#include <WiFi.h>

/************************************************
*  BEGIN local installatin elements
**************************************************/
const char *ssid = "YYYYYYYY";                        // Change this to your WiFi SSID
const char *WiFipassword = "XXXXXx";  // Change this to your WiFi password


bool isRoundDisplay = false;  // round displays use ana arc for volume and a narrower display space


/************************************************
*  END local installatin elements
**************************************************/

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
  5 /* DE */, 3 /* VSYNC */, 46 /* HSYNC */, 7 /* PCLK */,
  1 /* R0 */, 2 /* R1 */, 42 /* R2 */, 41 /* R3 */, 40 /* R4 */,
  39 /* G0 */, 0 /* G1 */, 45 /* G2 */, 48 /* G3 */, 47 /* G4 */, 21 /* G5 */,
  14 /* B0 */, 38 /* B1 */, 18 /* B2 */, 17 /* B3 */, 10 /* B4 */,

  // Esta configuración es la que mejor funciona de momento
  0 /* hsync_polarity */, 8 /* hsync_front_porch */, 4 /* hsync_pulse_width */, 8 /* hsync_back_porch */,
  0 /* vsync_polarity */, 8 /* vsync_front_porch */, 4 /* vsync_pulse_width */, 8 /* vsync_back_porch */,
  1 /* pclk_active_neg */, 14000000 /* prefer_speed */);

// Esta configuración es la que mejor funciona de momento
// 0 /* hsync_polarity */, 8 /* hsync_front_porch */, 4 /* hsync_pulse_width */, 8 /* hsync_back_porch */,
// 0 /* vsync_polarity */, 8 /* vsync_front_porch */, 4 /* vsync_pulse_width */, 8 /* vsync_back_porch */,
//  1 /* pclk_active_neg */, 14000000 /* prefer_speed */


Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
  800 /* width */,
  480 /* height */,
  rgbpanel,
  0 /* rotation */,
  true /* auto_flush */
);


bool GFXinit() {
  Serial.println("GFX init...");

  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
    return false;
  }

  gfx->fillScreen(0x003030);
  BLset(HIGH);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(4);
  gfx->setCursor(250, 200);
  gfx->println("Hello world");

  return true;
}

/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/
/*******************************************************************************
 * END LVGL declarations
 ******************************************************************************/
lv_display_t *disp;
lv_color_t *disp_draw_buf;
//lv_color_t *disp_draw_buf2;
//lv_obj_t *TouchPointLabel;

/*******************************************************************************
 * END LVGL declarations
 ******************************************************************************/
/*******************************************************************************
 * START TOUCH CONFIG
 ******************************************************************************/
// v 1.2.2 https://github.com/bitbank2/bb_captouch
#include <bb_captouch.h>

BBCapTouch bbct;

#define TOUCH_SDA 8
#define TOUCH_SCL 9
#define TOUCH_INT 4
#define TOUCH_RST 0

const char *szNames[] = { "Unknown", "FT6x36", "GT911", "CST820" };

void TouchInit() {
  Serial.println("Touch init...");
  // Init touch device
  bbct.init(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_INT);
  int iType = bbct.sensorType();
  Serial.printf("Touch sensor type = %s\n", szNames[iType]);
}

void TouchRead(lv_indev_t *indev, lv_indev_data_t *data) {
  TOUCHINFO thisTouch;
  if (bbct.getSamples(&thisTouch)) {  // if touch event happened
    data->state = LV_INDEV_STATE_PRESSED;
    /*
     Serial.printf("Touch x: %d y: %d size: %d\n", thisTouch.x[0], thisTouch.y[0], thisTouch.area[0]);
   */
    data->point.x = thisTouch.x[0];
    data->point.y = thisTouch.y[0];
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

/*******************************************************************************
 * END TOUCH CONFIG
 ******************************************************************************/
/******************************************************************************
* START IO Expander config
*********************************************************************************/
#include <ESP_IOExpander_Library.h>
ESP_IOExpander *expander;

// Extender Pin define
#define TP_RST 1
#define LCD_BL 2
#define LCD_RST 3
#define SD_CS 4
#define USB_SEL 5

// I2C Pin define
#define I2C_MASTER_NUM 0
#define I2C_MASTER_SDA_IO 8
#define I2C_MASTER_SCL_IO 9

void ExpanderInit() {
  Serial.println("IO expander init...");
  expander = new ESP_IOExpander_CH422G((i2c_port_t)I2C_MASTER_NUM, ESP_IO_EXPANDER_I2C_CH422G_ADDRESS);

  expander->init();
  expander->begin();
  expander->multiPinMode(TP_RST | LCD_BL | LCD_RST | SD_CS | USB_SEL, OUTPUT);
  BLset(HIGH);
}

void BLset(byte state) {
  expander->digitalWrite(LCD_BL, state);
}

/********************************************************************************
* END IOExpander Code
**********************************************************************************/

uint32_t screenWidth = 800;
uint32_t screenHeight = 480;
uint32_t bufSize;

uint32_t millis_cb(void) {
  return millis();
}

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);

  /*Call it to tell LVGL you are ready*/
  lv_disp_flush_ready(disp);
}

void LVGLInit() {
  lv_init();
  lv_tick_set_cb(millis_cb);
  Serial.println("Arduino_GFX LVGL_Arduino_v9 example ");
  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);

  bufSize = screenWidth * 40;
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  //disp_draw_buf2 = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

  if (!disp_draw_buf) {
    Serial.println("LVGL disp_draw_buf allocate failed!");
  } else {
    disp = lv_display_create(screenWidth, screenHeight);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);
  }

  /*Initialize the (dummy) input device driver*/
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, TouchRead);
  setup_controller_screen_lvgl_elements();
  setup_startup_screen_lvgl_elements();
}


void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("Initialising...");

  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  WiFi.begin(ssid, WiFipassword);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println(LVGL_Arduino);
  SonosInit();

  TouchInit();
  delay(200);
  ExpanderInit();
  bool GFXinitOK = GFXinit();
  if (GFXinitOK) {
    Serial.println("GFX display initialised");
  }
  LVGLInit();
  lv_scr_load(startupScreen);
}

void loop() {
  lv_task_handler(); /* let the GUI do its work */
  if (lv_scr_act() == controllerScreen) {
    updateDisplay();
    if (volume_change_flag) {  // this is a flag rather than a simple call back because the rotary dial version uses an interupt to set volume
      setSonosVolume();
    }
  }
  vTaskDelay(pdMS_TO_TICKS(100));
}
