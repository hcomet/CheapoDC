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
// ESP32 C3 Board configuration defaults
// *************************************************************************************
#define CDC_DEFAULT_CONTROLLER_PIN0 0  // set first Controller output default
#define CDC_DEFAULT_CONTROLLER_PIN1 1  // set second Controller output default
#define CDC_CONTROLLER_PWM_CHANNEL 0      // The PWM Channel used for Dew Controller managed Pins (Don't change)
#define CONTROLLER_PIN_NOT_CONFIGURED -1  // Used to indicate that a pin has not been configured

// *************************************************************************************
// Builtin LED or other pin to use for Status LED.
// Status LED is used to show WiFi mode and config changes via blinks
// 1 second blink (500ms on/500ms off) is successful Station Mode - CDC_DEFAULT_STATUS_BLINK
// 0.2 second blink (100ms on/100ms off) is fall back to Access Point Mode - CDC_WIFI_AP_STATUS_BLINK
// Blink will continue for 10 seconds then turn off - CDC_STATUS_LED_DELAY
// *************************************************************************************
#define CDC_DEFAULT_STATUS_LED_PIN 8                      // Pin to use for Status LED
#define CDC_DEFAULT_STATUS_BLINK 500          // every x millisec
#define CDC_WIFI_AP_STATUS_BLINK 100          // every x millisec
#define CDC_STATUS_LED_DELAY 10               // time until status LED is turned off in seconds (0 = infinite delay)
#define CDC_STATUS_LED_BLINK_ON_CONFIG_CHANGE // Blink for delay time when config item changed
#define CDC_STATUS_LED_BLINK_ON_POWER_CHANGE  // Blink for delay time when Power Output changes
//#define CDC_REVERSE_HIGH_LOW                // Some ESP32-C3 seem to have onboard LEDs with High/Low reversed. Uncomment to reverse.
#ifdef CDC_REVERSE_HIGH_LOW
#define CDC_STATUS_LED_HIGH LOW
#define CDC_STATUS_LED_LOW HIGH
#else
#define CDC_STATUS_LED_HIGH HIGH
#define CDC_STATUS_LED_LOW LOW
#endif

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
//#define CDC_ENABLE_WIFI_TX_POWER_MOD WIFI_POWER_8_5dBm // Uncomment to adjust WiFi TX power if you are having connection issues

// *************************************************************************************
// *************************************************************************************
// ***********         DO NOT change anything below here.                    ***********
// *************************************************************************************
// *************************************************************************************
// Defaults for Web Authentication
#define CDC_DEFAULT_WEB_ID "admin"       // Change to change logon ID
#define CDC_DEFAULT_WEB_PASSWORD "admin" // Change to change logon password
#define CDC_DEFAULT_WEB_REALM "CheapoDC" // Change to change logon realm

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
// Weather Query Service configuration values for supported services
// - Open-Meteo: https://open-meteo.com/
// - OpenWeather: https://home.openweathermap.org/users/sign_up (To get an API key)
// - External Source: Temperature and Humidity must be provided via the API
// ************************************************************************************* 
#define CDC_OPENMETEO_APIURL "https://api.open-meteo.com/v1/forecast?latitude=%s&longitude=%s&current=temperature_2m,relative_humidity_2m,is_day,weather_code&timeformat=unixtime"           
#define CDC_OPENMETEO_ICONURL "https://openweathermap.org/img/wn/%s@2x.png" // reuse the Open Weather Icon API - no API Key required
#define CDC_OPENWEATHER_APIURL "https://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric" // Need metric for temperature calculations to work
#define CDC_OPENWEATHER_ICONURL "https://openweathermap.org/img/wn/%s@2x.png"
#define CDC_EXTERNALSOURCE_APINURL "--"
#define CDC_EXTERNALSOURCE_ICONURL "/weatherIconNA.png"
#define CDC_EXTERNALSOURCE_DESC "Not available."
#define CDC_EXTERNALSOURCE_LOCATION_NAME "External Source"
// Set Weather Query defaults for Open-Meteo since it does not require an API key
#define CDC_DEFAULT_WEATHERSOURCE 0 // OPENMETEO 
#define CDC_DEFAULT_WEATHERAPIURL CDC_OPENMETEO_APIURL
#define CDC_DEFAULT_WEATHERICONURL CDC_OPENWEATHER_ICONURL
#define CDC_DEFAULT_WEATHERAPIKEY "--"   // Do not change the default. Enter your key throught the Web UI.
#define CDC_DEFAULT_WEATHER_QUERY_EVERY 5
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

// PWM Configuration
#define CDC_PWM_FREQUENCY 1000
#define CDC_PWM_RESOLUTION 8 // (0 to 255)
#define CDC_PWM_DUTY_MINIMUM 0
#define CDC_PWM_DUTY_MAXIMUM (2 ^ CDC_PWM_RESOLUTION) - 1

// LIttleFS Configuration files
#define CDC_CONFIG_FILE "/CDCConfig.json"
#define CDC_WIFI_CONFIG "/CDCWiFi.json"

// *************************************************************************************
// Configure Web services
// *************************************************************************************
#define CDC_DEFAULT_WEBSRVR_PORT 80
#define CDC_DEFAULT_TCP_SERVER_PORT 58000 // Port used for the TCP based API

// *************************************************************************************
// Web Socket support - Has been deprecated
// Read the README.md in https://github.com/hcomet/CheapoDC/tree/main/CheapoDC for
// changes needed to AsyncTCP.cpp and AsyncWebSocket.h  before enabling Web Sockets
// When Web Sockets are enabled then the post processing queue is also enabled so that  
// commands requiring longer actions like initiating a Wed Query go into a queue. The 
// asyncWebSocket implementation will not handle long transactions that block and cause 
// watchdog process timeouts.
// *************************************************************************************
//#define CDC_ENABLE_WEB_SOCKETS      // Do not uncomment has been deprecated
//#define CDC_DEFAULT_WEBSOCKET_URL "/ws"  // Do not uncomment has been deprecated

// Command post processing queue support
// Some API commands may auto generate post processing actions such as weather queries or 
// power output updates. These may cause long command transactions. If the API client is 
// making many asynchronous calls to the API and those cause long transactions then the 
// AsyncTCP queue can be overwhelmed and prevent the ESP32 watchdog timer from getting
// enough time causing panic crashes. Enabling this queue will cause the post processing
// actions to be put in a queue that is cleared every CDC_RUNCMDQUEUE_EVERY msec.
// For situations where API calls are synchronously called (ie: HTML embedded or the INDI 
// driver) the queue is not needed. This is the default which is disabled. For a situation
// where asynchronous calls are not limited (ie: the Web Sockets javascript client) then it
// should be enabled. Enabling Web Sockets will automatically enable the queue.
//#define CDC_ENABLE_CMDQUEUE         // Uncomment to enable command post processing queue
#define CDC_RUNCMDQUEUE_EVERY 10    // run command queue every x msec

// For Web based updates using HTTP OTA
#define CDC_ENABLE_HTTP_OTA   // Comment out to disable HTTP OTA Support if enabled esp32FOTA library is required
#define CDC_DEFAULT_HTTP_OTA_URL "https://hcomet.github.io/CheapoDC/httpOtaManifest.json" // URL for Web based firmware updates
#ifdef CDC_ENABLE_HTTP_OTA
#define CDC_NO_FW_UPDATE_RESPONSE "NOFWUPDATE"
#define CDC_HTTP_OTA_ROOT_CA_FILE "/root_ca.pem"     // Can be placed in the data partition to override the default below
#define WEB_OTA_ROOT_CA \
"-----BEGIN CERTIFICATE-----\n" \
"MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n" \
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n" \
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n" \
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n" \
"MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n" \
"BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n" \
"aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n" \
"dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n" \
"AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n" \
"3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n" \
"tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n" \
"Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n" \
"VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n" \
"79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n" \
"c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n" \
"Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n" \
"c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n" \
"UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n" \
"Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n" \
"BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n" \
"A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n" \
"Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n" \
"VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n" \
"ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n" \
"8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n" \
"iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n" \
"Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n" \
"XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n" \
"qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n" \
"VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n" \
"L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n" \
"jjxDah2nGN59PRbxYvnKkKj9\n" \
"-----END CERTIFICATE-----\n" \
""
#else
#define CDC_NO_FW_UPDATE_RESPONSE "NOSUPPORT"
#endif  // CDC_ENABLE_HTTP_OTA


// units - HTML Escaped
#define CDC_UNITS_DEGREES "&deg;"
#define CDC_UNITS_DEGREES_C "&deg;C"
#define CDC_UNITS_PERCENT "&percnt;"
#define CDC_UNITS_MILLISEC "msec"
#define CDC_UNITS_SECOND "sec"
#define CDC_UNITS_MINUTE "min"
#define CDC_UNITS_NONE ""

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
#define CDC_CMD_LED 30   // Status LED GPIO pin
#define CDC_CMD_NTP 31   // NTP serverName
#define CDC_CMD_OMIN 32  // DC Min output
#define CDC_CMD_OMAX 33  // DC Max output
#define CDC_CMD_CDT 34   // Current Date Time
#define CDC_CMD_ATPX 35  // External Temperature (Usually set by outside call to API)
#define CDC_CMD_CTP 36   // Current temperature track point based on DC settings
#define CDC_CMD_WUL 37   // Name or location of weather station from last weather query
#define CDC_CMD_CLC 38   // Cloud Coverage in percent *** Deprecated in V2.0 ***
#define CDC_CMD_LWUT 39  // Last weather update time taken from query result
#define CDC_CMD_LWUD 40  // Last weather update date taken from query result
#define CDC_CMD_UPT 41   // return uptime in hh:mm:ss:msec
#define CDC_CMD_WIFI 42  // WIFI mode AP (Access Point) or STA (Station Mode)
#define CDC_CMD_IP 43    // IP Address
#define CDC_CMD_HN 44    // Host name
#define CDC_CMD_WQEN 45  // Weather Query Enabled (false = 0, true = 1)
#define CDC_CMD_CPP0 46  // Controller Output 0 to Pin mapping
#define CDC_CMD_CPP1 47  // Controller Output 1 to Pin mapping
#define CDC_CMD_CPP2 48  // Controller Output 2 to Pin mapping
#define CDC_CMD_CPP3 49  // Controller Output 3 to Pin mapping
#define CDC_CMD_CPP4 50  // Controller Output 4 to Pin mapping
#define CDC_CMD_CPP5 51  // Controller Output 5 to Pin mapping
#define CDC_CMD_CPM0 52  // Controller Output 0 Mode (0 = Disabled, 1 = Controller) - GET only
#define CDC_CMD_CPM1 53  // Controller Output 1 Mode (0 = Disabled, 1 = Controller) - GET only
#define CDC_CMD_CPM2 54  // Controller Output 2 Mode (0 = Disabled, 1 = Controller, 2 = PWM, 3 = Boolean)
#define CDC_CMD_CPM3 55  // Controller Output 3 Mode (0 = Disabled, 1 = Controller, 2 = PWM, 3 = Boolean)
#define CDC_CMD_CPM4 56  // Controller Output 4 Mode (0 = Disabled, 1 = Controller, 2 = PWM, 3 = Boolean)
#define CDC_CMD_CPM5 57  // Controller Output 5 Mode (0 = Disabled, 1 = Controller, 2 = PWM, 3 = Boolean)
#define CDC_CMD_CPO0 58  // Controller Output Power 0 (Mode dependent: -1, 0 - 100, 0 or 1)
#define CDC_CMD_CPO1 59  // Controller Output Power 1 (Mode dependent: -1, 0 - 100, 0 or 1)
#define CDC_CMD_CPO2 60  // Controller Output Power 2 (Mode dependent: -1, 0 - 100, 0 or 1)
#define CDC_CMD_CPO3 61   // Controller Output Power 3 (Mode dependent: -1, 0 - 100, 0 or 1)
#define CDC_CMD_CPO4 62   // Controller Output Power 4 (Mode dependent: -1, 0 - 100, 0 or 1)
#define CDC_CMD_CPO5 63   // Controller Output Power 5 (Mode dependent: -1, 0 - 100, 0 or 1)
#define CDC_CMD_PWDH 64   // Password Hash
#define CDC_CMD_LEDH 65   // Status LED High, if 1 then HIGH = 1 if 0 then HIGH = 0 (LOW is opposite)
#define CDC_CMD_FWUP 66   // Firmware Update GET returns 1=yes 0=no POST initiates update PWD Hash required
#define CDC_CMD_UURL 67   // Update URL - points to the HTTP OTA update manifest.json

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
