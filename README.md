![CheapoDC Logo](images/logo.png)

The Cheapo Dew Controller, or CheapoDC, is a low cost, easy to build DIY dew controller based on an ESP32-C3 mini. Parts required include the ESP32-C3 mini, one or two MOSFET modules, a 12V to 5V buck converter, some proto board, a couple of RCA sockets, a 12V barrel socket and wire. Cost of the parts should be less than $20 for a unit that controls 2 dew heater straps. More harware details can be found in the [Hardware](/README.md#hardware) section.

A primary goal was to keep the build simple with minimal parts. This is done by leveraging the ESP32 WiFi capability to query one of the open weather service APIs. Either the [OpenWeather](https://openweathermap.org/) API or the [Open-Meteo](https://open-meteo.com/) API may be used for ambient temperature, humidity and dew point. This is then used for calculating CheapoDC's power output. No additional components are requied suchc as temperature or humidity probes. The responsiveness or aggressiveness of the controller can be adjusted through several settings. CheapoDC works best with Internet connectivity supporting the weather queries to OpenWeather but it can also be used in a limited way without Internet access. 


# How the Controller Works

## Controller Power Output Calculation
### Variables:
* **Set Point** = ***SP***
  - as set using the **Set Point** selection.
* **Reference Temperature** = ***RT***
  - as set using the **Temperature Mode** selection.
* **Track Point** = ***TP***
  - the temperature point where the **Tracking Range** starts. A **Reference Temperature** less than or equal 
to the **Track Point** will cause the controller to use **Maximum Output**. 
* **Track Point Offset** = ***TPO***
  - an offset applied to the **Set Point** when determining the **Track Point** relative to the **Set Point**. 
The **Track Point Offset** may be set from -5.0 to 5.0 degrees Celsius. The default is 0.0 degrees Celsius.
* **Tracking Range** = ***TR***
  - the temperature range starting at the **Track Point** where the controller output ramps up from **Minimum Output** at the high end of the range to **Maximum Output** at the low end of the range. The range may be set to values from 
4.0 to 10.0 degrees Celsius. The default is 4.0 degrees celsius.
* **Power Output** = ***PO***
  - the percentage of power the controller is outputting to the dew straps. It varies from the **Minimum Output** setting to the **Maximum Output** setting.
* **Minimum Output** = ***MinO***
  - the minimum percent power setting for the controller to output when not in the **Off** **Controller Mode**. Defaults to 0%.
* **Maximum Output** = ***MaxO***
  - the maximum percent power setting for the controller to output when at full output. Defaults to 100%.

### Calculations
$`TP = SP + TPO`$

$`IF`$ $`(RT <= TP )`$ $`THEN`$ $`PO= MaxO`$

$`IF`$ $`(RT >= (TP + TR))`$ $`THEN`$ $`PO = MinO`$

$`IF`$ $`(RT <(TP + TR))`$ $`THEN`$ $`PO = MinO + (MaxO - MinO) * ( 1 - (RT - TP)/(TR))`$

### Examples

1. Exampe 1 is primarily a reference image to illustrate the variables defined above.
   * Power Output curve is shown relative to the Track Point and the Tracking Range. While the Reference Temperature is greater than the high end of the Tracking Range the Power Output is set to Minimum Output. The Power Output ramps up linearly through the Tracking Range from the Minimum Output to the Maximum Output as the Reference Temperature drops.
   * This example shows a Reference Temperature, RT = 8°C, which is greater than the upper end of the Tracking Range, causing Power Output to be set to Minimum Output. In this case Minimum Output is set to 10%.
   * The upper end of the Tracking Range, at 7°C, is determined from the Set Point (SP = -2°C) plus the Track Point Offset (TPO = 4°C), creating a Tack Point, TP = 2°C, plus the Tracking Range (TR = 5°C).
 
![Example 1](images/example1.jpg)

2. Example 2 shows changing the values of the controller variables can affect the Power Output calculation.
   * The Set Point, SP = -2°C, and the Reference Temperature, RT = 8°C, the same as in Example 1. However changing the Track Point Offset, now TPO = 2°C, and the Tracking Range, now TR = 10°C, has change the Power Output as well as flattening  the Power Output curve.
   * 

![Example 1](images/example2.jpg)

## Dew Controller Settings
The following modes of operation are used in the algorithm that calculates controller output. The first item under each 
mode is the default.

### Controller Mode:
The Controller Mode selects the overall operating mode of the dew controller.
* #### Automatic  
  * Uses the **Set Point Mode** and **Temperature Mode** settings to calculate the controller output based on the Power Output calculations above. In Automatic mode the Power Output calculation will be run periodically based on the Update Output Every setting. This may be set to a value from 1 to 20 minutes. The default is 1 minute. Automatic mode may be used with or without internet access.
* #### Manual
  * Controller output is manually controlled by setting the **Dew Controller Output** either through the Web UI or the API. When in Manual mode the the periodic Power Output calculation is suspended. Using Manual control is an option if internet access for OpenWeather API queries is not available.
* #### Off
  * Controller output is set to Zero. 

### Set Point Mode:
The Set Point Mode selects what will be used as the SetPoint for calculating Power Output.
* #### Dew Point
  * Uses the **Dew Point** as the Set Point used for calculating output. This is the default selection.
* #### Temperature
  * Uses a **Temperature Set Point** value input via the Web UI or API as the Set Point for calculating output. If the CheapoDC is being used without internet access then this mode allows a Set Point to be defined when a the DeW Point cannot be determined through the OpenWeather API.
* #### Midpoint
  * Uses the midpoint between the current **Temperature Mode** temperature value and the Dew Point value as the Set Point for calculating output. The impact of selecting this mode is to effectively double the Tracking Range and further flatten the ramp of the Power Output curve.

### Temperature Mode:
The Temperature Mode selects how the Reference Temperature will be determined for calculating the Power Output.
* #### Weather Query
  * Uses the Ambient Temperature returned by the OpenWeather API weather query as the Reference Temperature for calculating the controller output. This requires that the CheapoDC have internet access.
* #### External Input
  * Use the **External Input** temperature set through the Web UI or API as the reference temperature for calculating the controller output. This may be the preferred mode when using the controller with KStars/Indi. The CheapoDC Indilib driver can use a temperature probe attached to a focuser as the external input. This is also the mode that should be used when the CheapoDC does not have access to the internat and the OpenWeather API.

# Hardware
One of the goals with the CheapoDC is to for it to be relatively easy to assemble. Although some soldering will be required the number of connections is minimized by using common modules.

Component list:
* ESP32-C3 SuperMini (Other ESP32 modules should work but this one is very small and low priced) Example: [https://www.aliexpress.com/item/1005005967641936.html?spm=a2g0o.order_list.order_list_main.10.3b2c1802dRy3Tw]
* Buck converter to take 12V to 5V. I used an [LM2596 Module](https://www.amazon.ca/dp/B08Q2YKJ6Q?psc=1&ref=ppx_yo2ov_dt_b_product_details). You could also use an [MP1584EN Module](https://www.amazon.ca/eBoot-MP1584EN-Converter-Adjustable-Module/dp/B01MQGMOKI/ref=pd_sbs_d_sccl_2_3/141-9725081-7037101?pd_rd_w=UMd8F&content-id=amzn1.sym.ca022dba-8a59-468d-95a1-3216f611a75e&pf_rd_p=ca022dba-8a59-468d-95a1-3216f611a75e&pf_rd_r=4WWMQ6QG50JQ2BTYP273&pd_rd_wg=OCFjo&pd_rd_r=381f4abb-88a8-4856-a51f-39d3190099fa&pd_rd_i=B01MQGMOKI&th=1). These modules often have an adjustable output. You'll need to use an Volt meter to adjust the output to 5 volts before hooking it up.
* Two [dual-MOSFET Modules](https://www.amazon.ca/dp/B08ZNDG6RY?psc=1&ref=ppx_yo2ov_dt_b_product_details) to handle output to the dew straps while being able to be triggered by the 3.3V levels from the ESP32 PWM pins.
* A resettable fuse that can handle 5A. An example [5A PPTC](https://www.amazon.ca/10pcs-5000MA-Resettable-RGEF500-GF500/dp/B092T9Q3QR/ref=sr_1_4?crid=3KNZXWN5ZIERR&dib=eyJ2IjoiMSJ9.y5Pp17w_i-KzaprejYOYzM_8u_S5MY_jz1z932C2gBBmx5zcGFKHMHtP6qYXScM_-6ii9W8lDuEq5tbkCUQdYOFESDzjnASBHIusx7zAFOkhc6SNPrOH4O8ExB9WzAI-XgtIUvz-EvjfyOzjX4IN8iGl2GSffYGCb1BvIzldhIbrwCyyvNRyEfUCehiFknfJ5Uz1PSdPnC0BJjzSZp7Frh_EDLOF4CjpyeUQckj0FTQ347ehfh3jy3kHSu3I2iTOEaQZMRdqjpkW_NBOUMMsZsbeRdkMtzq0cIrGcsbUdhk.8jKyynsByh2dYlS0gLu9IdNbxiIm4iDIQv6g0ucFzCc&dib_tag=se&keywords=5a+pptc&qid=1709138849&sprefix=5a+pptc%2Caps%2C80&sr=8-4)
* Some assorted hardware:
  * 12VDC 5.5mm x 2.1mm socket. Common socket size used for Astronomy.
  * Switch that will work for 12VDC at 5A.
  * Two RCA sockets. RCA plugs are commonly used for dew straps.

### Basic Wiring Diagram:

![Wiring Diagram](images/wiring.jpg)
        

# Firmware
How to build the CheapoDC firmware can be found [here](/CheapoDC/README.md).

## Weather Service
CheapoDC can leverage one of two open weather service APIs to retrieve current temperature, humidity and dew point for your location. The service to use is specified in the CDCDefines.h file and only one service may be support at a time. The selected service is built into the firmware.
### [OpenWeather](https://openweathermap.org/)
The OpenWeather API is the default configuration. In order to use the OpenWeather API a registered account and API key is required. The account is free and allows for up to 60 querries/minute and 1,000,000 per month. Weather updates can occur from 5 to 20 minutes apart. Doing an API call every 5 minutes is more than adequate for dew control and even with 2 or 3 CheapoDC's sharing a key should have no issue at the free account level. Register and get your API key [here](https://home.openweathermap.org/users/sign_up).
### [Open-Meteo](https://open-meteo.com/)
Use of the Open-Meteo API does not require any registration for current weather queries. The free level allows for 10,000 API calls per day. Weather updates seem to be regular at a 15 minute interval. AT a 5 minute query interval several CHeapoDCs can be running at the same time without issue.
### Which to use?
Both services require a location using Latitude and Longitude which you can set using the CheapoDC [Web UI](/README.md#web-ui) or INDI Lib driver. OpenWeather provides the name of the weather station closest to your coordinates in its response. Open-Meteo does not. If run side-by-side with the same co-ordinates they provide slightly different results. You may want to check which service provides the best results for your location.

As indicated the defaults service is OpenWeather API. But if you do not want to share information with yet another internet data collector then configure Open-Meteo.
## Web UI
CheapoDC comes with a Web UI that supports basic web authentication. The ID and password for the web authentication is set in the CDCdefines.h file as part of the build configuration. Default is "admin" for both. TLS (or HTTPS) is not supported so the security is minimal. The intention is to priovde a deterent to someone on your network from easily doing a Web OTA upgrade to the firmware or uploading files to the LittleFS partition.

The Web UI has 4 main pages, a dashboard, a configuration page, a device management page and a file management page.
### CheapoDC Dashboard
![CheapoDC Dashboard](images/dashboard.jpg)
### CheapoDC Configuration
![CheapoDC Configuration](images/configuration.jpg)
CheapoDC can be configured to support Web Sockets for the [CheapoDC API](/README.md#cheapodc-api). By default Web Socket support is Disabled but if it is Enabled then the configuration page will use the [Web Sockets API](/README.md#web-sockets-api) to display and update configuration data. Otherwise, by default the configuration page will use the basic [Web API](/README.md#web-api) utilizing the HTTP POST method.
### CheapoDC Device Management
![CheapoDC Device Management](images/devicemgmt.jpg)
The Device Management page provides the ability to do Over-The-Air (OTA) firmware updates. It also allows for remote Reboot of the CHeapoDC.
### CheapoDC File Management
![CheapoDC Device Management](images/filemgmt.jpg)
The CheapoDC uses LittleFS for file storage on the ESP32. Although LittleFS supports directories as well as files CheapoDC uses a flat structure and all files are managed at the root. The file management page supports upload, download and delete functions.

## CheapoDC API

The CheapoDC provides API access to all configuration and data items available through the [Web UI](/README.md#web-ui). There is no authentication support in the API but the API also does not support firmware OTA updates or file management. These can only be done through the Web UI.

CheapoDC supports three API mechanisms:
1. TCP API using JSON syntax
2. Basic Web API utilizing HTTP POST
3. Web Sockets API using JSON syntax

The APIs use the same commands which are listed in the top of [CDCommands.cpp](/CheapoDC/CDCommands.cpp). Commands are 2 to 4 character strings. For each command there is a map indicating:
* the number of the command,
* whether or not the value associated with the command is to be saved or loaded using the CDCConfig.json file,
* units used for the value. Units are HTML or JSON formated and may be:
  * None: empty string.
  * Degrees Celsius: "\&deg;C"
  * Decimal Degrees: "\&deg;"
  * Percent: "\&percnt;"
  * Milliseconds: "msec"
  * Seconds: "sec"
  * Minutes: "min"
  * JSON ENUM, for enumerated values like Controller Mode.
    * ie: "{"Mode":["Automatic","Manual","Off"]}"

All commands support getter API methods but not all support a setter method. Using an invalid command or trying a setter on a command not supporing a setter will return an error.

### TCP API
The TCP API uses JSON formated commands over TCP on port 8000. JSON strings must always be terminated with a newline character "\n". "cmd" in the following formats is the 2 to 4 character command identitified in [CDCommands.cpp](/CheapoDC/CDCommands.cpp).

#### Setter format:
* Send: **{"SET":{"cmd":"value"}}**
* Response: **{"RESULT":"value"}**  
value = 0 for success  
value = -1 for failure

#### Getter format:
* Send: **{\"GET\":"cmd"}**
* Response: **{"cmd":"value"}**  
a failure will return a response of {"RESULT":"-1"}

### Web API
The Web API uses POST and "x-www-form-urlencoded" with name value pairs.
#### Setter Format:
* Send: **cmd=value**  
* Response: HTTP status code of 200 on success.

#### Getter Format:
* Send: **get=cmd**  
* Response: **cmd=value**  
success has HTTP status code of 200.

### Web Sockets API
The Web Sockets API uses the same JSON formatted Send/Response strings as the TCP API. Strings do not need to be terminated with a newline character.

## [INDI Driver](#indi-driver)
An INDI driver is in development and should be available shortly. The design of CheapoDC took the capabilities of [INDI Library](https://indilib.org/) into account and will support getting location information from the INDI geographic information as well as getting Temperature from a focuser with a temperature probe.
