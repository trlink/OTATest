//includes
//////////
#include <Arduino.h>
#include "esp32fota.h"
#include <WiFi.h>
#include <SPIFFS.h>

//please uncomment for your hardware
////////////////////////////////////
#define HARDWARE_TBEAM
//#define HARDWARE_ESP

#ifdef HARDWARE_ESP
  #include "heltec.h"
  #define INSTALL_DIR "http://62.75.216.36/trlink/install/install.json"
  #define INSTALL_PORT 80
#endif
#ifdef HARDWARE_TBEAM
  #define INSTALL_DIR "http://62.75.216.36/trlink/install/tbinstall.json"
  #define INSTALL_PORT 80
#endif

//enter credentials to your WLan here
/////////////////////////////////////
const char *ssid = "";
const char *password = "";



esp32FOTA esp32FOTA("trlink", 1, INSTALL_DIR, INSTALL_PORT);

void setup()
{
  Serial.begin(115200);
  setup_wifi();

  if(!SPIFFS.begin())
  {
    Serial.println(F("An Error has occurred while mounting SPIFFS"));
   
    return;
  };

  
  bool formatted = SPIFFS.format();
 
  if(formatted){
    Serial.println("\n\nSuccess formatting");
  }else{
    Serial.println("\n\nError formatting");
  }
}

void setup_wifi()
{
  delay(10);
  Serial.print("Connecting to ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
}

void loop()
{
  bool updatedNeeded = esp32FOTA.execHTTPcheck();
  if (updatedNeeded)
  {
    esp32FOTA.execOTA();
  }
  delay(2000);
}
