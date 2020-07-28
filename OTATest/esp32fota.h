/*
   esp32 firmware OTA
   Date: December 2018
   Author: Chris Joyce <https://github.com/chrisjoyce911/esp32FOTA/esp32FOTA>
   Updated by: Christian Wallukat (https://github.com/trlink)
   Purpose: Perform an OTA update from a bin located on a webserver (HTTP Only)
*/

#ifndef esp32fota_h
#define esp32fota_h

//includes
//////////
#include "Arduino.h"


//struct containing the files to download to SPIFFS
struct _sFiles
{
  String  strSource;
  String  strDest;
  _sFiles *pNext;
};


class esp32FOTA
{
  public:
    esp32FOTA(String firwmareType, int firwmareVersion, String strInstallSource, int nWebserverPort);
    
    void forceUpdate(String firwmareHost, int firwmarePort, String firwmarePath);
    void execOTA();
    bool execHTTPcheck();
    
  
  private:
    //variables
    ///////////
    int       m_nFirwmareVersion;
    String    m_strHost;
    String    m_strBin;
    int       m_nPort;
    String    m_strFirwmareType;
    bool      m_bUseDeviceID;
    String    m_strCheckURL;
    _sFiles  *m_pFiles;

    
    String getDeviceID();
};




#endif
