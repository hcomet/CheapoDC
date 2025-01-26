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
#include <TimeLib.h>
#include "CDCdefines.h"
#include "CDCEasyLogger.h"
#include "CDCWebSrvr.h"
#include "CDCvars.h"
#include "CDCSetup.h"
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
#include "CDCommands.h"
#endif

char programName[] = "CheapoDC"; // Program name
char programVersion[] = "2.1.0";  // program version

CDCSetup *theSetup; // main setup class
dewController *theDController;
//ESP32Time *theTime;

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

#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
// run command queue timer in msec only for Web Sockets enabled
int runCmdQueueDelta = 0;
int runCmdQueueLast = 0;
#endif
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

#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
void runCmdQueueTimer(int timeCheck)
{

    if ((timeCheck - runCmdQueueLast) < 0)
    {
      runCmdQueueDelta = 1000 - runCmdQueueLast + timeCheck;
    }
    else
    {
      runCmdQueueDelta = timeCheck - runCmdQueueLast;
    }

    if (runCmdQueueDelta >= CDC_RUNCMDQUEUE_EVERY)
    {
      runCmdPostProcessQueue();
      runCmdQueueLast = timeCheck;
    }
}
#endif

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
      LOG_DEBUG("controllerUpdateTimer", "Call update output" << theSetup->getDateTime());
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

  sleep(1);
  Serial.begin(115200);
  sleep(1);
  LOG_ALERT("setup","Initializing CheapoDC.");

  theDController = new dewController();
  theSetup = new CDCSetup();
  theSetup->setWeatherQueryEnabled( false );

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
  now();

  #if LOG_LEVEL == LOG_LEVEL_DEBUG
  struct tm lTime;
  getLocalTime(&lTime);
  LOG_DEBUG("Main-setup", "Local time struct: ");
  LOG_DEBUG("Main-setup", "  seconds: " << lTime.tm_sec);
  LOG_DEBUG("Main-setup", "  minutes: " << lTime.tm_min);
  LOG_DEBUG("Main-setup", "  hours: " << lTime.tm_hour);
  LOG_DEBUG("Main-setup", "  m-day: " << lTime.tm_mday);
  LOG_DEBUG("Main-setup", "  month: " << lTime.tm_mon);
  LOG_DEBUG("Main-setup", "  year: " << lTime.tm_year);
  LOG_DEBUG("Main-setup", "  y-day: " << lTime.tm_yday);
  LOG_DEBUG("Main-setup", "  is DST: " << lTime.tm_isdst);
  #endif

  setupServers();

  theSetup->statusLEDDelay( RESET_DELAY );
  LOG_DEBUG("CDCSETUP", "Weather query every: " << theSetup->getWeatherQueryEvery());
  // Synchronize with RTC for scheduling
  milliLast = millis();
  secCount = theSetup->getSecond();
  minCount = theSetup->getMinute();

  // Enable and make first weather query
  theSetup->setWeatherQueryEnabled( true );
  if (theSetup->getWeatherQueryEvery() != 0)
  {
    theSetup->queryWeather();
    weatherQueryLast = minCount;
  }

  theDController->setEnabled();
  #if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
  clearCmdPostProcessQueue();
  #endif
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
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
    runCmdQueueTimer(milliCount);
#endif
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
