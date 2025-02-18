#pragma once
// LVGLV 9
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include "SonosControls.h"
#include "SonosUPnP.h"

extern uint32_t screenWidth;
extern uint32_t screenHeight;
extern uint32_t bufSize;
extern bool isRoundDisplay;
extern lv_display_t *disp;
extern lv_color_t *disp_draw_buf;
//extern lv_obj_t *TouchPointLabel;
extern Arduino_RGB_Display *gfx;
extern uint32_t screenTimeout;

extern lv_obj_t *controllerScreen;
extern lv_obj_t *RoomSelectorLabel;
extern lv_obj_t *SongTitleLabel;
extern lv_obj_t *ArtistLabel;
extern lv_obj_t *volumeAdjustment;
extern lv_obj_t *sonosPlayStateButton;
extern lv_obj_t *sonosPlayNextButton;
extern lv_obj_t *sonosPlayPreviousButton;
extern lv_obj_t *roomSelectorButton;
extern lv_style_t TitleLabelStyle;
extern lv_style_t ButtonLabelStyle;
extern lv_style_t RoomSelectorLabelStyle;
extern lv_style_t volumeAdjustmentStyle;
extern lv_style_t positionLabelStyle;
extern lv_style_t timerSliderStyle;
extern lv_obj_t *playStateLabel;
extern lv_obj_t *playNextLabel;
extern lv_obj_t *playPreviousLabel;
extern lv_obj_t *trackDurationLabel;
extern lv_obj_t *trackPositionLabel;
extern lv_obj_t *trackTimerSlider;

extern lv_obj_t *startupScreen;

extern lv_style_t style_transparent;

lv_obj_t *create_button(lv_obj_t *parent,
                        lv_event_cb_t event_cb,
                        lv_color_t color,
                        int x_offset,
                        int y_offset);

lv_obj_t *create_button_label(lv_obj_t *button, const char *labelText);
void setup_controller_screen_lvgl_elements();
void setup_startup_screen_lvgl_elements();
void updateDisplay();
void controller_screen_touch_cb(lv_event_t *e);

extern void BLset(byte state);
