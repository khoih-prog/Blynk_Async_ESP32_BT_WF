/****************************************************************************************************************************
   Async_ESP32_BLE_WF.ino
   For ESP32 using WiFi along with BlueTooth BLE

   Blynk_Async_ESP32_BT_WF is a library, using AsyncWebServer instead of (ESP8266)WebServer for inclusion of both ESP32 
   Blynk BT/BLE and WiFi libraries. Then select either one or both at runtime.
   
   Based on and modified from Blynk library v0.6.1 https://github.com/blynkkk/blynk-library/releases
   Built by Khoi Hoang https://github.com/khoih-prog/Blynk_Async_ESP32_BT_WF
   Licensed under MIT license
   
   Version: 1.0.6

   Version Modified By   Date      Comments
   ------- -----------  ---------- -----------
    1.0.6   K Hoang      25/08/2020 Initial coding to use (ESP)AsyncWebServer instead of (ESP8266)WebServer. 
                                    Bump up to v1.0.16 to sync with BlynkESP32_BT_WF v1.0.6.
 *****************************************************************************************************************************/
/****************************************************************************************************************************
   Important Notes:
   1) Sketch is ~0.9MB of code because only 1 instance of Blynk if #define BLYNK_USE_BT_ONLY  =>  true
   2) Sketch is very large (~1.3MB code) because 2 instances of Blynk if #define BLYNK_USE_BT_ONLY  =>    false
   3) To conmpile, use Partition Scheem with large APP size, such as
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

void set_led(byte status)
{
  digitalWrite(LED_BUILTIN, status);
}

void noticeAlive(void)
{ 
  if (USE_BLE)
    Blynk_BLE.virtualWrite(V0, F("OK"));
  else
    Blynk_WF.virtualWrite(V0, F("OK"));
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

char BLE_Device_Name[] = "GeigerCounter-BLE";

void setup()
{
  Serial.begin(115200);
  while (!Serial);

#if ( USE_SPIFFS)
  Serial.print(F("\nStarting Async_ESP32_BLE_WF using SPIFFS"));
#else
  Serial.print(F("\nStarting Async_ESP32_BLE_WF using EEPROM"));
#endif

#if USE_SSL
  Serial.println(" with SSL on " + String(ARDUINO_BOARD));
#else
  Serial.println(" without SSL on " + String(ARDUINO_BOARD));
#endif  

  pinMode(WIFI_BLE_SELECTION_PIN, INPUT_PULLUP);

#if BLYNK_USE_BLE_ONLY
  Blynk_BLE.setDeviceName(BLE_Device_Name);

  #if ESP32_BLE_WF_DEBUG
  Serial.println(F("Blynk_BLE begin"));
  #endif

  Blynk_BLE.begin(auth);
#else
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

    // Use this to default DHCP hostname to ESP8266-XXXXXX or ESP32-XXXXXX
    //Blynk.begin();
    // Use this to personalize DHCP hostname (RFC952 conformed)
    // 24 chars max,- only a..z A..Z 0..9 '-' and no '-' as last char
    //Blynk.begin("ESP32-BLE-WM");
  
    Blynk_WF.begin(BLE_Device_Name);
#else
    //Blynk_WF.begin(auth, ssid, pass);
#if ESP32_BLE_WF_DEBUG    
    Serial.println(F("Not USE_BLYNK_WM: Blynk_WF begin"));
#endif    
    Blynk_WF.begin(auth, ssid, pass, cloudBlynkServer.c_str(), BLYNK_SERVER_HARDWARE_PORT);
#endif
  }
  else
  {
    USE_BLE = true;
    Serial.println(F("GPIO14 LOW, Use BLE"));
    Blynk_BLE.setDeviceName(BLE_Device_Name);
#if USE_BLYNK_WM
    if (Blynk_WF.getBlynkBLEToken() == NO_CONFIG)        //String("blank"))
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
#endif

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
#endif

  // Important, need to keep constant communication to Blynk Server at least once per ~25s
  // Or Blynk will lost and have to (auto)reconnect
  timer.setInterval(10000L, noticeAlive);
}

#if (USE_BLYNK_WM && USE_DYNAMIC_PARAMETERS)
void displayCredentials(void)
{
  Serial.println(F("\nYour stored Credentials :"));

  for (int i = 0; i < NUM_MENU_ITEMS; i++)
  {
    Serial.println(String(myMenuItems[i].displayName) + " = " + myMenuItems[i].pdata);
  }
}
#endif

void loop()
{
#if BLYNK_USE_BLE_ONLY
  Blynk_BLE.run();
#else
  if (USE_BLE)
    Blynk_BLE.run();
  else
    Blynk_WF.run();
#endif

  timer.run(); 
  checkStatus();

#if (USE_BLYNK_WM && USE_DYNAMIC_PARAMETERS)
  static bool displayedCredentials = false;

  if (!displayedCredentials)
  {
    for (int i = 0; i < NUM_MENU_ITEMS; i++)
    {
      if (!strlen(myMenuItems[i].pdata))
      {
        break;
      }

      if ( i == (NUM_MENU_ITEMS - 1) )
      {
        displayedCredentials = true;
        displayCredentials();
      }
    }
  }
#endif    
}
