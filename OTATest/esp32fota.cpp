/*
   esp32 firmware OTA
   Date: December 2018
   Author: Chris Joyce <https://github.com/chrisjoyce911/esp32FOTA/esp32FOTA>
   Purpose: Perform an OTA update from a bin located on a webserver (HTTP Only)
*/

#include "esp32fota.h"
#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include "ArduinoJson.h"
#include <SPIFFS.h>



esp32FOTA::esp32FOTA(String firwmareType, int firwmareVersion, String strInstallSource, int nWebserverPort)
{
  this->m_strFirwmareType = firwmareType;
  this->m_nFirwmareVersion = firwmareVersion;
  this->m_bUseDeviceID = false;
  this->m_strCheckURL = strInstallSource;
  this->m_pFiles = NULL;
  this->m_nPort = nWebserverPort;
}



static void splitHeader(String src, String &header, String &headerValue)
{
  //variables
  ///////////
  int idx = 0;

  idx = src.indexOf(':');
  header = src.substring(0, idx);
  headerValue = src.substring(idx + 1, src.length());
  headerValue.trim();

  return;
};


void updateFile(String strHost, int nPort, String strUrl, String strFile) 
{
  //variables
  ///////////
  WiFiClient client;
  File f;
  size_t size;
  long len = -1;
  uint8_t buff[128];
  char szHost[strHost.length() + 1];
  long timeout;
  int contentLength = 0;
  bool isValidContentType = false;
  bool gotHTTPStatus = false;
  String header, headerValue, line, contentType;
  bool canBegin; 
  

  strHost.toCharArray(szHost, strHost.length() + 1);
  
  Serial.print(F("connecting to: "));
  Serial.println(strHost);
  
  if (!client.connect(szHost, nPort)) 
  {
    Serial.println(F("connection failed"));
    return;
  };

  Serial.println(String(F("[HTTP] GET: ")) + strUrl);
  
  client.print(String(F("GET ")) + strUrl + String(F(" HTTP/1.1\r\n")) +
               String(F("Host: ")) + strHost + String(F("\r\n")) +
               String(F("Cache-Control: no-cache\r\n")) +
               String(F("Connection: close\r\n\r\n")));

  timeout = millis();
    
  while(client.available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println(F("Client Timeout !"));
      client.stop();
      
      return;
    };
  };

  while(client.available())
  {
    header = "";
    headerValue = "";
    
    // read line till /n
    line = client.readStringUntil('\n');
    
    // remove space, to check if the line is end of headers
    line.trim();

    if(line.length() <= 0)
    {
      //headers ended
      break; 
    };

    // Check if the HTTP Response is 200
    // else break and Exit Update
    if(line.startsWith("HTTP/1.1"))
    {
      if(line.indexOf("200") < 0)
      {
        Serial.println(F("Got a non 200 status code from server. Exiting download."));
        break;
      };
      
      gotHTTPStatus = true;
    };

    if(false == gotHTTPStatus)
    {
      continue;
    };

    splitHeader(line, header, headerValue);

    // extract headers here
    // Start with content length
    if(header.equalsIgnoreCase("Content-Length"))
    {
      contentLength = headerValue.toInt();
      
      Serial.println(String(F("Need to download ")) + String(contentLength) + String(F(" bytes from server")));
      
      continue;
    };

    // Next, the content type
    if(header.equalsIgnoreCase(F("Content-type")))
    {
      contentType = headerValue;
      
      Serial.println(String(F("File is a ")) + contentType + String(F(" payload.")));
      
      isValidContentType = true;
    };
  };

  // check contentLength and content type
  if((contentLength > 0) && (isValidContentType == true))
  {
    f   = SPIFFS.open(strFile, "w");
    len = 0;
    
    if(f) 
    {
      timeout = millis();
      
      while(contentLength > 0)
      {
        size = client.available();
        
        if (size > 0) 
        {
          int c = client.readBytes(buff, ((size > 128) ? 128 : size));
          contentLength -= c;
          len += c;
          
          f.write(buff, c);

          timeout = millis();
        };

        if((millis() - timeout) > 10000)
        {
          Serial.println(F("Read timed out"));
        };
      };

      if(contentLength <= 0)
      {
        Serial.print(F("Read: "));
        Serial.print(len);
        Serial.println(" - done.");
      }
      else
      {
        Serial.println(String(F("Remaining: ")) + String(contentLength) + String(F(" - download failed...")));
      };
      
      f.close();
    }
    else
    {
      Serial.println(String(F("Failed to open file for writing: ")) + strFile);
    };
  };
};



// OTA Logic
void esp32FOTA::execOTA()
{
  //variables
  ///////////
  WiFiClient client;
  int contentLength = 0;
  bool isValidContentType = false;
  bool gotHTTPStatus = false;
  unsigned long timeout;
  String header, headerValue, line, contentType;
  bool canBegin; 
  size_t written;
  _sFiles *pFile = this->m_pFiles;
  
  

  Serial.println(String(F("Connecting to: ")) + String(this->m_strHost) + String(F(" at: ")) + String(this->m_nPort));
    
  // Connect to Webserver
  if(client.connect(this->m_strHost.c_str(), this->m_nPort))
  {
    // Connection Succeed.
    // Fetching the bin
    Serial.println(String(F("Fetching Bin: ")) + String(this->m_strBin));

    // Get the contents of the bin file
    client.print(String(F("GET ")) + this->m_strBin + String(F(" HTTP/1.1\r\n")) +
                 String(F("Host: ")) + this->m_strHost + String(F("\r\n")) +
                 String(F("Cache-Control: no-cache\r\n")) +
                 String(F("Connection: close\r\n\r\n")));

    timeout = millis();
    
    while(client.available() == 0)
    {
      if (millis() - timeout > 5000)
      {
        Serial.println(F("Client Timeout !"));
        client.stop();
        
        return;
      };
    };

    while(client.available())
    {
      header = "";
      headerValue = "";
      
      // read line till /n
      line = client.readStringUntil('\n');
      
      // remove space, to check if the line is end of headers
      line.trim();

      if(!line.length())
      {
        //headers ended
        break; // and get the OTA started
      };

      // Check if the HTTP Response is 200
      // else break and Exit Update
      if(line.startsWith("HTTP/1.1"))
      {
        if(line.indexOf("200") < 0)
        {
          Serial.println(F("Got a non 200 status code from server. Exiting OTA Update."));
          break;
        };
        
        gotHTTPStatus = true;
      }

      if(false == gotHTTPStatus)
      {
        continue;
      };

      splitHeader(line, header, headerValue);

      // extract headers here
      // Start with content length
      if(header.equalsIgnoreCase("Content-Length"))
      {
        contentLength = headerValue.toInt();
        
        Serial.println(String(F("Got ")) + String(contentLength) + String(F(" bytes from server")));
        
        continue;
      };

      // Next, the content type
      if (header.equalsIgnoreCase(F("Content-type")))
      {
        contentType = headerValue;
        
        Serial.println(String(F("Got ")) + contentType + String(F(" payload.")));
        
        if(contentType == String(F("application/octet-stream")))
        {
            isValidContentType = true;
        };
      };
    };
  }
  else
  {
    // Connect to webserver failed
    // May be try?
    // Probably a choppy network?
    Serial.println(String(F("Connection to ")) + String(this->m_strHost) + String(F(" failed. Please check your setup")));
    
    return;
  };

  // Check what is the contentLength and if content type is `application/octet-stream`
  Serial.println(String(F("contentLength : ")) + String(contentLength) + String(F(", isValidContentType : ")) + String(isValidContentType));

  // check contentLength and content type
  if (contentLength && isValidContentType)
  {
    // Check if there is enough to OTA Update
    canBegin = Update.begin(contentLength);

    // If yes, begin
    if(canBegin)
    {
      Serial.println(F("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!"));
      
      // No activity would appear on the Serial monitor
      // So be patient. This may take 2 - 5mins to complete
      written = Update.writeStream(client);

      if (written == contentLength)
      {
        Serial.println(String(F("Written : ")) + String(written) + String(F(" successfully")));
      }
      else
      {
        Serial.println(String(F("Written only : ")) + String(written) + String(F(" / ")) + String(contentLength));

        return;
      };

      if(Update.end())
      {
        Serial.println(F("OTA done!"));
        
        if(Update.isFinished())
        {
          if(this->m_pFiles != NULL)
          {
            Serial.println(F("Firmware Update successfully completed. Download additional files:"));
  
            while(pFile != NULL)
            {
              if(pFile->strSource.length() > 0)
              {
                updateFile(this->m_strHost, this->m_nPort, pFile->strSource, pFile->strDest);
              };

              pFile = pFile->pNext;
            };
          };
          
          Serial.println(F("Update successfully completed. Rebooting."));
          ESP.restart();
        }
        else
        {
          Serial.println(F("Update not finished? Something went wrong!"));
        };
      }
      else
      {
        Serial.println(String(F("Error Occurred. Error #: ")) + String(Update.getError()));
      };
    }
    else
    {
      // not enough space to begin OTA
      // Understand the partitions and
      // space availability
      Serial.println(F("Not enough space to begin OTA"));
      client.flush();
    };
  }
  else
  {
    Serial.println(F("There was no content in the response"));
    client.flush();
  }
}




bool esp32FOTA::execHTTPcheck()
{
  //variables
  ///////////
  String useURL;
  HTTPClient http;
  int httpCode;
  String payload;
  JsonArray jsonArray;
  int nPayloadLen;
  _sFiles *pFile;
  String fwtype;
  int plversion;
  
  if(this->m_bUseDeviceID == true)
  {
    useURL = this->m_strCheckURL + "?id=" + this->getDeviceID();
  }
  else
  {
    useURL = this->m_strCheckURL;
  };

  Serial.print(F("Getting: "));
  Serial.println(useURL);
  Serial.println(F("------"));

  //Check the current connection status
  if(WiFi.status() == WL_CONNECTED)
  { 
    http.begin(useURL);        //Specify the URL
    httpCode = http.GET(); //Make the request

    //Check is a file was returned
    if(httpCode == 200)
    { 
      Serial.println(F("Document retrieved..."));
      
      payload = http.getString();
      nPayloadLen = payload.length();

      Serial.print(String(F("Size: ")));
      Serial.println(nPayloadLen);
      
      char JSONMessage[nPayloadLen + 1];
      payload.toCharArray(JSONMessage, nPayloadLen + 1);

      Serial.println(JSONMessage);
    
      DynamicJsonDocument JSONDocument(nPayloadLen + 1000); //Memory pool
      DeserializationError err = deserializeJson(JSONDocument, JSONMessage);

      //Check for errors in parsing
      if(err)
      { 
        Serial.println(F("Parsing failed:"));
        Serial.println(payload);
        
        delay(5000);
        return false;
      };

      this->m_strHost = JSONDocument["host"].as<String>();
      this->m_strBin = JSONDocument["bin"].as<String>();
      this->m_nPort = JSONDocument["port"];

      plversion = JSONDocument["version"];
      fwtype = JSONDocument["type"].as<String>();

      //parse files array
      if(JSONDocument["files"] != NULL)
      {
        Serial.println(F("Read files"));
        
        jsonArray = JSONDocument["files"];

        this->m_pFiles = new _sFiles;
        this->m_pFiles->pNext = NULL;

        pFile = this->m_pFiles;

        for(JsonObject obj : jsonArray) 
        {
          pFile->strSource = obj["src"].as<String>();
          pFile->strDest = obj["dst"].as<String>();

          Serial.println(String(F("Add file: ")) + pFile->strSource);

          //create empty new file
          pFile->pNext = new _sFiles;
          pFile->pNext->strSource = "";
          pFile->pNext->strDest = "";

          //set to next
          pFile = pFile->pNext;
        };
      };

      http.end(); //Free the resources      

      if((plversion > this->m_nFirwmareVersion) && (fwtype == this->m_strFirwmareType))
      {
        Serial.println(F("Can update..."));
        
        return true;
      }
      else
      {
        Serial.println(F("update not needed..."));
        
        return false;
      };
    }
    else
    {
        Serial.println(F("Error on HTTP request"));
        return false;
    };
  };
  
  return false;
};



String esp32FOTA::getDeviceID()
{
  //variables
  ///////////
  char deviceid[21];
  uint64_t chipid;
  
  chipid = ESP.getEfuseMac();
  sprintf(deviceid, "%" PRIu64, chipid);
  
  String thisID(deviceid);
  
  return thisID;
};


// Force a firmware update regartless on current version
void esp32FOTA::forceUpdate(String firmwareHost, int firmwarePort, String firmwarePath)
{
  this->m_strHost = firmwareHost;
  this->m_strBin = firmwarePath;
  this->m_nPort = firmwarePort;
  
  this->execOTA();
};
