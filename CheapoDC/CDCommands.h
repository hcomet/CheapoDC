// ******************************************************************
// CheapoDC Command processor
// Cheap and easy Dew Controller
// Details at https://github.com/hcomet/CheapoDC
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************
#ifndef MY_CDCCMDS_H
#define MY_CDCCMDS_H
#include <Arduino.h>
#include "CDCdefines.h"
#include <map>
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

extern std::map<std::string, CDCommand> CDCCommands;

// Process CDC get commands and return results
cmdResponse getCmdProcessor(const String &var);

// Process CDC set commands
bool setCmdProcessor(const String &var, String newValue);

void setConfigUpdated();
void resetConfigUpdated();
bool getConfigUpdated();

#endif