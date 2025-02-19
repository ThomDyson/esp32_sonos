
#include "SonosDisplay.h"

lv_obj_t *controllerScreen;
lv_obj_t *RoomSelectorLabel;
lv_obj_t *SongTitleLabel;
lv_obj_t *ArtistLabel;
lv_obj_t *volumeAdjustment;
lv_obj_t *sonosPlayStateButton;
lv_obj_t *sonosPlayNextButton;
lv_obj_t *sonosPlayPreviousButton;
lv_obj_t *roomSelectorButton;
lv_obj_t *startupScreenSelectorButton;
lv_style_t TitleLabelStyle;
lv_style_t ButtonLabelStyle;
lv_style_t ButtonLabelStyleSmall;
lv_style_t RoomSelectorLabelStyle; // also used on the startup screen
lv_style_t volumeAdjustmentStyle;
lv_style_t positionLabelStyle;
lv_style_t timerSliderStyle;
lv_style_t style_transparent;
lv_obj_t *playStateLabel;
lv_obj_t *playNextLabel;
lv_obj_t *playPreviousLabel;
lv_obj_t *startupScreenSelectorLabel;
lv_obj_t *trackDurationLabel;
lv_obj_t *trackPositionLabel;
lv_obj_t *trackTimerSlider;
uint32_t screenTimeout = 0;

lv_obj_t *startupScreen;
lv_obj_t *closeStartupScreenButton;
lv_obj_t *closeStartupScreenButtonLabel;
lv_obj_t *startupScreeLabel1;

lv_obj_t *create_button(lv_obj_t *parent, lv_event_cb_t event_cb,
                        lv_color_t color, int x_offset, int y_offset) {
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
  lv_label_set_text(thisLabel, labelText); // Set label text
  lv_obj_set_style_text_color(thisLabel, lv_color_white(),
                              0); // Set text color to white
  lv_obj_add_style(thisLabel, &ButtonLabelStyle, 0);
  lv_obj_center(thisLabel); // Center the label in the button
  return thisLabel;
}

void close_startup_screen_buton_touch_cb(lv_event_t *e) {
  BLset(HIGH);
  screenTimeout = millis();
  Serial.println("screen touched");
  lv_scr_load(controllerScreen);
}

// load startup screen
void startup_screen_selector_button_event_cb(lv_event_t *e) {
  BLset(HIGH);
  screenTimeout = millis();
  Serial.println("screen touched");
  lv_scr_load(startupScreen);
}

void startup_screen_sonos_list_event_cb(lv_event_t *e) {

  screenTimeout = millis(); // reset the backlight timer
  roomSelector = * (int *)lv_event_get_user_data(e);

  /*
  Serial.print(" room int");
  Serial.println(*roomInt);


  roomSelector = *roomInt;
  Serial.print(" room selected still");
  Serial.println(roomSelector);
*/

  if (roomSelector == totalSonosCount) {
    roomSelector = 0;
    Serial.print(" reset room selector counter ");
    Serial.println(roomSelector);
  }
  strlcpy(currentSonosPlayerName, sonosDevices[roomSelector].title,
          sizeof(currentSonosPlayerName) - 1);
  currentSonosIP = sonosDevices[roomSelector].ip;
  Serial.print("Device ");
  Serial.print(currentSonosPlayerName);
  Serial.print(": IP ");
  Serial.println(currentSonosIP.toString());
  clearTrackValues();
  lv_scr_load(controllerScreen);
}

void setup_startup_screen_lvgl_elements() {
  startupScreen = lv_obj_create(NULL);
  lv_obj_clear_flag(startupScreen, LV_OBJ_FLAG_SCROLLABLE); // Disable scrolling
  lv_obj_set_style_bg_color(startupScreen, lv_color_hex(0xdddddd),
                            LV_PART_MAIN);
  // lv_obj_add_event_cb(startupScreen, startup_screen_touch_cb,
  // LV_EVENT_CLICKED, NULL);
  closeStartupScreenButton = create_button(startupScreen, close_startup_screen_buton_touch_cb, lv_color_black(), 0, (screenHeight / 2) - 65);
  closeStartupScreenButtonLabel =
      create_button_label(closeStartupScreenButton, LV_SYMBOL_CLOSE);

  startupScreeLabel1 = lv_label_create(startupScreen);
  lv_label_set_text(startupScreeLabel1, "Found Sonos players");
  lv_obj_set_style_text_align(startupScreeLabel1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_scrollbar_mode(startupScreeLabel1, LV_SCROLLBAR_MODE_OFF);
  lv_obj_align(startupScreeLabel1, LV_ALIGN_TOP_MID, 0, 50);
  lv_obj_add_style(startupScreeLabel1, &RoomSelectorLabelStyle, 0);

  lv_obj_t *sonos_list = lv_list_create(startupScreen);
  lv_obj_set_size(sonos_list, screenWidth * .9,  screenHeight * .5); // Adjust size
  lv_obj_align(sonos_list, LV_ALIGN_CENTER, 0, 0);

  for (int i = 0; i < totalSonosCount; i++) {

  // Allocate memory for button index - without this the callback won't have the param
    int *index = (int *)malloc(sizeof(int));
    *index = i;

    lv_obj_t *sonosListButton = lv_list_add_btn(sonos_list, NULL, sonosDevices[i].title);
    lv_obj_t *label = lv_label_create(sonosListButton);
    lv_obj_set_width(sonosListButton, lv_pct(100));
    lv_obj_set_width(label, lv_pct(45));
    String ipStr = sonosDevices[i].ip.toString();
    const char *ip_cstr = ipStr.c_str();
    lv_label_set_text_fmt(label, "%s", ip_cstr);
    lv_obj_add_style(sonosListButton, &RoomSelectorLabelStyle, 0);
    lv_obj_add_style(label, &RoomSelectorLabelStyle, 0);
    lv_obj_add_event_cb(sonosListButton, startup_screen_sonos_list_event_cb,  LV_EVENT_CLICKED, index);
  }
}

void setup_controller_screen_lvgl_elements() {
  controllerScreen = lv_obj_create(NULL);
  lv_style_init(&TitleLabelStyle);
  lv_style_init(&RoomSelectorLabelStyle);
  lv_style_init(&ButtonLabelStyle);
  lv_style_init(&ButtonLabelStyleSmall);
  lv_style_init(&positionLabelStyle);
  lv_style_init(&timerSliderStyle);
  lv_style_init(&style_transparent);

  lv_style_set_text_font(&RoomSelectorLabelStyle, &lv_font_montserrat_28);
  lv_style_set_text_font(&positionLabelStyle, &lv_font_montserrat_28);
  lv_style_set_text_font(&ButtonLabelStyleSmall, &lv_font_montserrat_16);

  lv_style_set_bg_opa(&style_transparent,
                      LV_OPA_TRANSP); // Make background fully transparent
  lv_style_set_border_opa(&style_transparent,
                          LV_OPA_TRANSP); // Make border transparent
  lv_style_set_outline_opa(&style_transparent,
                           LV_OPA_TRANSP); // Make outline transparent

  controllerScreen = lv_scr_act();
  lv_obj_set_style_bg_color(controllerScreen, lv_color_hex(0xdddddd),
                            LV_PART_MAIN);

  SongTitleLabel = lv_label_create(controllerScreen);
  lv_label_set_text(SongTitleLabel, "Waiting for data");

  lv_obj_set_style_text_align(SongTitleLabel, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_scrollbar_mode(SongTitleLabel, LV_SCROLLBAR_MODE_OFF);
  lv_label_set_long_mode(SongTitleLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_style_anim_duration(SongTitleLabel, 15000, LV_PART_MAIN);
  lv_obj_add_style(SongTitleLabel, &TitleLabelStyle, 0);

  ArtistLabel = lv_label_create(controllerScreen);
  lv_label_set_text(ArtistLabel, "waiting for data");

  lv_obj_set_scrollbar_mode(ArtistLabel, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_text_align(ArtistLabel, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(ArtistLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_add_style(ArtistLabel, &TitleLabelStyle, 0);
  lv_obj_set_style_anim_duration(ArtistLabel, 14000, LV_PART_MAIN);

  trackDurationLabel = lv_label_create(controllerScreen);
  lv_label_set_text(trackDurationLabel, "0:00:00");
  lv_obj_set_style_text_align(trackDurationLabel, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_scrollbar_mode(trackDurationLabel, LV_SCROLLBAR_MODE_OFF);
  lv_obj_add_style(trackDurationLabel, &positionLabelStyle, 0);

  trackPositionLabel = lv_label_create(controllerScreen);
  lv_label_set_text(trackPositionLabel, "0:00:00");
  lv_obj_set_style_text_align(trackPositionLabel, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_scrollbar_mode(trackPositionLabel, LV_SCROLLBAR_MODE_OFF);
  lv_obj_add_style(trackPositionLabel, &positionLabelStyle, 0);

  trackTimerSlider = lv_slider_create(controllerScreen);
  lv_obj_set_size(trackTimerSlider, screenWidth * .55,
                  5);                            // Set width and thin height
  lv_slider_set_range(trackTimerSlider, 1, 100); // Range from 0 to 100
  lv_slider_set_value(trackTimerSlider, 10, LV_ANIM_OFF); // Default to 10%
  // lv_obj_clear_flag(trackTimerSlider, LV_OBJ_FLAG_CLICKABLE);

  // Style the slider track
  lv_obj_set_style_bg_color(trackTimerSlider, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(trackTimerSlider, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_pad_all(trackTimerSlider, 0, LV_PART_KNOB); // Hide the knob
  lv_obj_set_style_bg_opa(trackTimerSlider, LV_OPA_TRANSP,
                          LV_PART_KNOB); // Hide the knob

  roomSelectorButton = lv_button_create(controllerScreen);
  lv_obj_set_size(roomSelectorButton, 300, 50);
  lv_obj_add_event_cb(roomSelectorButton, RoomSelectorLabel_event_cb,
                      LV_EVENT_CLICKED, NULL);
  lv_obj_add_style(roomSelectorButton, &style_transparent, 0);

  RoomSelectorLabel = lv_label_create(roomSelectorButton);
  lv_obj_center(RoomSelectorLabel);
  lv_obj_set_style_text_color(RoomSelectorLabel, lv_color_black(),
                              0); // Set text color to white
  lv_obj_set_content_height(RoomSelectorLabel, 35);
  lv_label_set_text(RoomSelectorLabel, currentSonosPlayerName);
  lv_label_set_long_mode(RoomSelectorLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_add_style(RoomSelectorLabel, &RoomSelectorLabelStyle, 0);

  if (isRoundDisplay) {
    // Create an Arc
    volumeAdjustment = lv_arc_create(controllerScreen);
    lv_obj_set_size(volumeAdjustment, screenWidth * .9, screenHeight * .9);
    lv_arc_set_rotation(volumeAdjustment, 180);
    lv_arc_set_bg_angles(volumeAdjustment, 0, 180);
    lv_arc_set_value(volumeAdjustment, 10);
    lv_obj_set_style_arc_width(volumeAdjustment, 35, LV_PART_INDICATOR);

    lv_obj_set_content_width(SongTitleLabel, round(screenWidth * .65));
    lv_obj_set_content_width(ArtistLabel, round(screenWidth * .60));
    lv_obj_align(SongTitleLabel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(ArtistLabel, LV_ALIGN_CENTER, 0, -(screenHeight * .25));

    lv_obj_align(roomSelectorButton, LV_ALIGN_BOTTOM_MID, 0, -60);
    lv_obj_align(trackDurationLabel, LV_ALIGN_RIGHT_MID, -70, -50);
    lv_obj_align(trackPositionLabel, LV_ALIGN_LEFT_MID, 70, -50);
    lv_obj_align(trackTimerSlider, LV_ALIGN_CENTER, 0, -80);

    startupScreenSelectorButton = lv_btn_create(controllerScreen);
    lv_obj_set_size(startupScreenSelectorButton, 40, 40);
    lv_obj_set_style_radius(startupScreenSelectorButton, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(startupScreenSelectorButton, lv_color_black(), 0);
    lv_obj_add_event_cb(startupScreenSelectorButton,
                        startup_screen_selector_button_event_cb,
                        LV_EVENT_CLICKED, NULL);
    lv_obj_align(startupScreenSelectorButton, LV_ALIGN_BOTTOM_MID, 0, -5);

  } else {
    // create a slider
    volumeAdjustment = lv_slider_create(controllerScreen);
    lv_obj_set_width(volumeAdjustment, screenWidth * .9); // Set slider width
    lv_slider_set_range(volumeAdjustment, 0, 100);        // Range from 0 to 100
    lv_slider_set_value(volumeAdjustment, 10, LV_ANIM_OFF); // Default to 10%
    lv_obj_align(volumeAdjustment, LV_ALIGN_CENTER, 0, 10);

    lv_obj_set_content_width(SongTitleLabel, round(screenWidth * .9));
    lv_obj_set_content_width(ArtistLabel, round(screenWidth * .9));
    lv_obj_align(SongTitleLabel, LV_ALIGN_CENTER, 0, -(screenHeight * .22));
    lv_obj_align(ArtistLabel, LV_ALIGN_CENTER, 0, -(screenHeight * .38));

    lv_obj_align(roomSelectorButton, LV_ALIGN_CENTER, 0, 175);
    lv_obj_align(trackDurationLabel, LV_ALIGN_RIGHT_MID, -20, -40);
    lv_obj_align(trackPositionLabel, LV_ALIGN_LEFT_MID, 20, -40);
    lv_obj_align(trackTimerSlider, LV_ALIGN_CENTER, 0, -40);

    startupScreenSelectorButton =
        create_button(controllerScreen, startup_screen_selector_button_event_cb,
                      lv_color_black(), 100, 100);
    lv_obj_align(startupScreenSelectorButton, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

  } // end if round display

  if (screenHeight > 400) {
    lv_style_set_text_font(&TitleLabelStyle, &lv_font_montserrat_48);
    lv_style_set_text_font(&ButtonLabelStyle, &lv_font_montserrat_40);
    lv_obj_set_content_height(ArtistLabel, 75);
    lv_obj_set_content_height(SongTitleLabel, 75);
  } else {
    lv_style_set_text_font(&TitleLabelStyle, &lv_font_montserrat_38);
    lv_style_set_text_font(&ButtonLabelStyle, &lv_font_montserrat_28);
    lv_obj_set_content_height(ArtistLabel, 45);
    lv_obj_set_content_height(SongTitleLabel, 45);
  }

  // this has to be late because volumeAdjustment is created late
  lv_obj_center(volumeAdjustment);
  lv_obj_add_event_cb(volumeAdjustment, volumeAdjustment_value_changed_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);
  lv_style_init(&volumeAdjustmentStyle);
  // lv_style_set_arc_color(&volumeAdjustmentStyle,
  // lv_palette_main(LV_PALETTE_RED));
  lv_obj_add_style(volumeAdjustment, &volumeAdjustmentStyle, LV_PART_INDICATOR);

  sonosPlayStateButton =
      create_button(controllerScreen, sonosPlayStateButton_event_cb,
                    lv_color_hex(0xFF5733), 0, 80);
  sonosPlayNextButton =
      create_button(controllerScreen, sonosPlayNextButton_event_cb,
                    lv_color_hex(0x33A1FF), 110, 80);
  sonosPlayPreviousButton =
      create_button(controllerScreen, sonosPlayPreviousButton_event_cb,
                    lv_color_hex(0x33A1FF), -110, 80);
  playStateLabel = create_button_label(sonosPlayStateButton, LV_SYMBOL_PLAY);
  playNextLabel = create_button_label(sonosPlayNextButton, LV_SYMBOL_NEXT);
  playPreviousLabel =
      create_button_label(sonosPlayPreviousButton, LV_SYMBOL_PREV);
  startupScreenSelectorLabel =
      create_button_label(startupScreenSelectorButton, LV_SYMBOL_LIST);
  // lv_obj_add_style(startupScreenSelectorLabel, &ButtonLabelStyleSmall, 0);

  lv_obj_add_event_cb(controllerScreen, controller_screen_touch_cb,
                      LV_EVENT_CLICKED, NULL);
  lv_obj_move_foreground(sonosPlayPreviousButton);
  lv_obj_move_foreground(sonosPlayNextButton);
  lv_obj_move_foreground(sonosPlayStateButton);
  lv_obj_move_foreground(roomSelectorButton);
  lv_obj_move_foreground(startupScreenSelectorButton);
  // lv_obj_move_background(controllerScreen);
}

// really just resets the timer
void controller_screen_touch_cb(lv_event_t *e) {
  BLset(HIGH);
  screenTimeout = millis();
  Serial.println("screen touched");
}

void updateDisplay() {
  constrain(sonosVolume, 1, 100);
  if (!volume_change_flag) {
    if (isRoundDisplay) {
      lv_arc_set_value(volumeAdjustment, sonosVolume);
    } else {
      lv_slider_set_value(volumeAdjustment, sonosVolume, LV_ANIM_ON);
    }
  }

  if (isRoundDisplay) {
    lv_style_set_arc_color(
        &volumeAdjustmentStyle,
        lv_palette_main(playerIsMute ? LV_PALETTE_RED : LV_PALETTE_BLUE));
  } else {
    lv_style_set_bg_color(
        &volumeAdjustmentStyle,
        lv_palette_main(playerIsMute ? LV_PALETTE_RED : LV_PALETTE_CYAN));
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
    lv_obj_set_style_bg_color(sonosPlayStateButton,
                              lv_palette_main(LV_PALETTE_RED),
                              0); // Set background color (e.g., orange)
    screenTimeout = millis();
    BLset(HIGH);
  } else {
    lv_label_set_text(playStateLabel, LV_SYMBOL_PLAY);
    lv_obj_set_style_bg_color(sonosPlayStateButton,
                              lv_palette_main(LV_PALETTE_GREEN),
                              0); // Set background color (e.g., orange)
    if (millis() - screenTimeout > (1000 * 60 * 5)) {
      BLset(LOW);
    }
  }
  lv_obj_invalidate(volumeAdjustment);
  lv_label_set_text(RoomSelectorLabel, sonosDevices[roomSelector].title);
  lv_label_set_text(trackPositionLabel, trackPosition);
  lv_label_set_text(trackDurationLabel, trackDuration);
  int currentPercent = round(100 * trackPositionSeconds /
                             (1 + trackDurationSeconds)); // div by zero bad
  lv_slider_set_value(trackTimerSlider, currentPercent, LV_ANIM_ON);
}