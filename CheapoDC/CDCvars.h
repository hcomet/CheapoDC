// ******************************************************************
// CheapoDC global variables
// Cheap and easy Dew Controller
// Details at https://github.com/hcomet/CheapoDC
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************
#ifndef MY_CDCVARS_H
#define MY_CDCVARS_H

#include <Arduino.h>
//#include <ESP32Time.h>
#include "CDCdefines.h"
#include "CDCSetup.h"
#include "CDController.h"

extern char        programName[];    // Program name
extern char        programVersion[];        // program version

class CDCSetup;
extern CDCSetup    *theSetup;                               // main setup class
class dewController;
extern dewController    *theDController;

#endif 