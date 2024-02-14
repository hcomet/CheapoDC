// ******************************************************************
// CheapoDC Web Server setup and
// Cheap and easy Dew Controller
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
class CDCWebSocket;
class CDCTCPServer;

void setupServers(void);

#endif