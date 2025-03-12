// ******************************************************************
// CheapoDC Setup and configuration
// Cheap and easy Dew Controller
// Details at https://github.com/hcomet/CheapoDC
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************

#include <Arduino.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>
#include <TimeLib.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <WebAuthentication.h>
#include "CDCdefines.h"
#include "CDCEasyLogger.h"
#include "CDCvars.h"
#include "CDCSetup.h"
#include "CDController.h"
#include "CDCommands.h"
#include "CDCwmoTools.h"

#define FORMAT_LITTLEFS_IF_FAILED true

//** TimeLib dayShortStr() not working so use my own:
const char *mydayShortStr[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char *mymonthShortStr[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

//********************************************************************************************
String httpGETRequest(const char *serverName, int retryCount = 0)
{
  HTTPClient http;
  String payload = "";
  int thisRetryCount = retryCount;

  LOG_DEBUG("httpGETRequest", "URL:" << serverName << " retry: " << thisRetryCount);

  if (thisRetryCount <= CDC_DEFAULT_HTTP_REQ_RETRY)
  {

    // Your IP address with path or Domain name with URL path
    if (!http.begin(serverName))
    {
      LOG_ERROR("httpGETRequest", "http.Begin failed: " << serverName << " on try: " << (thisRetryCount + 1));
      payload = httpGETRequest(serverName, (thisRetryCount + 1));
    }
    else
    {

      // Send HTTP POST request
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0)
      {
        LOG_DEBUG("httpGETRequest", "HTTP response code: " << httpResponseCode);
        payload = http.getString();
      }
      else
      {
        LOG_ERROR("httpGETRequest", "HTTP response error code: " << httpResponseCode << " on try: " << (thisRetryCount + 1));
        payload = httpGETRequest(serverName, (thisRetryCount + 1));
      }
    }
    // Free resources
    http.end();
  }
  return payload;
}

//********************************************************************************************
bool CDCSetup::queryWeather(void)
{
  char httpRequestURL[256];
#if ARDUINOJSON_VERSION_MAJOR>=7
	JsonDocument doc;
#else
	DynamicJsonDocument doc(1024);
#endif
  float alphaDP;

  if (!this->_weatherQueryEnabled)
  {
    LOG_DEBUG("queryWeather", "Weather Query is Disabled.");
    return true;
  }
  
  LOG_DEBUG("queryWeather", "Current weather source: " << this->_currentWeatherSource);
  if (this->_currentWeatherSource != EXTERNALSOURCE)
  {
    if (this->getInWiFiAPMode())
    {
      LOG_ERROR("queryWeather", "CheapoDC in Access Point mode. Cannot do a weather query.");
      return false;
    }

    if (this->_currentWeatherSource == OPENMETEO)
    {
      sprintf(httpRequestURL, this->_weatherAPIURL, this->_location.latitude, this->_location.longitude, "");
    }
    else
    {
      sprintf(httpRequestURL, this->_weatherAPIURL, this->_location.latitude, this->_location.longitude, this->_weatherAPIKey);
    }

    DeserializationError error = deserializeJson(doc, httpGETRequest(httpRequestURL));
    if (error)
    {
      LOG_ERROR("queryWeather", "Deserialization failed: " << error.c_str());
      return false;
    }

    LOG_DEBUG("queryWeather", "Last query time: " << this->getLastWeatherQueryTime());
    LOG_DEBUG("queryWeather", "Last query date: " << this->getLastWeatherQueryDate());

    strlcpy(this->_currentWeather.lastWeatherQueryTime, this->getTime().c_str(), sizeof(this->_currentWeather.lastWeatherQueryTime));
    strlcpy(this->_currentWeather.lastWeatherQueryDate, this->getDate().c_str(), sizeof(this->_currentWeather.lastWeatherQueryDate));

    LOG_DEBUG("queryWeather", "New query time: " << this->getLastWeatherQueryTime());
    LOG_DEBUG("queryWeather", "New query date: " << this->getLastWeatherQueryDate());

    if (this->_currentWeatherSource == OPENMETEO)
    {
      JsonObject main = doc["current"];
     
      time_t tempTime = main["time"] | 0;

      if (tempTime != 0)
      {
        strlcpy(this->_currentWeather.lastWeatherUpdateTime, this->getTime(tempTime).c_str(), sizeof(this->_currentWeather.lastWeatherUpdateTime));
        strlcpy(this->_currentWeather.lastWeatherUpdateDate, this->getDate(tempTime).c_str(), sizeof(this->_currentWeather.lastWeatherUpdateDate));      }
      else
      {
        strlcpy(this->_currentWeather.lastWeatherUpdateTime, CDC_NA, sizeof(this->_currentWeather.lastWeatherUpdateTime));
        strlcpy(this->_currentWeather.lastWeatherUpdateDate, CDC_NA, sizeof(this->_currentWeather.lastWeatherUpdateDate));
      }

      LOG_ALERT("queryWeather", "Last Update: " << this->_currentWeather.lastWeatherUpdateDate << " - " << this->_currentWeather.lastWeatherUpdateTime);

      // Get temperature, Humidity

      this->_currentWeather.ambientTemperature = main["temperature_2m"] | 0.0;
      this->_currentWeather.humidity = main["relative_humidity_2m"] | 0.0;

      strlcpy(this->_currentWeather.weatherUpdateLocation, this->_location.name, sizeof(this->_currentWeather.weatherUpdateLocation));
      unsigned int wmoCode = main["weather_code"] | 0;
      unsigned int isDay = main["is_day"] | 1;

      String tempDescription = mapWMOCode(wmoCode, (isDay == 1), true);
      String tempIcon = mapWMOCode(wmoCode, (isDay == 1), false);

      strlcpy(this->_currentWeather.weatherDescription, tempDescription.c_str(), sizeof(this->_currentWeather.weatherDescription));
      strlcpy(this->_currentWeather.weatherIcon, tempIcon.c_str(), sizeof(this->_currentWeather.weatherIcon));

    }
    else   // OpenWeather
    {
      JsonObject main = doc["main"];
      
      // Do Date-Time items which are in LINUX UTC Time
      time_t tempTime = doc["dt"] | 0;
      time_t tempTZ = doc["timezone_offset"] | 0;
      //tempTime = tempTime + tempTZ;

      if (tempTime != 0)
      {
        strlcpy(this->_currentWeather.lastWeatherUpdateTime, this->getTime(tempTime).c_str(), sizeof(this->_currentWeather.lastWeatherUpdateTime));
        strlcpy(this->_currentWeather.lastWeatherUpdateDate, this->getDate(tempTime).c_str(), sizeof(this->_currentWeather.lastWeatherUpdateDate));      }
      else
      {
        strlcpy(this->_currentWeather.lastWeatherUpdateTime, CDC_NA, sizeof(this->_currentWeather.lastWeatherUpdateTime));
        strlcpy(this->_currentWeather.lastWeatherUpdateDate, CDC_NA, sizeof(this->_currentWeather.lastWeatherUpdateDate));
      }

      LOG_ALERT("queryWeather", "Last Update: " << this->_currentWeather.lastWeatherUpdateDate << " - " << this->_currentWeather.lastWeatherUpdateTime);

      // Get temperature, Humidity and calculate Dew point

      this->_currentWeather.ambientTemperature = main["temp"] | 0.0;
      this->_currentWeather.humidity = main["humidity"] | 0.0;

      strlcpy(this->_currentWeather.weatherUpdateLocation, doc["name"] | CDC_NA, sizeof(this->_currentWeather.weatherUpdateLocation));
      String tempDescription = String(doc["weather"][0]["main"] | CDC_NA);
      tempDescription += String(" - ") + String(doc["weather"][0]["description"] | CDC_NA);
      tempDescription.toCharArray(this->_currentWeather.weatherDescription, sizeof(this->_currentWeather.weatherDescription));
      strlcpy(this->_currentWeather.weatherIcon, doc["weather"][0]["icon"] | CDC_NA, sizeof(this->_currentWeather.weatherIcon));
    }

    this->calculateAndSetDewPoint();
  }

  LOG_DEBUG("queryWeather", "Current Weather: " << this->_currentWeather.lastWeatherQueryDate << " " << this->_currentWeather.lastWeatherQueryTime << " From: " << this->_currentWeather.weatherUpdateLocation);
  LOG_DEBUG("queryWeather", "    temperature: " << this->_currentWeather.ambientTemperature);
  LOG_DEBUG("queryWeather", "    humidity: " << this->_currentWeather.humidity);
  LOG_DEBUG("queryWeather", "    dewPoint: " << this->_currentWeather.dewPoint);
  LOG_DEBUG("queryWeather", "    description: " << this->_currentWeather.weatherDescription);
  LOG_DEBUG("queryWeather", "    weatherIcon: " << this->_currentWeather.weatherIcon);
  LOG_DEBUG("queryWeather", "    data updated: " << this->_currentWeather.lastWeatherUpdateDate << " " << this->_currentWeather.lastWeatherUpdateTime);

  return true;
}

//********************************************************************************************
CDCSetup::CDCSetup(void)
{
  LOG_DEBUG("CDCSetup", "Starting setup");
  // Load defaults
  
  this->_loadDefaults();

  // Disable Weather queries until we are ready
  this->setWeatherQueryEnabled( false );
  sleep(5);
  // Filesystem first
  LOG_DEBUG("CDCSetup", "Setup LittleFS");
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
  {
    LOG_ERROR("CDCSetup", "LittleFS Mount Failed");
    return;
  } else {
    LOG_ALERT("CDCSetup", "LittleFS Mounted. Size: " << LittleFS.totalBytes() << " Used: " << LittleFS.usedBytes());
  }
  
  strlcpy(this->_currentWeather.lastWeatherQueryTime, CDC_NA, sizeof(this->_currentWeather.lastWeatherQueryTime));
  strlcpy(this->_currentWeather.lastWeatherQueryDate, CDC_NA, sizeof(this->_currentWeather.lastWeatherQueryDate));
}

//********************************************************************************************
bool CDCSetup::SaveConfig(void)
{
  char cmdBuf[10] = "";
  char valueBuf[128] = "";

  if (getConfigUpdated())
  {
#if ARDUINOJSON_VERSION_MAJOR>=7
	  JsonDocument doc;
#else
	  DynamicJsonDocument doc(1024);
    LOG_DEBUG("SaveConfig", "JsonDocument size: " << doc.capacity());
#endif

    File file = LittleFS.open(CDC_CONFIG_FILE, FILE_WRITE);
    if (!file)
    {
      LOG_ERROR("SaveConfig", "Failed to open: " << CDC_CONFIG_FILE);
      return false;
    }

    for (auto &[cmdTxt, cmdInfo] : CDCCommands)
    {
      // String  cmd = String(cmdTxt.c_str());
      int cmdNumber = cmdInfo.number;
      bool saveInfo = cmdInfo.saveToConfig;

      if (saveInfo)
      {
        String cmd = String(cmdTxt.c_str());
        cmdResponse response = getCmdProcessor(cmd);
        String value = response.response;

        LOG_DEBUG("SaveConfig", "Add to doc: " << cmd << " value: " << value);
        doc[cmd] = value;
      }
    }

    if (serializeJson(doc, file) == 0)
    {
      LOG_ERROR("SaveConfig", "Failed to serialize config to: " << CDC_CONFIG_FILE);
      file.close();
      return false;
    }

    this->resetConfigUpdated();
    file.close();
  }
#if LOG_LEVEL == LOG_LEVEL_DEBUG
  else
  {
    LOG_DEBUG("SaveConfig", "Save Config skipped since no change to config items since last save");
  }
#endif

  return true;
}

//********************************************************************************************
void CDCSetup::_loadDefaults(void)
{

  LOG_DEBUG("_loadDefaults", "Loading CDCSetup defaults: Starting.");

  memset(this->_passwordHash, '\0', sizeof(this->_passwordHash));
  strlcpy(this->_passwordHash, generateDigestHash(CDC_DEFAULT_WEB_ID, CDC_DEFAULT_WEB_PASSWORD, CDC_DEFAULT_WEB_REALM).c_str(), sizeof(this->_passwordHash));

  #ifdef CDC_DEFAULT_STATUS_LED_PIN
  this->setStatusLEDPin( CDC_DEFAULT_STATUS_LED_PIN);
  #else
  this->setStatusLEDPin( -1 ); // Disable for now
  #endif

  this->setStatusLEDHigh( CDC_STATUS_LED_HIGH);

  memset(this->_wifiConfig.hostname, '\0', sizeof(this->_wifiConfig.hostname));
  strlcpy(this->_wifiConfig.hostname, CDC_DEFAULT_HOSTNAME, sizeof(this->_wifiConfig.hostname));

  memset(this->_IPAddress, '\0', sizeof(this->_IPAddress));

  this->_inWiFiAPMode = false;

  this->_wifiConfig.connectAttempts = CDC_DEFAULT_WIFI_CONNECTATTEMPTS;

  this->_wifiConfig.tryAPs = CDC_DEFAULT_WIFI_TRYAPS;

  memset(this->_wifiConfig.ssid, '\0', sizeof(this->_wifiConfig.ssid));
  strlcpy(this->_wifiConfig.ssid, CDC_DEFAULT_WIFI_SSID, sizeof(this->_wifiConfig.ssid));

  memset(this->_wifiConfig.password, '\0', sizeof(this->_wifiConfig.password));
  strlcpy(this->_wifiConfig.password, CDC_DEFAULT_WIFI_PASSWORD, sizeof(this->_wifiConfig.password));

  memset(this->_location.latitude, '\0', sizeof(this->_location.latitude));
  strlcpy(this->_location.latitude, CDC_DEFAULT_LATITUDE, sizeof(this->_location.latitude));

  memset(this->_location.longitude, '\0', sizeof(this->_location.longitude));
  strlcpy(this->_location.longitude, CDC_DEFAULT_LONGITUDE, sizeof(this->_location.longitude));

  memset(this->_location.name, '\0', sizeof(this->_location.name));
  strlcpy(this->_location.name, CDC_DEFAULT_LOCATION_NAME, sizeof(this->_location.name));

  this->_location.timezone = CDC_DEFAULT_TIMEZONE;

  this->_location.DSTOffset = CDC_DEFAULT_DSTOFFSET;

  this->_ambientTemperatureExternal = 0.0;

  this->_currentWeatherSource = (weatherSource)CDC_DEFAULT_WEATHERSOURCE;

  memset(this->_weatherAPIURL, '\0', sizeof(this->_weatherAPIURL));
  strlcpy(this->_weatherAPIURL, CDC_DEFAULT_WEATHERAPIURL, sizeof(this->_weatherAPIURL));

  memset(this->_weatherIconURL, '\0', sizeof(this->_weatherIconURL));
  strlcpy(this->_weatherIconURL, CDC_DEFAULT_WEATHERICONURL, sizeof(this->_weatherIconURL));

  memset(this->_weatherAPIKey, '\0', sizeof(this->_weatherAPIKey));
  strlcpy(this->_weatherAPIKey, CDC_DEFAULT_WEATHERAPIKEY, sizeof(this->_weatherAPIKey));
  
  this->_queryWeatherEvery = CDC_DEFAULT_WEATHER_QUERY_EVERY;
  this->_controllerUpdateEvery = CDC_DEFAULT_UPDATE_OUTPUT_EVERY;

  this->_statusBlinkEvery = CDC_DEFAULT_STATUS_BLINK;

  memset(this->_NTPServer, '\0', sizeof(this->_NTPServer));
  strlcpy(this->_NTPServer, CDC_DEFAULT_NTPSERVER, sizeof(this->_NTPServer));

  memset(this->_currentWeather.weatherUpdateLocation, '\0', sizeof(this->_currentWeather.weatherUpdateLocation));
  strlcpy(this->_currentWeather.weatherUpdateLocation, CDC_NA, sizeof(this->_currentWeather.weatherUpdateLocation));

  memset(this->_currentWeather.lastWeatherQueryTime, '\0', sizeof(this->_currentWeather.lastWeatherQueryTime));
  strlcpy(this->_currentWeather.lastWeatherQueryTime, CDC_NA, sizeof(this->_currentWeather.lastWeatherQueryTime));

  memset(this->_currentWeather.lastWeatherQueryDate, '\0', sizeof(this->_currentWeather.lastWeatherQueryDate));
  strlcpy(this->_currentWeather.lastWeatherQueryDate, CDC_NA, sizeof(this->_currentWeather.lastWeatherQueryDate));

  memset(this->_currentWeather.lastWeatherUpdateTime, '\0', sizeof(this->_currentWeather.lastWeatherUpdateTime));
  strlcpy(this->_currentWeather.lastWeatherUpdateTime, CDC_NA, sizeof(this->_currentWeather.lastWeatherUpdateTime));

  memset(this->_currentWeather.lastWeatherUpdateDate, '\0', sizeof(this->_currentWeather.lastWeatherUpdateDate));
  strlcpy(this->_currentWeather.lastWeatherUpdateDate, CDC_NA, sizeof(this->_currentWeather.lastWeatherUpdateDate));

  this->_currentWeather.ambientTemperature = 0.0;
  this->_currentWeather.humidity = 0.0;
  this->_currentWeather.dewPoint = 0.0;
  memset(this->_currentWeather.weatherDescription, '\0', sizeof(this->_currentWeather.weatherDescription));
  memset(this->_currentWeather.weatherIcon, '\0', sizeof(this->_currentWeather.weatherIcon));

  memset(this->_httpOTAURL, '\0', sizeof(this->_httpOTAURL));
  strlcpy(this->_httpOTAURL, CDC_DEFAULT_HTTP_OTA_URL, sizeof(this->_httpOTAURL));

  this->resetConfigUpdated();
  
  LOG_DEBUG("_loadDefaults", "Loading CDCSetup defaults: completed.");
}

//********************************************************************************************
bool CDCSetup::LoadConfig(void)
{
  // Load CheapoDC config from file
#if ARDUINOJSON_VERSION_MAJOR>=7
	JsonDocument doc;
#else
	DynamicJsonDocument doc(1024);
#endif
  bool upgradeConfigFile = false;
  String upgradeString;


  if (!theDController)
  {
    LOG_ERROR("LoadConfig", "theDController object is NOT defined.");
    return false;
  }

  File file = LittleFS.open(CDC_CONFIG_FILE, FILE_READ);
  if (!file)
  {
    LOG_ERROR("LoadConfig", "Failed to open: " << CDC_CONFIG_FILE);
    return false;
  }
  else
  {
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
      LOG_ERROR("LoadConfig", "Failed to deserialize: " << CDC_CONFIG_FILE);
      return false;
    }
  }

  JsonObject config = doc.as<JsonObject>();

#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
  clearCmdPostProcessQueue();
#endif

  for (JsonPair kv : config)
  {
    LOG_DEBUG("LoadConfig", "From config file: " << file.name() << " Key: " << kv.key().c_str() << " Value: " << kv.value().as<const char *>());
    if (!kv.value().isNull())
    {
      std::map<std::string, CDCommand>::const_iterator pos = CDCCommands.find(kv.key().c_str());
      if (pos != CDCCommands.end()) 
      {
        bool loadValue = pos->second.saveToConfig;

        if (loadValue)
          setCmdProcessor(String(kv.key().c_str()), String(kv.value().as<const char *>()), LOADCONFIG);
      }
      if (String(kv.key().c_str()).compareTo(String("WAPI")) == 0)
      {
        upgradeConfigFile = true;
        upgradeString = String(kv.value().as<const char *>());
      }
    }
    else
    {
      LOG_ALERT("LoadConfig", "Config file value for: " << kv.key().c_str() << " is nullptr");
    }
  }

  // Check for V1 config file with Source API value and use it to import and set the Weather Source
  if (upgradeConfigFile) {
    LOG_DEBUG("LoadConfig", "Config file upgrade required, found WAPI: " << upgradeString.c_str())
    if (upgradeString.indexOf("openweathermap") >= 0)
    { 
      this->setWeatherSource(OPENWEATHERSOURCE);
      LOG_ALERT("LoadConfig", "V1 Config found setting WS to OPENWEATHER.");
    }
    else if (upgradeString.indexOf("open-meteo") >= 0)
    {
      this->setWeatherSource(OPENMETEO);
      LOG_ALERT("LoadConfig", "V1 Config found setting WS to OPENMETEO.");
    }
    this->setConfigUpdated();
    this->SaveConfig();

  }
  
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
  runCmdPostProcessQueue();
#endif
  LOG_DEBUG("LoadConfig", "Load config complete");
  this->resetConfigUpdated();
  file.close();
  return true;
}

//********************************************************************************************
bool CDCSetup::_connectWiFi(void)
{
  int count = 0;
  int statusLEDEvery = this->getStatusBlinkEvery();

  this->blinkStatusLEDEvery(0);
  this->statusLEDOn();
  WiFi.setHostname(this->_wifiConfig.hostname);
  WiFi.setSleep(false); // Disable sleep since there were reports it may affect mDNS
  WiFi.begin(this->_wifiConfig.ssid, this->_wifiConfig.password);
  LOG_DEBUG("_connectWiFi", "Connect to WiFi:" << this->_wifiConfig.ssid);
  
  #ifdef CDC_ENABLE_WIFI_TX_POWER_MOD
  if (!WiFi.setTxPower(CDC_ENABLE_WIFI_TX_POWER_MOD)) {
    LOG_ERROR("_connectWiFi", "WiFi TX Power could not be set to: " << CDC_ENABLE_WIFI_TX_POWER_MOD);
  } else {
    char buffer[16];
    sprintf(buffer, "%.1f dbm", WiFi.getTxPower()/4.0);
    LOG_ALERT("_connectWiFi", "WiFi TX Power set to: " << String(buffer));
  }
  #endif // CDC_ENABLE_WIFI_TX_POWER_MOD

  // Wait for connection and attempt _wifiConfig.connectAttempts times
  while ((WiFi.status() != WL_CONNECTED) && (count < this->_wifiConfig.connectAttempts))
  {
    count++;
    delay(500);
    this->blinkStatusLED();
    LOG_DEBUG("_connectWiFi", "..wait for connect");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    LOG_ERROR("_connectWiFi", "Could not connect to AP: " << this->_wifiConfig.ssid);
    this->statusLEDOn();
    this->blinkStatusLEDEvery(statusLEDEvery);
    return false;
  }


  strlcpy(this->_IPAddress, WiFi.localIP().toString().c_str(), sizeof(this->_IPAddress));
  LOG_ALERT("_connectWiFi", this->_wifiConfig.hostname << ".local connected to: " << this->_wifiConfig.ssid << " with IP address: " << this->_IPAddress);

  /*use mdns for host name resolution*/
  if (!MDNS.begin(this->_wifiConfig.hostname))
  { // http://'host'.local
    LOG_ERROR("_connectWiFi", "Error setting up MDNS responder!");
  }
  else
  {
    LOG_ALERT("_connectWiFi", "mDNS responder started");
  }

  this->statusLEDOff();
  this->blinkStatusLEDEvery(statusLEDEvery);

  return true;
}

//********************************************************************************************
bool CDCSetup::setupWiFi(void)
{
  // Connect to WiFi network
  bool useDefaults = false;
  int count = 0;

  this->_inWiFiAPMode = false;
  // Try to load WiFi config from json file CDC_WIFI_CONFIG
#if ARDUINOJSON_VERSION_MAJOR>=7
  JsonDocument doc;
#else
  StaticJsonDocument<512> doc;
#endif

  File file = LittleFS.open(CDC_WIFI_CONFIG,FILE_READ);
  if (!file)
  {
    LOG_ERROR("setupWiFi", "Failed to open: " << CDC_WIFI_CONFIG);
    useDefaults = true;
  }
  else
  {
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
      LOG_ERROR("setupWiFi", "Failed to deserialize: " << CDC_WIFI_CONFIG);
      useDefaults = true;
    }
  }

  if (useDefaults)
  {
    LOG_DEBUG("setupWiFi", "Using WiFi Defaults");
    strlcpy(this->_wifiConfig.hostname, CDC_DEFAULT_HOSTNAME, sizeof(this->_wifiConfig.hostname));
    this->_wifiConfig.connectAttempts, CDC_DEFAULT_WIFI_CONNECTATTEMPTS;
    this->_wifiConfig.tryAPs, CDC_DEFAULT_WIFI_TRYAPS;
    strlcpy(this->_wifiConfig.ssid, CDC_DEFAULT_WIFI_SSID, sizeof(this->_wifiConfig.ssid));
    strlcpy(this->_wifiConfig.password, CDC_DEFAULT_WIFI_PASSWORD, sizeof(this->_wifiConfig.password));

    while (count < this->_wifiConfig.tryAPs)
    {
      if (this->_connectWiFi())
      {
        this->blinkStatusLEDEvery(CDC_DEFAULT_STATUS_BLINK);
        this->statusLEDOn();
        return true;
      }
      count++;
    }
  }
  else
  {

    strlcpy(this->_wifiConfig.hostname, doc["hostname"] | CDC_DEFAULT_HOSTNAME, sizeof(this->_wifiConfig.hostname));
    this->_wifiConfig.connectAttempts = (doc["connectAttempts"] | CDC_DEFAULT_WIFI_CONNECTATTEMPTS);
    this->_wifiConfig.tryAPs = (doc["tryAPs"] | CDC_DEFAULT_WIFI_TRYAPS);

    LOG_DEBUG("setupWiFi", "hostname:" << this->_wifiConfig.hostname);
    LOG_DEBUG("setupWiFi", "Connection attempts:" << this->_wifiConfig.connectAttempts);
    LOG_DEBUG("setupWiFi", "Try APs:" << this->_wifiConfig.tryAPs);

    while (count < this->_wifiConfig.tryAPs)
    {
      for (JsonObject wifi_item : doc["wifi"].as<JsonArray>())
      {
        strlcpy(this->_wifiConfig.ssid, wifi_item["ssid"] | CDC_DEFAULT_WIFI_SSID, sizeof(this->_wifiConfig.ssid));
        strlcpy(this->_wifiConfig.password, wifi_item["password"] | CDC_DEFAULT_WIFI_PASSWORD, sizeof(this->_wifiConfig.password));
        LOG_DEBUG("setupWiFi", "WiFi " << count << "ssid " << this->_wifiConfig.ssid);
        LOG_DEBUG("setupWiFi", "WiFi " << count << "password " << this->_wifiConfig.password);

        if (this->_connectWiFi())
        {
          this->blinkStatusLEDEvery(CDC_DEFAULT_STATUS_BLINK);
          this->statusLEDOn();
          return true;
        }
      }
      count++;
    }
  }

  // Could not connect to WiFi. Set up in AP mode
  WiFi.mode(WIFI_MODE_NULL);
  WiFi.setHostname(this->_wifiConfig.hostname);
  WiFi.softAP(CDC_DEFAULT_WIFI_AP_SSID, CDC_DEFAULT_WIFI_AP_PASSWORD);
  this->_inWiFiAPMode = true;
  strlcpy(this->_IPAddress, WiFi.softAPIP().toString().c_str(), sizeof(this->_IPAddress));
  LOG_ALERT("setupWiFi", "WiFi in AP mode SSID " << CDC_DEFAULT_WIFI_AP_SSID << " with IP address: " << this->_IPAddress);

  /*use mdns for host name resolution*/
  if (!MDNS.begin(this->_wifiConfig.hostname))
  { // http://'host'.local
    LOG_ERROR("setupWiFi", "Error setting up MDNS responder!");
  }
  else
  {
    LOG_ALERT("setupWiFi", "mDNS responder started");
  }

  this->blinkStatusLEDEvery(CDC_WIFI_AP_STATUS_BLINK);
  this->statusLEDOn();

  return false;
}

// status LED
void CDCSetup::setStatusLEDPin(int pin)
{
  if (pin == -1)
  {
    this->_statusLEDPin = -1;
  }
  else if ((pin >= 0) && (pin <= 39))
  {
    for (int i = 0; i < MAX_CONTROLLER_PINS; i++)
    {
      if (pin == theDController->getControllerPinPin(i))
      {
        LOG_ERROR("setStatusLEDPin", "Status LED Pin is already in use: " << this->_statusLEDPin);
        return;
      }
    }
    this->_statusLEDPin = pin;
    pinMode(this->_statusLEDPin, OUTPUT);
  }
}

void CDCSetup::setStatusLEDHigh( int highValue )
{
  if ((highValue == 0) || (highValue == 1))
  {
    this->_statusLEDHigh = (bool) highValue;
  }
  else
  {
    LOG_ERROR("setStatusLEDHigh", "Invalid value for LED High: " << highValue);
  }
}


void CDCSetup::_writeStatusLED(uint8_t value)
{
  if (this->_statusLEDPin >= 0)
  {
    int writeValue = (value == this->_statusLEDHigh);
    LOG_DEBUG("_writeStatusLED", "LedHIGH: " << this->_statusLEDHigh << " Write Value: " << value << " Written: " << writeValue);
    digitalWrite(this->_statusLEDPin, writeValue);
  }
}

int CDCSetup::_readStatusLED(void)
{
  if (this->_statusLEDPin >= 0)
  {
    int readValue = (digitalRead(this->_statusLEDPin) == this->_statusLEDHigh);
    return readValue;
  }
  return (this->_statusLEDHigh==1) ? 0 : 1;
}

void CDCSetup::blinkStatusLED()
{
  if (this->_statusLEDEnabled)
    this->_writeStatusLED(!this->_readStatusLED());
}

void CDCSetup::statusLEDOn()
{
  this->_statusLEDEnabled = true;
  this->_writeStatusLED(HIGH);
  this->statusLEDDelay(RESET_DELAY);
}

void CDCSetup::statusLEDOff()
{
  this->_statusLEDEnabled = false;
  this->_writeStatusLED(LOW);
}

void CDCSetup::blinkStatusLEDEvery(int blinkEvery)
{
  if ((blinkEvery >= 0) && (blinkEvery <= 1000))
  {
    this->_statusBlinkEvery = blinkEvery;
    if (blinkEvery == 0)
      this->statusLEDOff();
    else
      this->statusLEDOn();
  }
};

int CDCSetup::getStatusBlinkEvery() { return this->_statusBlinkEvery; };

void CDCSetup::statusLEDDelay(statusLEDDelayCmd cmd)
{
  switch (cmd)
  {
  case INC_DELAY:
    if ((this->_statusLEDDelayEnabled) && (this->_statusLEDDelay < CDC_STATUS_LED_DELAY))
      this->_statusLEDDelay = this->_statusLEDDelay + 1;
    break;
  case DEC_DELAY:
    if ((this->_statusLEDDelayEnabled) && (this->_statusLEDDelay > 0))
    {
      this->_statusLEDDelay = this->_statusLEDDelay - 1;
      if (this->_statusLEDDelay <= 0)
        this->statusLEDOff();
    }
    break;
  case RESET_DELAY:
    this->_statusLEDDelay = CDC_STATUS_LED_DELAY;
    this->_statusLEDDelayEnabled = true;
    break;
  case DISABLE_DELAY:
    this->_statusLEDDelay = CDC_STATUS_LED_DELAY;
    this->_statusLEDDelayEnabled = false;
    break;
  case ZERO_DELAY:
    this->_statusLEDDelay = 0;
    break;
  default:
    break;
  }
} 

bool CDCSetup::statusLEDDelayComplete()
{
  return ((this->_statusLEDDelayEnabled) && (this->_statusLEDDelay <= 0));
}

// Weather and Location functions
void CDCSetup::setWeatherQueryEvery(int queryEvery)
{
  LOG_DEBUG("setWeatherQueryEvery", "queryEvery = " << queryEvery);

  if ((queryEvery >= 0) && (queryEvery <= 59))
    this->_queryWeatherEvery = queryEvery;

  LOG_DEBUG("setWeatherQueryEvery", "this->_queryWeatherEvery = " << this->_queryWeatherEvery);
}

void CDCSetup::setLocationLatitude(float latitude)
{
  String newLatitude = String(latitude);

  if (strcmp(newLatitude.c_str(), this->_location.latitude) != 0)
  {
    if ((latitude >= -90.0) & (latitude <= 90.0))
    {
      strlcpy(this->_location.latitude, newLatitude.c_str(), sizeof(this->_location.latitude));
    }
    else
    {
      LOG_ERROR("setLocationLatitude", "New latitude out of range -90 to 90: " << latitude);
    }
  }
}

void CDCSetup::setLocationLongitude(float longitude)
{
  String newLongitude = String(longitude);

  if (strcmp(newLongitude.c_str(), this->_location.longitude) != 0)
  {
    if ((longitude >= -180.0) & (longitude <= 180.0))
    {
      strlcpy(this->_location.longitude, newLongitude.c_str(), sizeof(this->_location.longitude));
    }
    else
    {
      LOG_ERROR("setLocationLongitude", "New longitude out of range -180 to 180: " << longitude);
    }
  }
}

void CDCSetup::setLocationTimeZone(int timezone)
{

  // In seconds for GMT-12 to GMT+14
  if ((timezone >= (-12 * 3600)) & (timezone <= (14 * 3600)))
  {
    this->_location.timezone = timezone;
    // Apply Timezone change
    configTime(this->getLocation().timezone, this->getLocation().DSTOffset, this->getNTPServerURL());
    now();
  }
  else
  {
    LOG_ERROR("setLocationTimeZone", "New Time Zone out of range GMT-12 to GNT+14 in seconds: " << timezone);
  }
}

void CDCSetup::setLocationDST(int DSTOffset)
{

  // In seconds for plus or minus 3600 seconds
  if ((DSTOffset >= -3600) & (DSTOffset <= 3600))
  {
    this->_location.DSTOffset = DSTOffset;
    // Apply DST change
    configTime(this->getLocation().timezone, this->getLocation().DSTOffset, this->getNTPServerURL());
    now();
  }
  else
  {
    LOG_ERROR("setLocationTimeZone", "New DST offset out of range -3600 to +3600 seconds: " << DSTOffset);
  }
}

void CDCSetup::setUpdateOutputEvery(int updateEvery)
{
  LOG_DEBUG("setUpdateOutputEvery", "updateEvery = " << updateEvery);

  if ((updateEvery >= 0) && (updateEvery <= 59))
    this->_controllerUpdateEvery = updateEvery;

  LOG_DEBUG("setUpdateOutputEvery", "this->_controllerUpdateEvery = " << this->_controllerUpdateEvery);
}

void CDCSetup::setAmbientTemperatureExternal(float temperature)
{
  this->_ambientTemperatureExternal = temperature;
}

void CDCSetup::setWeatherSource(weatherSource source)
{
    if ((source < OPENMETEO) || (source > EXTERNALSOURCE))
    {
        LOG_ALERT("setWeatherSource", "Invalid weather source: " << source << " set to default " << (weatherSource)CDC_DEFAULT_WEATHERSOURCE);
        this->_currentWeatherSource = (weatherSource)CDC_DEFAULT_WEATHERSOURCE;
    }
    else
    {
        this->_currentWeatherSource = source;
        memset(this->_weatherAPIURL, '\0', sizeof(this->_weatherAPIURL));
        memset(this->_weatherIconURL, '\0', sizeof(this->_weatherIconURL));
        if (source ==  OPENMETEO)
        {
          strlcpy(this->_weatherAPIURL, CDC_OPENMETEO_APIURL, sizeof(this->_weatherAPIURL));
          strlcpy(this->_weatherIconURL, CDC_OPENMETEO_ICONURL, sizeof(this->_weatherIconURL));
        }
        else if (source == OPENWEATHERSOURCE)
        {
          strlcpy(this->_weatherAPIURL, CDC_OPENWEATHER_APIURL, sizeof(this->_weatherAPIURL));
          strlcpy(this->_weatherIconURL, CDC_OPENWEATHER_ICONURL, sizeof(this->_weatherIconURL));
        }
        else
        {
          strlcpy(this->_weatherAPIURL, CDC_EXTERNALSOURCE_APINURL, sizeof(this->_weatherAPIURL));
          strlcpy(this->_weatherIconURL, CDC_EXTERNALSOURCE_ICONURL, sizeof(this->_weatherIconURL));
          memset(this->_currentWeather.weatherDescription, '\0', sizeof(this->_currentWeather.weatherDescription));
          strlcpy(this->_currentWeather.weatherDescription, CDC_EXTERNALSOURCE_DESC, sizeof(this->_currentWeather.weatherDescription));
          memset(this->_currentWeather.weatherIcon, '\0', sizeof(this->_currentWeather.weatherIcon));
          strlcpy(this->_currentWeather.weatherIcon, CDC_NA, sizeof(this->_currentWeather.weatherIcon));
          strlcpy(this->_currentWeather.lastWeatherQueryTime, CDC_NA, sizeof(this->_currentWeather.lastWeatherQueryTime));
          strlcpy(this->_currentWeather.lastWeatherQueryDate, CDC_NA, sizeof(this->_currentWeather.lastWeatherQueryDate));
          strlcpy(this->_currentWeather.weatherUpdateLocation, CDC_EXTERNALSOURCE_LOCATION_NAME, sizeof(this->_currentWeather.weatherUpdateLocation));

        }
    }
    LOG_DEBUG("setWeatherSource", "Set weather source set to: " << this->_currentWeatherSource);
}

void CDCSetup::calculateAndSetDewPoint()
{
  float alphaDP;

  if ((this->_currentWeather.ambientTemperature == 0.0) && (this->_currentWeather.humidity == 0.0))
  {
    this->_currentWeather.dewPoint = 0.0;
  }
  else
  {
    alphaDP = log(this->_currentWeather.humidity / 100) + ((CDC_MC_A * this->_currentWeather.ambientTemperature) / (CDC_MC_B + this->_currentWeather.ambientTemperature));
    this->_currentWeather.dewPoint = (CDC_MC_B * alphaDP) / (CDC_MC_A - alphaDP);
  }
  LOG_DEBUG("calculateAndSetDewPoint", "Dew Point calculated and set to: " << this->_currentWeather.dewPoint);
}

void CDCSetup::setAmbientTemperatureWQ(float temperature)
{
  if (this->getCurrentWeatherSource() != EXTERNALSOURCE)
  {
    LOG_ERROR("setAmbientTemperatureWQ","Set temperature requires Weather Source set to EXTERNALSOURCE.");
  }
  else
  {
    bool calculateDewPoint = (this->_currentWeather.ambientTemperature != temperature);
    this->_currentWeather.ambientTemperature = temperature;
    strlcpy(this->_currentWeather.lastWeatherUpdateTime, this->getTime().c_str(), sizeof(this->_currentWeather.lastWeatherUpdateTime));
    strlcpy(this->_currentWeather.lastWeatherUpdateDate, this->getDate().c_str(), sizeof(this->_currentWeather.lastWeatherUpdateDate));
    strlcpy(this->_currentWeather.lastWeatherQueryTime, CDC_NA, sizeof(this->_currentWeather.lastWeatherQueryTime));
    strlcpy(this->_currentWeather.lastWeatherQueryDate, CDC_NA, sizeof(this->_currentWeather.lastWeatherQueryDate));
    LOG_DEBUG("setAmbientTemperatureWQ", "Temperature set to: " << temperature);
    if (calculateDewPoint)
      this->calculateAndSetDewPoint();
  }
  return;
}

void CDCSetup::setHumidity(float humidity)
{
  if (this->getCurrentWeatherSource() != EXTERNALSOURCE)
  {
    LOG_ERROR("setHumidity","Set humidity requires Weather Source set to EXTERNALSOURCE.");
  }
  else
  {
    bool calculateDewPoint = (this->_currentWeather.humidity != humidity);
    this->_currentWeather.humidity = humidity;
    strlcpy(this->_currentWeather.lastWeatherUpdateTime, this->getTime().c_str(), sizeof(this->_currentWeather.lastWeatherUpdateTime));
    strlcpy(this->_currentWeather.lastWeatherUpdateDate, this->getDate().c_str(), sizeof(this->_currentWeather.lastWeatherUpdateDate));
    strlcpy(this->_currentWeather.lastWeatherQueryTime, CDC_NA, sizeof(this->_currentWeather.lastWeatherQueryTime));
    strlcpy(this->_currentWeather.lastWeatherQueryDate, CDC_NA, sizeof(this->_currentWeather.lastWeatherQueryDate));
    LOG_DEBUG("setHumidity", "Humidity set to: " << humidity);
    if (calculateDewPoint)
      this->calculateAndSetDewPoint();
  }
  return;
}

// Time and Date function
String CDCSetup::getDateTime()
{
  char dtBuff[CDC_MAX_DATETIME_STRING] = {};
  struct tm lTime;

  getLocalTime(&lTime);
  
  sprintf(dtBuff, CDC_DATE_TIME, mydayShortStr[lTime.tm_wday], mymonthShortStr[lTime.tm_mon], lTime.tm_mday, (lTime.tm_year + 1900), lTime.tm_hour, lTime.tm_min, second(lTime.tm_sec));

  return String(dtBuff);
}

String CDCSetup::getDateTime( time_t t )
{
  char dtBuff[CDC_MAX_DATETIME_STRING];
  struct tm lTime;
  tmElements_t lTElements;
  time_t local_t = t + this->_location.timezone;

  getLocalTime(&lTime);

  if (lTime.tm_isdst == 1)
    local_t = local_t + this->_location.DSTOffset;

  breakTime(local_t, lTElements);
  
  sprintf(dtBuff, CDC_DATE_TIME, mydayShortStr[lTElements.Wday-1], mymonthShortStr[lTElements.Month-1], lTElements.Day, (lTElements.Year+1970), lTElements.Hour, lTElements.Hour, second(lTElements.Second));

return String(dtBuff);
}
String CDCSetup::getDate( )
{
  char dtBuff[CDC_MAX_DATETIME_STRING] = {};
  struct tm lTime;

  getLocalTime(&lTime);
  
  sprintf(dtBuff, CDC_DATE, mydayShortStr[lTime.tm_wday], mymonthShortStr[lTime.tm_mon], lTime.tm_mday, (lTime.tm_year + 1900));

return String(dtBuff);
}

String CDCSetup::getDate( time_t t )
{
  char dtBuff[CDC_MAX_DATETIME_STRING] = {};
  struct tm lTime;
  tmElements_t lTElements;
  time_t local_t = t + this->_location.timezone;

  getLocalTime(&lTime);

  if (lTime.tm_isdst == 1)
    local_t = local_t + this->_location.DSTOffset;
  
  breakTime(local_t, lTElements);
  
  sprintf(dtBuff, CDC_DATE, mydayShortStr[lTElements.Wday-1], mymonthShortStr[lTElements.Month-1], lTElements.Day, (lTElements.Year+1970));

return String(dtBuff);
}

String CDCSetup::getTime( )
{
  char dtBuff[CDC_MAX_DATETIME_STRING];
  struct tm lTime;

  getLocalTime(&lTime);
  
  sprintf(dtBuff, CDC_TIME, lTime.tm_hour, lTime.tm_min, second(lTime.tm_sec));

return String(dtBuff);
}

String CDCSetup::getTime( time_t t )
{
  char dtBuff[CDC_MAX_DATETIME_STRING];
  struct tm lTime;
  tmElements_t lTElements;
  time_t local_t = t + this->_location.timezone;

  getLocalTime(&lTime);
  
  if (lTime.tm_isdst == 1)
    local_t = local_t + this->_location.DSTOffset;

  breakTime(local_t, lTElements);
  sprintf(dtBuff, CDC_TIME, lTElements.Hour, lTElements.Minute, second(lTElements.Second));

return String(dtBuff);
}

int CDCSetup::getSecond()
{
  struct tm lTime;

  getLocalTime(&lTime);

  return lTime.tm_sec;
}

int CDCSetup::getMinute()
{
  struct tm lTime;

  getLocalTime(&lTime);

  return lTime.tm_min;
}

// Password Hash - MD5 (32 characters)
void    CDCSetup::setPasswordHash( String pwdHash ) 
{
  strlcpy(this->_passwordHash, pwdHash.c_str(), sizeof(this->_passwordHash));
}

bool CDCSetup::backupConfig() {
  File file = LittleFS.open(CDC_WIFI_CONFIG,FILE_READ);
  if (!file)
  {
    LOG_ERROR("backupConfig", "Failed to open: " << CDC_WIFI_CONFIG);
    return false;
  }
  else
  {
    DeserializationError error = deserializeJson(this->_backupWiFiConfig, file);
    if (error)
    {
      LOG_ERROR("backupConfig", "Failed to deserialize: " << CDC_WIFI_CONFIG);
      return false;
    }

  }
  file.close();
  LittleFS.end();
  return true;
}

bool CDCSetup::restoreConfig() {
  if (!LittleFS.begin(false))
  {
    return false;
  }

  File file = LittleFS.open(CDC_WIFI_CONFIG,FILE_WRITE);
  if (serializeJson(this->_backupWiFiConfig, file) == 0)
  {
    LOG_ERROR("restoreConfig", "Failed to serialize config to: " << CDC_WIFI_CONFIG);
    file.close();
    return false;
  }
  file.close();

  this->setConfigUpdated();
  return this->SaveConfig();
}

bool CDCSetup::saveWiFiConfig( String wifiConfigJson ) {
  JsonDocument doc, filter;
  DeserializationError error = deserializeJson(doc, wifiConfigJson, DeserializationOption::Filter(filter));
  if (error)
  {
    LOG_ERROR("saveWiFiConfig", "Failed to deserialize and parse: " << wifiConfigJson.c_str());
    return false;
  }
  error = deserializeJson(this->_backupWiFiConfig, wifiConfigJson);
  if (error)
  {
    LOG_ERROR("saveWiFiConfig", "Failed to deserialize: " << wifiConfigJson.c_str());
    return false;
  } else {
    File file = LittleFS.open(CDC_WIFI_CONFIG,FILE_WRITE);
    if (!file)
    {
      LOG_ERROR("saveWiFiConfig", "Failed to open: " << CDC_WIFI_CONFIG);
      return false;
    }
    if (!file.print(wifiConfigJson.c_str()))
    {
      LOG_ERROR("saveWiFiConfig", "Failed to write to config to: " << wifiConfigJson.c_str());
      file.close();
      return false;
    }
    file.close();
  }
  
  return true;  
}