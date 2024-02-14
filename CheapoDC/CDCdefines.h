// ******************************************************************
// CheapoDC defines
// Cheap and easy Dew Controller
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************
#ifndef MY_CDCDEFINES_H
#define MY_CDCDEFINES_H

#include <Arduino.h>

// Builtin LED_BUILTIN
#define CDC_STATUS_LED 8
#define CDC_DEFAULT_STATUS_BLINK 500 // every x millisec, 0 disables, max 999

//#define CDCTIMEZONE -18000
//#define CDCDLOFFSET 3600
//#define CDCNTPSERVER "0.ca.pool.ntp.org"

// Constants for calculating Dew Point from Temperature & Humidity
#define CDC_MC_A 17.625 
#define CDC_MC_B 243.04

#define CDC_ENABLE_WEB_AUTH
#define CDC_DEFAULT_WEB_ID "admin"
#define CDC_DEFAULT_WEB_PASSWORD "admin"

#define CDC_DEFAULT_HOSTNAME "cheapodc"
#define CDC_DEFAULT_WIFI_SSID "defaultSSID"
#define CDC_DEFAULT_WIFI_PASSWORD "defaultPassword"
#define CDC_DEFAULT_WIFI_AP_SSID "cheapodc"
#define CDC_DEFAULT_WIFI_AP_PASSWORD "cheapodc"
#define CDC_DEFAULT_WIFI_CONNECTATTEMPTS 15
#define CDC_DEFAULT_WIFI_TRYAPS 1
#define CDC_DEFAULT_WEBSRVR_PORT 80
#define CDC_DEFAULT_WEBSOCKET_URL "/ws"
#define CDC_DEFAULT_TCP_SERVER_PORT 8000

#define CDC_DEFAULT_LOCATION_NAME "Greenwich"
#define CDC_DEFAULT_LATITUDE "51.479"
#define CDC_DEFAULT_LONGITUDE "0"
#define CDC_DEFAULT_TIMEZONE 0
#define CDC_DEFAULT_DSTOFFSET 0
#define CDC_DEFAULT_NTPSERVER "north-america.pool.ntp.org"

#define CDC_DEFAULT_WEATHERSOURCE "OpenWeatherMap"
#define CDC_DEFAULT_WEATHERAPIURL "http://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s"
#define CDC_DEFAULT_WEATHERICONURL "http://openweathermap.org/img/wn/%s@2x.png"
#define CDC_DEFAULT_WEATHERAPIKEY ""
#define CDC_DEFAULT_WEATHER_QUERY 5
#define CDC_DEFAULT_WEATHERUNITS "metric" // standard (kelvin), metric (celsius), imperial (fahrenheit)
#define CDC_DEFAULT_HTTP_REQ_RETRY 1 



// Defines for Logging via EasyLogger
/* This LOG_LEVEL can be set to LOG_LEVEL_EMERGENCY, LOG_LEVEL_ALERT, LOG_LEVEL_CRITICAL
   LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_INFO, LOG_LEVEL_DEBUG
   in order to filter out the lowest level information.
   If set to LOG_LEVEL_NONE, no logging will be done.
   If not defined the the level will default to LOG_LEVEL_DEBUG. Everything will get logged
*/

#define LOG_LEVEL LOG_LEVEL_ERROR
// #define LOG_FILTER "handleWebsocketMessage,queryWeather,processor,getCmdProcessor,SaveConfig,processClientRequest"
// #define LOG_FILTER_EXCLUDE

/* LOG_FORMATTING can be set to LOG_FORMATTING_HMS, LOG_FORMATTING_MILLIS or LOG_FORMATTING_NOTIME
   If not defined, the standard will be LOG_FORMAT_HMS */

#define LOG_FORMATTING LOG_FORMATTING_HMS

/* If you want to direct output to a port other than Serial or softserial (eg Serial2) you can define LOG_OUTPUT
   If nothing is defined log will output to Serial */
#define LOG_OUTPUT Serial

// Configuration files 
#define CDC_CONFIG_FILE "/CDCConfig.json"
#define CDC_WIFI_CONFIG "/CDCWiFi.json"

// ESP32 C3 Board definition
#define CDC_PWM_OUPUT_PIN1 0
#define CDC_PWM_OUPUT_PIN2 1
#define CDC_PWM_CHANNEL 0
#define CDC_PWM_FREQUENCY 1000
#define CDC_PWM_RESOLUTION 8  // (0 to 255)
#define CDC_PWM_DUTY_MINIMUM 0
#define CDC_PWM_DUTY_MAXIMUM (2^CDC_PWM_RESOLUTION) - 1
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
#define CDC_DEFAULT_SAVE_CONFIG_EVERY 20 // in seconds

// units - HTML Escaped
#define CDC_UNITS_DEGREES "&deg;"
#define CDC_UNITS_DEGREES_C "&deg;C"
#define CDC_UNITS_PERCENT "&percnt;"
#define CDC_UNITS_MILLISEC "msec"
#define CDC_UNITS_SECOND "sec"
#define CDC_UNITS_MINUTE "min"
#define CDC_UNITS_NONE ""

// CDC Commands
#define CDC_CMD_TMFL 0   // Filelist
#define CDC_CMD_WICON 1  // Weather Icon
#define CDC_CMD_WDESC 2  // Weather description
#define CDC_CMD_ATPQ 3     // ambient temperature
#define CDC_CMD_HU 4     // humidity
#define CDC_CMD_DP 5     // Dew point
#define CDC_CMD_SP 6     // set point
#define CDC_CMD_TPO 7    // Track Point Offset
#define CDC_CMD_TKR 8    // Tracking Range
#define CDC_CMD_DCO 9    // Dew Controller Output
#define CDC_CMD_WS 10    // Weather source
#define CDC_CMD_LQT 11   // last weather query time
#define CDC_CMD_LQD 12   // last weather query date
#define CDC_CMD_WRD 13   // weather report date time
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
#define CDC_CMD_OMIN 32   // DC Min output
#define CDC_CMD_OMAX 33   // DC Max output
#define CDC_CMD_CDT 34  // Current Date Time
#define CDC_CMD_ATPX 35  // Local Temperature (Usually set by outside call to API)
#define CDC_CMD_CTP 36  // Current temperature track point based on DC settings 
#define CDC_CMD_WUL 37 // Name or location of weather station from last weather query
#define CDC_CMD_CLC 38 // Cloud Coverage in percent

#endif 
