// ******************************************************************
// CheapoDC Setup and configuration
// Cheap and easy Dew Controller
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************
#ifndef MY_CDCONTROLLER_H
#define MY_CDCONTROLLER_H

#include <Arduino.h>
#include "CDCdefines.h"

#include "CDCvars.h"

#define CDC_CONTROLLERMODE_JSONARRAY "{\"Mode\":[\"Automatic\",\"Manual\",\"Off\"]}"
enum    controllerMode{
            AUTOMATIC,          // 0
            MANUAL,             // 1
            OFF                 // 2   
};

#define CDC_TEMPERATUREMODE_JSONARRAY "{\"Mode\":[\"Weather Query\",\"External Input\"]}"
enum    temperatureMode{
            WEATHER_QUERY,       // 0
            EXTERNAL_INPUT          // 1
};

#define CDC_SETPOINTMODE_JSONARRAY "{\"Mode\":[\"Dew Point\",\"Temperature\",\"Midpoint\"]}"
enum    setPointMode {
            DEWPOINT,           // 0
            TEMPERATURE,         // 1
            MIDPOINT             // 2       
};


class   dewController {
    public:
            dewController(void);
    void    updateOutput( int output = -1 );

    void    setControllerMode( controllerMode mode );
    void    setTemperatureMode( temperatureMode mode );
    void    setSetPointMode( setPointMode mode );
    void    setSetPoint( float setPointTemperature );
    void    setTrackPointOffset( float trackPointOffset);
    void    setTrackingRange( float trackingRange );
    
    void    setMinOutput(int output);
    void    setMaxOutput(int output);

    int     getOutput() {return this->_currentOutput;};
    int     getMinOutput() {return this->_minimumOutputSetting;};
    int     getMaxOutput() {return this->_maximumOutputSetting;};
    controllerMode  getControllerMode(void) {return this->_currentControllerMode;};
    temperatureMode getTemperatureMode(void) {return this->_currentTemperatureMode;};
    setPointMode    getSetPointMode(void) {return this->_currentSetPointMode;};
    float           getSetPoint(void) {return this->_currentTemperatureSetPoint;};
    float           getTrackPointOffset(void) {return this->_currentTrackPointOffset;};
    float           getTrackPoint(void) {return this->_currentTrackPoint;}
    float           getTrackingRange(void) {return this->_currentTrackingRange;};

 
    private:
        controllerMode  _currentControllerMode;
        temperatureMode _currentTemperatureMode;
        setPointMode    _currentSetPointMode;
        int             _currentOutput;
        float           _currentTemperatureSetPoint;
        float _currentTrackPointOffset;
        float _currentTrackingRange;
        float _currentTrackPoint;
        int _minimumOutputSetting;
        int _maximumOutputSetting;
        
        int _calculateOutput( float currentTemperature, float setPoint, float range, float offset);

};

#endif