# Installing CheapoDC

## WebFlash, the Easy Option

If you are using an [ESP32-C3 SuperMini](https://michiel.vanderwulp.be/domotica/Modules/ESP32-C3-SuperMini/) or any other ESP32-C3 module then the easiest way to load the CheapoDC firmware onto your device is to use [WebFlash](https://hcomet.github.io/CheapoDC/CheapoDCFlash.html). All other configuration can then be done through the [Web UI](../README.md#web-ui). WebFlash is primarily for new installs but you can also use it to upgrade your CheapoDC to the latest firmware. Details in the [Upgrading from a Previous Release](#2-upgrade-using-usb-connected-device-and-webflash) section.

You need to connect your ESP32-C3 to your computer via USB to use WebFlash. Go to the [WebFlash page](https://hcomet.github.io/CheapoDC/CheapoDCFlash.html) to install CheapoDC on your ESP32-C3. Then follow the instructions bellow in the [First Time Device Configuration](#first-time-device-configuration) section.



If you prefer to build the firmware yourself using the Arduino IDE please continue on to [Building and Installing CheapoDC](#building-and-installing-cheapodc)

>[NOTE!]
>CheapoDC V2.2.0 added Web UI support for a semi-automatic web-based upgrade that preserves your configuration files. See the documentation on [Web OTA Update](https://hcomet.github.io/CheapoDC/CheapoDCWebUpdate.html) for details.

## Changes in CheapoDC V2

1. As of V2.2.0, CheapDC now supports firmware and data partition upgrades using the Web UI and HTTP OTA. See the documentation on [Web OTA Update](https://hcomet.github.io/CheapoDC/CheapoDCWebUpdate.html) for details.  
> [!IMPORTANT]
> After the release of V2.3.0 (Nov. 2025) GitHub Pages changed the root CA for HTTPS causing Web OTA Update to stop working. V2.3.1 (June 2026) fixes Web OTA Update by using a new version of esp32FOTA, 0.3.0, which supports using the Mozilla trusted CA list as a CA bundle.
2. V2 contains changes to support new library versions:
   * Arduino core for ESP32 version 3.1.x support. Core Version 2.x support now deprecated as of CheapoDC V2.1.0.
   * Switch to the [ESP32 Asynchronous Networking Organization](https://github.com/ESP32Async) versions of
   ESPAsyncWebServer and AsyncTCP.
   * ArduinoJson version 7. Version 6 should also continue to work but migration to version 7 is encouraged.
3. New libraries required in V2:
   * As of V2.3.0, [Sensirion Core by Sensirion](https://github.com/Sensirion/arduino-core/) and [Sensirion I²C SHT3X by Sensirion](https://github.com/Sensirion/arduino-i2c-sht3x) are required for humidity sensor support.
   * As of V2.3.1, [ESP32CertBundle](https://github.com/tanakamasayuki/ESP32CertBundle) is required to support [Web OTA Update](https://hcomet.github.io/CheapoDC/CheapoDCWebUpdate.html).
4. The following libraries are no longer required:
   * EasyLogger - a fixed copy of this library is included in the CheapoDC source to remove a macro definition conflict with ArduinoJSON.
   * ESP32Time.
5. As of V2.2.0, all major items that were previously required to be configured at compile time can now be configured through the Web UI or API at runtime. This includes:
   * [WiFi Configuration](../README.md#wifi-configuration): WiFi SSID and password can be set on the Device Management page.
   * Weather Source: The Weather Source may now be set through the Web UI or API. Open-Meteo is now the default Weather Source.
   * [Output to GPIO Pin mapping](../README.md#controller-output-configuration). All controller outputs may be configured using the Web UI or API. This includes the original two core dew controller outputs as well as the four new additional outputs added in V2.2.0.
   * [Status LED GPIO Pin](../README.md#status-led-configuration). The pin used for the status LED as well as whether or not it is active High or Low may be configured through the Web UI or API.
   * [Change Password](../README.md#change-password). The user Id remains **admin** but the password may now be changed in the Web UI. Password security has been improved by moving to using a [Digest access authentication](https://en.wikipedia.org/wiki/Digest_access_authentication#:~:text=In%20contrast%2C%20basic%20access%20authentication,It%20uses%20the%20HTTP%20protocol.) MD5 Password Hash.
6. New Weather Sources added for more flexibility and autonomous operation:
   * V2.2.0 added support for integration with personal weather stations or other weather services. When the Weather Source is set to ***External Source*** the local ambient temperature and humidity may be set through the API. The Dew Point and dew controller power output will then be calculated based on these externally set values.
   * V2.3.0 added support for connecting an SHT3x humidity/temperature sensor. When the Weather Source is set to ***Internal Source***, local ambient temperature and humidty will be taken from a temperature/humidity sensor connected to the CheapoDC. The SHT3x must be properly [connected and configured](https://hcomet.github.io/CheapoDC/CheapoDCSensor.html) for ***Internal Source*** to work.
7. API changes:
   * ***`CPPx`***, ***`CPMx`*** and ***`CPOx`*** API commands added to allow outputs **x = 0 to 5** to be configured for GPIO Pin mapping, Output Mode and Output Power respectively.
   * The ***`LED`*** API command changed to set the GPIO pin for the Status LED while ***`LEDH`*** sets the value of High to 1 or 0.
   * The Weather Source (***`WS`***) API command now supports a SET capability. This API command has also been changed
   to use an integer value to specify the Weather Source:  
   `[Open-Meteo = 0, OpenWeather = 1, External Source = 2, Internal Source = 3]`  
   External Source added in V2.2.0. Internal Source added in V2.3.0.
   * The Ambient Temperature from Weather Query (***`ATPQ`***) API command now supports a SET capability when the Weather Source is set to External Source.
   * The Humidity (***`HU`***) API command now supports a SET capability when the Weather Source is set to External Source.
   * ***`SDAP`*** and ***`SCLP`*** API commands added in V2.3.0 to allow for configuration of an internal humidity/temperature sensor. ***`SDAP`*** sets or gets the I2C SDA (data) pin. ***`SCLP`*** sets or gets the I2C SCL (clock) pin. Pins 10 and 9 are the recommended pins for SDA and SCL, respectively. A value of -1 (default) will disable the sensor. A reboot of the device is required for I2C changes to take effect.
   * The Weather API URL (***`WAPI`***) API command no longer supports a SET capability.
   * The Weather Icon URL (***`WIURL`***) API command no longer supports a SET capability.
   * The Cloud Coverage (***`CLC`***) API command has been deprecated.
8. Support for the Web Sockets based API has been deprecated and removed in V2.2.0. The TCP API continues to be supported and used by the INDI driver.
9. CheapoDC V2 will automatically upgrade the CDCConfig.json on your CheapoDC device when the new firmware version is installed. Previous configuration settings will be maintained. Follow the [Upgrade Steps](#upgrading-from-a-previous-release) below to preserve your
CheapoDC configuration files.
10. Change in ESP32 Partition Scheme required for CheapoDC V2.1.x. See [Partition Scheme](#partition-scheme) for details.

11. As of V2.3.0, when in Access Point Mode, CheapoDC will use the hostname for the SSID.
12. See the CheapoDC [release notes](https://github.com/hcomet/CheapoDC/releases) for additional details.

## Building and Installing CheapoDC

### Partition Scheme

The ESP32 Partition Scheme is a term used by the Arduino IDE to select a partition table that defines the flash memory organization and the different kinds of data that will be stored in each partition. The ESP32-C3, which this project uses, generally has 4MB of flash memory with the default partition scheme of *Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)*.

The *Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)* partition scheme provides two application partitions of 1.2MB to support OTA updates plus a 1.5MB SPIFFS partition used for LittleFS file storage. The latest version of CheapoDC using the latest Arduino library versions requires about 1.6MB of application space.

To allow for OTA updates, LittleFS storage and some expansion room for CheapoDC firmware the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* partition scheme is used. This scheme provides too little space for the CheapoDC HTML, CSS and JS files without compression. The repository [**data source**](../data%20source/) folder contains he original source for these files. The repository [**CheapoDC/data**](../CheapoDC/data/) folder contains minified or compressed versions for installation in the LittleFS formatted data partition.

### Setting Up Your Build Environment

1. Install the Arduino IDE.  
  **NOTE:** Version 2.3.8 of the Arduino IDE is required. As of CheapoDC V2.3.1 verification of
  builds will only be done on IDE version 2.3.8 or newer.
2. Add support for ESP32 modules using the Boards Manager in the Arduino IDE.  
  **IMPORTANT:**
    * Install the boards plugin for ESP32 from Espressif Systems version 3.3.8 (Arduino core for the ESP32). CheapoDC V2.3.1 verification has been done with version 3.3.8.
    * CheapoDC is intended for use on ESP32 devices. The current version has been verified on an [ESP32-C3 SuperMini](https://michiel.vanderwulp.be/domotica/Modules/ESP32-C3-SuperMini/) board..
3. Install the ESP32 Sketch data uploader with support for LittleFS. The CheapoDC uses the LittleFS file system for configuration files and web pages uploaded from the ***CheapoDC/data*** folder. If the data is uploaded using any other file system format, such as SPIFFS, the CheapoDC firmware will not run properly.  
**NOTE:** The following link provides information on how to install a data uploader plugin with LittleFS support:
    * Arduino IDE 2.3.2+: [https://randomnerdtutorials.com/arduino-ide-2-install-esp32-littlefs/](https://randomnerdtutorials.com/arduino-ide-2-install-esp32-littlefs/)

4. Install the following libraries if not already installed:
   * [ArduinoJson by Benoit Blanchon](https://arduinojson.org/)  
     Version: 7.4.3
   * [ESP Async WebServer by ESP32Async](https://github.com/ESP32Async/ESPAsyncWebServer)  
     Version: 3.11.0
   * [Async TCP by ESP32Async](https://github.com/ESP32Async/AsyncTCP)  
     Version: 3.4.10
   * [Sensirion I²C SHT3X by Sensirion](https://github.com/Sensirion/arduino-i2c-sht3x)  
     Version 1.0.1
   * [Sensirion Core by Sensirion](https://github.com/Sensirion/arduino-core/)  
     Version 0.7.3
   * [Time by Michael Margolis](https://playground.arduino.cc/Code/Time/)  
     Version: 1.6.1
   * [ESP32CertBundle](https://github.com/tanakamasayuki/ESP32CertBundle)  
     Version: 20260514.0.0

   **NOTES:**
   * Make sure that the only versions of **ESP Async WebServer** and **Async TCP** are the ones by ESP32Async. If you have other versions of the ESPSyncWebServer or AsyncTCP not from the links above they will need to be uninstalled to prevent issues with the build.
   * In general, if newer versions of a library are available then only use new subversions not new major versions. CheapoDC v2.3.1 was verified using the versions of libraries listed above.

5. Download the latest firmware release source code from <https://github.com/hcomet/CheapoDC/releases>  
  **NOTE:** After extracting the release to your file system open the CheapoDC.ino file in the Arduino IDE. This will open the full set of source files in the IDE. Now configure the firmware before building it.

>[!TIP]
>* Many ESP32-C3 board configurations (Dev Module, mini, super mini, micro etc..) have *USB CDC On BOOT:* ***"Disabled"***. This needs to be ***"ENABLED"*** or the Serial Monitor will not work.
>* CheapoDC uses *115200 baud* for the Serial Monitor Connection. Leave the default baud rate for the upload.
>* Its good practice to set *Erase All Flash Before Sketch Upload:* ***"Enabled"*** the first time you upload a new project to an ESP32. After that, make sure its set to ***"Disabled"*** to preserve the LittleFS partition content between uploads.

### Configure Firmware in the CDCdefines.h file

As of V2.2.0 changes to the ***CDCdefines.h*** file are **not** required since all key items can now be modified at runtime, via the [WebUI](../README.md#web-ui), and are saved to the [CDCConfig.json](#cdcconfigjson). If you do wish to to configure/customize the firmware before building the following items may be modified in the ***CDCdefines.h*** file:

1. Set the default ESP32 GPIO pins to be used for Controller Outputs 0 & 1. ***DEFAULT: 0 & 1***.  
    ```#define CDC_DEFAULT_CONTROLLER_PIN0 0```  
    ```#define CDC_DEFAULT_CONTROLLER_PIN1 1```  
    *May be changed through the Web UI or API*
2. Set the PWM channel to use for Controller Outputs. ***DEFAULT: 0***.  
    ```#define CDC_CONTROLLER_PWM_CHANNEL 0```  
  **NOTE:** Changing this value may have a negative impact on CheapoDC operation. PWM channels 1 through 4 are reserved for the use of the additional Controller Outputs 2 through 5.
3. Set the pin for the status LED on you ESP32. ***DEFAULT: 8***.  
    ```#define CDC_DEFAULT_STATUS_LED_PIN 8```  
    *May be changed through the Web UI or API*  
    **NOTE:** Some ESP32-C3 modules have the Status LED wired so that setting the status pin high is off or reversed. Uncomment this line to reverse the High/Low setting for the status LED pin.  
    ```//#define CDC_REVERSE_HIGH_LOW```  
    *May be changed through the Web UI or API*
4. Set up the WiFi configuration.  The CheapoDC may operate in either Access Point (AP) mode or Station (ST) mode. The CheapoDC defaults to AP mode when it cannot connect to an access point.  The AP mode SSID will be set to be the same as the hostname which defaults to ***cheapodc***.  
**NOTE:** Changing the WiFi configuration is **NOT** recommended. ST Mode settings can be changed in the Web UI while AP Mode settings will be overwritten if Web OTA Update is used.
    * AP Mode settings must be configured in the ***CDCdefines.h*** file:  
      * Change AP mode password. ***DEFAULT: cheapodc***.  
        ```#define CDC_DEFAULT_WIFI_AP_PASSWORD "cheapodc"```  
    * ST Mode settings may be configured in either the ***CDCdefines.h*** file or the ***CDCWiFi.json*** file found in the ***data*** folder. Values in found in the ***CDCWiFi.json*** files will be used before using the values in the ***CDCdefines.h*** file. Using the ***CDCWiFi.json*** file to configure WiFi access allows multiple APs and credentials to be defined. How to configure the ***CDCWiFi.json*** file is found [here](#cdcwifijson) while configuring default values in the ***CDCdefines.h*** file is as follows:
      * Change ST mode SSID. ***DEFAULT: defaultSSID***.  
        ```#define CDC_DEFAULT_WIFI_SSID "defaultSSID"```  
      * Change ST mode password. ***DEFAULT: defaultPassword***.  
        ```#define CDC_DEFAULT_WIFI_PASSWORD "defaultPassword"```
    * WiFi TX Power setting may be optionally modified if you are having issues with an ESP32-C3 board connecting to your WiFi gateway. Set the WiFi TX Power:
      * Enable setting WiFi Tx Power. ***DEFAULT: Disabled***.  
        ```//#define CDC_ENABLE_WIFI_TX_POWER_MOD WIFI_POWER_8_5dBm```  
        Uncomment to enable. If enabled the default value sets TX power to 8.5dbm. Any of the `wifi_power_t` values found [here](https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiGeneric.h#L51-L67) may be used. Extensive testing with WiFi TX Power settings has **NOT** been done.

5. The hostname for the CheapoDC may also be changed. Its recommended that this be done using the [Web UI](../README.md#wifi-configuration). This is really only needed if you have multiple CheapoDC controllers on the same network.  To change the hostname in the ***CDCdefines.h*** file:  
<br>**NOTE:** When in access point (AP) mode the SSID for the AP will be the same as the hostname.
    * Change the host name. ***DEFAULT: cheapodc***.  
      ```#define CDC_DEFAULT_HOSTNAME "cheapodc"```  
      *Recommended to be modified only through the Web UI or API*
6. As of firmware V2.2.0, CheapoDC supports a [Web Update](https://hcomet.github.io/CheapoDC/CheapoDCWebUpdate.html) capability. This is enabled by default but can be disabled by commenting out the following line:  
    ```#define CDC_ENABLE_HTTP_OTA```
7. Do not modify any of the ```#define``` values below the line  ```DO NOT change anything below here```. The Weather Source and OpenWeather API key should be set or changed through the [Web UI](../README.md#web-ui) not in the ***CDCdefines.h*** file.  
<br>**IMPORTANT:** **DO NOT** modify the URLs for API or Icon calls.

### Build and Configure a New Device

#### Build and Install the Firmware

1. After [configuring the Arduino IDE](#setting-up-your-build-environment), build and upload the firmware to your ESP32.  
   Make sure to have selected the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* partition scheme.  
   **NOTE:** If the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* partition scheme is not listed as one of the partition schemes available for your selected ESP32-C3 board type you will need to select a different board type. Often the AliExpress type boards only list a minimal set of partition schemes. If you do not see the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* in the list of available partition schemes for your board the best recommendation is to select a Dev Module board in the Arduino IDE. For the ESP32-C3 that would be the *ESP32C3 Dev Module*.

2. Optionally, modify the CDCWiFi.json file ([see below](#cdcwifijson)) in the sketch ***CheapoDC/data*** folder to configure WiFi for your network or modify the hostname.

3. Upload the files in the ***CheapoDC/data*** folder using the Sketch data uploading tool. Please READ the following points before uploading the data files.  
   * **LittleFS file system formatting MUST be used for the data upload.**
   * These files are needed for the Web UI as well as configuration of the CheapoDC.
  
#### First Time Device Configuration

If you have not modified the default CDCWiFi.json then the CheapoDC will enter AP (Access Point) mode when booted the first time.

* Connect to the access point, SSID: cheapodc and Password: cheapodc.
* Browse to the CheapoDC Web UI at [http://cheapodc.local](http://cheapodc.local) to configure your device. When prompted login using the default userid and password: admin, admin. You should be on the [Dashboard](../README.md#cheapodc-dashboard) page.
* Go to the [Device Management](../README.md#cheapodc-device-management) page by clicking on the button at the bottom of the page.
* Scroll down to [WiFi Configuration](../README.md#wifi-configuration) and configure your WiFi SSID and Password. You'll need to [Reboot](../README.md#reboot-device) the CheapoDC for the WiFi changes to take effect.
* After rebooting. The CheapoDC should be connected to your WiFi network. Refresh your browser and stay on the Device Management page.
* Configure the [Controller Outputs](../README.md#controller-output-configuration) if needed. Output 0 and 1 are always dew controller outputs and by default mapped to GPIO pins 0 and 1. Outputs 2 to 5 may be configured as Controller, PWM or Boolean. They are Disabled by default. Outputs **must** be mapped to a GPIO pin first before an [Output Mode](../README.md#output-modes) may be set.  
  For the ESP32-C3 SuperMini GPIO pins 0, 1, 3, 4, 5 and 7 have been validated.
* [Change your password](../README.md#change-password).
* Set up the Humidity Sensor if you have [added one to your device](../README.md#humidity-sensor-configuration). A [Reboot](../README.md#reboot-device) of the device will be required for SDA and SCL pin changes to take effect.  
  For the ESP32-C3 SuperMini GPIO pins 9 (SCL) anf 10 (SDA) have been validated.
* [Change the Status LED](../README.md#status-led-configuration) GPIO pin and High Value setting if needed. GPIO pin 8 is the default while setting it to -1 will disable the LED.
* Go to the [Controller Configuration](../README.md#cheapodc-controller-configuration) page to set up the parameters for the dew controller, weather query and your location. Dew controller parameters are explained in the [Dew Control Algorithm](../README.md#how-the-dew-control-algorithm-works) section.

Your CheapoDC should now be ready to use.

### Upgrading from a previous release

There are four methods described below for upgrading CheapoDC firmware. Two methods require a computer and USB connection to the device while the other two leverage Over The Air (OTA) update methods.

**Note:** Upgrade method [2. Upgrade using USB Connected Device and WebFlash](#2-upgrade-using-usb-connected-device-and-webflash) must be used if your device does not use the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* partition scheme.

#### 1. Upgrade using USB Connected Device and Arduino IDE

1. Preserve your current CheapoDC configuration and WiFi settings by first downloading the CDCConfig.json and CDCWiFi.json files
from your CheapoDC. This can be done using the Web UI from the [File Management](../README.md#cheapodc-file-management) page.
Place both files in the sketch data folder: ***CheapoDC/data***.
2. Build and upload the new firmware to your device.
   * If upgrading from a pre-V2.1.0 release, make sure to set *Erase All Flash Before Sketch Upload:* ***"Enabled"***  
   **NOTE:** Make sure to reset this to **"Disabled"** once you are done with the upgrade or the LittleFS partition will be erased with each
   firmware upload and you will need to upload your sketch data every time.
   * Make sure to have selected the *Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)* partition scheme.
3. Upload the files in the CheapoDC/data folder using the Sketch data uploading tool. Make sure to use an upload tool for the LittleFS filesystem.
4. Your upgrade should now be complete.

#### 2. Upgrade using USB Connected Device and WebFlash

1. WebFlash will erase all flash memory on your CheapoDC. You must backup your current CheapoDC configuration and WiFi settings by first downloading the **CDCConfig.json** and **CDCWiFi.json** files
from your CheapoDC. This can be done using the Web UI from the [File Management](../README.md#cheapodc-file-management) page.
2. Use [WebFlash](https://hcomet.github.io/CheapoDC/CheapoDCFlash.html) to install the latest CheapoDC firmware onto your device.
3. Once the device has rebooted, it will be in access point mode. Use your computer to connect to the access point, SSID: cheapodc and Password: cheapodc.
4. Browse to the CheapoDC Web UI at http://cheapodc.local and login using the default userid and password: admin, admin.
5. Upload the previously backed up **CDCConfig.json** and **CDCWifi.json** files using the File Management page.
5. Goto to the Device Management page and reboot the device to complete the upgrade.

#### 3. Web OTA Update

Introduced in the V2.2.0 release, the aim of [Web OTA Update](https://hcomet.github.io/CheapoDC/CheapoDCWebUpdate.html) is to make upgrades simple and easy. 
When a new release is available it will be indicated in the **Web OTA Update**
section on the **Device Management** of the WebUI.

>[!IMPORTANT]
> Due to a change to the issuing CA for GitHub Pages HTTPS certificates, Web OTA Update from V2.3.0 or earlier no longer works. One of the other upgrade methods must be used to update to V2.3.1 or newer. V2.3.1 fixes the issue and Web OTA Update should work properly from that version onwards.

#### 4. Manual OTA Update

Manual OTA Update allows you to do an OTA update using your own copy of the CheapoDC firmware binary image. Use the following steps:

1. Download the latest firmware release source code from <https://github.com/hcomet/CheapoDC/releases> and extract it to your local file system.
2. Get or generate the firmware binary image file by either:

   1. Downloading the latest CheapoDC binary image file from [https://hcomet.github.io/CheapoDC/firmware/CheapoDC.ESP32-C3.bin](https://hcomet.github.io/CheapoDC/firmware/CheapoDC.ESP32-C3.bin).
   2. Or, generating the firmware binary image using the Arduino IDE and selecting `Sketch->Export Compiled Binary`. This will create a ***CheapoDC.ino.bin*** file under a ***build*** folder created in your sketch folder.  
   **Note:** Follow the instructions in [Build and Install the Firmware](#build-and-install-the-firmware) to set up the Arduino IDE for CheapoDC builds.

3. Use the **Manual OTA Update** `Choose File` in the Web UI to select the ***.bin*** file from step 2. Then click `Update` to flash the CheapoDC to the new version.
4. After the reboot, go to the [File Management](../README.md#cheapodc-file-management) page to upload the new release data files. Use `Choose Files` to browse to the ***CheapoDC/CheapoDC/data*** folder in the downloaded release from step 1. Then select all files **EXCEPT** the CDCWiFi.json file. Then click `Upload`.
5. Your upgrade should now be complete.

### Configuration Files

All of the configuration information for the CheapoDC is stored in two files save to the data partition on the device.

#### CDCWiFi.json

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
**Note:** Changing the hostname will also change the SSID for the device when in Access Point mode.
4. Changing "connectAttempts" will change then number of times the CheapoDC will attempt to connect to an access point before moving to the next access point in the list.  
```"connectAttempts":"10"```
5. Changing "tryAPs" will change the number of times the CheapoDC will loop through the list of access points before switching to Access Point (AP) mode.  
```"tryAPs":"1"```

If CheapoDC fails to connect to an access point in Station Mode then it will switch to Access Point (AP) mode. The AP SSID will be the same as the hostname with a default of *cheapodc*. The AP password is always *cheapodc* and can only be changed in the  [***CDCdefines.h***](#configure-firmware-in-the-cdcdefinesh-file) file.

If no ***CDCWiFi.json*** file is found then the values for WiFi configuration will be taken from the [***CDCdefines.h***](#configure-firmware-in-the-cdcdefinesh-file) file.

#### CDCConfig.json

This file is used to save the dew controller configuration between power cycles. If the file is not present then the values defined in the ***CDCdefines.h*** file (the firmware defaults) will be used. The CheapoDC checks for changes to the controller's configuration every 10 seconds and if a change is detected then the full ***CDCdefines.h*** file will be written to the LittleFS file system on the controller.

Controller configuration may be modified via the [Web UI](../README.md/#web-ui) or the [CheapoDC API](../README.md/#cheapodc-api). The [CheapoDC commands](../README.md/#cheapodc-commands) which support a ***Setter*** function also represent the values stored in the ***CDCConfig.json*** file.