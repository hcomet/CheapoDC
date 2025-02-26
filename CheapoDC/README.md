# Building and installing CheapoDC

## If you are Upgrading from a Previous Release

This note is to highlight the change in recommended partition scheme implemented for release v2.1.0. If you are upgrading from a release prior to v2.1.0 then you will need to switch partition schemes as part of the upgrade. Please read the [Partition Scheme](#partition-scheme) section for the reasons for the change.

## Changes in CheapoDC V2

1. V2 contains changes to support new library versions:
   * Arduino core for ESP32 version 3.1.x support. Core Version 2.x support now deprecated as of CheapoDC v2.1.0.
   * Switch to the [ESP32 Asynchronous Networking Organization](https://github.com/ESP32Async) versions of
   ESPAsyncWebServer and AsyncTCP.
   * ArduinoJson version 7. Version 6 should also continue to work but migration to version 7 is encouraged.
2. The following libraries are no longer required:
   * EasyLogger - a fixed copy of this library is included in the CheapoDC source to remove a macro definition conflict with ArduinoJSON.
   * ESP32Time.
3. The Weather Source is now runtime selectable and is no longer a build configuration option. The Weather Source may now be set through the Web UI or API. Open-Meteo is now the default Weather Source.
4. A new Weather Source has been added to support integration with personal weather stations or other weather services. When the Weather Source is set to ***External Source*** the local ambient temperature and humidity may be set through the API. The Dew Point and dew controller power output will then be calculated based on these externally set values.
5. API changes:
   * The Weather Source (***`WS`***) API command now supports a SET capability. This API command has also been changed
   to use an integer value to specify the Weather Source:  
   `[Open-Meteo = 0, OpenWeather = 1, External Source = 2]`  
   This has unfortunately
   made the GET API call NOT backwards compatible to V1.
   * The Ambient Temperature from Weather Query (***`ATPQ`***) API command now supports a SET capability when the Weather Source is set to External Source.
   * The Humidity (***`HU`***) API command now supports a SET capability when the Weather Source is set to External Source.
   * The Weather API URL (***`WAPI`***) API command no longer supports a SET capability.
   * The Weather Icon URL (***`WIURL`***) API command no longer supports a SET capability.
   * The Cloud Coverage (***`CLC`***) API command has been deprecated.
6. CheapoDC V2 will automatically upgrade the CDCConfig.json on your CheapoDC device when the new firmware version is installed. Previous configuration settings will be maintained. Follow the [Upgrade Steps](#upgrading-to-v21x) below to preserve your
CheapoDC configuration files.
7. Change in ESP32 Partition Scheme required for CheapoDC V2.1.x. See [Partition Scheme](#partition-scheme) for details.
8. See the CheapoDC [release notes](https://github.com/hcomet/CheapoDC/releases) for additional details.

## Setting Up Your Build Environment

1. Install the Arduino IDE.  
  <u>**NOTE:**</u> Version 2.3.4 of the Arduino IDE is a very stable release. As of CheapoDC v2.1.0 verification of
  builds will only be done on IDE version 2.3.4 or newer.
2. Add support for ESP32 modules using the Boards Manager in the Arduino IDE.  
  <u>**IMPORTANT:**</u>
    * Install the boards plugin for ESP32 from Espressif Systems version 3.1.x (Arduino core for the ESP32). CheapoDC verification has been done with version 3.1.1.
    * CheapoDC is intended for use on ESP32 devices. The current version has been verified on an ESP32-C3 Supermini board and an ESP32-S3 mini board.
3. Install the ESP32 Sketch data uploader with support for LittleFS. The CheapoDC uses the LittleFS file system for configuration files and web pages uploaded from the ***CheapoDC/data*** folder. If the data is uploaded using any other file system format, such as SPIFFS, the CheapoDC firmware will not run properly.  
<u>**NOTE:**</u> The following link provides information on how to install a data uploader plugin with LittleFS support:
    * Arduino IDE 2.3.2+: https://randomnerdtutorials.com/arduino-ide-2-install-esp32-littlefs/

4. Install the following libraries if not already installed:
   * [ArduinoJson by Benoit Blanchon](https://arduinojson.org/)  
     Version: 7.3.0
   * [ESP Async WebServer by Me-No-Dev](https://github.com/mathieucarbou/ESPAsyncWebServer)  
     Version: 3.6.0 - [ESP32Async Organization](https://github.com/ESP32Async) version
   * [Async TCP by Me-No-Dev](https://github.com/mathieucarbou/AsyncTCP)  
     Version: 3.3.2 - [ESP32Async Organization](https://github.com/ESP32Async) version
   * [Time by Michael Margolis](https://playground.arduino.cc/Code/Time/)  
     Version: 1.6.1

   <u>**NOTES:**</u>
   * CheapoDC v2.1.0 has migrated from the [dvarrel](https://github.com/dvarrel) versions of EspAsyncWebServer and AsyncTCP to the [ESP32Async Organization](https://github.com/ESP32Async) versions. ESP32Async Organization looks to be very proactive in maintaining the libraries and also contains members of the Arduino core for ESP32 team helping to keep it in sync. Currently the Arduino Library Manager shows Me-No-Dev as the owner but will likely change to ESP32Async.
   * If you have other versions of the ESPSyncWebServer or AsyncTCP not from the links above they will need to be uninstalled to prevent issues with the build.
   * In general, if newer versions of a library are available then only use new subversions not new major versions.

5. Download the latest firmware release source code from <https://github.com/hcomet/CheapoDC/releases>  
  <u>**NOTE:**</u> After extracting the release to your file system open the CheapDC.ino file in the Arduino IDE. This will open the full set of source files in the IDE. Now configure the firmware before building it.

### IDE Tips:
* Many ESP32-C3 board (Dev Module, mini, super mini, micro etc..) configurations have *USB CDC On BOOT:* ***"Disabled"***. This needs to be ***"ENABLED"*** or the Serial Monitor will not work.
* CheapoDC uses *115200 baud* for the Serial Monitor Connection. Leave the default baud rate for the upload.
* Its good practice to set *Erase All Flash Before Sketch Upload:* ***"Enabled"*** the first time you upload a new project to an ESP32. After that, make sure its set to ***"Disabled"*** to preserve the LittleFS partition content between uploads.
## Configure Firmware in the CDCdefines.h file

Edit the ***CDCdefines.h*** file to configure/customize the firmware before building:

  1. Set the ESP32 GPIO pins to be used for Controller Pins 0 & 1. ***DEFAULT: 0 & 1***.  
     ```#define CDC_DEFAULT_CONTROLLER_PIN0 0```  
     ```#define CDC_DEFAULT_CONTROLLER_PIN1 1```
  2. Set the PWM channel to use for Controller Pins. ***DEFAULT: 0***.  
     ```#define CDC_CONTROLLER_PWM_CHANNEL 0```  
     <u>**NOTE:**</u> Changing this value may have a negative impact on CheapoDC operation. If using additional Controller Pins 2 through 5 then PWM channels 1 through 4 are reserved for their use.
  3. Set the pin for the status LED on you ESP32. ***DEFAULT: 8***.  
     ```#define CDC_DEFAULT_STATUS_LED_PIN 8```  
     <u>**NOTE:**</u> Some ESP32-C3 modules have the Status LED wired so that setting the status pin high is off or reversed. Uncomment this line to reverse the High/Low setting for the status LED pin.  
     ```//#define CDC_REVERSE_HIGH_LOW```
  4. Enable/Disable Basic Web Authentication. ***DEFAULT: Enabled***.  
     ```#define CDC_ENABLE_WEB_AUTH```  
     <u>**NOTE:**</u> Comment out the ```#define``` to disable.
  5. Change the Web Authentication ID and Password. ***DEFAULT: Admin, Admin***.  
      ```#define CDC_DEFAULT_WEB_ID "admin"```  
      ```#define CDC_DEFAULT_WEB_PASSWORD "admin"```  
      <u>**NOTE:**</u> Web Authentication is the only security for OTA firmware updates.
  6. Enable/Disable Web Socket API support. ***DEFAULT: Disabled***.  
     ```// #define CDC_ENABLE_WEB_SOCKETS```  
    <u>**NOTE:**</u> Uncomment the ```#define``` to enable.  
    Web Socket API support is not needed to use the CheapoDC or the associated INDILIB driver. If the Web Socket API support is enabled then the command post processing is also enabled. See item 7. below for details.
  7. Enable/Disable Command post processing queue support ***DEFAULT: Disabled***.  
     ```// #define CDC_ENABLE_WEB_SOCKETS```  
    <u>**NOTE:**</u> Uncomment the ```#define``` to enable.  
    Some API SET commands may auto generate post processing actions such as weather queries or power output updates. These may cause long command transactions. If the API client is making many asynchronous calls to the API and which are long transactions then the AsyncTCP queue can be overwhelmed and prevent the ESP32 watchdog timer from getting enough time causing panic crashes. Enabling this queue will cause the post processing actions to be put in a queue that is processed every `CDC_RUNCMDQUEUE_EVERY` milliseconds (default is 10 msec). For situations where API calls are synchronously called (ie: HTML embedded or the INDI driver) the queue is not needed. Which is why the default is disabled. For situations where asynchronous calls are not limited (ie: the Web Sockets javascript client) then it should be enabled. Enabling Web Sockets will automatically enable the queue.  

      If issues with the ESP32 watchdog timer are seen when making many asynchronous parallel API calls or WebSocket calls don't complete even with the queue enabled then either have the client limit the number of asynchronous API calls executed in parallel or increase the AsyncWebSocket queue size:

      * In the AsyncWebSocket.h file in your Arduino Libraries folder,  
    Change  
    ```#define WS_MAX_QUEUED_MESSAGES 32```  
    to  
    ```#define WS_MAX_QUEUED_MESSAGES 64```  

  8. Set up the WiFi configuration.  The CheapoDC may operate in either Access Point (AP) mode or Station (ST) mode. The CheapoDC defaults to AP mode when it cannot connect to an access point.
     * AP Mode settings must be configured in the ***CDCdefines.h*** file:
       * Change AP mode SSID. ***DEFAULT: cheapodc***.  
         ```#define CDC_DEFAULT_WIFI_AP_SSID "cheapodc"```  
       * Change AP mode password. ***DEFAULT: cheapodc***.  
         ```#define CDC_DEFAULT_WIFI_AP_PASSWORD "cheapodc"```  
     * ST Mode settings may be configured in either the ***CDCdefines.h*** file or the ***CDCWiFi.json*** file found in the ***data*** folder. Values in found in the ***CDCWiFi.json*** files will be used before using the values in the ***CDCdefines.h*** file. Using the ***CDCWiFi.json*** file to configure WiFi access allows multiple APs and credentials to be defined. How to configure the ***CDCWiFi.json*** file is found [here](#cdcwifijson) while configuring default values in the ***CDCdefines.h*** file is as follows:
       * Change ST mode SSID. ***DEFAULT: defaultSSID***.  
         ```#define CDC_DEFAULT_WIFI_SSID "defaultSSID"```  
       * Change ST mode password. ***DEFAULT: defaultPassword***.  
         ```#define CDC_DEFAULT_WIFI_PASSWORD "defaultPassword"```
      * WiFi TX Power setting may be optionally modified if you are having issues with an ESP32-C3 board connecting to your WiFi gateway. Set the WiFi TX Power:
        * Enable setting WiFi Tx Power. ***DEFAULT: Disabled***.  
          ```//#define CDC_ENABLE_WIFI_TX_POWER_MOD WIFI_POWER_8_5dBm ```  
          Uncomment to enable. If enabled the default value sets TX power to 8.5dbm. Any of the `wifi_power_t` values found [here](https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiGeneric.h#L51-L67) may be used.

  9. The hostname for the CheapoDC may also be changed. This is really only needed if you have multiple CheapoDC controllers on the same network.  Its recommended that this be done in the ***CDCWiFi.json*** file to allow a single firmware image to be used across multiple CheapoDC controllers. To change the hostname in the ***CDCdefines.h*** file:
     * Change the host name. ***DEFAULT: cheapodc***.  
       ```#define CDC_DEFAULT_HOSTNAME "cheapodc"```
  10. Do not modify any of the ```#define``` values below the line  ```DO NOT change anything below here```. The Weather Source and OpenWeather API key should be set or changed through the [Web UI](../README.md#web-ui) not in the ***CDCdefines.h*** file.  
      <u>**IMPORTANT:**</u>
      * **DO NOT** modify the URLs for API or Icon calls.

## Build and Run

### Upgrading to V2.1.x

Since v2.1.0 requires a change in the [Partition Scheme](#partition-scheme) upgrades must be done using a USB connection to the device. 

New versions of ESPAsyncWebServer and AsyncTCP libraries are also required for Arduino core of ESP32 version 3.1.x. Please read the [Setting Up Your Build Environment](#setting-up-your-build-environment) section to make sure you have the correct libraries installed in the Arduino IDE. Be sure to have only one version of the AsyncTCP library installed.

#### Upgrade using USB Connected Device

1. Preserve your current CheapoDC configuration and WiFi settings by first downloading the CDCConfig.json and CDCWiFi.json files
from your CheapoDC. This can be done using the Web UI from the [File Management](../README.md#cheapodc-file-management) page.
Place both files in the sketch data folder: *CheapoDC/data*.
2. Build and upload the new firmware to your device.
   * Make sure to set *Erase All Flash Before Sketch Upload:* ***"Enabled"***  
   <u>**NOTE:**</u> Make sure to reset this to **"Disabled"** once you are done with the upgrade or the LittleFS partition will be erased with each 
   firmware upload and you will need upload your sketch data every time.
   * Make sure to have selected the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* partition scheme.
3. Upload the files in the CheapoDC/data folder using the Sketch data uploading tool as indicated step 2. of the
build steps below.
4. Your V2.1.xQueueCreate upgrade should now be complete.

#### Upgrade using Web OTA Firmware Update

Due to the change in [Partition Scheme](#partition-scheme) in v2.1.0, upgrades from releases prior to v2.1.0 cannot be done using an OTA firmware upgrade.

### New Device Build Steps

1. After configuring values in the CDCdefines.h, using the Arduino IDE build and upload the firmware to your ESP32.
   * Make sure to have selected the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* partition scheme.  
   <u>**NOTE:**</u> If the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* partition scheme is not listed as one of the partition schemes available for your selected ESP32-C3 board type you will need to slect a different board type. Often the AliExpress type boards generally only list a minimal set of partition schemes. If you do not see the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* in the list of available partition schemes for your board the best recommendation is to select a Dev Module board in the Arduino IDE. For the ESP32-C3 that would be the *ESP32C3 Dev Module*.

2. Upload the files in the CheapoDC/data folder using the Sketch data uploading tool. Please READ the following points before uploading the data files.
    * **LittleFS file system formatting MUST be used for the data upload.**
    * these files are needed for the Web UI as well as configuration of the CheapoDC.
    * Modify the CDCWiFi.json file ([see below](#cdcwifijson)) to configure CheapoDC to access your wireless network.

## Configuration Files

### CDCWiFi.json

This file provides a list of WiFi access points that will be tried by the CheapoDC to establish a network connection. This file must be edited on your PC and uploaded using the Sketch data upload tool.

The file content is json formatted using "name":"value" pairs.

`{"hostname":"cheapodc","connectAttempts":"10","tryAPs":"1","wifi":[{"ssid":"FakeSSID1","password":"Password1"},{"ssid":"FakeSSID2","password":"Password2"}]}`

The example shows settings for two access points in the "wifi" section. This can be 1 or several access points. The CheapoDC, in station mode (STA), will step through the list sequentially until a successful WiFi connection is achieved.

1. Replace "FakeSSID1" and "Password1" for the first AP to try with your first SSID and password.
```{"ssid":"FakeSSID1","password":"Password1"}```
2. To set up a second AP (or more) then update "FakeSSID2" and "Password2" for the second AP (or add more entries separated by commas for more APs). If you only want one AP then delete the entry including the preceding comma.  
```{"ssid":"FakeSSID2","password":"Password2"}```
3. You can also change the "hostname" from the default of ***cheapodc***.  
```"hostname":"cheapodc"```
4. Changing "connectAttempts" will change then number of times the CheapoDC will attempt to connect to an access point before moving to the next access point in the list.  
```"connectAttempts":"10"```
5. Changing "tryAPs" will change the number of times the CheapoDC will loop through the list of access points before switching to Access Point (AP) mode.  
```"tryAPs":"1"```

If the CheapoDC fails to connect to an access point in Station Mode then it will switch to Access Point mode. The SSID and Password for AP mode is set in the [***CDCdefines.h***](#configure-firmware-in-the-cdcdefinesh-file) file.

If no ***CDCWiFi.json*** file is found then the values for WiFi configuration will be taken from the [***CDCdefines.h***](#configure-firmware-in-the-cdcdefinesh-file) file.

### CDCConfig.json

This file is used to save the dew controller configuration between power cycles. If the file is not present then the values defined in the ***CDCdefines.h*** file (the firmware defaults) will be used. The CheapoDC checks for changes to the controller's configuration every 10 seconds and if a change is detected then the full ***CDCdefines.h*** file will be written to the LittleFS file system on the controller.

Controller configuration may be modified via the [Web UI](../README.md/#web-ui) or the [CheapoDC API](../README.md/#cheapodc-api). The [CheapoDC commands](../README.md/#cheapodc-commands) which support a ***Setter*** function also represent the values stored in the ***CDCConfig.json*** file.

## Partition Scheme

The ESP32 Partition Scheme is a term used by the Arduino IDE to select a partition table that defines the flash memory organization and the different kinds of data that will be stored in each partition. The ESP32-C3, which this project uses, generally has 4MB of flash memory with the default partition scheme of *Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)*.

The *Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)* partition scheme provides two application partitions of 1.2MB to support OTA updates plus a 1.5MB SPIFFS partition used for LittleFS file storage. I selected this as the recommended partition scheme for the initial releases of CheapoDC because:

1. Being a default scheme, it was available for all ESP32-C3 board types.
2. The initial release of CheapoDC only needed 80% of the application partition space and I felt that future upgrades and fixes wouldn't need a lot more than that.
3. The other predefined partition schemes didn't provide a good option for larger application size, OTA support and a partition large enough for the LittleFS data CheapoDC needed.

It turns out that I was wrong about the second point. Although I have not added a lot of code to the initial version of CheapoDC the underlying libraries it relies on have changed and expanded in size. A combination of moving to the Arduino core for ESP32 3.1.x,
Arduino 2.3.x, adopting the ESPAsync Organization version of ESPAsync Web Server plus other new library releases has put the CheapoDC v2.1.0 release at 98.6% of the application space. This is without Debug logging or WebSockets support enabled, which can no longer be enabled.

The only solution is to move to a different partition scheme starting with CheapoDC release v2.1.0. As of v2.1.0 the recommended partition scheme is *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)*. This scheme should hopefully provide plenty of application space going forward. The basic configuration of CheapoDC v2.1.0 takes 62% of the available space. In order to fit in the smaller SPIFFS partition the HTML, CSS and JS files used by CheapoDC have been run through a minimizer and additional compression applied to the PNG images. 


### Impact of the change in Partition Scheme

If this is your first time building the CheapoDC then you are not affected. The [new device build instructions](#new-device-build-steps) still apply.

If you are upgrading to v2.1.x from a release prior to v2.1.0 then you are affected. Read and follow the [Upgrading to v2.1.x](#upgrading-to-v21x) instructions.






