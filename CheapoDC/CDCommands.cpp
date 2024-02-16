#include <Arduino.h>
#include "CDCdefines.h"
#include "FS.h"
#include <LittleFS.h>
#include <map>
#include <EasyLogger.h>
#include "CDCvars.h"
#include "CDCSetup.h"
#include "CDCWebSrvr.h"
#include "CDCommands.h"

std::map<std::string, CDCommand> CDCCommands = {
    {"TMFL", {CDC_CMD_TMFL, 0, CDC_UNITS_NONE }},   // Filelist
    {"WICON", {CDC_CMD_WICON, 0, CDC_UNITS_NONE }}, // Weather Icon
    {"WDESC", {CDC_CMD_WDESC, 0, CDC_UNITS_NONE }}, // Weather description
    {"ATPQ", {CDC_CMD_ATPQ, 0, CDC_UNITS_DEGREES_C }},       // ambient temperature
    {"HU", {CDC_CMD_HU, 0, CDC_UNITS_PERCENT }},       // humidity
    {"DP", {CDC_CMD_DP, 0, CDC_UNITS_DEGREES_C }},       // Dew point
    {"SP", {CDC_CMD_SP, 1, CDC_UNITS_DEGREES_C }},       // set point
    {"TPO", {CDC_CMD_TPO, 1, CDC_UNITS_DEGREES_C }},     // Track Point offset
    {"TKR", {CDC_CMD_TKR, 1, CDC_UNITS_DEGREES_C }},     // Tracking range
    {"DCO", {CDC_CMD_DCO, 1, CDC_UNITS_PERCENT }},     // Dew Controller Output
    {"WS", {CDC_CMD_WS, 0, CDC_UNITS_NONE }},       // Weather source
    {"LQT", {CDC_CMD_LQT, 0, CDC_UNITS_NONE }},     // last weather query time
    {"LQD", {CDC_CMD_LQD, 0, CDC_UNITS_NONE }},     // last weather query date
    {"WRD", {CDC_CMD_WRD, 0, CDC_UNITS_NONE }},     // weather report date time
    {"FW", {CDC_CMD_FW, 0, CDC_UNITS_NONE }},       // firmware version
    {"HP", {CDC_CMD_HP, 0, CDC_UNITS_NONE }},       // Heap size
    {"LFS", {CDC_CMD_LFS, 0, CDC_UNITS_NONE }},     // LittleFS remaining space
    {"DCM", {CDC_CMD_DCM, 1, CDC_CONTROLLERMODE_JSONARRAY }},     // dew controller mode
    {"DCTM", {CDC_CMD_DCTM, 1, CDC_TEMPERATUREMODE_JSONARRAY }},   // dew controller temperature mode
    {"SPM", {CDC_CMD_SPM, 1, CDC_SETPOINTMODE_JSONARRAY }},     // dew controller set point mode
    {"WQE", {CDC_CMD_WQE, 1, CDC_UNITS_MINUTE }},     // Weather query every
    {"UOE", {CDC_CMD_UOE, 1, CDC_UNITS_MINUTE }},      // update outputs every
    {"WAPI", {CDC_CMD_WAPI, 1, CDC_UNITS_NONE }},   // Weather API URL
    {"WIURL", {CDC_CMD_WIURL, 1, CDC_UNITS_NONE }}, // Weather Icon URL
    {"WKEY", {CDC_CMD_WKEY, 1, CDC_UNITS_NONE }},   // Weather API Key
    {"LAT", {CDC_CMD_LAT, 1, CDC_UNITS_DEGREES }},     // Location latitude
    {"LON", {CDC_CMD_LON, 1, CDC_UNITS_DEGREES }},     // Location longitude
    {"LNM", {CDC_CMD_LNM, 1, CDC_UNITS_NONE }},     // Location name
    {"TMZ", {CDC_CMD_TMZ, 1, CDC_UNITS_SECOND }},    // Location time zone (seconds)
    {"DST", {CDC_CMD_DST, 1, CDC_UNITS_SECOND }},     // Location DST offset (seconds)
    {"LED", {CDC_CMD_LED, 1, CDC_UNITS_MILLISEC }},     // Status LED Blink every (msec)
    {"NTP", {CDC_CMD_NTP, 1, CDC_UNITS_NONE }},     // NTP serverName
    {"OMIN", {CDC_CMD_OMIN, 1, CDC_UNITS_PERCENT }},     // DC Min output
    {"OMAX", {CDC_CMD_OMAX, 1, CDC_UNITS_PERCENT }},  // DC Max output
    {"CDT", {CDC_CMD_CDT, 0, CDC_UNITS_NONE }},      // Current Date Time
    {"ATPX", {CDC_CMD_ATPX, 0, CDC_UNITS_DEGREES_C }},  // Local Temperature input by external app
    {"CTP", {CDC_CMD_CTP, 0, CDC_UNITS_DEGREES_C }},     // Current Track Point Temperature
    {"WUL", {CDC_CMD_WUL, 0, CDC_UNITS_NONE}},           // Location of Weather station reported in query
    {"CLC", {CDC_CMD_CLC, 0, CDC_UNITS_PERCENT }},       // Cloud Coverage in percent
    {"LWUT", {CDC_CMD_LWUT, 0, CDC_UNITS_NONE }},      // Cloud Coverage in percent
    {"LWUD", {CDC_CMD_LWUD, 0, CDC_UNITS_NONE }}       // Cloud Coverage in percent
};

bool configUpdated = false;

void resetConfigUpdated() {configUpdated = false;};
void setConfigUpdated() {configUpdated = true;};
bool getConfigUpdated() {return configUpdated;};

// Process CDC get commands and return results
cmdResponse getCmdProcessor(const String &var )
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
    newResponse.response = String(theSetup->getWeatherSource());

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
  case CDC_CMD_WRD:
  {
    newResponse.response = String("Command NOT implemented");

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
    newResponse.response = String(theTime->getDateTime());
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
  case CDC_CMD_CLC:
  {
    char buf[16] = "";
    sprintf(buf, "%d", theSetup->getCloudCoverage());
    newResponse.response = String(buf);
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
  default:
    return newResponse;
    break;
  }

  newResponse.units =  String(CDCCommands.at(var.c_str()).units.c_str());
  return newResponse;
}

//************************************************************************************************
// Process CDC set commands
bool setCmdProcessor(const String &var, String newValue)
{
   int command;
  // std::unordered_map<std::string, int>::const_iterator found = CDCCommands.find(var.c_str());

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
    case CDC_CMD_SP:
    {
        theDController->setSetPoint(newValue.toFloat());
        
        break;
    }
    case CDC_CMD_TPO:
    {
        theDController->setTrackPointOffset(newValue.toFloat());
        
        break;
    }
    case CDC_CMD_TKR:
    {
        theDController->setTrackingRange(newValue.toFloat());
        
        break;
    }
    case CDC_CMD_DCO:
    {
        theDController->updateOutput(newValue.toInt());
        
        break;
    }
    case CDC_CMD_DCM:
    {
        theDController->setControllerMode((controllerMode)newValue.toInt());
        break;
    }
    case CDC_CMD_DCTM:
    {
        theDController->setTemperatureMode((temperatureMode)newValue.toInt());
        break;
    }
    case CDC_CMD_SPM:
    {
        theDController->setSetPointMode((setPointMode)newValue.toInt());
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
    case CDC_CMD_WAPI:
    {
        theSetup->setWeatherQueryAPIURL(newValue);
        break;
    }
    case CDC_CMD_WIURL:
    {
        theSetup->setWeatherQueryIconURL(newValue);
        break;
    }
    case CDC_CMD_WKEY:
    {
        theSetup->setWeatherQueryAPIKey(newValue);
        break;
    }
    case CDC_CMD_LAT:
    {
        theSetup->setLocationLatitude(newValue.toFloat());
        break;
    }
    case CDC_CMD_LON:
    {
        theSetup->setLocationLongitude(newValue.toFloat());
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
        theDController->setMinOutput(newValue.toInt());
        break;
    }
    case CDC_CMD_OMAX:
    {
        theDController->setMaxOutput(newValue.toInt());
        break;
    }
    case CDC_CMD_ATPX:
    {
      theSetup->setAmbientTemperatureExternal(newValue.toFloat());
      break;
    }
    default:
    {
        LOG_ALERT("setCmdProcessor", "SET function not supported for: " << var);
        return false;
        break;
    }
  }

  if (CDCCommands.at(var.c_str()).saveToConfig) {
    setConfigUpdated();
  };
  return true;
}

