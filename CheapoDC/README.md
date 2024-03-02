# Building and installing CheapoDC

CheapoDC is intended for use on ESP32 devices. The current version has been verified on an ESP32-C3 supermini board.

* Use the Arduino IDE.
* You will need the ESP32 Sketch data uploader with support for LittleFS.
* Add support for esp32 boards to the Arduino IDE - use the Espressif version 2.0.x
* make sure the following libraries are installed:
  * AsyncTCP by Me-No-Dev Version 1.1.4
  * ESPAsyncWebSrv by Me-No-Dev Version 1.2.6
  * FS Built-In by Hristo Gochkov, Ivan Grokhtkov Version 2.0.0
  * LittleFS Built-In Version 2.0.0 
  * Time by Michael Margolis Version 1.6.1
  * ESP32Time by fbiego Version 2.0.4
  * WiFiClientSecure Built-In by Evandro Luis Copercini Version 2.0.0
  * WiFi(esp32) Built-In by Hristo Gochkov Version 2.0.0
  * HTTPClient Built-In by Markus Sattler Version 2.0.0
  * Update Built-In by Hristo Gochkov Version 2.0.0
  * EasyLogger by Alex Skov Jensen Version 1.1.4 

* Download the latest release from <https://github.com/hcomet/CheapoDC/releases>
## Configure CDCDefines.h
* go to CDCdefines.h to configure the firmware before building:
  * Pick which ESP32 pins will be used for PWM output. Default 0 & 1
  * Pick PWM channel to use. Default of 0 should be fine.
  * Set the pin for the status LED on you ESP32. Default 8.
  * Enable/Disable Basic Web Authentication. Default Enabled.
    * change ID and Password. Default is Admin, Admin.
  * Enable/Disable Web Socket API support. Default is Disabled.  
    ***NOTE:*** If the Web Socket API support is enabled then **two** changes are required to increase both the Web Sockets message queue and the AsyncTCP async queue or refreshing the 
    Web UI config page will cause random crashes due to AsyncTCP locking up on a full queue.
    1. In the AsyncTCP.cpp file in your Arduino Libraries folder:  
    Change  
    ```_async_queue = xQueueCreate(32, sizeof(lwip_event_packet_t *));```  
    to  
    ```_async_queue = xQueueCreate(64, sizeof(lwip_event_packet_t *));```
    2. In the AsyncWebSocket.h file in your Arduino Libraries folder,  
    Change  
    ```#define WS_MAX_QUEUED_MESSAGES 32```  
    to  
    ```#define WS_MAX_QUEUED_MESSAGES 64```  

  * Set up WiFi configuration (most can also be configured in CDCWiFi.json)
    * set host name. Default cheapodc.
    * set SSID and Password for your access point. Needed for Station Mode.
    * change default AP SSID and password. Needed for AP mode when a WiFi connection to your network cannot be established. Cannot be configured in CDCWiFi.json.
  * Set the name, coordinates and time zone for your default location. Default is for Greenwich. Can be set in CDCConfig.json
  * Select which weather service to use for weather queries. Default OpenWeather
    * if you use [OpenWeather](https://openweathermap.org/) then you will need to register at OpenWeather for a free account and API key. <https://home.openweathermap.org/users/sign_up>
    * if you use [Open-Meteo](https://open-meteo.com/) then no registration is required and no API key is needed.
    * DO NOT modify the URLs for API or Icon calls.
## Build  
* Now build and upload the firmware to your ESP32.
* Prepare to upload the Sketch data.
    * these files are needed for the Web UI as well as configuration of the CheapoDC.

## CDCConfig.json

This file contains the dew controller configuration. If not present then it will be created from the defaults and updated every 10 seconds. Configuration can then be set up via the Web UI. This file is used to save configuration of the CheapoDC between power cycles.

## CDCWiFi.json

This file provides a list of WiFi access points that will be tried by the CheapoDC to establish a network connection. This file must be edited on your PC and uploaded using the Sketch data upload tool.

The file content is json formatted using "name":"value" pairs.
`{"hostname":"cheapodc","connectAttempts":"10","tryAPs":"1","wifi":[{"ssid":"FakeSSID1","password":"Password1"},{"ssid":"FakeSSID2","password":"Password2"}]}`

The example shows settings for two access points in the "wifi" section. This can be 1 or several. The CheapoDC, in station mode (STA), will step through the list until a successful WiFi connection is achieved. Replace "FakeSSID1" and "Password1" with your SSID and password. You can also change the "hostname". Changing "connectAttempts" will change then number of times the CheapoDC will attempt to connect to an access point before moving to the next access point in the list. Changing "tryAPs" will change the number of times the CheapoDC will loop through the list of access points before switching to Access Point mode (AP).

If the CheapoDC fails to connect to an access point in Station Mode then it will switch to Access Point mode. The SSID and Password for AP mode is set in the CDCdefines.h file.



    