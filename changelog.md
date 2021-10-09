## Blynk_Async_ESP32_BT_WF

[![arduino-library-badge](https://www.ardu-badge.com/badge/Blynk_Async_ESP32_BT_WF.svg?)](https://www.ardu-badge.com/Blynk_Async_ESP32_BT_WF)
[![GitHub release](https://img.shields.io/github/release/khoih-prog/Blynk_Async_ESP32_BT_WF.svg)](https://github.com/khoih-prog/Blynk_Async_ESP32_BT_WF/releases)
[![GitHub](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/khoih-prog/Blynk_Async_ESP32_BT_WF/blob/master/LICENSE)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/khoih-prog/Blynk_Async_ESP32_BT_WF.svg)](http://github.com/khoih-prog/Blynk_Async_ESP32_BT_WF/issues)

---
---

## Table of Contents

* [Changelog](#changelog)
  * [Releases v1.2.2](#releases-v122)
  * [Releases v1.2.1](#releases-v121)
  * [Major Releases v1.2.0](#major-releases-v120)
  * [Releases v1.1.1](#releases-v111)
  * [Major Releases v1.1.0](#major-releases-v110)
  * [Releases v1.0.6](#releases-v106)
  
---
---

## Changelog

### Releases v1.2.2

1. Update `platform.ini` and `library.json` to use original `khoih-prog` instead of `khoih.prog` after PIO fix

### Releases v1.2.1

1. Add auto-reconnect feature for BLE. Check [Handle BLE disconnects #2](https://github.com/khoih-prog/Blynk_Async_ESP32_BT_WF/issues/2)


### Major Releases v1.2.0

 1. Enable scan of WiFi networks for selection in Configuration Portal. Check [PR for v1.3.0 - Enable scan of WiFi networks #10](https://github.com/khoih-prog/WiFiManager_NINA_Lite/pull/10). Now you can select optional **SCAN_WIFI_NETWORKS**, **MANUAL_SSID_INPUT_ALLOWED** to be able to manually input SSID, not only from a scanned SSID lists and **MAX_SSID_IN_LIST** (from 2-15)
 2. Fix invalid "blank" Config Data treated as Valid.
 3. Permit optionally inputting one set of WiFi SSID/PWD by using `REQUIRE_ONE_SET_SSID_PW == true`
 4. Enforce WiFi PWD minimum length of 8 chars
 5. Minor enhancement to not display garbage when data is invalid
 6. Fix issue of custom Blynk port (different from 8080 or 9443) not working on ESP32. Check [Custom Blynk port not working for BlynkSimpleEsp32_Async_WM.h #4](https://github.com/khoih-prog/Blynk_Async_WM/issues/4)
 7. To permit auto-reset after configurable timeout if DRD/MRD or non-persistent forced-CP. Check [**Good new feature: Blynk.resetAndEnterConfigPortal() Thanks & question #27**](https://github.com/khoih-prog/Blynk_WM/issues/27)
 8. Fix rare Config Portal bug not updating Config and dynamic Params data successfully in very noisy or weak WiFi situation
 9. Tested with [**Latest ESP32 Core 1.0.6**](https://github.com/espressif/arduino-esp32) for ESP32-based boards.
10. Update examples

### Releases v1.1.1

1. Add functions to control Config Portal from software or Virtual Switches. Check [How to trigger a Config Portal from code #25](https://github.com/khoih-prog/Blynk_WM/issues/25)
2. Fix rare Config Portal bug not updating Config and dynamic Params data successfully in very noisy or weak WiFi situation
3. To permit autoreset after configurable timeout if DRD/MRD or non-persistent forced-CP. Check [**Good new feature: Blynk.resetAndEnterConfigPortal() Thanks & question #27**](https://github.com/khoih-prog/Blynk_WM/issues/27)

### Major Releases v1.1.0

1. Add support to LittleFS for ESP32 using [LITTLEFS](https://github.com/lorol/LITTLEFS) Library
2. Clean-up all compiler warnings possible.
3. Add Table of Contents
4. Add Version String

### Releases v1.0.6

1. Initial coding to use (ESP)AsyncWebServer instead of (ESP8266)WebServer. 
2. Bump up to v1.0.6 to sync with [BlynkESP32_BT_WF library v1.0.6](https://github.com/khoih-prog/BlynkESP32_BT_WF).



