/* Tamaguino
  by Alojz Jakob <http://jakobdesign.com>
  modified by TheBrutzler

 ********** TAMAGUINO ***********
   Tamagotchi clone for Arduino
 ********************************

*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#include <WiFi.h>
#include <ArduinoUniqueID.h>
#include <PubSubClient.h>

#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "animations.h"
#include "settings.h"

//ESP32 Sleep
#include <esp_wifi.h>
#include <esp_bt.h>
#include "esp_deep_sleep.h"
#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define uS_TO_mS_FACTOR 1000ULL   /* Conversion factor for micro seconds to milli seconds */

//#define SLEEPDELAY
// Pins


// PIN defines
#define RGB_LED 2         // RGB LED OUTPUT (RESERV)
#define LEFT_DET 4        // BUTTON INPUT LEFT (PULLUP)
#define DOWN_DET 5        // BUTTON INPUT DOWN (PULLUP)
#define USB3_DET 12       // USB3 INPUT DETECTION
#define UP_DET 13         // BUTTON INPUT UP (PULLUP)
#define PWR2_EN 14        // USB2 ENABLE OUTPUT
#define PWR3_EN 15        // USB3 ENABLE OUTPUT
#define USB4_DET 16       // USB4 INPUT DETECTION
#define PWR4_EN 17        // USB4 ENABLE OUTPUT
#define USB5_DET 18       // USB5 INPUT DETECTION
#define PWR5_EN 19        // USB5 ENABLE OUTPUT
#define OLED_SDA 21       // DISPLAY SDA DATA BIDIRECTIONAL
#define OLED_SCL 22       // DISPLAY SCL CLOCK OUTPUT
#define MID_DET 23        // BUTTON INPUT MIDDLE (PULLUP)
#define USB1_DET 25       // USB1 INPUT DETECTION
#define PWR1_EN 26        // USB1 ENABLE OUTPUT
#define USB2_DET 27       // USB2 INPUT DETECTION
#define USB1_AMP_ANA 32   // USB1 ANALOG INPUT CURRENT 400mV/Amp 
#define USB2_AMP_ANA 33   // USB2 ANALOG INPUT CURRENT
#define USB3_AMP_ANA 34   // USB3 ANALOG INPUT CURRENT
#define USB4_AMP_ANA 35   // USB4 ANALOG INPUT CURRENT
#define RIGHT_DET 36      // BUTTON INPUT RIGHT (PULLUP)
#define USB5_AMP_ANA 39   // USB5 ANALOG INPUT CURRENT

int ANALOG_USB[5] = {USB1_AMP_ANA, USB2_AMP_ANA, USB3_AMP_ANA, USB4_AMP_ANA, USB5_AMP_ANA};
int ANALOG_READ_USB[5];

Adafruit_SSD1306 display(128, 64, &Wire); //, OLED_RESET);

#define TIMEOUT_MILLIS 60000
unsigned long timeout = 0;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASS;

void display_display()
{
  if (millis() - timeout > TIMEOUT_MILLIS)
    display.clearDisplay();
  display.display();
}

void setup()
{
  Serial.begin(115200);
  UniqueIDdump(Serial);
  
  Wire.begin(OLED_SDA, OLED_SCL);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false);
  display.dim(0);
  display.clearDisplay();

  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int cnt = 0;
  bool second_run = false;
  while (WiFi.status() != WL_CONNECTED)
  {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(F("    Connect WiFi"));
    display.drawBitmap(34, 10, block[cnt % 13], 54, 54, WHITE);
    display_display();
    cnt++;
    // irgendein bug - keine ahnung - wird dadurch behoben
    // manchmal funktioniert die wlan verbindung nach einem reset nicht
    // und ein weiterer reset ist nötig -> wird dadurch umgangen
    if (cnt > 13 * 5 && !second_run)
    {
      WiFi.begin(ssid, password);
      cnt = 0;
      second_run = true;
    }
    else if (second_run)
    {
      break;
    }
    delay(100);
  }
  Serial.println();
  if (WiFi.isConnected())
  {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(F("      Connected"));
    display.drawBitmap(34, 10, block[0], 54, 54, WHITE);
    display_display();
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());
  }
  else
  {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print(F("       No Wifi!"));
    display.drawBitmap(34, 10, block[0], 54, 54, WHITE);
    display_display();
  }

  
  if(WiFi.status() == WL_CONNECTED)
  {
  ArduinoOTA.setHostname("PORN_HUB");
  ArduinoOTA
      .onStart([]()
         {
           String type;
           if (ArduinoOTA.getCommand() == U_FLASH)
             type = "sketch";
           else // U_SPIFFS
             type = "filesystem";

           // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
           Serial.println("Start updating " + type);
           display.clearDisplay();
           display.setCursor(2, 2);
           display.print("Starting OTA...");
           display_display();
         })
      .onEnd([]()
         {
           Serial.println("\nEnd");
           display.setCursor(2, 22);
           display.print("OTA finished");
           display_display();
         })
      .onProgress([](unsigned int progress, unsigned int total)
        {
          Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
          display.setCursor(2, 12);
          display.printf("Progress: %u%%\r", (progress / 
          (total / 100)));
           display_display();
        })
      .onError([](ota_error_t error)
       {
         String error_msg = "";
         Serial.printf("Error[%u]: ", error);
         display.setCursor(2, 22);
         display.printf("Error[%u]: ", error);
         if (error == OTA_AUTH_ERROR)
           error_msg = "Auth Failed";
         else if (error == OTA_BEGIN_ERROR)
           error_msg = "Begin Failed";
         else if (error == OTA_CONNECT_ERROR)
           error_msg = "Connect Failed";
         else if (error == OTA_RECEIVE_ERROR)
           error_msg = "Receive Failed";
         else if (error == OTA_END_ERROR)
           error_msg = "End Failed";

         Serial.println(error_msg);
         display.setCursor(2, 32);
         display.print(error_msg);
         display_display();
       });
       
       
    ArduinoOTA.begin();
  }
    

// USB POWER ENABLE PINS
  digitalWrite(PWR1_EN, HIGH); 
  pinMode(PWR1_EN, OUTPUT);
  digitalWrite(PWR2_EN, HIGH); 
  pinMode(PWR2_EN, OUTPUT);
  digitalWrite(PWR3_EN, HIGH); 
  pinMode(PWR3_EN, OUTPUT);
  digitalWrite(PWR4_EN, HIGH); 
  pinMode(PWR4_EN, OUTPUT);
  digitalWrite(PWR5_EN, HIGH); 
  pinMode(PWR5_EN, OUTPUT);

// USB INPUT DETECTION PINS
  pinMode(USB1_DET, INPUT);
  pinMode(USB2_DET, INPUT);
  pinMode(USB3_DET, INPUT);
  pinMode(USB4_DET, INPUT);
  pinMode(USB5_DET, INPUT);

// CONTROLLS CROSS
  pinMode(UP_DET,INPUT_PULLUP);
  pinMode(MID_DET,INPUT_PULLUP);
  pinMode(RIGHT_DET,INPUT_PULLUP);
  pinMode(DOWN_DET,INPUT_PULLUP);
  pinMode(LEFT_DET,INPUT_PULLUP);

// ANALOG CURRENT INPUTS
  pinMode(USB1_AMP_ANA, INPUT);
  pinMode(USB2_AMP_ANA, INPUT);
  pinMode(USB3_AMP_ANA, INPUT);
  pinMode(USB4_AMP_ANA, INPUT);
  pinMode(USB5_AMP_ANA, INPUT);
  analogRead(USB1_AMP_ANA);
  analogRead(USB2_AMP_ANA);
  analogRead(USB3_AMP_ANA);
  analogRead(USB4_AMP_ANA);
  analogRead(USB5_AMP_ANA);
  

}

void loop()
{
  
  //if(WiFi.status() == WL_CONNECTED)
  ArduinoOTA.handle();
  //DelayLightSleep(50);
  for(int counter = 0; counter < 5; counter++)
  {
    ANALOG_READ_USB[counter] = analogRead(ANALOG_USB[counter]);
    delay(10);
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(F("1USB: "));
  display.println(ANALOG_READ_USB[0]);
  display.print(F("2USB: "));
  display.println(ANALOG_READ_USB[1]);
  display.print(F("3USB: "));
  display.println(ANALOG_READ_USB[2]);
  display.print(F("4USB: "));
  display.println(ANALOG_READ_USB[3]);
  display.print(F("5USB: "));
  display.println(ANALOG_READ_USB[4]);
  
  //Serial.print(F("1USB: "));
  Serial.print(ANALOG_READ_USB[0]);
  Serial.print(F(" "));
  //Serial.print(F("2USB: "));
  Serial.print(ANALOG_READ_USB[1]);
  Serial.print(F(" "));
  //Serial.print(F("3USB: "));
  Serial.print(ANALOG_READ_USB[2]);
  Serial.print(F(" "));
  //Serial.print(F("4USB: "));
  Serial.print(ANALOG_READ_USB[3]);
  Serial.print(F(" "));
  //Serial.print(F("5USB: "));
  Serial.println(ANALOG_READ_USB[4]);
  display_display();
  /*display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.print(F("USB1: "));
  display.println(ANALOG_READ_USB[1]);
  display.print(F("USB2: "));
  display.println(ANALOG_READ_USB[2]);
  display.print(F("USB3: "));
  display.println(ANALOG_READ_USB[3]);
  display.print(F("USB4: "));
  display.println(ANALOG_READ_USB[4]);
  display.print(F("USB5: "));
  display.println(ANALOG_READ_USB[5]);*/
 
  //DelayLightSleep(5000);
  /*
  display.setCursor(0, 56);
  display.setTextColor(WHITE);
  display.print(F("LvL: "));
  display.setCursor(64, 56);
  display.setTextColor(WHITE);
  display.print(F("Punkte: "));
  display.fillRect(24, 11, 80, 15, BLACK);
  display.fillRect(25, 12, 78, 13, WHITE);
  display.setCursor(47, 15);
  display.setTextColor(BLACK);
  display.println(F("Pause"));*/
}

void DelayLightSleep(int milis)
{
  esp_sleep_enable_timer_wakeup(milis * uS_TO_mS_FACTOR);
  esp_light_sleep_start();
}
