// *************************************************************************************
// CheapoDC defines
// Cheap and easy Dew Controller
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// *************************************************************************************
// Before building configure the following in the defines below:
// 1 - Configure the PWM pins as appropriate for your ESP32
// 2 - Enable Basic Web Authentication and change the ID/Password
// 3 - Configure Network settings for WiFI (Can also be done through CDCWiFi,json file )
// 4 - Set up a free OpenWeather API account and get an API Key
//
// Almost everything else has defaults that should be left alone and then configuration
// done through the Web based Configuration UI. http://cheapo.local/config
// *************************************************************************************
//
#ifndef MY_CDCDEFINES_H
#define MY_CDCDEFINES_H

#include <Arduino.h>

// *************************************************************************************
// ESP32 C3 Board configuration
// *************************************************************************************
#define CDC_ENABLE_PWM_OUTPUT // comment to disable output pins (mainly for debugging)
#define CDC_PWM_OUPUT_PIN1 0  // set first Output channel Pin
#define CDC_PWM_OUPUT_PIN2 1  // Uncomment and set Pin value to enable second channel
#define CDC_PWM_CHANNEL 0     // Currently both output Pin 1 an 2 share a channel

// *************************************************************************************
// Builtin LED or other pin to use for Status LED.
// Status LED is used to show WiFi mode and config changes via blinks
// 1 second blink (500ms on/500ms off) is successful Station Mode - CDC_DEFAULT_STATUS_BLINK
// 0.2 second blink (100ms on/100ms off) is fall back to Access Point Mode - CDC_WIFI_AP_STATUS_BLINK
// Blink will continue for 10 seconds then turn off - CDC_STATUS_LED_DELAY
// *************************************************************************************
#define CDC_STATUS_LED 8                      // Pin to use for Status LED
#define CDC_DEFAULT_STATUS_BLINK 500          // every x millisec
#define CDC_WIFI_AP_STATUS_BLINK 100          // every x millisec
#define CDC_STATUS_LED_DELAY 10               // time until status LED is turned off in seconds (0 = infinite delay)
#define CDC_STATUS_LED_BLINK_ON_CONFIG_CHANGE // Blink for delay time when config item changed
#define CDC_STATUS_LED_BLINK_ON_POWER_CHANGE  // Blink for delay time when Power Output changes
//#define CDC_REVERSE_HIGH_LOW               // Some ESP32-C3 seem to have Status Pin High/Low reversed. Uncomment to reverse.
#ifdef CDC_REVERSE_HIGH_LOW
#define CDC_STATUS_LED_HIGH LOW
#define CDC_STATUS_LED_LOW HIGH
#else
#define CDC_STATUS_LED_HIGH HIGH
#define CDC_STATUS_LED_LOW LOW
#endif

// *************************************************************************************
// Configure Web services
// *************************************************************************************
#define CDC_ENABLE_WEB_AUTH              // Uncomment this line to enable Basic Web Authentication
#define CDC_DEFAULT_WEB_ID "admin"       // Change to change logon ID
#define CDC_DEFAULT_WEB_PASSWORD "admin" // Change to change logon password

// *************************************************************************************
// Web Socket support
// Read the README.md in https://github.com/hcomet/CheapoDC/tree/main/CheapoDC for
// changes needed to AsyncTCP.cpp and AsyncWebSocket.h  before enabling Web Sockets
// *************************************************************************************
// #define CDC_ENABLE_WEB_SOCKETS  // Uncomment this line to enable Web Sockets

// *************************************************************************************
// Network configuration
// *************************************************************************************
#define CDC_DEFAULT_HOSTNAME "cheapodc"             // default hostname. Can also be set in CDCWiFi.json
#define CDC_DEFAULT_WIFI_SSID "defaultSSID"         // default SSID for STA mode. Can also be set in CDCWiFi.json
#define CDC_DEFAULT_WIFI_PASSWORD "defaultPassword" // default WiFi Password. Can also be set in CDCWiFi.json
#define CDC_DEFAULT_WIFI_AP_SSID "cheapodc"         // Name used for SSID of AP mode if STA mode fails
#define CDC_DEFAULT_WIFI_AP_PASSWORD "cheapodc"     // WiFi password for AP mode
#define CDC_DEFAULT_WIFI_CONNECTATTEMPTS 15         // Number of times to try to connect to an AP when in STA mode
#define CDC_DEFAULT_WIFI_TRYAPS 1                   // Number of times to cycle through APs when in STA mode

// *************************************************************************************
// Default Location and time settings
// *************************************************************************************
#define CDC_DEFAULT_LOCATION_NAME "Greenwich"
#define CDC_DEFAULT_LATITUDE "51.479"
#define CDC_DEFAULT_LONGITUDE "0"
#define CDC_DEFAULT_TIMEZONE 0
#define CDC_DEFAULT_DSTOFFSET 0
#define CDC_DEFAULT_NTPSERVER "north-america.pool.ntp.org"

// *************************************************************************************
// Pick your Weather Service
// *************************************************************************************
#define CDC_USE_OPEN_WEATHER // Comment out to use Open Meteo
#ifdef CDC_USE_OPEN_WEATHER
// *************************************************************************************
// Open WeatherMap API configuration.
// Register with OpenWeather for a free account to get your API Key
// https://home.openweathermap.org/users/sign_up
// *************************************************************************************
#define CDC_DEFAULT_WEATHERAPIKEY "" // Register to get an API Key
#define CDC_DEFAULT_WEATHERSOURCE "OpenWeather"
#define CDC_DEFAULT_WEATHERAPIURL "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s"
#define CDC_DEFAULT_WEATHERUNITS "metric" // Need metric for temperature calculations to work
#define CDC_DEFAULT_WEATHERICONURL "https://openweathermap.org/img/wn/%s@2x.png"
#else
// *************************************************************************************
// Open-Meteo API configuration
// no registration is required so no API key is required
// *************************************************************************************
#define CDC_USE_OPEN_METEO
#define CDC_DEFAULT_WEATHERAPIKEY "Not Required"
#define CDC_DEFAULT_WEATHERSOURCE "Open-Meteo"
#define CDC_DEFAULT_WEATHERAPIURL "https://api.open-meteo.com/v1/forecast?latitude=%s&longitude=%s&current=temperature_2m,relative_humidity_2m,is_day,weather_code,cloud_cover,wind_speed_10m,wind_direction_10m,dew_point_2m"
#define CDC_DEFAULT_WEATHERUNITS "metric"                                        // Need metric for temperature calculations to work
#define CDC_DEFAULT_WEATHERICONURL "https://openweathermap.org/img/wn/%s@2x.png" // reuse the Open Weather Icon API - no API Key required
#endif

#define CDC_DEFAULT_WEATHER_QUERY 5
#define CDC_DEFAULT_HTTP_REQ_RETRY 1

// *************************************************************************************
// Default controller configuration values
// *************************************************************************************
#define CDC_MAXIMUM_CONTROLLER_OUTPUT 100
#define CDC_MINIMUM_CONTROLLER_OUTPUT 0
#define CDC_DEFAULT_CONTROLLER_MODE AUTOMATIC
#define CDC_DEFAULT_TEMPERATURE_MODE WEATHER_QUERY
#define CDC_DEFAULT_SET_POINT_MODE DEWPOINT
#define CDC_DEFAULT_TEMPERATURE_SET_POINT 0.0
#define CDC_DEFAULT_TRACK_POINT_OFFSET 0.0
#define CDC_MINIMUM_TRACK_POINT_OFFSET -5.0
#define CDC_MAXIMUM_TRACK_POINT_OFFSET 5.0
#define CDC_DEFAULT_TRACKING_RANGE 5.0
#define CDC_MINIMUM_TRACKING_RANGE 4.0
#define CDC_MAXIMUM_TRACKING_RANGE 10.0
#define CDC_DEFAULT_UPDATE_OUTPUT_EVERY 1 // in minutes
#define CDC_DEFAULT_SAVE_CONFIG_EVERY 20  // in seconds

// *************************************************************************************
// *************************************************************************************
// ***********         Don't change anything below here.                      ***********
// *************************************************************************************
// *************************************************************************************

// PWM Configuration
#define CDC_PWM_FREQUENCY 1000
#define CDC_PWM_RESOLUTION 8 // (0 to 255)
#define CDC_PWM_DUTY_MINIMUM 0
#define CDC_PWM_DUTY_MAXIMUM (2 ^ CDC_PWM_RESOLUTION) - 1

// LIttleFS Configuration files
#define CDC_CONFIG_FILE "/CDCConfig.json"
#define CDC_WIFI_CONFIG "/CDCWiFi.json"

// Server configuration

#define CDC_DEFAULT_WEBSRVR_PORT 80
#define CDC_DEFAULT_WEBSOCKET_URL "/ws"
#define CDC_DEFAULT_TCP_SERVER_PORT 58000 // Port used for the TCP based API

// units - HTML Escaped
#define CDC_UNITS_DEGREES "&deg;"
#define CDC_UNITS_DEGREES_C "&deg;C"
#define CDC_UNITS_PERCENT "&percnt;"
#define CDC_UNITS_MILLISEC "msec"
#define CDC_UNITS_SECOND "sec"
#define CDC_UNITS_MINUTE "min"
#define CDC_UNITS_NONE ""

// Date Time format string
#define CDC_DATE_TIME "%s, %s %02d %4d %02d:%02d:%02d" // DDD, MMM dd yyyy hh:mm:ss
// Used for tracking weather update times
#ifdef CDC_USE_OPEN_WEATHER
#define CDC_TIME "%02d:%02d:%02d"  // hh:mm:ss
#define CDC_DATE "%s, %s %02d %4d" // DDD, MMM dd yyyy
#else
#define CDC_TIME "%02d:%02d GMT" // hh:mm GMT
#define CDC_DATE "%s %02d, %4d"  // MMM dd, yyyy
#endif
#define CDC_NA "--"
#define CDC_BLANK ""

// CDC Commands
#define CDC_CMD_TMFL 0   // Filelist
#define CDC_CMD_WICON 1  // Weather Icon
#define CDC_CMD_WDESC 2  // Weather description
#define CDC_CMD_ATPQ 3   // ambient temperature
#define CDC_CMD_HU 4     // humidity
#define CDC_CMD_DP 5     // Dew point
#define CDC_CMD_SP 6     // set point
#define CDC_CMD_TPO 7    // Track Point Offset
#define CDC_CMD_TKR 8    // Tracking Range
#define CDC_CMD_DCO 9    // Dew Controller Output
#define CDC_CMD_WS 10    // Weather source
#define CDC_CMD_LQT 11   // last weather query time
#define CDC_CMD_LQD 12   // last weather query date
#define CDC_CMD_QN 13    // Query Weather Now (Set only command)
#define CDC_CMD_FW 14    // firmware version
#define CDC_CMD_HP 15    // Heap size
#define CDC_CMD_LFS 16   // LittleFS remaining space
#define CDC_CMD_DCM 17   // dew controller mode
#define CDC_CMD_DCTM 18  // dew controller temperature mode
#define CDC_CMD_SPM 19   // dew controller set point mode
#define CDC_CMD_WQE 20   // Weather query every
#define CDC_CMD_UOE 21   // update outputs every
#define CDC_CMD_WAPI 22  // Weather API URL
#define CDC_CMD_WIURL 23 // Weather Icon URL
#define CDC_CMD_WKEY 24  // Weather API Key
#define CDC_CMD_LAT 25   // Location latitude
#define CDC_CMD_LON 26   // Location longitude
#define CDC_CMD_LNM 27   // Location name
#define CDC_CMD_TMZ 28   // Location time zone (seconds)
#define CDC_CMD_DST 29   // Location DST offset (seconds)
#define CDC_CMD_LED 30   // Status LED Blink rate (x/1000 msec)
#define CDC_CMD_NTP 31   // NTP serverName
#define CDC_CMD_OMIN 32  // DC Min output
#define CDC_CMD_OMAX 33  // DC Max output
#define CDC_CMD_CDT 34   // Current Date Time
#define CDC_CMD_ATPX 35  // External Temperature (Usually set by outside call to API)
#define CDC_CMD_CTP 36   // Current temperature track point based on DC settings
#define CDC_CMD_WUL 37   // Name or location of weather station from last weather query
#define CDC_CMD_CLC 38   // Cloud Coverage in percent
#define CDC_CMD_LWUT 39  // Last weather update time taken from query result
#define CDC_CMD_LWUD 40  // Last weather update date taken from query result
#define CDC_CMD_UPT 41   // return uptime in hh:mm:ss:msec
#define CDC_CMD_WIFI 42  // WIFI mode AP (Access Point) or STA (Station Mode)
#define CDC_CMD_IP 43    // IP Address
#define CDC_CMD_HN 44    // Host name

// Constants for calculating Dew Point from Temperature & Humidity
#define CDC_MC_A 17.625
#define CDC_MC_B 243.04

// Defines for Logging via EasyLogger
/* This LOG_LEVEL can be set to LOG_LEVEL_EMERGENCY, LOG_LEVEL_ALERT, LOG_LEVEL_CRITICAL
   LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_INFO, LOG_LEVEL_DEBUG
   in order to filter out the lowest level information.
   If set to LOG_LEVEL_NONE, no logging will be done.
   If not defined the the level will default to LOG_LEVEL_DEBUG. Everything will get logged
*/

#define LOG_LEVEL LOG_LEVEL_ERROR
// #define LOG_FILTER "handleWebsocketMessage,processor,getCmdProcessor,SaveConfig,processClientRequest"
// #define LOG_FILTER_EXCLUDE

/* LOG_FORMATTING can be set to LOG_FORMATTING_HMS, LOG_FORMATTING_MILLIS or LOG_FORMATTING_NOTIME
   If not defined, the standard will be LOG_FORMAT_HMS */

#define LOG_FORMATTING LOG_FORMATTING_HMS

/* If you want to direct output to a port other than Serial or softserial (eg Serial2) you can define LOG_OUTPUT
   If nothing is defined log will output to Serial */
#define LOG_OUTPUT Serial
#endif
