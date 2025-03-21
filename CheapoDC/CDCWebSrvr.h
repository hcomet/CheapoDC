// ******************************************************************
// CheapoDC Web Server, Web Sockets and TCP API 
// Cheap and easy Dew Controller
// Details at https://github.com/hcomet/CheapoDC
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************
#ifndef MY_CDCWEBSRVR
#define MY_CDCWEBSRVR
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "CDCdefines.h"

// Webserver and TCP for Web UI and API
class CDCWebServer;
class CDCTCPServer;

void setupServers(void);

const char* httpFirmwareUpdateAvailable(void);
void updateFirmware(void);
void setupHttpOTA();
void cleanupHttpOTA();

#endif