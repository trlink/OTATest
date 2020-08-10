//includes
//////////
#include <Arduino.h>
#include "heltec.h"
#include "esp32fota.h"
#include <WiFi.h>
#include <SPIFFS.h>


#define INSTALL_DIR "http://62.75.216.36/trlink/install/install.json"
#define INSTALL_PORT 80


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
