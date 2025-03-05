// ******************************************************************
// CheapoDC Command processor
// Cheap and easy Dew Controller
// Details at https://github.com/hcomet/CheapoDC
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************
#ifndef MY_CDCCMDS_H
#define MY_CDCCMDS_H
#include <Arduino.h>
#include <map>
#include "CDCdefines.h"
#include "CDCvars.h"

struct CDCommand
{
    int number;
    uint8_t saveToConfig;
    std::string units;
};

struct cmdResponse
{
    String response;
    String units;
};

enum    cmdCaller
{
    LOADCONFIG = 0,
    WEBAPI,
    TCPAPI,
    MAXCMDCALLERS
};

extern std::map<std::string, CDCommand> CDCCommands;

// Process CDC get commands and return results
cmdResponse getCmdProcessor(const String &var);

// Process CDC set commands
bool setCmdProcessor(const String &var, String newValue, cmdCaller caller);

#if defined(CDC_ENABLE_CMDQUEUE) || defined(CDC_ENABLE_WEB_SOCKETS)
// Additional operations queue for keeping longer processes from impacting command responses

extern void clearCmdPostProcessQueue();
extern void runCmdPostProcessQueue();

#endif // CDC_ENABLE_CMDQUEUE

#endif // MY_CDCCMDS_H