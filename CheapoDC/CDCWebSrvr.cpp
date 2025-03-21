// ******************************************************************
// CheapoDC Web Server, Web Sockets and TCP API 
// Cheap and easy Dew Controller
// Details at https://github.com/hcomet/CheapoDC
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebAuthentication.h>
#include <Update.h>
#include "FS.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <map>
//#include <esp_task_wdt.h>
#include "CDCdefines.h"
#include "CDCEasyLogger.h"
#include "CDCSetup.h"
#include "CDCWebSrvr.h"
#include "CDCommands.h"
#ifdef CDC_ENABLE_HTTP_OTA
#include "CDCesp32FOTA.hpp"
#endif

size_t content_len;
// Webserver and eventsource for messaging
AsyncWebServer * CDCWebServer;
AsyncEventSource * CDCEvents;

// Web based Firmware update 
#ifdef CDC_ENABLE_HTTP_OTA
esp32FOTA * CDCFota;
#endif
bool updateFirmwareNow = false;
char newFirmwareVersion[16] = "";

// TCP API service
AsyncServer * CDCTCPServer;

AsyncAuthenticationMiddleware * basicAuth;
const char* http_username = CDC_DEFAULT_WEB_ID;
const char* http_password = CDC_DEFAULT_WEB_PASSWORD;
String digestHash = generateDigestHash(http_username, http_password, "CheapoDC");

const char* filelist = "%TMFL%";

const char* reboot =
"<head><title>reboot</title>"
"</head><body><h1>Reboot Device</h1></body>";

// Manual OTA Update support
// Print progress for manual OTA Update
void printProgress(size_t prg, size_t sz) {
    LOG_ALERT("OTA Update", "OTA progress: " << ((prg*100)/content_len));
}

// Manual OTA update handler
void handleDoOTAUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) 
{
    
  if (!index){
    LOG_ALERT("handleDoOTAUpdate","Firmware Update");
    content_len = request->contentLength();
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }

  if (final) {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "20");  
    response->addHeader("Location", "/otaIndex");
    request->send(response);
    if (!Update.end(true)){
      Update.printError(Serial);
    } else {
      LOG_ALERT("handleDoOTAUpdate","Update complete");
      Serial.flush();
      ESP.restart();
    }
  }
}

// Handler for Web HTTP OTA update
#ifdef CDC_ENABLE_HTTP_OTA
String handleDoHtmlFOTAUpdate(AsyncWebServerRequest *request )
{
  String response;
  // Set for update to happen out of band if Password matches
  const AsyncWebParameter* p = request->getParam("password", true);
  if (p->value().equals(String(theSetup->getPasswordHash()))) 
  {
    updateFirmwareNow = true;
    response = String("Update scheduled.")
    LOG_DEBUG("handleDoHtmlFOTAUpdate", "HTTP OTA Update scheduled.");
  } else {
    response = String("Password wrong.")
    LOG_DEBUG("handleDoHtmlFOTAUpdate", "HTTP OTA Failure: Wrong Password.");
  }

  return response;
}
#endif // CDC_ENABLE_HTTP_OTA

// Page not found
void notFound(AsyncWebServerRequest *request) {

  request->send(404, "text/plain", "Not found");
}

// File upload handler
void handleDoFileUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) 
{
    class   file;
    const char* mode;
  

  if (!index){
    LOG_DEBUG("handleDoFileUpload","Upload file name: " << filename );
    LOG_DEBUG("handleDoFileUpload","Upload file size: " << request->contentLength() );
    LOG_DEBUG("handleDoFileUpload","Upload index: " << index );
    LOG_DEBUG("handleDoFileUpload","Upload len: " << len );
    LOG_DEBUG("handleDoFileUpload","Upload final: " << final );
    content_len = request->contentLength();
    LOG_DEBUG("handleDoFileUpload","Start file: ");

    mode = FILE_WRITE;
  } else {
    mode = FILE_APPEND;
  }

  File file = LittleFS.open("/" + filename, mode);
  if(!file){
    LOG_ERROR("handleDoFileUpload", "Failed to open file for writing: " << "/" + filename);
    return ;
  } 

  for (int i = 0; i < len; i++) {
    
    if ( file.write( (char) data[i] ) == 0) {
      LOG_ERROR("handleDoFileUpload", "Failed to write character to file");
      file.close();
      return;
    }

  }

  file.close();

  if (final) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain");
    response->addHeader("Server", "ESP Async Web Server");
    request->send(response);

    LOG_DEBUG("handleDoFileUpload","End file." );
  }
}



// File download processor
void processDownload(AsyncWebServerRequest *request ) {
  const AsyncWebParameter* p = request->getParam("file");

  LOG_DEBUG("processDownload", "p.name: " << p->name().c_str());
  LOG_DEBUG("processDownload", "p.value: " << p->value().c_str());
  request->send(LittleFS, p->value().c_str(), String(), true);
  
}

// File deletion processor
void processDelete(AsyncWebServerRequest *request ) {
  const AsyncWebParameter* p = request->getParam("file");

  LOG_DEBUG("processDelete", "p.name: " << p->name().c_str());
  LOG_DEBUG("processDelete", "p.value: " << p->value().c_str());

  LittleFS.remove(p->value().c_str());
}

String htmlEscape(const char* inputString) {
  String escapeString = String(inputString);

  escapeString.replace( "&", "&amp;");
  escapeString.replace( "%", "&percnt;");
  escapeString.replace( "\"", "&quot;");
  escapeString.replace( "'", "&#39;");
  escapeString.replace( "<", "&lt;");
  escapeString.replace( ">", "&gt;");
  escapeString.replace( "@", "&commat;");

  return escapeString;
}

String processor(const String &var) {
  String response = String();
  cmdResponse getResponse;
  int command;

  if (CDCCommands.count(var.c_str()) == 0)
  {
    LOG_DEBUG("processor", "Command not found: " << var);
    return response;
  }

  command = CDCCommands.at(var.c_str()).number;
  
  getResponse = getCmdProcessor( var );

  LOG_DEBUG("processor", "Process Command: " << var << " number " << command);

  switch (command)
  {
    case CDC_CMD_TMFL: 
    {
      response = getResponse.response.c_str();
      break;
    }
    case CDC_CMD_DCM:
    case CDC_CMD_DCTM:
    case CDC_CMD_SPM: 
    case CDC_CMD_WS:
    case CDC_CMD_CPM0:
    case CDC_CMD_CPM1:
    case CDC_CMD_CPM2:
    case CDC_CMD_CPM3:
    case CDC_CMD_CPM4:
    case CDC_CMD_CPM5:
    {
#if ARDUINOJSON_VERSION_MAJOR>=7
      JsonDocument doc;
#else
      StaticJsonDocument<256> doc;
#endif
      String values = getResponse.units;
      int unitValue = getResponse.response.toInt();

      DeserializationError error = deserializeJson(doc, values);

      if (error) {
        LOG_ERROR("processor", "Deserialization error: " << error.c_str());
      } else {
        JsonArray unitText;
        if (command == CDC_CMD_WS)
        {
          unitText = doc["Source"];
        }
        else
        {
          unitText = doc["Mode"];
        }
        response = String( unitText[ unitValue ].as<const char*>());
      }

      break;
    }
    
    case CDC_CMD_WICON:
      {
        if (theSetup->getInWiFiAPMode()){
          response = String("/WiFiAPmode.png");
        } else  {
          response = getResponse.response.c_str();
        }
      }
      break;

    default:
      response = htmlEscape(getResponse.response.c_str());
      response += getResponse.units;
      break;

  }

  return response;
}

String processClientRequest(uint8_t *data, size_t len) {
  String response = "";
#if ARDUINOJSON_VERSION_MAJOR>=7
    JsonDocument requestJson;
#else
  const uint8_t size = JSON_OBJECT_SIZE(6);
  StaticJsonDocument<size> requestJson;
#endif

  DeserializationError err = deserializeJson(requestJson, data);
  if (err) {
    LOG_ERROR("processClientRequest", "Deserialization error: " << err.c_str());
    response = String("{\"RESULT\":-2}");
    return response;
  } 
  
  if (requestJson["GET"]) {
#if ARDUINOJSON_VERSION_MAJOR>=7
    JsonDocument responseJson;
#else
    StaticJsonDocument<256> responseJson;
#endif
    char responseBuffer[256] = "";
    String  getCommand = String(requestJson["GET"].as<String>());
    cmdResponse getResponse;

    getResponse = getCmdProcessor( getCommand );

    if (getResponse.response != String()) {
      responseJson[getCommand] = getResponse.response;
      responseJson["UNITS"] = getResponse.units;

      LOG_DEBUG("processClientRequest", "response: <"<< getResponse.response.length() << ">: " << getResponse.response);
      LOG_DEBUG("processClientRequest", "response: <"<< getResponse.units.length() << ">: " << getResponse.units);
      LOG_DEBUG("processClientRequest", "responseJson[\"" << getCommand << "\"]: "  << responseJson[getCommand].as<String>());
      LOG_DEBUG("processClientRequest", "responseJson[\"UNITS\"]: "  << responseJson["UNITS"].as<String>());
    
    } else {
      // Command not found
      responseJson["RESULT"] = -2;
    }

    if (serializeJson(responseJson, responseBuffer) == 0) {
      LOG_ERROR("processClientRequest", "Failed to serialize GET request for: " << getCommand);
      response = String("{\"RESULT\":-1}");
    } else {
      response = String(responseBuffer);
    }

    LOG_DEBUG("processClientRequest", "GET request: " << getCommand << " Response = " << response);
    
  } else if (requestJson["SET"]) {  
    JsonObject putCommand = requestJson["SET"];
    response = String("{\"RESULT\":-2}");
    for (JsonPair kv : putCommand) {
      LOG_DEBUG("processClientRequest", "Processing SET command: " << kv.key().c_str() << " value: " << kv.value().as<const char*>() );

      if (!setCmdProcessor(String(kv.key().c_str() ), String(kv.value().as<const char*>() ), TCPAPI)) {
        LOG_ERROR("processClientRequest", "SET command failure: " << kv.key().c_str() << " value: " << kv.value().as<const char*>() );
        response = String("{\"RESULT\":-2}");
      } else {
        response = String("{\"RESULT\":0}");
      }
      
    }

  }  else {
    response = String("{\"RESULT\":-2}");
  }  
  return response;
}

void handleTCPData(void *arg, AsyncClient *client, void *data, size_t len)
{
  String response = String();
  LOG_DEBUG("handleTCPData", "Data received from client: " << client->remoteIP().toString().c_str());
	LOG_DEBUG("handleTCPData", "<<" << String((uint8_t *)data, len) << ">>");
  
  if (( *(uint8_t *)data == '\003') || ( *(uint8_t *)data == '\004')) {
    LOG_ALERT("handleTCPData","Close connection request from client: " << client->remoteIP().toString());
    client->close();
  } else {
    response = processClientRequest( (uint8_t *)data, len);

    if (response.length() != 0) {
      response += String("\n");
      if (client->space() > strlen(response.c_str()) && client->canSend())
      {
        client->add(response.c_str(), strlen(response.c_str()));
        if (!client->send()) {
          LOG_ERROR("handleTCPData", "Error sending response to: " << client->remoteIP().toString());
        }
      }
    }
  }
}


static void handleTCPError(void *arg, AsyncClient *client, int8_t error)
{
	LOG_ERROR("handleTCPError", "Connection error: " << client->errorToString(error) << " from client: " << client->remoteIP().toString().c_str());
}

static void handleTCPDisconnect(void *arg, AsyncClient *client)
{
  LOG_ALERT("handleTCPDisconnect", "Client disconnected: " << client->remoteIP().toString().c_str());
}

static void handleTCPTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
  LOG_ALERT("handleTCPTimeOut", "Client ACK timeout: " << client->remoteIP().toString().c_str());
}

void handleTCPClient(void *arg, AsyncClient *client)
{
	LOG_ALERT("handleTCPClient", "New client has been connected to TCP server, ip: " << client->remoteIP().toString());
	// register events
	client->onData(&handleTCPData, NULL);
	client->onError(&handleTCPError, NULL);
	client->onDisconnect(&handleTCPDisconnect, NULL);
	client->onTimeout(&handleTCPTimeOut, NULL);
}

String handleSetValuePost(AsyncWebServerRequest *request)
{
  String response = String();
  int args = request->args();
  char strBuf[512] = {};
  for (int i = 0; i < args; i++)
  {
    if (!setCmdProcessor(request->argName(i).c_str(), String(request->arg(i).c_str()), WEBAPI))
    {
      LOG_ERROR("handleSetValuePost", "SET command failure: " << request->argName(i).c_str() << " value: " << request->arg(i).c_str());
      return response;
    }
    sprintf(strBuf, "{\"%s\":\"%s\"}", request->argName(i).c_str(), request->arg(i).c_str());
    
    response += String(strBuf);
  }
  LOG_DEBUG("handleSetValuePost", "Response: " << response.c_str());
  return response;
}

String handleGetValue(AsyncWebServerRequest *request)
{
  String response = String();
  int args = request->args();
  char strBuf[512] = {};
  for (int i = 0; i < args; i++)
  {
    sprintf(strBuf, "{\"%s\":\"%s\"}", request->arg(i).c_str(), processor( request->arg(i) ).c_str());
    response += String(strBuf);
  }
  LOG_DEBUG("handleGetValue", "Response: " << response);
  return response;
}

/*
 * setup function
 */
void setupServers(void) {
  // Initialize HTTP OTA to check for FW update
  setupHttpOTA();
  // Cleanup HTTP OTA to free heap
  cleanupHttpOTA();

  // Create Server objects
  CDCWebServer = new AsyncWebServer(CDC_DEFAULT_WEBSRVR_PORT);
  CDCEvents = new AsyncEventSource("/events");
  
  basicAuth = new AsyncAuthenticationMiddleware();
  basicAuth->setUsername(CDC_DEFAULT_WEB_ID);
  //basicAuth->setPassword(http_password);
  basicAuth->setRealm("CheapoDC");
  basicAuth->setPasswordHash(theSetup->getPasswordHash());
  basicAuth->setAuthFailureMessage("Authentication Failed");
  basicAuth->setAuthType(AsyncAuthType::AUTH_DIGEST);
  //basicAuth->generateHash();
  CDCWebServer->addMiddleware(basicAuth);
  CDCEvents->addMiddleware(basicAuth);

  // Enable TCP Server for TCP API
  CDCTCPServer = new AsyncServer(CDC_DEFAULT_TCP_SERVER_PORT);

  // TCP Server Handlers
  CDCTCPServer->onClient(&handleTCPClient, CDCTCPServer);
  CDCTCPServer->begin();
  
  // Webserver Handlers
  CDCWebServer->on("/CDCStyle.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/CDCStyle.css", "text/css");
  });

  CDCWebServer->on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/config.html", "text/html", false, processor);
  });

  // javascript 
  CDCWebServer->on("/CDCjs.js", HTTP_GET, [](AsyncWebServerRequest *request) {
   request->send(LittleFS, "/CDCjs.js", "text/javascript");
  });

  // dashboard is default page
  CDCWebServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
   request->send(LittleFS, "/dashboard.html", "text/html", false, processor);
  });

  CDCWebServer->on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/dashboard.html", "text/html", false, processor);
  });

  // Device management
  CDCWebServer->on("/otaIndex", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/otaindex.html",  "text/html", false, processor);
  });

  // File management
  CDCWebServer->on("/listFiles", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/listfiles.html", String(), false, processor);
  });

  CDCWebServer->on("/filelist", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", filelist, processor);
  });

  // Assorted special function pages and images
  CDCWebServer->on(CDC_EXTERNALSOURCE_ICONURL, HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, CDC_EXTERNALSOURCE_ICONURL, "image/png");
  });

  CDCWebServer->on("/WiFiAPmode.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/WiFiAPmode.png", "image/png");
  });

  CDCWebServer->on("/logout.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/logout.png", "image/png");
  });

  CDCWebServer->on("/cheapo.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/cheapo.png", "image/png");
  });

  CDCWebServer->on("/favicon.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/favicon.png", "image/png");
  });

  CDCWebServer->on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    processDownload(request);
  });

  CDCWebServer->on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    processDelete(request);
    request->send(200, "text/html", filelist, processor);
  });

  CDCWebServer->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", reboot);
    ESP.restart();
  });

  CDCWebServer->on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(401);
    LOG_ALERT("setupServers", "Logout web client");
  });

  // Handling the Firmware Manual File OTA Update
  CDCWebServer->on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
  },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
      handleDoOTAUpdate(request, filename, index, data, len, final);
    }
  );

  // Handling the Firmware Web HTML OTA Update
  #ifdef CDC_ENABLE_HTTP_OTA
  CDCWebServer->on("/updatehtml", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", handleDoHtmlFOTAUpdate(request));
    }
  );   
  #endif   // CDC_ENABLE_HTTP_OTA                                                                          


  #ifdef ESP32
  Update.onProgress(printProgress);
  #endif

  // Handle file uploads
  CDCWebServer->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
  },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
      handleDoFileUpload(request, filename, index, data, len, final);
    }
  );

  // Handle SET commands sent via HTTP_POST
  CDCWebServer->on("/setvalue", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", handleSetValuePost(request));
    } 
  );

  // Handle GET commands sent via HTTP_GET
  CDCWebServer->on("/getvalue", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", handleGetValue(request));
    } 
  );

  // Handle SET Password
  CDCWebServer->on("/setpassword", HTTP_POST, [](AsyncWebServerRequest *request) {
    if ((request->hasParam("newpassword", true)) && (request->hasParam("oldpassword", true))) {
      const AsyncWebParameter* nP = request->getParam("newpassword", true);
      const AsyncWebParameter* oP = request->getParam("oldpassword", true);
      if (oP->value().equals(String(theSetup->getPasswordHash()))) 
      {
        theSetup->setPasswordHash(nP->value().c_str());
        basicAuth->setPasswordHash(theSetup->getPasswordHash());
        LOG_DEBUG("/setpassword", "Parameter: " << nP->value().c_str() << ":" << oP->value().c_str());
        request->send(200, "text/plain", "Password set");
      } else {
        LOG_ERROR("/setpassword", "Old password wrong: " << oP->value() << " vs " << theSetup->getPasswordHash());
        request->send(200, "text/plain", "Password not set");
      }
    } else {
      LOG_ERROR("/setpassword", "No newpassword parameter.");
      request->send(200, "text/plain", "Password not set");
    }
  });

  // Handle WiFi configuration
  // Allow fetching of WiFi configuration file
  CDCWebServer->on("/CDCWiFi.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/CDCWiFi.json", "text/html");
  });

  // Handle WiFi update
  CDCWebServer->on("/setwifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("wifi", true)) {
      const AsyncWebParameter* p = request->getParam("wifi", true);
      if (!theSetup->saveWiFiConfig(p->value())) {
        LOG_ERROR("/setwifi", "Failed to save wifi configuration.");
        request->send(200, "text/plain", "Wifi not set");
      } else {
        LOG_DEBUG("/setwifi", "Parameter: " << p->value().c_str());
        request->send(200, "text/plain", "Wifi set");
      }
    } else {
      LOG_ERROR("/setwifi", "No wifi parameter.");
      request->send(200, "text/plain", "Wifi not set");
    }
  });

  CDCWebServer->onNotFound([](AsyncWebServerRequest *request){request->send(404);});

  CDCEvents->onConnect([](AsyncEventSourceClient *client) {
    LOG_INFO("CDCEvents","SSE Client connected! ID: " << client->lastId());
    client->send("Perform HTTP OTA", NULL, millis(), 1000);
  });

  CDCEvents->onDisconnect([](AsyncEventSourceClient *client) {
    LOG_INFO("CDCEvents", "SSE Client disconnected! ID: " << client->lastId());
  });
  
  CDCWebServer->addHandler(CDCEvents);
  CDCWebServer->begin();

}


// Web HTTP based OTA Functions

const char* httpFirmwareUpdateAvailable(void) {
  return newFirmwareVersion;
}

#ifdef CDC_ENABLE_HTTP_OTA
// Progress callback
void CDCFotaProgressCB(size_t progress, size_t size) {
  int percentDone = (progress*100)/size;
  if ((percentDone%5)==0) {
    CDCEvents->send(String(percentDone), "progress", millis());
    LOG_ALERT("CDCFotaProgressCB", "OTA progress: " << percentDone);
    //esp_task_wdt_reset();
  }
}

// Partition Update Finished callback
void CDCFotaFinishedCB(int partition, bool needs_restart) {
  String message;
  message = String("Update complete for " + 
    String(partition==U_SPIFFS ? "data" : "firmware") + " partition" +
    String(needs_restart ? ". Wait for Reboot." : "."));
  CDCEvents->send( message, "message", millis());
  LOG_ALERT("CDCFotaFinishedCB", "OTA Update complete for partition: " << partition << " Restart: " << needs_restart);
  if (needs_restart) {
    CDCEvents->send( "Refresh", "reboot", millis());
    sleep(1);
    ESP.restart();
  }
}

// Error Callback
void CDCFotaCheckErrorCB(int partition, int errorCode) {
  String message = String("Error: Update failure for " + String(partition==U_SPIFFS ? "data" : "firmware") + " partition.");
  LOG_ALERT("CDCFotaCheckErrorCB", message.c_str() << " Error: " << errorCode);
  CDCEvents->send( message, "error", millis());

}

void CDCFotaUpdateErrorCB(int partition) {
  String message = String("Error: Update failure for " + String(partition==U_SPIFFS ? "data" : "firmware") + " partition.");
  LOG_ALERT("CDCFotaErrorCB", message.c_str());
  CDCEvents->send( message, "error", millis());
}
#endif // CDC_ENABLE_HTTP_OTA

// Set up HTTP OTA update for firmware
//
void setupHttpOTA() {
#ifdef CDC_ENABLE_HTTP_OTA
  String CDCFotaID = String(String(programName) + "." + ESP.getChipModel());
  LOG_ALERT("setupHttpOTA", "Setting up HTTP-FOTA for: " << CDCFotaID);
  CDCFota = new esp32FOTA( CDCFotaID.c_str(), programVersion, false);
  
  // Set up the root CA for HTTPS
  File file = LittleFS.open(CDC_HTTP_OTA_ROOT_CA_FILE, "r");
  if (!file)
  {
    const char* root_ca = WEB_OTA_ROOT_CA;
    LOG_DEBUG("setupHttpOTA", "No Root CA file found: " << CDC_HTTP_OTA_ROOT_CA_FILE << " Using default.");
    CryptoMemAsset  *MyRootCA = new CryptoMemAsset("Root CA", root_ca, strlen(root_ca)+1 );
    CDCFota->setRootCA( MyRootCA );
  } else {
    file.close();
    LOG_ALERT("setupHttpOTA", "Root CA file found: " << CDC_HTTP_OTA_ROOT_CA_FILE << " Using it instead of default.");
    CryptoFileAsset *MyRootCA =  new CryptoFileAsset(CDC_HTTP_OTA_ROOT_CA_FILE, &LittleFS);
    CDCFota->setRootCA( MyRootCA );  
  }
  
  CDCFota->setManifestURL( theSetup->getHttpOTAURL() );
  CDCFota->setCertFileSystem(nullptr);
      
  CDCFota->setProgressCb( CDCFotaProgressCB );
  CDCFota->setUpdateFinishedCb( CDCFotaFinishedCB );
  CDCFota->setUpdateBeginFailCb(CDCFotaUpdateErrorCB);
  CDCFota->setUpdateCheckFailCb(CDCFotaCheckErrorCB);

  memset(newFirmwareVersion, '\0', sizeof(newFirmwareVersion));
  if (CDCFota->execHTTPcheck())
  {
    CDCFota->getPayloadVersion(newFirmwareVersion);
    LOG_ALERT("setupHttpOTA", "New version available: " << newFirmwareVersion);
  } else {
    LOG_DEBUG("setupHttpOTA", "NO new version available");
    strlcpy(newFirmwareVersion, CDC_NO_FW_UPDATE_RESPONSE, sizeof(newFirmwareVersion));
  }
#else // CDC_ENABLE_HTTP_OTA
  LOG_ALERT("setupHttpOTA", "Support for HTTP OTA not enabled.");
  memset(newFirmwareVersion, '\0', sizeof(newFirmwareVersion));
  strlcpy(newFirmwareVersion, CDC_NO_FW_UPDATE_RESPONSE, sizeof(newFirmwareVersion));
#endif // CDC_ENABLE_HTTP_OTA
}

void cleanupHttpOTA() {
#ifdef CDC_ENABLE_HTTP_OTA
  if (CDCFota) {
    delete CDCFota;
    CDCFota = nullptr;
    LOG_DEBUG("cleanupHttpOTA", "HTTP OTA object deleted.");
  }
#endif // CDC_ENABLE_HTTP_OTA
}

void updateFirmware(void) {
#ifdef CDC_ENABLE_HTTP_OTA
  if (updateFirmwareNow) 
  {
    bool continueUpdate = true;
    bool updateFWPartition = false;
    bool updateDataPartition = false;
    updateFirmwareNow = false;
    LOG_ALERT("updateFirmware", "Do HTML OTA.");

    setupHttpOTA();
    updateFWPartition = (CDCFota->getFirmwareURL()[0] != '\0');
    updateDataPartition = (CDCFota->getFlashFS_URL()[0] != '\0');

    CDCEvents->send(String((updateDataPartition)?"The data and firmware partitions will be updated.":"The firmware partition will be updated."), "message", millis());
    if (updateDataPartition)
    {
      CDCEvents->send("Backup data partition configuration files.", "status", millis());
      if (!theSetup->backupConfig()) {
        LOG_ERROR("updateFirmware", "Backup configuration files failed.");
        CDCEvents->send("Error: Data partition backup failed.", "error", millis());
        continueUpdate = false;
      } else {
        CDCEvents->send("Updating data partition. ", "status", millis());
        if (!CDCFota->execSPIFFSOTA()) {
          LOG_ERROR("updateFirmware", "Data partition update failure.");
          CDCEvents->send("Error: Data partition update failed.", "error", millis());
          continueUpdate = false;
        } else {
          if (!theSetup->restoreConfig()) {
            LOG_ERROR("updateFirmware", "Restore configs failed.");
            CDCEvents->send("Error: Restore data partition configuration files failed.", "error", millis());
            continueUpdate = false;
          }
        }
      }
    }
    if (continueUpdate && updateFWPartition) {
      CDCEvents->send("0", "progress", millis());
      CDCEvents->send("Updating firmware partition. ", "status", millis());
      if (!CDCFota->execOTA(U_FLASH, true)) {
        LOG_ERROR("updateFirmware", "Firmware partition update failure.");
        CDCEvents->send("Error: Firmware update failed.", "error", millis());
        continueUpdate = false;
      }
    } 
    if (!continueUpdate) {
      LOG_ERROR("updateFirmware","HTTP OTA Failure.");
      CDCEvents->send("Update failed, rebooting. Refresh browser when ready.", "message", millis());
      CDCEvents->send( "Wait", "reboot", millis());
      sleep(1);
      ESP.restart();
    }

  }
#endif // CDC_ENABLE_HTTP_OTA
}