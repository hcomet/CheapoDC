// ******************************************************************
// CheapoDC Web Server, Web Sockets and TCP API 
// Cheap and easy Dew Controller
// Details at https://github.com/hcomet/CheapoDC
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************
#ifndef MY_CDCWEBSRVR
#define MY_CDCWEBSRVR
#include <Arduino.h>
#include "CDCdefines.h"
#include <AsyncTCP.h>
#include "ESPAsyncWebSrv.h"

// AsyncWebServer CDCserver(CDC_DEFAULT_WEBSRVR_PORT);
class CDCWebServer;
#ifdef CDC_ENABLE_WEB_SOCKETS
class CDCWebSocket;
#endif
class CDCTCPServer;

void setupServers(void);

#endif