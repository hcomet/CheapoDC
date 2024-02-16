// ******************************************************************
// CheapoDC Setup and configuration
// Cheap and easy Dew Controller
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************

#include <Arduino.h>
#include "CDCdefines.h"
//#include "time.h"
#include <ArduinoJson.h>
#include <EasyLogger.h>
#include "FS.h"
#include <LittleFS.h>
#include <TimeLib.h>
#include <ESP32Time.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>

#include "CDCvars.h"
#include "CDCSetup.h"
#include "CDController.h"
#include "CDCommands.h"

#define FORMAT_LITTLEFS_IF_FAILED true

//** TimeLib dayShortStr() not working so use my own:
char* mydayShortStr[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};



//********************************************************************************************
String httpGETRequest(const char* serverName, int retryCount = 0) {
  HTTPClient http;
  String payload = "";
  int thisRetryCount = retryCount;

  LOG_DEBUG("httpGETRequest", "URL:" << serverName << " retry: " << thisRetryCount);

  if (thisRetryCount <= CDC_DEFAULT_HTTP_REQ_RETRY) {
  
    // Your IP address with path or Domain name with URL path 
    if (!http.begin(serverName)) {
      LOG_ERROR("httpGETRequest", "http.Begin failed: " << serverName << " on try: " << (thisRetryCount + 1));
      payload = httpGETRequest(serverName, (thisRetryCount+1) );
    } else {  
      
      // Send HTTP POST request
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        LOG_DEBUG("httpGETRequest", "HTTP response code: " << httpResponseCode);
        payload = http.getString();
      } else {
        LOG_ERROR("httpGETRequest", "HTTP response error code: " << httpResponseCode << " on try: " << (thisRetryCount + 1));
        payload = httpGETRequest(serverName, (thisRetryCount+1) );
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
    time_t  tempTime = 0;
    time_t  tempTZ = 0;
    char httpRequestURL[256];
    DynamicJsonDocument doc(1024);
    float alphaDP;

    if (theSetup->getInWiFiAPMode()) {
      LOG_ERROR("queryWeather", "CheapoDC in Access Point mode. Cannot do a weather query.");
      return false;
    }

    sprintf( httpRequestURL, this->_weatherAPIURL, this->_location.latitude, this->_location.longitude, this->_weatherAPIKey, CDC_DEFAULT_WEATHERUNITS);

    DeserializationError error = deserializeJson(doc, httpGETRequest( httpRequestURL ));
    if (error) {
        LOG_ERROR("queryWeather", "Deserialization failed: " << error.c_str());
        return false;
    }

    JsonObject main = doc["main"];
    LOG_DEBUG("queryWeather","Last query time: " << this->getLastWeatherQueryTime());
    LOG_DEBUG("queryWeather","Last query date: " << this->getLastWeatherQueryDate());
    String date = theTime->getDate();
    String time = theTime->getTime();
    
    date.toCharArray(this->_currentWeather.lastWeatherQueryDate, sizeof(this->_currentWeather.lastWeatherQueryDate));
    time.toCharArray(this->_currentWeather.lastWeatherQueryTime, sizeof(this->_currentWeather.lastWeatherQueryTime));
    
    LOG_DEBUG("queryWeather","New query time: " << this->getLastWeatherQueryTime());
    LOG_DEBUG("queryWeather","ESP32 date: <" << theTime->getDateTime() << ">");
    LOG_DEBUG("queryWeather","New query date: " << this->getLastWeatherQueryDate());

    // Do Date-Time items which are in LINUX UTC Time
    tempTime = doc["dt"] | 0;
    tempTZ = doc["timezone"] | 0;
    tempTime = tempTime + tempTZ;

    if (tempTime != 0 ) {
      sprintf(this->_currentWeather.lastWeatherUpdateTime, CDC_TIME, hour(tempTime),minute(tempTime),second(tempTime));
      sprintf(this->_currentWeather.lastWeatherUpdateDate, CDC_DATE, mydayShortStr[weekday(tempTime)-1], monthShortStr(month(tempTime)), day(tempTime),year(tempTime));
    } else {
      strlcpy(this->_currentWeather.lastWeatherUpdateTime, CDC_NA, sizeof(this->_currentWeather.lastWeatherUpdateTime));
      strlcpy(this->_currentWeather.lastWeatherUpdateDate, CDC_NA, sizeof(this->_currentWeather.lastWeatherUpdateDate));
    }

    LOG_ALERT("queryWeather", "Last Update: " << this->_currentWeather.lastWeatherUpdateDate << " - " << this->_currentWeather.lastWeatherUpdateTime);

    tempTime = doc["sys"]["sunrise"] | 0;
    tempTime = tempTime + tempTZ;
    if (tempTime != 0 ) 
      sprintf(this->_currentWeather.sunrise, CDC_TIME, hour(tempTime),minute(tempTime),second(tempTime));
    else
      strlcpy(this->_currentWeather.sunrise, CDC_NA, sizeof(this->_currentWeather.sunrise));

    tempTime = doc["sys"]["sunset"] | 0;
    tempTime = tempTime - tempTZ;
    if (tempTime != 0 ) 
      sprintf(this->_currentWeather.sunset, CDC_TIME, hour(tempTime),minute(tempTime),second(tempTime));
    else
      strlcpy(this->_currentWeather.sunset, CDC_NA, sizeof(this->_currentWeather.sunset));

    // Get temperature, Humidity and calculate Dew point

    this->_currentWeather.ambientTemperature = main["temp"] | 0.0;
    this->_currentWeather.humidity = main["humidity"] | 0.0;
    if ((this->_currentWeather.ambientTemperature == 0.0) && (this->_currentWeather.humidity == 0.0))
    {
      this->_currentWeather.dewPoint = 0.0;
    }
    else
    {
      alphaDP = log(this->_currentWeather.humidity / 100) + ((CDC_MC_A * this->_currentWeather.ambientTemperature) / (CDC_MC_B + this->_currentWeather.ambientTemperature));
      this->_currentWeather.dewPoint = (CDC_MC_B * alphaDP) / (CDC_MC_A - alphaDP);
    }

    this->_currentWeather.minTemperature = main["temp_min"] | 0.0;
    this->_currentWeather.maxTemperature = main["temp_max"] | 0.0;
    this->_currentWeather.pressure = main["pressure"] | 0;
    this->_currentWeather.visibility = doc["visibility"] | 0;
    this->_currentWeather.windSpeed = doc["wind"]["speed"] | 0.0;
    this->_currentWeather.windDirection = doc["wind"]["deg"] | 0;
    this->_currentWeather.cloudCoverage = doc["clouds"]["all"] | 0;
     
    strlcpy(this->_currentWeather.weatherUpdateLocation, doc["name"] | CDC_NA, sizeof(this->_currentWeather.weatherUpdateLocation));
    String tempDescription = String(doc["weather"][0]["main"] | CDC_NA);
    tempDescription += String(" - ") + String(doc["weather"][0]["description"] | CDC_NA);
    tempDescription.toCharArray(this->_currentWeather.weatherDescription, sizeof(this->_currentWeather.weatherDescription));
    strlcpy(this->_currentWeather.weatherIcon, doc["weather"][0]["icon"] | CDC_NA, sizeof(this->_currentWeather.weatherIcon));
    
    LOG_DEBUG("queryWeather", "Current Weather: " << this->_currentWeather.lastWeatherQueryDate << " " << this->_currentWeather.lastWeatherQueryTime << " From: " << this->_currentWeather.weatherUpdateLocation );
    LOG_DEBUG("queryWeather", "    temperature: " << this->_currentWeather.ambientTemperature );
    LOG_DEBUG("queryWeather", "    humidity: " << this->_currentWeather.humidity );
    LOG_DEBUG("queryWeather", "    dewPoint: " << this->_currentWeather.dewPoint );
    LOG_DEBUG("queryWeather", "    pressure: " << this->_currentWeather.pressure );
    LOG_DEBUG("queryWeather", "    visibility: " << this->_currentWeather.visibility );
    LOG_DEBUG("queryWeather", "    windSpeed: " << this->_currentWeather.windSpeed );
    LOG_DEBUG("queryWeather", "    windDirection: " << this->_currentWeather.windDirection );
    LOG_DEBUG("queryWeather", "    cloudCoverage: " << this->_currentWeather.cloudCoverage );
    LOG_DEBUG("queryWeather", "    description: " << this->_currentWeather.weatherDescription );
    LOG_DEBUG("queryWeather", "    weatherIcon: " << this->_currentWeather.weatherIcon );
    LOG_DEBUG("queryWeather", "    sunrise: " << this->_currentWeather.sunrise );
    LOG_DEBUG("queryWeather", "    sunset: " << this->_currentWeather.sunset );
    LOG_DEBUG("queryWeather", "    data updated: " << this->_currentWeather.lastWeatherUpdateDate << " " << this->_currentWeather.lastWeatherUpdateTime );

    return true;

}

//********************************************************************************************
CDCSetup::CDCSetup(void)
{
  LOG_DEBUG("CDCSetup", "Starting setup");
  // Load defaults
  this->_loadDefaults();

  // initialize status LED as an output.
  pinMode(CDC_STATUS_LED, OUTPUT);
  this->blinkStatusLEDEvery(0);
  this->statusLEDOff();

  // Filesystem first
  LOG_DEBUG("CDCSETUP", "Setup LittleFS");
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
      LOG_ERROR("CDCSetup","LittleFS Mount Failed");
      return;
  }

  // Set up WiFi
  LOG_DEBUG("CDCSETUP", "Setup WiFi");
  if (!this->_setupWiFi()) {
      LOG_ERROR("CDCSetup","Wifi connection failure");
  }


  
  // Start RTC/NTP correct the timezone later after we load the CDC Config file  
  LOG_DEBUG("CDCSETUP", "Start RTC");
  theTime = new ESP32Time(0);
  

  strlcpy(this->_currentWeather.lastWeatherQueryTime, CDC_NA, sizeof(this->_currentWeather.lastWeatherQueryTime));
  strlcpy(this->_currentWeather.lastWeatherQueryDate, CDC_NA, sizeof(this->_currentWeather.lastWeatherQueryDate));
   
  return;
}

//********************************************************************************************
bool CDCSetup::SaveConfig(void)
{
  char cmdBuf[10] = "";
  char valueBuf[128] = "";

  if (getConfigUpdated()) {

    DynamicJsonDocument doc(1024);

    LOG_DEBUG("SaveConfig", "JsonDocument size: " << doc.capacity());

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

    resetConfigUpdated();
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
void CDCSetup::_loadDefaults(void) {

  LOG_DEBUG("_loadDefaults", "Loading CDCSetup defaults.");

  memset(this->_wifiConfig.hostname, '\0', sizeof(this->_wifiConfig.hostname));
  strlcpy(this->_wifiConfig.hostname, CDC_DEFAULT_HOSTNAME, sizeof(this->_wifiConfig.hostname));

  this->_wifiConfig.connectAttempts = CDC_DEFAULT_WIFI_CONNECTATTEMPTS;

  this->_wifiConfig.tryAPs = CDC_DEFAULT_WIFI_TRYAPS;

  memset(this->_wifiConfig.ssid , '\0', sizeof(this->_wifiConfig.ssid));
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

  memset(this->_weatherSource, '\0', sizeof(this->_weatherSource));
  strlcpy(this->_weatherSource, CDC_DEFAULT_WEATHERSOURCE, sizeof(this->_weatherSource));
  
  memset(this->_weatherAPIURL, '\0', sizeof(this->_weatherAPIURL));
  strlcpy(this->_weatherAPIURL, CDC_DEFAULT_WEATHERAPIURL, sizeof(this->_weatherAPIURL));
  
  memset(this->_weatherIconURL, '\0', sizeof(this->_weatherIconURL));
  strlcpy(this->_weatherIconURL, CDC_DEFAULT_WEATHERICONURL, sizeof(this->_weatherIconURL));
  
  memset(this->_weatherAPIKey, '\0', sizeof(this->_weatherAPIKey));
  strlcpy(this->_weatherAPIKey, CDC_DEFAULT_WEATHERAPIKEY, sizeof(this->_weatherAPIKey));

  this->_queryWeatherEvery = CDC_DEFAULT_WEATHER_QUERY;
  this->_controllerUpdateEvery = CDC_DEFAULT_UPDATE_OUTPUT_EVERY;

  this->_statusBlinkEvery = CDC_DEFAULT_STATUS_BLINK;

  memset(this->_NTPServer, '\0', sizeof(this->_NTPServer));
  strlcpy(this->_NTPServer, CDC_DEFAULT_NTPSERVER, sizeof(this->_NTPServer));
  
  memset(this->_currentWeather.weatherUpdateLocation, '\0', sizeof(this->_currentWeather.weatherUpdateLocation));
  memset(this->_currentWeather.lastWeatherQueryTime, '\0', sizeof(this->_currentWeather.lastWeatherQueryTime));
  memset(this->_currentWeather.lastWeatherQueryDate, '\0', sizeof(this->_currentWeather.lastWeatherQueryDate));
  memset(this->_currentWeather.lastWeatherUpdateTime, '\0', sizeof(this->_currentWeather.lastWeatherUpdateTime));
  memset(this->_currentWeather.lastWeatherUpdateDate, '\0', sizeof(this->_currentWeather.lastWeatherUpdateDate));

  this->_currentWeather.ambientTemperature = 0.0;
  this->_currentWeather.humidity = 0.0;
  this->_currentWeather.dewPoint = 0.0;
  this->_currentWeather.minTemperature = 0.0;
  this->_currentWeather.maxTemperature = 0.0;
  this->_currentWeather.pressure = 0;
  this->_currentWeather.visibility = 0;
  this->_currentWeather.windSpeed = 0.0;
  this->_currentWeather.windDirection = 0;
  this->_currentWeather.cloudCoverage = 0;
  memset(this->_currentWeather.weatherDescription, '\0', sizeof(this->_currentWeather.weatherDescription));
  memset(this->_currentWeather.weatherIcon, '\0', sizeof(this->_currentWeather.weatherIcon));
  memset(this->_currentWeather.sunrise, '\0', sizeof(this->_currentWeather.sunrise));
  memset(this->_currentWeather.sunset, '\0', sizeof(this->_currentWeather.sunset));

}

//********************************************************************************************
bool CDCSetup::LoadConfig(void)
{
  // Load CheapoDC config from file
  DynamicJsonDocument doc(1024);

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

  for (JsonPair kv : config ) {
    LOG_DEBUG("LoadConfig", "From config file: " << file.name() << " Key: " << kv.key().c_str() << " Value: " << kv.value().as<const char*>());
    if (!kv.value().isNull()) {
      setCmdProcessor( String(kv.key().c_str()), String(kv.value().as<const char*>()));
    } else {
      LOG_ALERT("LoadConfig", "Config file value for: " << kv.key().c_str() << " is nullptr");
    }
  }

  LOG_DEBUG("LoadConfig", "Load config complete" );
  resetConfigUpdated();
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
    WiFi.setHostname(CDC_DEFAULT_HOSTNAME);
    WiFi.begin(this->_wifiConfig.ssid, this->_wifiConfig.password);
    LOG_DEBUG("_connectWiFi","Connect to WiFi:" << this->_wifiConfig.ssid);

  // Wait for connection and attempt _wifiConfig.connectAttempts times
    while ((WiFi.status() != WL_CONNECTED) && (count < this->_wifiConfig.connectAttempts)) {
        count++;
        delay(500);
        this->blinkStatusLED();
        LOG_DEBUG("_connectWiFi","..wait for connect");
    }

    if (WiFi.status() != WL_CONNECTED) {
        LOG_ERROR("_connectWiFi","Could not connect to AP: " << this->_wifiConfig.ssid);
        this->statusLEDOn(); 
        this->blinkStatusLEDEvery(statusLEDEvery);
        return false;
    }

    LOG_ALERT("_connectWiFi", "Connected to: " << this->_wifiConfig.ssid << " with IP address: " << WiFi.localIP() );
  
    /*use mdns for host name resolution*/
    if (!MDNS.begin(this->_wifiConfig.hostname)) { //http://'host'.local
        LOG_ERROR("_connectWiFi", "Error setting up MDNS responder!");
    } else {
        LOG_ALERT("_connectWiFi", "mDNS responder started");
    }

    this->statusLEDOff();
    this->blinkStatusLEDEvery(statusLEDEvery);

    return true;
}

//********************************************************************************************
bool CDCSetup::_setupWiFi(void)
{
  // Connect to WiFi network  
  bool useDefaults = false;
  int count = 0;
  
  this->_inWiFiAPMode = false;
  // Try to load WiFi config from json file CDC_WIFI_CONFIG
  StaticJsonDocument<512> doc;

  File file = LittleFS.open(CDC_WIFI_CONFIG);
  if (!file) {
    LOG_ERROR("_setupWiFi", "Failed to open: " << CDC_WIFI_CONFIG );
    useDefaults = true;
  } else {
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        LOG_ERROR("_setupWiFi", "Failed to deserialize: " << CDC_WIFI_CONFIG);
        useDefaults = true;
    }
  }

  if (useDefaults) {
    LOG_DEBUG("_setupWiFi", "Using WiFi Defaults");
    strlcpy(this->_wifiConfig.hostname, CDC_DEFAULT_HOSTNAME, sizeof(this->_wifiConfig.hostname));
    this->_wifiConfig.connectAttempts, CDC_DEFAULT_WIFI_CONNECTATTEMPTS;
    this->_wifiConfig.tryAPs, CDC_DEFAULT_WIFI_TRYAPS;
    strlcpy(this->_wifiConfig.ssid, CDC_DEFAULT_WIFI_SSID, sizeof(this->_wifiConfig.ssid));
    strlcpy(this->_wifiConfig.password, CDC_DEFAULT_WIFI_PASSWORD, sizeof(this->_wifiConfig.password));

    while (count < this->_wifiConfig.tryAPs) {
        if (this->_connectWiFi())
            return true;
        count++;
    }

    this->blinkStatusLEDEvery(0);
    this->statusLEDOn();

    return false;

  } else {

    strlcpy(this->_wifiConfig.hostname, doc["hostname"] | CDC_DEFAULT_HOSTNAME, sizeof(this->_wifiConfig.hostname));
    this->_wifiConfig.connectAttempts = (doc["connectAttempts"] | CDC_DEFAULT_WIFI_CONNECTATTEMPTS);
    this->_wifiConfig.tryAPs = (doc["tryAPs"] | CDC_DEFAULT_WIFI_TRYAPS);

    LOG_DEBUG("_setupWiFi", "hostname:" << this->_wifiConfig.hostname);
    LOG_DEBUG("_setupWiFi", "Connection attempts:" << this->_wifiConfig.connectAttempts);
    LOG_DEBUG("_setupWiFi", "Try APs:" << this->_wifiConfig.tryAPs);
    
    while (count < this->_wifiConfig.tryAPs) { 
      for (JsonObject wifi_item : doc["wifi"].as<JsonArray>()) {
          strlcpy(this->_wifiConfig.ssid, wifi_item["ssid"] | CDC_DEFAULT_WIFI_SSID, sizeof(this->_wifiConfig.ssid));
          strlcpy(this->_wifiConfig.password, wifi_item["password"] | CDC_DEFAULT_WIFI_PASSWORD, sizeof(this->_wifiConfig.password));
          LOG_DEBUG("_setupWiFi", "WiFi " << count << "ssid " << this->_wifiConfig.ssid);
          LOG_DEBUG("_setupWiFi", "WiFi " << count << "password " << this->_wifiConfig.password);
          
          if (this->_connectWiFi())
              return true; 
      }
      count++;
    }

  }

  // Could not connect to WiFi. Set up in AP mode
  WiFi.mode(WIFI_MODE_NULL);
  WiFi.setHostname(CDC_DEFAULT_HOSTNAME);
  WiFi.softAP(CDC_DEFAULT_WIFI_AP_SSID, CDC_DEFAULT_WIFI_AP_PASSWORD);
  this->_inWiFiAPMode = true;
  LOG_ALERT("_setupWiFi", "WiFi in AP mode SSID " << CDC_DEFAULT_WIFI_AP_SSID << " with IP address: " << WiFi.softAPIP() );

  /*use mdns for host name resolution*/
  if (!MDNS.begin(this->_wifiConfig.hostname)) { //http://'host'.local
      LOG_ERROR("_setupWiFi", "Error setting up MDNS responder!");
  } else {
      LOG_ALERT("_setupWiFi", "mDNS responder started");
  }

  this->blinkStatusLEDEvery(0);
  this->statusLEDOn();

  return false;
}

// status LED
void    CDCSetup::blinkStatusLED() {digitalWrite(CDC_STATUS_LED, !digitalRead(CDC_STATUS_LED));};
void    CDCSetup::statusLEDOn() {digitalWrite(CDC_STATUS_LED, 1);};
void    CDCSetup::statusLEDOff() {digitalWrite(CDC_STATUS_LED, 0);};
void    CDCSetup::blinkStatusLEDEvery( int blinkEvery ) { if ((blinkEvery >= 0) && (blinkEvery <= 999)) this->_statusBlinkEvery = blinkEvery;};
int     CDCSetup::getStatusBlinkEvery() { return this->_statusBlinkEvery;};


void    CDCSetup::setWeatherQueryEvery( int queryEvery ) { 
    LOG_DEBUG("setWeatherQueryEvery", "queryEvery = " << queryEvery);
    
    if ((queryEvery >= 0) && (queryEvery <= 59)) this->_queryWeatherEvery = queryEvery;

    LOG_DEBUG("setWeatherQueryEvery", "this->_queryWeatherEvery = " << this->_queryWeatherEvery );
}

void    CDCSetup::setUpdateOutputEvery( int updateEvery ) { 
    LOG_DEBUG("setUpdateOutputEvery", "updateEvery = " << updateEvery);
    
    if ((updateEvery >= 0) && (updateEvery <= 59)) this->_controllerUpdateEvery = updateEvery;

    LOG_DEBUG("setUpdateOutputEvery", "this->_controllerUpdateEvery = " << this->_controllerUpdateEvery );
}

void    CDCSetup::setLocationLatitude( float latitude ) {

    if ((latitude >= -90.0) & (latitude <= 90.0)) {
      strlcpy(this->_location.latitude, String(latitude).c_str(), sizeof(this->_location.latitude));
      if (this->queryWeather())
        theDController->updateOutput();
    } else {
      LOG_ERROR("setLocationLatitude", "New latitude out of range -90 to 90: " << latitude);
    }
    return;
}

void    CDCSetup::setLocationLongitude( float longitude ) {

    if ((longitude >= -90.0) & (longitude <= 90.0)) {
      strlcpy(this->_location.longitude, String(longitude).c_str(), sizeof(this->_location.longitude));
      if (this->queryWeather())
        theDController->updateOutput();
    } else {
      LOG_ERROR("setLocationLongitude", "New longitude out of range -90 to 90: " << longitude);
    }
    return;
}

void    CDCSetup::setLocationTimeZone( int timezone ) {

    // In seconds for GMT-12 to GMT+14
    if ((timezone >= (-12*3600)) & (timezone <= (14*3600))) {
      this->_location.timezone = timezone;
    } else {
      LOG_ERROR("setLocationTimeZone", "New Time Zone out of range GMT-12 to GNT+14 in seconds: " << timezone);
    }
    return;
}

void    CDCSetup::setLocationDST( int DSTOffset ) {

    // In seconds for plus or minus 3600 seconds
    if ((DSTOffset >= -3600) & (DSTOffset <= 3600)) {
      this->_location.DSTOffset = DSTOffset;
    } else {
      LOG_ERROR("setLocationTimeZone", "New DST offset out of range -3600 to +3600 seconds: " << DSTOffset);
    }
    return;
}

void    CDCSetup::setAmbientTemperatureExternal( float temperature ) { 
  this->_ambientTemperatureExternal = temperature; 
  theDController->updateOutput();
}
