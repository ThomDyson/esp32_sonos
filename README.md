# Rotary Dial for Sonos
A long time ago, Sonos made  stand alone remote to control their speakers. Since everyone has a smartphone, they have long since discontinued the remote. This project uses a Ma Touch 2.1" rotary display as a basic control for Sonos players.  At this time, it only controls volume and mute, advances to prev/next song, and starts and stops playback. If there is nothing in the sonos queue, it will not add anything to the queue. I use Alexa for that.

<img src="./assets/matouch_21_display.jpg" alt="Alt Text" width="300" height="400">

## Requirements

The [Ma Touch 2.1 Rotary display](https://www.makerfabs.com/matouch-esp32-s3-rotary-ips-display-with-touch-2-1-st7701.html) is an ESP32 with a a built-in 2.1" LCD display (480x480) touchscreen with a rotary bezel. The support page is [here](https://wiki.makerfabs.com/MaTouch_ESP32_S3_2.1_Rotary_TFT_with_Touch.html)

## Dependencies

This project uses the following libraries.  

**[Arduino GFX](https://github.com/moononournation/Arduino_GFX)**
The controller sfotware has been tested with version 1.5.x of this graphics driver library. This library requires version 3.1.x of esp32 by Espressif Systems. This is under Board Manager in the Arduino IDE. Platfomio doesn't support current versions of the esp32 library by default. 

**[LVGL](https://github.com/lvgl/lvgl)**
I use LVGL 9 for the display widgets. Using LVGL with Arduino requires some extra steps, be sure to read the docs [here](https://docs.lvgl.io/master/details/integration/framework/arduino.html). This project doesn't need any special configuration; just use a standard ESP32, Arduino file. **Note**:This project does not use the TFT_eSPI library mentiuoned in the LVGL documents. This ues Arduino GFX instead. 

**MicroXPath**
This is a hard fork of the XML procesor found [here](https://github.com/tmittet/microxpath). 

**SonosUPnP**
This is a hard fork of the Sonos processing library found [here](https://github.com/javos65/Sonos-ESP32). This is an amazing library. I've nly scratched the surface of the functions, but at same time it is showing its age. For example, there is no support for straming services like Spotify. 

**touch**
A simple library to interact with the touch sensor.

## Instructions
Download all the source files. You will need to configure two variables, "ssid" and "WiFipassword" for your Wifi setup and the list of your Sonos players.

```cpp
const char *ssid         = "My Home Wifi";               // Change this to your WiFi SSID
const char *WiFipassword = "MySuperSecretPassword";      // Change this to your WiFi password
```

```cpp
SonosDevice SonosPlayers[] = { { IPAddress(192, 168, 0, 1), "Kitchesn" },
                               { IPAddress(192, 168, 0, 2), "Livingroom" },
                               { IPAddress(192, 168, 0, 3), "Gym" } };
                               
#define PLAYER_ARRAY_SIZE 3  // Define the size of the SonosDevices array                               
```
Use the smart phone app to get the IP addresses of your players. Change PLAYER_ARRAY_SIZE to the right number. In the S1 player on Andriod, IP addresses are under Settings -> System -> About My System.


## Usage
Once the code is installed, the rotary bezel is the volume dial. Pressing the bezel toggles mute. Touch the zone name to change to the next zone in your list. The zone list will wrap around to the first entry whenit reaches the end.

## ToDo
I'm not sure I'll add much to this. Obvious improvements would include:

Display and toggle modes like shuffle and repeat.  This would nbt be hard, but I don't use those features much.

The abiity to update/change the queue. This is clearly the missing function, but we use Alexa to start playback, so I'm not sure it is worth the time. Let me know.