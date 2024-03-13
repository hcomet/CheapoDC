// ******************************************************************
// CheapoDC Web Server, Web Sockets and TCP API 
// Cheap and easy Dew Controller
// Details at https://github.com/hcomet/CheapoDC
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************

#include <Arduino.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebSrv.h"
#include <Update.h>
#include "FS.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <map>
#include "CDCdefines.h"
#include <EasyLogger.h>
#include "CDCSetup.h"
#include "CDCWebSrvr.h"
#include "CDCommands.h"

size_t content_len;
AsyncWebServer * CDCWebServer;

#ifdef CDC_ENABLE_WEB_SOCKETS
#if (WS_MAX_QUEUED_MESSAGES < 64)
#error "WS_MAX_QUEUED_MESSAGES in AsyncWebSocket.h must be set to 64 or higher to handle web socket queue properly!"
#endif
AsyncWebSocket * CDCWebSocket;
#endif
AsyncServer * CDCTCPServer;

#ifdef CDC_ENABLE_WEB_AUTH
const char* http_username = CDC_DEFAULT_WEB_ID;
const char* http_password = CDC_DEFAULT_WEB_PASSWORD;
#endif

const char* filelist = "%TMFL%";

const char* reboot =
"<head><title>reboot</title>"
"</head><body><h1>Reboot Device</h1></body>";

// Print progress for OTA Update
void printProgress(size_t prg, size_t sz) {
    LOG_ALERT("OTA Update", "OTA progress: " << ((prg*100)/content_len));
}

void notFound(AsyncWebServerRequest *request) {

    request->send(404, "text/plain", "Not found");
}

// Web OTA update handler
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
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "20");  
    response->addHeader("Location", "/");
    request->send(response);

    LOG_DEBUG("handleDoFileUpload","End file." );
  }
}



// File download processor
void processDownload(AsyncWebServerRequest *request ) {
  AsyncWebParameter* p = request->getParam("file");

  LOG_DEBUG("processDownload", "p.name: " << p->name().c_str());
  LOG_DEBUG("processDownload", "p.value: " << p->value().c_str());

  request->send(LittleFS, p->value().c_str(), String(), true);
}

// File deletion processor
void processDelete(AsyncWebServerRequest *request ) {
  AsyncWebParameter* p = request->getParam("file");

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
    {
      StaticJsonDocument<256> doc;
      String values = getResponse.units;
      int mode = getResponse.response.toInt();

      DeserializationError error = deserializeJson(doc, values);

      if (error) {
        LOG_ERROR("processor", "Deserialization error: " << error.c_str());
      } else {
        JsonArray modeText = doc["Mode"];
        response = String( modeText[ mode ].as<const char*>());
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

#ifdef CDC_ENABLE_WEB_SOCKETS
// Websocket based command processor
void handleWebsocketMessage( AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len ) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  String response = "";

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

      const uint8_t size = JSON_OBJECT_SIZE(6);
      StaticJsonDocument<size> requestJson;

      DeserializationError err = deserializeJson(requestJson, data);
      if (err) {
        LOG_ERROR("handleWebsocketMessage", "Deserialization error: " << err.c_str());
        return;
      } 
      
      if (requestJson["GET"]) {
        StaticJsonDocument<256> responseJson;
        char responseBuffer[256] = "";
        String  getCommand = String(requestJson["GET"].as<String>());
        cmdResponse getResponse;

        getResponse = getCmdProcessor( getCommand );

        if (getResponse.response != String()) {
          responseJson[getCommand] = getResponse.response;
          responseJson["UNITS"] = getResponse.units;

          LOG_DEBUG("handleWebsocketMessage", "response: <"<< getResponse.response.length() << ">: " << getResponse.response);
          LOG_DEBUG("handleWebsocketMessage", "response: <"<< getResponse.units.length() << ">: " << getResponse.units);
          LOG_DEBUG("handleWebsocketMessage", "responseJson[\"" << getCommand << "\"]: "  << responseJson[getCommand].as<String>());
          LOG_DEBUG("handleWebsocketMessage", "responseJson[\"UNITS\"]: "  << responseJson["UNITS"].as<String>());
        
        } else {
          responseJson["ERROR"] = "Undefined";
        }

        if (serializeJson(responseJson, responseBuffer) == 0) {
          LOG_ERROR("SaveConfig", "Failed to serialize GET request for: " << getCommand);
        } else {
          client->text(responseBuffer);
        }

        LOG_DEBUG("handleWebsocketMessage", "GET request: " << getCommand << " Response = " << response);
      } 

      if (requestJson["SET"]) {  
        JsonObject putCommand = requestJson["SET"];
        
        for (JsonPair kv : putCommand) {
          LOG_DEBUG("handleWebsocketMessage", "Processing SET command: " << kv.key().c_str() << " value: " << kv.value().as<const char*>() );

          if (!setCmdProcessor(String(kv.key().c_str() ), String(kv.value().as<const char*>() ))) {
            LOG_ERROR("handleWebsocketMessage", "SET command failure: " << kv.key().c_str() << " value: " << kv.value().as<const char*>() );
          }
          
        }

      }    
  }
}

void handleWebsocketEvent( AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
        LOG_ALERT("handleWebsocketEvent", "WebSocket client " << client->id() << " connected from " << client->remoteIP().toString().c_str());
        // Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        LOG_ALERT("handleWebsocketEvent", "WebSocket client " << client->id() << " disconnected");
        //Serial.printf("WebSocket client #%u disconnected\n", client->id());
        server->cleanupClients();
        break;
    case WS_EVT_DATA:
        handleWebsocketMessage( client, arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
  }
}
#endif
String processClientRequest(uint8_t *data, size_t len) {
  String response = "";
  const uint8_t size = JSON_OBJECT_SIZE(6);
  StaticJsonDocument<size> requestJson;

  DeserializationError err = deserializeJson(requestJson, data);
  if (err) {
    LOG_ERROR("processClientRequest", "Deserialization error: " << err.c_str());
    response = String("{\"RESULT\":-1}");
    return response;
  } 
  
  if (requestJson["GET"]) {
    StaticJsonDocument<256> responseJson;
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
      responseJson["RESULT"] = -2;
    }

    if (serializeJson(responseJson, responseBuffer) == 0) {
      LOG_ERROR("processClientRequest", "Failed to serialize GET request for: " << getCommand);
      response = String("{\"RESULT\":-1}");
    } else {
      response = String(responseBuffer);
    }

    LOG_DEBUG("processClientRequest", "GET request: " << getCommand << " Response = " << response);
  } 

  if (requestJson["SET"]) {  
    JsonObject putCommand = requestJson["SET"];
    response = String("{\"RESULT\":0}");
    for (JsonPair kv : putCommand) {
      LOG_DEBUG("handleWebsocketMessage", "Processing SET command: " << kv.key().c_str() << " value: " << kv.value().as<const char*>() );

      if (!setCmdProcessor(String(kv.key().c_str() ), String(kv.value().as<const char*>() ))) {
        LOG_ERROR("handleWebsocketMessage", "SET command failure: " << kv.key().c_str() << " value: " << kv.value().as<const char*>() );
        response = String("{\"RESULT\":-1}");
      }
      
    }

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
    if (!setCmdProcessor(request->argName(i).c_str(), String(request->arg(i).c_str())))
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

  // Create Server objects
  CDCWebServer = new AsyncWebServer(CDC_DEFAULT_WEBSRVR_PORT);
  #ifdef CDC_ENABLE_WEB_SOCKETS
  // Enable Webseckets on the web server for Websocket API
  CDCWebSocket = new AsyncWebSocket( CDC_DEFAULT_WEBSOCKET_URL);
  
  // Websocket handlers
  CDCWebSocket->onEvent(&handleWebsocketEvent);
  CDCWebServer->addHandler(CDCWebSocket);
  #endif

  // Enable TCP Server for TCP API
  CDCTCPServer = new AsyncServer(CDC_DEFAULT_TCP_SERVER_PORT);

  // TCP Server Handlers
  CDCTCPServer->onClient(&handleTCPClient, CDCTCPServer);
  CDCTCPServer->begin();
  
   #ifdef CDC_ENABLE_WEB_SOCKETS
  #endif

  // Webserver Handlers
  CDCWebServer->on("/CDCStyle.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/CDCStyle.css", "text/css");
  });

  // If Websockets enabled then set up the web socket based config page as default.
  #ifdef CDC_ENABLE_WEB_SOCKETS

  CDCWebServer->on("/CDCws.js", HTTP_GET, [](AsyncWebServerRequest *request) {
   request->send(LittleFS, "/CDCws.js", "text/javascript");
  });

  CDCWebServer->on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
    request->send(LittleFS, "/configws.html", "text/html", false, processor);
  });

  // Allow non-web socket config page to be served but it will need an explicit reference by a browser
  CDCWebServer->on("/hiddenconfig", HTTP_GET, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
    request->send(LittleFS, "/config.html", "text/html", false, processor);
  });

  #else

  // No websockets so only enable the non-websocket config
  CDCWebServer->on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
    request->send(LittleFS, "/config.html", "text/html", false, processor);
  });

  #endif

  // non-webseocket javascript can always be served
  CDCWebServer->on("/CDCjs.js", HTTP_GET, [](AsyncWebServerRequest *request) {
   request->send(LittleFS, "/CDCjs.js", "text/javascript");
  });

  // dashboard is default page
  CDCWebServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
   request->send(LittleFS, "/dashboard.html", "text/html", false, processor);
  });

  CDCWebServer->on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
    request->send(LittleFS, "/dashboard.html", "text/html", false, processor);
  });

  // Device management
  CDCWebServer->on("/otaIndex", HTTP_GET, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
    request->send(LittleFS, "/otaindex.html",  "text/html", false, processor);
  });

  // File management
  CDCWebServer->on("/listFiles", HTTP_GET, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
    request->send(LittleFS, "/listfiles.html", String(), false, processor);
  });

  CDCWebServer->on("/filelist", HTTP_GET, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
    request->send_P(200, "text/html", filelist, processor);
  });

  // Assorted special function pages and images
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
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
    processDownload(request);
    request->send(LittleFS, "/listfiles.html", String(), false, processor);
  });

  CDCWebServer->on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
    processDelete(request);
    request->send_P(200, "text/html", filelist, processor);
  });

  CDCWebServer->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
    request->send(200, "text/html", reboot);
    ESP.restart();
  });

  #ifdef CDC_ENABLE_WEB_AUTH
  CDCWebServer->on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(401);
    LOG_ALERT("setupServers", "Logout web client");
  });
  #endif

  // Handling the Firmware Web OTA Update
  CDCWebServer->on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
  },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
      handleDoOTAUpdate(request, filename, index, data, len, final);
    }
  );

   #ifdef ESP32
  Update.onProgress(printProgress);
  #endif

  // Handle file uploads
  CDCWebServer->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
  },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
      handleDoFileUpload(request, filename, index, data, len, final);
    }
  );

  // Handle SET commands sent via HTTP_POST
  CDCWebServer->on("/setvalue", HTTP_POST, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
    request->send(200, "text/plain", handleSetValuePost(request));
    } 
  );

  // Handle GET commands sent via HTTP_GET
  CDCWebServer->on("/getvalue", HTTP_POST, [](AsyncWebServerRequest *request) {
  #ifdef CDC_ENABLE_WEB_AUTH
    if(!request->authenticate(http_username, http_password))
    return request->requestAuthentication();
  #endif
    request->send(200, "text/plain", handleGetValue(request));
    } 
  );

  CDCWebServer->onNotFound([](AsyncWebServerRequest *request){request->send(404);});
  CDCWebServer->begin();

}
