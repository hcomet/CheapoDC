// ******************************************************************
// CheapoDC variables
// Cheap and easy Dew Controller
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************
#ifndef MY_CDCVARS_H
#define MY_CDCVARS_H

#include <Arduino.h>
#include "CDCdefines.h"
#include <ESP32Time.h>
//#include <arduino-timer.h>

#include "CDCSetup.h"
#include "CDController.h"

extern char        programName[];    // Program name
extern char        programVersion[];        // program version

//extern Timer<> *CDCTimer;

class CDCSetup;
extern CDCSetup    *theSetup;                               // main setup class
class dewController;
extern dewController    *theDController;
extern ESP32Time   *theTime;

//extern bool        blink;
//extern int         blinkCount; 
//extern int         secCount;

//extern int lastMinute;


#endif 