// ******************************************************************
// CheapoDC Command processor
// Cheap and easy Dew Controller
// Details at https://github.com/hcomet/CheapoDC
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************
#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>
#include <map>
#include "CDCdefines.h"
#include "CDCEasyLogger.h"
#include "CDCvars.h"
#include "CDCSetup.h"
#include "CDCWebSrvr.h"
#include "CDCommands.h"

// ******************************************************************
// CheapoDC Command map
// ******************************************************************
std::map<std::string, CDCommand> CDCCommands = {
    {"TMFL", {CDC_CMD_TMFL, 0, CDC_UNITS_NONE}},                // Filelist
    {"WICON", {CDC_CMD_WICON, 0, CDC_UNITS_NONE}},              // Weather Icon
    {"WDESC", {CDC_CMD_WDESC, 0, CDC_UNITS_NONE}},              // Weather description
    {"ATPQ", {CDC_CMD_ATPQ, 0, CDC_UNITS_DEGREES_C}},           // ambient temperature
    {"HU", {CDC_CMD_HU, 0, CDC_UNITS_PERCENT}},                 // humidity
    {"DP", {CDC_CMD_DP, 0, CDC_UNITS_DEGREES_C}},               // Dew point
    {"SP", {CDC_CMD_SP, 1, CDC_UNITS_DEGREES_C}},               // set point
    {"TPO", {CDC_CMD_TPO, 1, CDC_UNITS_DEGREES_C}},             // Track Point offset
    {"TKR", {CDC_CMD_TKR, 1, CDC_UNITS_DEGREES_C}},             // Tracking range
    {"DCM", {CDC_CMD_DCM, 1, CDC_CONTROLLERMODE_JSONARRAY}},    // dew controller mode (DCM must be before DCO for Save/Load)
    {"DCO", {CDC_CMD_DCO, 1, CDC_UNITS_PERCENT}},               // Dew Controller Output
    {"WS", {CDC_CMD_WS, 1, CDC_WEATHERSOURCE_JSONARRAY}},       // Weather source
    {"LQT", {CDC_CMD_LQT, 0, CDC_UNITS_NONE}},                  // last weather query time
    {"LQD", {CDC_CMD_LQD, 0, CDC_UNITS_NONE}},                  // last weather query date
    {"QN", {CDC_CMD_QN, 0, CDC_UNITS_NONE}},                    // Query Weather Now (Set only command)
    {"FW", {CDC_CMD_FW, 1, CDC_UNITS_NONE}},                    // firmware version
    {"HP", {CDC_CMD_HP, 0, CDC_UNITS_NONE}},                    // Heap size
    {"LFS", {CDC_CMD_LFS, 0, CDC_UNITS_NONE}},                  // LittleFS remaining space
    {"DCTM", {CDC_CMD_DCTM, 1, CDC_TEMPERATUREMODE_JSONARRAY}}, // dew controller temperature mode
    {"SPM", {CDC_CMD_SPM, 1, CDC_SETPOINTMODE_JSONARRAY}},      // dew controller set point mode
    {"WQE", {CDC_CMD_WQE, 1, CDC_UNITS_MINUTE}},                // Weather query every
    {"UOE", {CDC_CMD_UOE, 1, CDC_UNITS_MINUTE}},                // update output every
    {"WAPI", {CDC_CMD_WAPI, 0, CDC_UNITS_NONE}},                // Weather API URL
    {"WIURL", {CDC_CMD_WIURL, 0, CDC_UNITS_NONE}},              // Weather Icon URL
    {"WKEY", {CDC_CMD_WKEY, 1, CDC_UNITS_NONE}},                // Weather API Key
    {"LAT", {CDC_CMD_LAT, 1, CDC_UNITS_DEGREES}},               // Location latitude
    {"LON", {CDC_CMD_LON, 1, CDC_UNITS_DEGREES}},               // Location longitude
    {"LNM", {CDC_CMD_LNM, 1, CDC_UNITS_NONE}},                  // Location name
    {"TMZ", {CDC_CMD_TMZ, 1, CDC_UNITS_SECOND}},                // Location time zone (seconds)
    {"DST", {CDC_CMD_DST, 1, CDC_UNITS_SECOND}},                // Location DST offset (seconds)
    {"LED", {CDC_CMD_LED, 0, CDC_UNITS_MILLISEC}},              // Status LED Blink every (msec)
    {"NTP", {CDC_CMD_NTP, 1, CDC_UNITS_NONE}},                  // NTP serverName
    {"OMIN", {CDC_CMD_OMIN, 1, CDC_UNITS_PERCENT}},             // DC Min output
    {"OMAX", {CDC_CMD_OMAX, 1, CDC_UNITS_PERCENT}},             // DC Max output
    {"CDT", {CDC_CMD_CDT, 0, CDC_UNITS_NONE}},                  // Current Date Time
    {"ATPX", {CDC_CMD_ATPX, 0, CDC_UNITS_DEGREES_C}},           // External Temperature input by external app
    {"CTP", {CDC_CMD_CTP, 0, CDC_UNITS_DEGREES_C}},             // Current Track Point Temperature
    {"WUL", {CDC_CMD_WUL, 0, CDC_UNITS_NONE}},                  // Location of Weather station reported in query
    {"CLC", {CDC_CMD_CLC, 0, CDC_UNITS_PERCENT}},               // Cloud Coverage in percent *** Deprecated in V2.0 ***
    {"LWUT", {CDC_CMD_LWUT, 0, CDC_UNITS_NONE}},                // Last weather update time taken from query result
    {"LWUD", {CDC_CMD_LWUD, 0, CDC_UNITS_NONE}},                // Last weather update date taken from query result
    {"UPT", {CDC_CMD_UPT, 0, CDC_UNITS_NONE}},                  // return uptime in hhh:mm:ss:msec
    {"WIFI", {CDC_CMD_WIFI, 0, CDC_UNITS_NONE}},                // WIFI mode AP (Access Point) or STA (Station Mode)
    {"IP", {CDC_CMD_IP, 0, CDC_UNITS_NONE}},                    // IP Address
    {"HN", {CDC_CMD_HN, 0, CDC_UNITS_NONE}},                    // Host name
    {"WQEN", {CDC_CMD_WQEN, 0, CDC_UNITS_NONE}}                 // Weather Query Enabled (false = 0, true = 1) 
};

bool configUpdated = false;

void resetConfigUpdated() { configUpdated = false; };
void setConfigUpdated() 
{ 
  configUpdated = true;
  #ifdef CDC_STATUS_LED_BLINK_ON_CONFIG_CHANGE
  theSetup->statusLEDOn();
  #endif
};
bool getConfigUpdated() { return configUpdated; };

#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
// command queue processor for Web Socket transactions
struct {
    bool doWeatherQuery = false;
    bool doUpdateOutput = false;
} cmdPostProcessQueue;

void clearCmdPostProcessQueue()
{
  cmdPostProcessQueue.doUpdateOutput = false;
  cmdPostProcessQueue.doWeatherQuery = false;
};

void runCmdPostProcessQueue()
{
  if (cmdPostProcessQueue.doWeatherQuery)
    theSetup->queryWeather();
    
  if (cmdPostProcessQueue.doUpdateOutput)
    theDController->updateOutput();

  clearCmdPostProcessQueue();
};

#endif // CDC_ENABLE_CMDQUEUE

// Process CDC get commands and return results
cmdResponse getCmdProcessor(const String &var)
{
  cmdResponse newResponse;
  int command;

  newResponse.response = String();
  newResponse.units = String();

  // std::unordered_map<std::string, int>::const_iterator found = CDCCommands.find(var.c_str());

  if (CDCCommands.count(var.c_str()) == 0)
  {
    LOG_DEBUG("getCmdProcessor", "Command not found: " << var);
    return newResponse;
  }

  command = CDCCommands.at(var.c_str()).number;
  LOG_DEBUG("getCmdProcessor", "Get Command: " << var << " number " << command);

  switch (command)
  {
  case CDC_CMD_TMFL:
  {
    File root = LittleFS.open("/");
    if (!root)
    {
      LOG_ERROR("processor", "Failed to open directory: /");
      return newResponse;
    }
    newResponse.response = String("");
    File file = root.openNextFile();
    while (file)
    {
      if (file.isDirectory())
      {
        LOG_INFO("processor", "PATH: " << file.path());
        newResponse.response += String("<h2>Directory: ") + String(file.path()) + String("</h2>");
      }
      else
      {
        LOG_INFO("processor", "--FILE: " << file.name() << " tSIZE: " << file.size());
        newResponse.response += String("<div class=\"filelist\"><fileItem>") + String(file.name()) + String("</fileItem><fileAction1>");
        newResponse.response += String(" <a href=\"./download?file=") + String(file.path()) + String("\">Download</a></fileAction1><fileAction2>");
        // newResponse.response += String("<li>File: <a href=\"./download?file=") + String(file.path()) + String("\">") + String(file.name()) + String("</a>");
        newResponse.response += String("<div class=\"danger\" onclick=\"deleteFile('") + String(file.path()) + String("')\" >Delete</div></fileAction2></div></li>");
      }
      file = root.openNextFile();
    }

    break;
  }
  case CDC_CMD_WICON:
  {
    char buf[256] = "";
    sprintf(buf, theSetup->getWeatherIconURL(), theSetup->getCurrentWeatherIcon());
    newResponse.response = String(buf);

    break;
  }
  case CDC_CMD_WDESC:
  {
    newResponse.response = String(theSetup->getCurrentWeatherDescription());

    break;
  }
  case CDC_CMD_ATPQ:
  {
    char buf[16] = "";

    sprintf(buf, "%.1f", theSetup->getAmbientTemperatureWQ());
    newResponse.response = String(buf);

    break;
  }
  case CDC_CMD_HU:
  {
    char buf[12] = "";
    sprintf(buf, "%.0f", theSetup->getHumidity());
    newResponse.response = String(buf);

    break;
  }
  case CDC_CMD_DP:
  {
    char buf[16] = "";
    sprintf(buf, "%.1f", theSetup->getDewPoint());
    newResponse.response = String(buf);

    break;
  }
  case CDC_CMD_SP:
  {
    char buf[16] = "";
    sprintf(buf, "%.1f", theDController->getSetPoint());
    newResponse.response = String(buf);

    break;
  }
  case CDC_CMD_TPO:
  {
    char buf[16] = "";
    sprintf(buf, "%.1f", theDController->getTrackPointOffset());
    newResponse.response = String(buf);

    break;
  }
  case CDC_CMD_TKR:
  {
    char buf[16] = "";
    sprintf(buf, "%.1f", theDController->getTrackingRange());
    newResponse.response = String(buf);

    break;
  }
  case CDC_CMD_DCO:
  {
    char buf[16] = "";
    sprintf(buf, "%d", theDController->getOutput());
    newResponse.response = String(buf);

    break;
  }
  case CDC_CMD_WS:
  {
    newResponse.response = String(int(theSetup->getCurrentWeatherSource()));

    break;
  }
  case CDC_CMD_LQT:
  {
    newResponse.response = String(theSetup->getLastWeatherQueryTime());

    break;
  }
  case CDC_CMD_LQD:
  {
    newResponse.response = String(theSetup->getLastWeatherQueryDate());

    break;
  }
  case CDC_CMD_FW:
  {
    newResponse.response = String(programVersion);

    break;
  }
  case CDC_CMD_HP:
  {
    char buf[10] = ""; // 3 chars for "xxx"
    sprintf(buf, "%'d", ESP.getFreeHeap());
    newResponse.response = String(buf);

    break;
  }
  case CDC_CMD_LFS:
  {
    char buf[12] = ""; // 3 chars for "xxx"
    sprintf(buf, "%'d", (LittleFS.totalBytes() - LittleFS.usedBytes()));
    newResponse.response = String(buf);

    break;
  }
  case CDC_CMD_DCM:
  {
    newResponse.response = String(int(theDController->getControllerMode()));

    break;
  }
  case CDC_CMD_DCTM:
  {
    newResponse.response = String(int(theDController->getTemperatureMode()));

    break;
  }
  case CDC_CMD_SPM:
  {
    newResponse.response = String(int(theDController->getSetPointMode()));

    break;
  }
  case CDC_CMD_WQE:
  {
    newResponse.response = String(theSetup->getWeatherQueryEvery());

    break;
  }
  case CDC_CMD_UOE:
  {
    newResponse.response = String(theSetup->getControllerUpdateEvery());

    break;
  }
  case CDC_CMD_WAPI:
  {
    newResponse.response = String(theSetup->getWeatherAPIURL());

    break;
  }
  case CDC_CMD_WIURL:
  {
    newResponse.response = String(theSetup->getWeatherIconURL());

    break;
  }
  case CDC_CMD_WKEY:
  {
    newResponse.response = String(theSetup->getWeatherAPIKey());

    break;
  }
  case CDC_CMD_LAT:
  {
    CDCLocation myLocation = theSetup->getLocation();
    newResponse.response = String(myLocation.latitude);

    break;
  }
  case CDC_CMD_LON:
  {
    CDCLocation myLocation = theSetup->getLocation();
    newResponse.response = String(myLocation.longitude);

    break;
  }
  case CDC_CMD_LNM:
  {
    CDCLocation myLocation = theSetup->getLocation();
    newResponse.response = String(myLocation.name);

    break;
  }
  case CDC_CMD_TMZ:
  {
    CDCLocation myLocation = theSetup->getLocation();
    newResponse.response = String(myLocation.timezone);

    break;
  }
  case CDC_CMD_DST:
  {
    CDCLocation myLocation = theSetup->getLocation();
    newResponse.response = String(myLocation.DSTOffset);

    break;
  }
  case CDC_CMD_LED:
  {
    newResponse.response = String(theSetup->getStatusBlinkEvery());

    break;
  }
  case CDC_CMD_NTP:
  {
    newResponse.response = String(theSetup->getNTPServerURL());

    break;
  }
  case CDC_CMD_OMIN:
  {
    newResponse.response = String(theDController->getMinOutput());

    break;
  }
  case CDC_CMD_OMAX:
  {
    newResponse.response = String(theDController->getMaxOutput());

    break;
  }
  case CDC_CMD_CDT:
  {
    newResponse.response = String(theSetup->getDateTime());
    break;
  }
  case CDC_CMD_ATPX:
  {
    char buf[16] = "";
    sprintf(buf, "%.1f", theSetup->getAmbientTemperatureExternal());
    newResponse.response = String(buf);

    break;
  }
  case CDC_CMD_CTP:
  {
    char buf[16] = "";
    sprintf(buf, "%.1f", theDController->getTrackPoint());
    newResponse.response = String(buf);

    break;
  }
  case CDC_CMD_WUL:
  {
    newResponse.response = String(theSetup->getLastWeatherUpdateLocation());

    break;
  }
  case CDC_CMD_CLC: // *** Deprecated in V2.0 ***
  {
    newResponse.response = String("NA");
    break;
  }
  case CDC_CMD_LWUD:
  {
    newResponse.response = String(theSetup->getLastWeatherUpdateDate());
    break;
  }
  case CDC_CMD_LWUT:
  {
    newResponse.response = String(theSetup->getLastWeatherUpdateTime());
    break;
  }
  case CDC_CMD_UPT:
  {
    char buf[16] = "";
    unsigned long upTime = millis();
    unsigned long hours = upTime/3600000;
    unsigned long mins = (upTime - (hours*3600000))/60000;
    unsigned long secs = (upTime - (hours*3600000)-(mins*60000))/1000;
    unsigned long msecs = upTime%1000;

    sprintf(buf, "%03d:%02d:%02d:%03d", hours, mins, secs, msecs);
    newResponse.response = String(buf);
    break;
  }
  case CDC_CMD_WIFI:
  {
    if (theSetup->getInWiFiAPMode())
      newResponse.response = String("AP");
    else
      newResponse.response = String("STA");
    break;
  }
  case CDC_CMD_IP:
  {
    newResponse.response = String(theSetup->getIPAddress());
    break;
  }
  case CDC_CMD_HN:
  {
    newResponse.response = String(theSetup->getWiFiHostname());
    break;
  }
  case CDC_CMD_WQEN:
  {
    char buf[16] = "";

    sprintf(buf, "%d", theSetup->getWeatherQueryEnabled());
    newResponse.response = String(buf);
    break;
  }
  default:
    return newResponse;
    break;
  }

  newResponse.units = String(CDCCommands.at(var.c_str()).units.c_str());
  return newResponse;
}

//************************************************************************************************
// Process CDC set commands
//************************************************************************************************
bool setCmdProcessor(const String &var, String newValue)
{
  int command;

  if (CDCCommands.count(var.c_str()) == 0)
  {
    LOG_DEBUG("setCmdProcessor", "Command not found: " << var);
    return false;
  }

  LOG_DEBUG("setCmdProcessor", "SET Command: " << var << "  Value: " << newValue);

  command = CDCCommands.at(var.c_str()).number;
  LOG_DEBUG("setCmdProcessor", "SET Command: " << var << " maps to " << command);

  switch (command)
  {
  case CDC_CMD_ATPQ:
  {

    if (theSetup->getCurrentWeatherSource() != EXTERNALSOURCE )
    {
      LOG_ALERT("setCmdProcessor", "Weather Source must be EXTERNALSOURCE to set ATPQ");
      return false;
    }
    else
    {
      bool doUpdateOutput = (theSetup->getAmbientTemperatureWQ() != newValue.toFloat());
      // Want to set last update every call
      theSetup->setAmbientTemperatureWQ(newValue.toFloat());
      if (doUpdateOutput)
      {
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
        cmdPostProcessQueue.doUpdateOutput = true;
#else
        theDController->updateOutput();
#endif
      }
    }
    break;
  }
  case CDC_CMD_HU:
  {

    if (theSetup->getCurrentWeatherSource() != EXTERNALSOURCE )
    {
      LOG_ALERT("setCmdProcessor", "Weather Source must be EXTERNALSOURCE to set HU");
      return false;
    }
    else
    {
      bool doUpdateOutput = (theSetup->getHumidity() != newValue.toFloat());
      // Want to update the last weather update info every call
      theSetup->setHumidity(newValue.toFloat());

      if (doUpdateOutput)
      {
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
        cmdPostProcessQueue.doUpdateOutput = true;
#else
        theDController->updateOutput();
#endif
      }
    }
    break;
  }
  case CDC_CMD_SP:
  {
    float oldValue = theDController->getSetPoint();
      
    if (oldValue != newValue.toFloat())
    {
      theDController->setSetPoint(newValue.toFloat());
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
    cmdPostProcessQueue.doUpdateOutput = true;
#else
      theDController->updateOutput();
#endif
    }
    break;
  }
  case CDC_CMD_TPO:
  {
    float oldValue = theDController->getTrackPointOffset();
      
    if (oldValue != newValue.toFloat())
    {
      theDController->setTrackPointOffset(newValue.toFloat());
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
    cmdPostProcessQueue.doUpdateOutput = true;
#else
      theDController->updateOutput();
#endif
    }
    break;
  }
  case CDC_CMD_TKR:
  {
    float oldValue = theDController->getTrackingRange();
      
    if (oldValue != newValue.toFloat())
    {
      theDController->setTrackingRange(newValue.toFloat());
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
    cmdPostProcessQueue.doUpdateOutput = true;
#else
      theDController->updateOutput();
#endif
    }
    break;
  }
  case CDC_CMD_DCO:
  {
    float oldValue = theDController->getOutput();
      
    if (oldValue != newValue.toFloat())
    {
      theDController->updateOutput(newValue.toInt());
    }
    break;
  }
  case CDC_CMD_WS:
  {
    int oldValue = (int)theSetup->getWeatherSource();
    if (oldValue != newValue.toInt())
    {
      theSetup->setWeatherSource((weatherSource)newValue.toInt()); 
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
      cmdPostProcessQueue.doWeatherQuery = true;
      cmdPostProcessQueue.doUpdateOutput = true;
#else
      theSetup->queryWeather();
      theDController->updateOutput();
#endif
    }
    break;
  }
  case CDC_CMD_QN:
  {
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
    cmdPostProcessQueue.doWeatherQuery = true;
#else
    theSetup->queryWeather();
#endif
    break;
  }
  case CDC_CMD_DCM:
  {
    int oldValue = (int)theDController->getControllerMode();
    if (oldValue != newValue.toInt())
    {
      theDController->setControllerMode((controllerMode)newValue.toInt());
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
      cmdPostProcessQueue.doUpdateOutput = true;
#else
      theDController->updateOutput();
#endif
    }
    break;
  }
  case CDC_CMD_DCTM:
  {
    int oldValue = (int)theDController->getTemperatureMode();
    if (oldValue != newValue.toInt())
    {
      theDController->setTemperatureMode((temperatureMode)newValue.toInt());
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
      cmdPostProcessQueue.doUpdateOutput = true;
#else
      theDController->updateOutput();
#endif
    }
    break;
  }
  case CDC_CMD_SPM:
  {
    int oldValue = (int)theDController->getSetPointMode();
    if (oldValue != newValue.toInt())
    {
      theDController->setSetPointMode((setPointMode)newValue.toInt());
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
      cmdPostProcessQueue.doUpdateOutput = true;
#else
      theDController->updateOutput();
#endif
    }
    break;
  }
  case CDC_CMD_WQE:
  {
    theSetup->setWeatherQueryEvery(newValue.toInt());
    break;
  }
  case CDC_CMD_UOE:
  {
    theSetup->setUpdateOutputEvery(newValue.toInt());
    break;
  }
  case CDC_CMD_WKEY:
  {
    theSetup->setWeatherQueryAPIKey(newValue);
    break;
  }
  case CDC_CMD_LAT:
  {
    float oldValue = atof(theSetup->getLocation().latitude);
    
    if (newValue.toFloat() != oldValue)
    {
      theSetup->setLocationLatitude(newValue.toFloat());
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
      cmdPostProcessQueue.doWeatherQuery = true;
      cmdPostProcessQueue.doUpdateOutput = true;
#else
      theSetup->queryWeather();
      theDController->updateOutput();
#endif
    }
    break;
  }
  case CDC_CMD_LON:
  {
    float oldValue = atof(theSetup->getLocation().longitude);
    
    if (newValue.toFloat() != oldValue)
    {
      theSetup->setLocationLongitude(newValue.toFloat());
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
      cmdPostProcessQueue.doWeatherQuery = true;
      cmdPostProcessQueue.doUpdateOutput = true;
#else
      theSetup->queryWeather();
      theDController->updateOutput();
#endif
    }
    break;
  }
  case CDC_CMD_LNM:
  {
    theSetup->setLocationName(newValue);
    break;
  }
  case CDC_CMD_TMZ:
  {
    theSetup->setLocationTimeZone(newValue.toInt());
    break;
  }
  case CDC_CMD_DST:
  {
    theSetup->setLocationDST(newValue.toInt());
    break;
  }
  case CDC_CMD_LED:
  {
    theSetup->blinkStatusLEDEvery(newValue.toInt());
    break;
  }
  case CDC_CMD_NTP:
  {
    theSetup->setNTPServer(newValue);
    break;
  }
  case CDC_CMD_OMIN:
  {
    int oldValue = (int)theDController->getMinOutput();
    if (oldValue != newValue.toInt())
    {
      theDController->setMinOutput(newValue.toInt());
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
      cmdPostProcessQueue.doUpdateOutput = true;
#else
      theDController->updateOutput();
#endif
    }
    break;
  }
  case CDC_CMD_OMAX:
  {
    int oldValue = (int)theDController->getMaxOutput();
    if (oldValue != newValue.toInt())
    {
      theDController->setMaxOutput(newValue.toInt());
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
    cmdPostProcessQueue.doUpdateOutput = true;
#else
      theDController->updateOutput();
#endif
    }
    break;
  }
  case CDC_CMD_ATPX:
  {
    float oldValue = theSetup->getAmbientTemperatureExternal();
    if (oldValue != newValue.toFloat())
    {
      theSetup->setAmbientTemperatureExternal(newValue.toFloat());
      if (theDController->getTemperatureMode() == EXTERNAL_INPUT)
      {
#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
        cmdPostProcessQueue.doUpdateOutput = true;
#else
        theDController->updateOutput();
#endif
      }
    }
    break;
  }
  case CDC_CMD_WQEN:
  {
    theSetup->setWeatherQueryEnabled((bool)newValue.toInt());
    break;
  }
  default:
  {
    LOG_ALERT("setCmdProcessor", "SET function not supported for: " << var);
    return false;
    break;
  }
  }

  if (CDCCommands.at(var.c_str()).saveToConfig)
  {
    setConfigUpdated();
  };
  return true;
}
