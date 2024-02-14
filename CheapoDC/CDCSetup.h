// ******************************************************************
// CheapoDC Setup and configuration
// Cheap and easy Dew Controller
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************
#ifndef MY_CDCSETUP_H
#define MY_CDCSETUP_H

#include <Arduino.h>
#include "CDCdefines.h"
#include <ESP32Time.h>

#include "CDCvars.h"

//ESP32Time   *theTime;

struct CDCWiFiConfig {
                char hostname[64];
                int  connectAttempts;
                int  tryAPs;
                char ssid[32];
                char password[32];
};

struct CDCLocation {
                char latitude[8];
                char longitude[8];
                char name[32];
                int  timezone;
                int  DSTOffset;
};
/*
#define CDC_WEATHERSOURCE_JSONARRAY "{\"SOURCE\":[\"Open-Meteo\",\"OpenWeather\"]}"
enum    weatherSource {
            OPENMETEO,          // 0
            OPENWEATHERSOURCE   // 1      
}; */

struct          weatherData
                {
                char  weatherUpdateLocation[32];
                char  lastWeatherQueryTime[32];
                char  lastWeatherQueryDate[32];
                float ambientTemperature;
                float humidity;
                float dewPoint;
                float minTemperature;
                float maxTemperature;
                int pressure;
                int visibility;
                float windSpeed;
                int windDirection;
                int cloudCoverage;
                char weatherDescription[32];
                char weatherIcon[4];
                long weatherUpdated;
                long sunrise;
                long sunset;
};

// weatherData currentWeatherData[2];

class CDCSetup
{
    public:
                CDCSetup(void);
        bool LoadConfig(void);
        bool SaveConfig(void);
        bool queryWeather(void);

        // status LED
        void    blinkStatusLED();
        void    statusLEDOn();
        void    statusLEDOff();
        void    blinkStatusLEDEvery( int blinkEvery );
        int     getStatusBlinkEvery();

        // getters
        CDCLocation     getLocation() {return this->_location;};
        float           getAmbientTemperatureWQ() {return this->_currentWeather.ambientTemperature;};
        float           getAmbientTemperatureExternal() {return this->_ambientTemperatureExternal;};
        float           getHumidity() {return this->_currentWeather.humidity;};
        float           getDewPoint() {return this->_currentWeather.dewPoint;};
        int             getCloudCoverage() {return this->_currentWeather.cloudCoverage;};
        const char*     getWeatherSource() {return this->_weatherSource;};
        const char*     getWeatherAPIURL() {return this->_weatherAPIURL;};
        const char*     getWeatherIconURL() {return this->_weatherIconURL;};
        const char*     getWeatherAPIKey() {return this->_weatherAPIKey;};
        int             getWeatherQueryEvery() { return this->_queryWeatherEvery;};
        const char*     getLastWeatherQueryTime() {return this->_currentWeather.lastWeatherQueryTime;};
        const char*     getLastWeatherQueryDate() {return this->_currentWeather.lastWeatherQueryDate;};
        const char*     getLastWeatherUpdateLocation() {return this->_currentWeather.weatherUpdateLocation;};
        const char*     getCurrentWeatherIcon() {return this->_currentWeather.weatherIcon;};
        const char*     getCurrentWeatherDescription() {return this->_currentWeather.weatherDescription;};
        int             getControllerUpdateEvery() {return this->_controllerUpdateEvery;};
        const char*     getNTPServerURL() {return this->_NTPServer;};
        bool            getInWiFiAPMode() {return this->_inWiFiAPMode;};

        // setters
        void    setWeatherQueryAPIURL( String newURL ) {strlcpy(this->_weatherAPIURL, newURL.c_str(), sizeof(this->_weatherAPIURL));};
        void    setWeatherQueryIconURL( String newURL ) {strlcpy(this->_weatherIconURL, newURL.c_str(), sizeof(this->_weatherIconURL));};
        void    setWeatherQueryAPIKey( String newURL ) {strlcpy(this->_weatherAPIKey, newURL.c_str(), sizeof(this->_weatherAPIKey));};   
        void    setWeatherQueryEvery( int queryEvery );
        void    setAmbientTemperatureExternal( float temperature );
        void    setUpdateOutputEvery( int updateEvery );
        void    setNTPServer( String server ) {strlcpy(this->_NTPServer, server.c_str(), sizeof(this->_NTPServer));};
        void    setLocationLatitude( float latitude );
        void    setLocationLongitude( float longitude );
        void    setLocationName( String name ) {strlcpy(this->_location.name, name.c_str(), sizeof(this->_location.name));};
        void    setLocationTimeZone( int timezone);
        void    setLocationDST( int DSTOffset );

        
        
     private:
        bool _setupWiFi(void);
        bool _connectWiFi(void);
        void _loadDefaults(void);

        CDCWiFiConfig   _wifiConfig;
        bool            _inWiFiAPMode;
        CDCLocation     _location;
        float           _ambientTemperatureExternal;
        char            _weatherSource[32];
        char            _weatherAPIURL[256];
        char            _weatherIconURL[256];
        char            _weatherAPIKey[64];
        int             _queryWeatherEvery;
        int             _controllerUpdateEvery; // in minutes
        int             _statusBlinkEvery;
        char            _NTPServer[64];
        weatherData     _currentWeather;
                        
};
#endif 