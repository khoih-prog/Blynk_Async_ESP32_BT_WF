/****************************************************************************************************************************
  Async_PET_Check.ino
  For ESP32 using WiFi along with BlueTooth BLE
  
  Blynk_Async_ESP32_BT_WF is a library, using AsyncWebServer instead of (ESP8266)WebServer for inclusion of both ESP32 
  Blynk BT/BLE and WiFi libraries. Then select either one or both at runtime.
  
  Based on and modified from Blynk library v0.6.1 https://github.com/blynkkk/blynk-library/releases
  Built by Khoi Hoang https://github.com/khoih-prog/Blynk_Async_ESP32_BT_WF
  Licensed under MIT license
  
  Version: 1.2.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.6   K Hoang      25/08/2020 Initial coding to use (ESP)AsyncWebServer instead of (ESP8266)WebServer. 
                                  Bump up to v1.0.6 to sync with BlynkESP32_BT_WF v1.0.6.
  1.1.0   K Hoang      30/12/2020 Add support to LittleFS. Remove possible compiler warnings. Update examples
  1.1.1   K Hoang      31/01/2021 Add functions to control Config Portal (CP) from software or Virtual Switches
                                  Fix CP and Dynamic Params bugs. To permit autoreset after timeout if DRD/MRD or forced CP 
  1.2.0   K Hoang      24/04/2021 Enable scan of WiFi networks for selection in Configuration Portal and many new features.
 *****************************************************************************************************************************/
/****************************************************************************************************************************
  Example Created by Miguel Alexandre Wisintainer
  See https://nina-b302-scanner-presenca.blogspot.com/2020/06/nina-w102-ble-detector-presenca-de-pet.html
  Date: 06/06/2020

  Important Notes:
  1) Sketch is ~0.9MB of code because only 1 instance of Blynk if #define BLYNK_USE_BT_ONLY  =>  true
  2) Sketch is very large (~1.3MB code) because 2 instances of Blynk if #define BLYNK_USE_BT_ONLY  =>    false
  3) To conmpile, use Partition Scheme with large APP size, such as
    a) 8MB Flash (3MB APP, 1.5MB FAT) if use EEPROM
    b) No OTA (2MB APP, 2MB SPIFFS)
    c) No OTA (2MB APP, 2MB FATFS)  if use EEPROM
    d) Huge APP (3MB No OTA, 1MB SPIFFS)   <===== Preferable if use SPIFFS
    e) Minimal SPIFFS (1.9MB APP with OTA, 190KB SPIFFS)
  *****************************************************************************************************************************/

#include "defines.h"
#include "Credentials.h"
#include "dynamicParams.h"

bool USE_BLE = true;
long timePreviousMeassure  = 0;

#define WIFI_BLE_SELECTION_PIN     14   //Pin D14 mapped to pin GPIO14/HSPI_SCK/ADC16/TOUCH6/TMS of ESP32

BlynkTimer timer;

#include <Ticker.h>
Ticker     led_ticker;

int NEAR_PET = 0;

#if USE_BLYNK_WM

#define BLYNK_PIN_FORCED_CONFIG           V10
#define BLYNK_PIN_FORCED_PERS_CONFIG      V20

// Use button V10 (BLYNK_PIN_FORCED_CONFIG) to forced Config Portal
BLYNK_WRITE(BLYNK_PIN_FORCED_CONFIG)
{ 
  if (param.asInt())
  {
    Serial.println( F("\nCP Button Hit. Rebooting") ); 

    // This will keep CP once, clear after reset, even you didn't enter CP at all.
    Blynk.resetAndEnterConfigPortal(); 
  }
}

// Use button V20 (BLYNK_PIN_FORCED_PERS_CONFIG) to forced Persistent Config Portal
BLYNK_WRITE(BLYNK_PIN_FORCED_PERS_CONFIG)
{ 
  if (param.asInt())
  {
    Serial.println( F("\nPersistent CP Button Hit. Rebooting") ); 
   
    // This will keep CP forever, until you successfully enter CP, and Save data to clear the flag.
    Blynk.resetAndEnterConfigPortalPersistent();
  }
}
#endif

void set_led(byte status)
{
  digitalWrite(LED_BUILTIN , status);
}

void noticeAlive(void)
{
  if (Blynk_WF.connected())
  {
    if (USE_BLE)
      Blynk_BLE.virtualWrite(V0, F("OK"));
    else
      Blynk_WF.virtualWrite(V0, F("OK"));

    if (NEAR_PET == 0) //NOT FOUND PET ON SCAN!!!!! ALERT THE BLYNK
      Blynk_WF.notify("Hi, i cant find the PET!!!!");

    NEAR_PET = 0; //SCAN AGAIN
  }
}

void heartBeatPrint(void)
{
  static int num = 1;

  if (Blynk.connected())
  {
    set_led(HIGH);
    led_ticker.once_ms(111, set_led, (byte) LOW);
    Serial.print(F("B"));
  }
  else
  {
    Serial.print(F("F"));
  }

  if (num == 80)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    Serial.print(F(" "));
  }
}

void checkStatus()
{
  static unsigned long checkstatus_timeout = 0;

#define STATUS_CHECK_INTERVAL     60000L

  // Send status report every STATUS_REPORT_INTERVAL (60) seconds: we don't need to send updates frequently if there is no status change.
  if ((millis() > checkstatus_timeout) || (checkstatus_timeout == 0))
  {
    if (!USE_BLE)
    {
      // report Blynk connection
      heartBeatPrint();
    }

    checkstatus_timeout = millis() + STATUS_CHECK_INTERVAL;
  }
}

char BLE_Device_Name[] = "PET-Check-BLE";

int scanTime = 5; //In seconds
BLEScan* pBLEScan;
bool BLE_Initialized = false;

char BLE_Manufacturer_Data[] = "4c000215fda50693a4e24fb1afcfc6eb07647825271b271bc9";

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      char* manufacturerdata = BLEUtils::buildHexData(NULL, (uint8_t*)advertisedDevice.getManufacturerData().data(), advertisedDevice.getManufacturerData().length());
      Serial.println("Advertised Device: " + String(manufacturerdata));

      if (strcmp(BLE_Manufacturer_Data, manufacturerdata) == 0)
      {
        NEAR_PET++;  //just to identify that found the PET near!
        Serial.println(F("Found PET"));
      }
    }
};

void checkPet(void)
{
  if (BLE_Initialized)
  {
    Serial.println(F("Scanning..."));
  
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    Serial.print(F("Devices found: "));
    Serial.println(foundDevices.getCount());
    Serial.println(F("Scan done!"));
    // delete results fromBLEScan buffer to release memory
    pBLEScan->clearResults();
  }
}

void init_BLE()
{
  if (Blynk_WF.connected() && !BLE_Initialized)
  {
    BLEDevice::init("");

    BLE_Initialized = true;

    //create new scan
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());

    //active scan uses more power, but get results faster
    pBLEScan->setActiveScan(true);

    pBLEScan->setInterval(100);
    // less or equal setInterval value
    pBLEScan->setWindow(99);
  }
}

#if USING_CUSTOMS_STYLE
const char NewCustomsStyle[] /*PROGMEM*/ = "<style>div,input{padding:5px;font-size:1em;}input{width:95%;}body{text-align: center;}\
button{background-color:blue;color:white;line-height:2.4rem;font-size:1.2rem;width:100%;}fieldset{border-radius:0.3rem;margin:0px;}</style>";
#endif

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  delay(200);

#if (USE_LITTLEFS)
  Serial.print(F("\nStarting Async_PET_Check_BLE using LITTLEFS"));
#elif (USE_SPIFFS)
  Serial.print(F("\nStarting Async_PET_Check_BLE using SPIFFS"));  
#else
  Serial.print(F("\nStarting Async_PET_Check_BLE using EEPROM"));
#endif

#if USE_SSL
  Serial.print(F(" with SSL on "));
#else
  Serial.print(F(" without SSL on "));
#endif

  Serial.println(ARDUINO_BOARD);
  Serial.println(BLYNK_ASYNC_ESP32_BT_WF_VERSION);
  
#if USE_BLYNK_WM  
  Serial.println(ESP_DOUBLE_RESET_DETECTOR_VERSION);
#endif 

  pinMode(WIFI_BLE_SELECTION_PIN, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);

  if (digitalRead(WIFI_BLE_SELECTION_PIN) == HIGH)
  {
    USE_BLE = false;
    Serial.println(F("GPIO14 HIGH, Use WiFi"));
#if USE_BLYNK_WM
#if ESP32_BLE_WF_DEBUG
    Serial.println(F("USE_BLYNK_WM: Blynk_WF begin"));
#endif
    // Set config portal SSID and Password
    Blynk.setConfigPortal("TestPortal-ESP32", "TestPortalPass");
    // Set config portal IP address
    Blynk.setConfigPortalIP(IPAddress(192, 168, 232, 1));

    // Set config portal channel, default = 1. Use 0 => random channel from 1-13 to avoid conflict
    Blynk_WF.setConfigPortalChannel(0);

    // From v1.0.6, select either one of these to set static IP + DNS
    Blynk.setSTAStaticIPConfig(IPAddress(192, 168, 2, 232), IPAddress(192, 168, 2, 1), IPAddress(255, 255, 255, 0));
    //Blynk.setSTAStaticIPConfig(IPAddress(192, 168, 2, 232), IPAddress(192, 168, 2, 1), IPAddress(255, 255, 255, 0),
    //                           IPAddress(192, 168, 2, 1), IPAddress(8, 8, 8, 8));
    //Blynk.setSTAStaticIPConfig(IPAddress(192, 168, 2, 232), IPAddress(192, 168, 2, 1), IPAddress(255, 255, 255, 0),
    //                           IPAddress(4, 4, 4, 4), IPAddress(8, 8, 8, 8));

    //////////////////////////////////////////////
    
    #if USING_CUSTOMS_STYLE
      Blynk.setCustomsStyle(NewCustomsStyle);
    #endif
    
    #if USING_CUSTOMS_HEAD_ELEMENT
      Blynk.setCustomsHeadElement("<style>html{filter: invert(10%);}</style>");
    #endif
    
    #if USING_CORS_FEATURE  
      Blynk.setCORSHeader("Your Access-Control-Allow-Origin");
    #endif
    
    //////////////////////////////////////////////

    // Use this to default DHCP hostname to ESP8266-XXXXXX or ESP32-XXXXXX
    //Blynk.begin();
    // Use this to personalize DHCP hostname (RFC952 conformed)
    // 24 chars max,- only a..z A..Z 0..9 '-' and no '-' as last char
    //Blynk.begin("PET-Check-BLE");

    Blynk_WF.begin(BLE_Device_Name);
#else
    //Blynk_WF.begin(auth, ssid, pass);
#if ESP32_BLE_WF_DEBUG
    Serial.println(F("Not USE_BLYNK_WM: Blynk_WF begin"));
#endif
    Blynk_WF.begin(auth, ssid, pass, cloudBlynkServer.c_str(), BLYNK_SERVER_HARDWARE_PORT);
#endif    //USE_BLYNK_WM
  }
  else
  {
    USE_BLE = true;

    Serial.println(F("GPIO14 LOW, Use BLE"));

    Blynk_BLE.setDeviceName(BLE_Device_Name);

#if USE_BLYNK_WM
    if (Blynk_WF.getBlynkBLEToken() == NO_CONFIG)
    {
      Serial.println(F("No valid stored BLE auth. Have to run WiFi then enter config portal"));
      USE_BLE = false;

#if ESP32_BLE_WF_DEBUG
      Serial.println(F("USE_BLYNK_WM: No BLE Token. Blynk_WF begin"));
#endif

      Blynk_WF.begin(BLE_Device_Name);
    }
    String BLE_auth = Blynk_WF.getBlynkBLEToken();
#else
    String BLE_auth = auth;
#endif    //USE_BLYNK_WM

    if (USE_BLE)
    {
      Serial.print(F("Connecting Blynk via BLE, using auth = "));
      Serial.println(BLE_auth);

#if ESP32_BLE_WF_DEBUG
      Serial.println(F("USE_BLE: Blynk_BLE begin"));
#endif

      Blynk_BLE.begin(BLE_auth.c_str());
    }
  }

  init_BLE();

  // Important, need to keep constant communication to Blynk Server at least once per ~25s
  // Or Blynk will lost and have to (auto)reconnect
  timer.setInterval(10000L, noticeAlive);

  // Scan for Pet every 60s (5.1s)
  timer.setInterval(60000L,  checkPet);
}

void loop()
{
  init_BLE();
  
  if (USE_BLE)
    Blynk_BLE.run();
  else
    Blynk_WF.run();
  
  timer.run();
  checkStatus();
}
