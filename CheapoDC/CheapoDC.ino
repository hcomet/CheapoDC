// ******************************************************************
// CheapoDC main
// Cheap and easy Dew Controller
// Details at https://github.com/hcomet/CheapoDC
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************
//
// Look at CDCdefines.h to configure ESP32 and WiF before building
//
// ******************************************************************
#include <Arduino.h>
#include <ESP32Time.h>
#include "CDCdefines.h"
#include <EasyLogger.h>
#include "CDCWebSrvr.h"
#include "CDCvars.h"
#include "CDCSetup.h"

char programName[] = "CheapoDC"; // Program name
char programVersion[] = "1.0.1";  // program version

CDCSetup *theSetup; // main setup class
dewController *theDController;
ESP32Time *theTime;

// Counters used for time based scheduler in main loop
int milliCount = 0;
unsigned long milliLast = 0;
unsigned long milliDelta = 0;
int secCount = 0;
unsigned int secLast = 0;
unsigned int secDelta = 0;
int minCount = 0;
unsigned int minLast = 0;
unsigned int minDelta = 0;

// Counters for items scheduled
//
// Status LED timer in milliseconds
int statusLEDDelta = 0;
int statusLEDLast = 0;
int statusLEDCountDown = CDC_STATUS_LED_DELAY;

// Weather query timer in minutes
int weatherQueryDelta = 0;
int weatherQueryLast = 0;
// Controller Output update timer in minutes
int controllerUpdateDelta = 0;
int controllerUpdateLast = 0;
// Save configuration check in seconds
int saveConfigDelta = 0;
int saveConfigLast = 0;

// Timer functions for scheduled services
void ledTimer(int timeCheck)
{

  if (theSetup->getStatusBlinkEvery() != 0)
  {
    if ((timeCheck - statusLEDLast) < 0)
    {
      statusLEDDelta = 1000 - statusLEDLast + timeCheck;
    }
    else
    {
      statusLEDDelta = timeCheck - statusLEDLast;
    }

    if (statusLEDDelta >= theSetup->getStatusBlinkEvery())
    {
      theSetup->blinkStatusLED();
      statusLEDDelta = 0;
      statusLEDLast = timeCheck;
    }
  }
}

void weatherQueryTimer(int timeCheck)
{

  if (theSetup->getWeatherQueryEvery() != 0)
  {
    if ((timeCheck - weatherQueryLast) < 0)
    {
      weatherQueryDelta = 60 - weatherQueryLast + timeCheck;
    }
    else
    {
      weatherQueryDelta = timeCheck - weatherQueryLast;
    }

    if (weatherQueryDelta >= theSetup->getWeatherQueryEvery())
    {
      theSetup->queryWeather();
      LOG_ALERT("weatherQueryTimer", "Time, Weather (temperature [C], humidity[%], dew point[C]) " << theTime->getDateTime().c_str() << ", "
                << theSetup->getAmbientTemperatureWQ() << ", " << theSetup->getHumidity() << ", " << theSetup->getDewPoint());
      weatherQueryLast = timeCheck;
    }
  }
}

void controllerUpdateTimer(int timeCheck)
{
  if (theSetup->getControllerUpdateEvery() != 0)
  {
    if ((timeCheck - controllerUpdateLast) < 0)
    {
      controllerUpdateDelta = 60 - controllerUpdateLast + timeCheck;
    }
    else
    {
      controllerUpdateDelta = timeCheck - controllerUpdateLast;
    }

    if (controllerUpdateDelta >= theSetup->getControllerUpdateEvery())
    {
      LOG_DEBUG("controllerUpdateTimer", "Call update output" << theTime->getDateTime());
      theDController->updateOutput();
      controllerUpdateLast = timeCheck;
    }
  }
}

void saveConfigTimer(int timeCheck)
{
  if ((timeCheck - saveConfigLast) < 0)
  {
    saveConfigDelta = 60 - saveConfigLast + timeCheck;
  }
  else
  {
    saveConfigDelta = timeCheck - saveConfigLast;
  }

  if (saveConfigDelta >= CDC_DEFAULT_SAVE_CONFIG_EVERY)
  {
    // Update config file too
    saveConfigLast = timeCheck;
    if (!theSetup->SaveConfig())
    {
      LOG_ALERT("saveConfigTimer", "Failure to save config file");
    }
  }
}

// Main setup
void setup()
{

  Serial.begin(115200);

  theDController = new dewController();
  theDController->setEnabled();
  theSetup = new CDCSetup();

  LOG_DEBUG("Main-setup", "Load CheapoDC configuration");
  if (!theSetup->LoadConfig())
  {
    LOG_ERROR("Main-setup", "Load Configuration Failure. Will continue on defaults.");
  }

  if (theSetup->getInWiFiAPMode())
  {
    theDController->setControllerMode(OFF);
  }

  // Adjust Timezone etc....
  configTime(theSetup->getLocation().timezone, theSetup->getLocation().DSTOffset, theSetup->getNTPServerURL());

  LOG_ALERT("Main-setup", "CHeapoDC Webserver Startup: " << theTime->getDateTime());
  setupServers();

  // Synchronize with RTC for scheduling
  milliLast = millis();
  secCount = theTime->getSecond();
  minCount = theTime->getMinute();
  // lastMinute = theTime->getMinute();

  theSetup->statusLEDDelay( RESET_DELAY );
  LOG_DEBUG("CDCSETUP", "Weather query every: " << theSetup->getWeatherQueryEvery());
  if (theSetup->getWeatherQueryEvery() != 0)
  {
    theSetup->queryWeather();
    weatherQueryLast = minCount;
  }

}

// Main loop
void loop()
{
  // put your main code here, to run repeatedly:
  milliDelta = millis() - milliLast;
  if (milliDelta > 0)
  { // Only do things if loop >= 1 millisec
    milliCount = milliCount + milliDelta;
    milliLast = millis();

    // Set up elapsed millisec, sec, minutes
    if (milliCount >= 1000)
    {
      milliCount = 0;
      secCount++;
      if (secCount >= 60)
      {
        secCount = 0;
        minCount++;
        if (minCount >= 60)
          minCount = 0;
      }
    }

    // Do millisec timer items
    ledTimer(milliCount);
    // End milliseconds check

    // Do seconds timer items
    if ((secCount - secLast) != 0)
    {
      secLast = secCount;
      
      theSetup->statusLEDDelay( DEC_DELAY );

      // call save config file timer
      saveConfigTimer(secCount);
      // End Seconds check

      // Do minute timer items
      if ((minCount - minLast) != 0)
      {
        minLast = minCount;

        // call weather query timer
        weatherQueryTimer(minCount);

        // call update controller timer
        controllerUpdateTimer(minCount);

      } // End Minutes check
    }
  }
}
