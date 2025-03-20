# WebFlash as an Option

If you are using an [ESP32-C3 SuperMini](https://michiel.vanderwulp.be/domotica/Modules/ESP32-C3-SuperMini/) or any other ESP32-C3 module then the easiest way to load the CheapoDC firmware onto your device is to use [WebFlash](https://hcomet.github.io/CheapoDC/CheapoDCFlash.html). All other configuration can then be done through the [Web UI](../README.md#web-ui). WebFlash is primarily for new installs but you can also use it for a firmware upgrade. WebFlash will erase all flash memory on your ESP32-C3, so if upgrading, **remember to backup your configuration files (CDCConfig.json & CDCWifi.json)**. 

Go to the [WebFlash page](https://hcomet.github.io/CheapoDC/CheapoDCFlash.html) to install CheapoDC on your ESP32-C3. Then follow the instructions bellow in the [First Time Device Configuration](#first-time-device-configuration) section.

If you prefer to build the firmware yourself using the Arduino IDE please continue on to [Building and Installing CheapoDC](#building-and-installing-cheapodc)

Note that as of CheapoDC v2.2.0, the Web UI supports an semi-automatic web-based upgrade that preserves your configuration files. Web OTA Update while upgrading the firmware and data partitions. See the documentation on [Web OTA Update](https://hcomet.github.io/CheapoDC/CheapoDCWebUpdate.html) for details.

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
3. As of V2.2.0, all major itens that were previously required to be configured at compile time can now be configured through the Web UI or API at runtime. This includes:
   * [WiFi Configuration](../README.md#wifi-configuration): WiFi SSID and password can be set on the Device Management page.
   * Weather Source: The Weather Source may now be set through the Web UI or API. Open-Meteo is now the default Weather Source.
   * [Output to GPIO Pin mapping](../README.md#controller-output-configuration). All controller outputs may be configured using the Web UI or API. This includes the original two core dew controller outputs as well as the four new additional outputs added in V2.2.0.
   * [Status LED GPIO Pin](../README.md#status-led-configuration). The pin used for the status LED as well as whether or not it is active High or Low may be configured through the Web UI or API.
   * [Change Password](../README.md#change-password). The user Id remains **admin** but the password may now be changed in the Web UI. Password security has been improved by moving to using a [Digest access authentication](https://en.wikipedia.org/wiki/Digest_access_authentication#:~:text=In%20contrast%2C%20basic%20access%20authentication,It%20uses%20the%20HTTP%20protocol.) MD5 Password Hash.
4. A new Weather Source has been added to support integration with personal weather stations or other weather services. When the Weather Source is set to ***External Source*** the local ambient temperature and humidity may be set through the API. The Dew Point and dew controller power output will then be calculated based on these externally set values.
5. API changes:
   * ***`CPPx`***, ***`CPMx`*** and ***`CPOx`*** API commands added to allow outputs **x = 0 to 5** to be configured for GPIO Pin mapping, Output Mode and Output Power respectively.
   * The ***`LED`*** API command changed to set the GPIO pin for the Status LED while ***`LEDH`*** sets the value of High to 1 or 0.
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
6. Support for the Web Sockets based API has been deprecated and removed in V2.2.0. The TCP API continues to be supported and used by the INDI driver.
7. CheapoDC V2 will automatically upgrade the CDCConfig.json on your CheapoDC device when the new firmware version is installed. Previous configuration settings will be maintained. Follow the [Upgrade Steps](#upgrading-to-v21x) below to preserve your
CheapoDC configuration files.
8. Change in ESP32 Partition Scheme required for CheapoDC V2.1.x. See [Partition Scheme](#partition-scheme) for details.
9. As of V2.2.0, CheapDC now supports firmware and data partition upgrades using the Web UI and HTTP OTA. See the documentation on [Web OTA Update](https://hcomet.github.io/CheapoDC/CheapoDCWebUpdate.html) for details.
10. See the CheapoDC [release notes](https://github.com/hcomet/CheapoDC/releases) for additional details.

## Setting Up Your Build Environment

1. Install the Arduino IDE.  
  <u>**NOTE:**</u> Version 2.3.4 of the Arduino IDE is a very stable release. As of CheapoDC v2.1.0 verification of
  builds will only be done on IDE version 2.3.4 or newer.
2. Add support for ESP32 modules using the Boards Manager in the Arduino IDE.  
  <u>**IMPORTANT:**</u>
    * Install the boards plugin for ESP32 from Espressif Systems version 3.1.x (Arduino core for the ESP32). CheapoDC verification has been done with version 3.1.1.
    * CheapoDC is intended for use on ESP32 devices. The current version has been verified on an [ESP32-C3 SuperMini](https://michiel.vanderwulp.be/domotica/Modules/ESP32-C3-SuperMini/) board..
3. Install the ESP32 Sketch data uploader with support for LittleFS. The CheapoDC uses the LittleFS file system for configuration files and web pages uploaded from the ***CheapoDC/data*** folder. If the data is uploaded using any other file system format, such as SPIFFS, the CheapoDC firmware will not run properly.  
<u>**NOTE:**</u> The following link provides information on how to install a data uploader plugin with LittleFS support:
    * Arduino IDE 2.3.2+: https://randomnerdtutorials.com/arduino-ide-2-install-esp32-littlefs/

4. Install the following libraries if not already installed:
   * [ArduinoJson by Benoit Blanchon](https://arduinojson.org/)  
     Version: 7.3.1
   * [ESP Async WebServer by ESP32Async](https://github.com/ESP32Async/ESPAsyncWebServer)  
     Version: 3.7.2
   * [Async TCP by ESP32Async](https://github.com/ESP32Async/AsyncTCP)  
     Version: 3.3.6
   * [Time by Michael Margolis](https://playground.arduino.cc/Code/Time/)  
     Version: 1.6.1

   <u>**NOTES:**</u>
   * CheapoDC v2.1.0 has migrated from the [dvarrel](https://github.com/dvarrel) versions of EspAsyncWebServer and AsyncTCP to the [ESP32Async Organization](https://github.com/ESP32Async) versions. ESP32Async Organization looks to be very proactive in maintaining the libraries and also contains members of the Arduino core for ESP32 team helping to keep it in sync. Currently the Arduino Library Manager shows Me-No-Dev as the owner but will likely change to ESP32Async.
   * If you have other versions of the ESPSyncWebServer or AsyncTCP not from the links above they will need to be uninstalled to prevent issues with the build.
   * In general, if newer versions of a library are available then only use new subversions not new major versions.

5. Download the latest firmware release source code from <https://github.com/hcomet/CheapoDC/releases>  
  <u>**NOTE:**</u> After extracting the release to your file system open the CheapoDC.ino file in the Arduino IDE. This will open the full set of source files in the IDE. Now configure the firmware before building it.

### IDE Tips:
* Many ESP32-C3 board (Dev Module, mini, super mini, micro etc..) configurations have *USB CDC On BOOT:* ***"Disabled"***. This needs to be ***"ENABLED"*** or the Serial Monitor will not work.
* CheapoDC uses *115200 baud* for the Serial Monitor Connection. Leave the default baud rate for the upload.
* Its good practice to set *Erase All Flash Before Sketch Upload:* ***"Enabled"*** the first time you upload a new project to an ESP32. After that, make sure its set to ***"Disabled"*** to preserve the LittleFS partition content between uploads.
## Configure Firmware in the CDCdefines.h file

As of V2.2.0 changes to the ***CDCdefines.h*** file are **not** required since all key items can now be modified at runtime and are saved to the [CDCConfig.json](#cdcconfigjson). If you do wish to to configure/customize the firmware before building the following items may be modified in the ***CDCdefines.h*** file:

1. Set the default ESP32 GPIO pins to be used for Controller Outputs 0 & 1. ***DEFAULT: 0 & 1***.  
    ```#define CDC_DEFAULT_CONTROLLER_PIN0 0```  
    ```#define CDC_DEFAULT_CONTROLLER_PIN1 1```  
    _*May be changed through the Web UI or API_
2. Set the PWM channel to use for Controller Outputs. ***DEFAULT: 0***.  
    ```#define CDC_CONTROLLER_PWM_CHANNEL 0```  
  <u>**NOTE:**</u> Changing this value may have a negative impact on CheapoDC operation. PWM channels 1 through 4 are reserved for the use of the additional Controller Outputs 2 through 5.
3. Set the pin for the status LED on you ESP32. ***DEFAULT: 8***.  
    ```#define CDC_DEFAULT_STATUS_LED_PIN 8```  
    _*May be changed through the Web UI or API_  
    <u>**NOTE:**</u> Some ESP32-C3 modules have the Status LED wired so that setting the status pin high is off or reversed. Uncomment this line to reverse the High/Low setting for the status LED pin.  
    ```//#define CDC_REVERSE_HIGH_LOW```  
    _*May be changed through the Web UI or API_
 4. Set up the WiFi configuration.  The CheapoDC may operate in either Access Point (AP) mode or Station (ST) mode. The CheapoDC defaults to AP mode when it cannot connect to an access point.  
 <u>**NOTE:**</u> Changing the WiFi configuration is **NOT** recommended. ST Mode settings can be changed in the Web UI while AP Mode settings will be overwritten if Web OTA Update is used.
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
          Uncomment to enable. If enabled the default value sets TX power to 8.5dbm. Any of the `wifi_power_t` values found [here](https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiGeneric.h#L51-L67) may be used. Extensive testing with WiFi TX Power settings has **NOT** been done.

5. The hostname for the CheapoDC may also be changed. Its recommended that this be done using the [Web UI](../README.md#wifi-configuration). This is really only needed if you have multiple CheapoDC controllers on the same network.  To change the hostname in the ***CDCdefines.h*** file:
    * Change the host name. ***DEFAULT: cheapodc***.  
      ```#define CDC_DEFAULT_HOSTNAME "cheapodc"```
6. As of firmware V2.2.0, CheapoDC supports a [Web Update](https://hcomet.github.io/CheapoDC/CheapoDCWebUpdate.html) capability. This is enabled by default but can be disabled by commenting out the following line:  
    ```#define CDC_ENABLE_HTTP_OTA```
7. Do not modify any of the ```#define``` values below the line  ```DO NOT change anything below here```. The Weather Source and OpenWeather API key should be set or changed through the [Web UI](../README.md#web-ui) not in the ***CDCdefines.h*** file.  
    <u>**IMPORTANT:**</u>
    * **DO NOT** modify the URLs for API or Icon calls.

## Build and Configure a New Device

### Build and Install the Firmware

1. After [configuring the Arduino IDE](#setting-up-your-build-environment), build and upload the firmware to your ESP32.  
   Make sure to have selected the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* partition scheme.  
   <u>**NOTE:**</u> If the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* partition scheme is not listed as one of the partition schemes available for your selected ESP32-C3 board type you will need to select a different board type. Often the AliExpress type boards only list a minimal set of partition schemes. If you do not see the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* in the list of available partition schemes for your board the best recommendation is to select a Dev Module board in the Arduino IDE. For the ESP32-C3 that would be the *ESP32C3 Dev Module*.

2. Optionally, modify the CDCWiFi.json file ([see below](#cdcwifijson)) in the sketch ***CheapoDC/data*** folder to configure WiFi for your network or modify the hostname.

3. Upload the files in the ***CheapoDC/data*** folder using the Sketch data uploading tool. Please READ the following points before uploading the data files.  
   * **LittleFS file system formatting MUST be used for the data upload.**
   * These files are needed for the Web UI as well as configuration of the CheapoDC.
  
### First Time Device Configuration

If you have not modified the default CDCWiFi.json then the CheapoDC will enter AP (Access Point) mode when booted the first time.

* Connect to the access point, SSID: cheapodc and Password: cheapodc.
* Browse to the CheapoDC Web UI at http://cheapodc.local to configure your device. When prompted login using the default userid and password: admin, admin. You should be on the [Dashboard](../README.md#cheapodc-dashboard) page.
* Go to the [Device Management](../README.md#cheapodc-device-management) page by clicking on the button at the bottom of the page.
* Scroll down to [WiFi Configuration](../README.md#wifi-configuration) and configure your WiFi SSID and Password. You'll need to [Reboot](../README.md#reboot-device) the CheapoDC for the WiFi changes to take effect.
* After rebooting. The CheapoDC should be connected to your WiFi network. Refresh your browser and stay on the Device Management page.
* Configure the [Controller Outputs](../README.md#controller-output-configuration) if needed. Output 0 and 1 are always dew controller outputs and by default mapped to GPIO pins 0 and 1. Outputs 2 to 5 may be configured as Controller, PWM or Boolean. They are Disabled by default. Outputs **must** be mapped to a GPIO pin first before an [Output Mode](../README.md#output-modes) may be set.  
  For the ESP32-C3 SuperMini GPIO pins 0, 1, 3, 5, 6 and 7 have been validated.
* [Change your password](../README.md#change-password).
* [Change the Status LED](../README.md#status-led-configuration) GPIO pin and High Value setting if needed.
* Go to the [Controller Configuration](../README.md#cheapodc-controller-configuration) page to set up the parameters for the dew controller, weather query and your location. Dew controller parameters are explained in the [Dew Control Algorithm](../README.md#how-the-dew-control-algorithm-works) section.

Your CheapoDC should now be ready to use.

## Upgrading from a previous release

Note that v2.1.0 required a change in the [Partition Scheme](#partition-scheme) and upgrades from a pre-V2.1.0 release must be done using a USB connection to the device.

New versions of ESPAsyncWebServer and AsyncTCP libraries are also required for Arduino core of ESP32 version 3.1.x. Please read the [Setting Up Your Build Environment](#setting-up-your-build-environment) section to make sure you have the correct libraries installed in the Arduino IDE. Be sure to have only one version of the AsyncTCP library installed.

### Upgrade using USB Connected Device

1. Preserve your current CheapoDC configuration and WiFi settings by first downloading the CDCConfig.json and CDCWiFi.json files
from your CheapoDC. This can be done using the Web UI from the [File Management](../README.md#cheapodc-file-management) page.
Place both files in the sketch data folder: ***CheapoDC/data***.
2. Build and upload the new firmware to your device.
   * If upgrading from a pre-V2.1.0 release, make sure to set *Erase All Flash Before Sketch Upload:* ***"Enabled"***  
   <u>**NOTE:**</u> Make sure to reset this to **"Disabled"** once you are done with the upgrade or the LittleFS partition will be erased with each
   firmware upload and you will need to upload your sketch data every time.
   * Make sure to have selected the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* partition scheme.
3. Upload the files in the CheapoDC/data folder using the Sketch data uploading tool. Make sure to use an upload tool for the LittlFS filesystem.
4. Your upgrade should now be complete.

### OTA Upgrade using the Web UI Firmware Update

If upgrading from a release prior to v2.1.0, an OTA firmware upgrade cannot be used. Go back to the [USB connected upgrade](#upgrade-using-usb-connected-device).

If upgrading from V2.1.0 or newer then use **Manual Update** on the Web UI [Device Management](../README.md#cheapodc-device-management) page.

#### Web OTA Update

Introduced in the V2.2.0 release, [Web OTA Update](https://hcomet.github.io/CheapoDC/CheapoDCWebUpdate.html) which makes upgrades simple and easy.

#### Manual OTA Update

If you have firmware without Web OTA Update support or you prefer to do your own builds and updates then:

* Generate the firmware binary image using the Arduino IDE and selecting `Sketch->Export Compiled Binary`. This will create a ***CheapoDC.ino.bin*** file under a ***build*** folder created in your sketch folder. 
* Use the **Manual OTA Update** `Choose File` in the Web UI to select the new bin file. Then click `Update` to flash the CheapoDC to the new version.
* After the reboot, go to the [File Managemet](../README.md#cheapodc-file-management) page to upload the new release data files. Use `Choose Files` to browse to the ***CheapoDC/data*** folder in your sketch folder. Then select all files **EXCEPT** the CDCWiFi.json file. Then click `Upload`.
* Your upgrade should now be complete.

## Configuration Files

All of the configuration information for the CheapoDC is stored in two files save to the data partition on the device.

### CDCWiFi.json

This file provides a list of WiFi access points that will be tried by the CheapoDC to establish a network connection. The Hostname and WiFi access point information may be modified using the [Web UI](../README.md#wifi-configuration). However, This file may be edited and uploaded using the [File Management](../README.md#cheapodc-file-management) page.

The file content is json formatted using "name":"value" pairs.

`{"hostname":"cheapodc","connectAttempts":"10","tryAPs":"1","wifi":[{"ssid":"FakeSSID1","password":"Password1"},{"ssid":"FakeSSID2","password":"Password2"}]}`

The example shows settings for two access points in the "wifi" section. This can be 1 or several access points. CheapoDC will step through the list sequentially until a successful WiFi connection is established in Station Mode.

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

If CheapoDC fails to connect to an access point in Station Mode then it will switch to Access Point mode. The default SSID and Password for AP mode are bot *CheapoDC* by default. This may be changed in the [***CDCdefines.h***](#configure-firmware-in-the-cdcdefinesh-file) file.

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






