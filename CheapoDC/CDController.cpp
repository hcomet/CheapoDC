// ******************************************************************
// CheapoDC Dew Controller configuration and management
// Cheap and easy Dew Controller
// Details at https://github.com/hcomet/CheapoDC
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************

#include <Arduino.h>
#include <ArduinoJson.h>
#include "CDCdefines.h"
#include "CDCEasyLogger.h"
#include "CDCvars.h"
#include "CDCSetup.h"
#include "CDController.h"

dewController::dewController(void)
{
    // Initilize all Controller pins as disabled
    LOG_DEBUG("dewController", "Setup and configure dew controller PWM outputs");
    for (int i = 0; i < MAX_CONTROLLER_PINS; i++)
    {
        this->_controllerPinSettings[i].controllerPinPin = CONTROLLER_PIN_NOT_CONFIGURED;
        this->_controllerPinSettings[i].controllerPinMode = CONTROLLER_PIN_MODE_DISABLED;
        this->_controllerPinSettings[i].controllerPinOutput = 0;
    }

    // If defaults set in CDCdefines.h for PIN0 and PIN1 then configure
#ifdef CDC_DEFAULT_CONTROLLER_PIN0 // PIN0 is required before PIN1
    LOG_DEBUG("dewController", "Controller Pin 0: " << CDC_DEFAULT_CONTROLLER_PIN0);
  
    this->_controllerPinSettings[CONTROLLER_PIN0].controllerPinPin = CDC_DEFAULT_CONTROLLER_PIN0;
    this->_controllerPinSettings[CONTROLLER_PIN0].controllerPinMode = CONTROLLER_PIN_MODE_CONTROLLER;

#ifdef CDC_DEFAULT_CONTROLLER_PIN1
    LOG_DEBUG("dewController", "Contorller Pin 1: " << CDC_DEFAULT_CONTROLLER_PIN1);

    this->_controllerPinSettings[CONTROLLER_PIN1].controllerPinPin = CDC_DEFAULT_CONTROLLER_PIN1;
    this->_controllerPinSettings[CONTROLLER_PIN1].controllerPinMode = CONTROLLER_PIN_MODE_CONTROLLER;
#endif // CDC_DEFAULT_CONTROLLER_PIN1
#endif // CDC_DEFAULT_CONTROLLER_PIN0

    this->_currentControllerMode = CDC_DEFAULT_CONTROLLER_MODE;
    this->_currentTemperatureMode = CDC_DEFAULT_TEMPERATURE_MODE;
    this->_currentSetPointMode = CDC_DEFAULT_SET_POINT_MODE;
    this->_currentOutput = CDC_MINIMUM_CONTROLLER_OUTPUT;
    this->_currentTemperatureSetPoint = CDC_DEFAULT_TEMPERATURE_SET_POINT;
    this->_currentTrackPointOffset = CDC_DEFAULT_TRACK_POINT_OFFSET;
    this->_currentTrackingRange = CDC_DEFAULT_TRACKING_RANGE;
    this->_currentTrackPoint = 0.0;
    this->_minimumOutputSetting = CDC_MINIMUM_CONTROLLER_OUTPUT;
    this->_maximumOutputSetting = CDC_MAXIMUM_CONTROLLER_OUTPUT;
    this->_controllerEnabled = false;
}

bool dewController::setEnabled()
{
    this->_controllerEnabled = true;
    LOG_DEBUG("setEnabled", "Controller enabled");
    // Enable all configured Pins
    for (int i = CONTROLLER_PIN0; i < MAX_CONTROLLER_PINS; i++)
    {
        if (this->_controllerPinSettings[i].controllerPinPin != CONTROLLER_PIN_NOT_CONFIGURED) {
            if(!this->setControllerPinMode(i, this->_controllerPinSettings[i].controllerPinMode)) {
                LOG_ERROR("setEnabled", "Controller pin failure " << i << ". Controller cannot be enabled.");
                this->_controllerEnabled = false;
                return false;
            }
        }
    }
    this->updateOutput();
    return true;
}

bool dewController::setControllerPinPin(int controllerPin, int pin)
{
    if ((controllerPin < CONTROLLER_PIN0) || (controllerPin > CONTROLLER_PIN5))
    {
        LOG_ERROR("setControllerPinPin", "Invalid controller pin value: " << controllerPin);
        return false;
    }

    if (pin == -1) {
      this->setControllerPinMode( controllerPin, CONTROLLER_PIN_MODE_DISABLED);
      return true;
    }

    for (int i = 0; i < MAX_CONTROLLER_PINS; i++)
    {
        if ((this->_controllerPinSettings[i].controllerPinPin == pin) && (i != controllerPin))
        {
            LOG_ERROR("setControllerPinPin", "Pin " << pin << " already assigned to controller pin " << i);
            return false;
        }
    }

    if ((pin < 0) || (pin > 39))
    {
        LOG_ERROR("setControllerPinPin", "Invalid pin number: " << pin);
        return false;
    }

    if ((this->_controllerEnabled) && ((this->_controllerPinSettings[controllerPin].controllerPinMode != CONTROLLER_PIN_MODE_DISABLED) || 
        (this->_controllerPinSettings[controllerPin].controllerPinPin != CONTROLLER_PIN_NOT_CONFIGURED)))
    {
        if (!this->setControllerPinMode(controllerPin, CONTROLLER_PIN_MODE_DISABLED)) {
            LOG_ERROR("setControllerPinPin", "Controller pin " << controllerPin << " is enabled. Pin cannot be changed.");
            return false;
        }
    }

    this->_controllerPinSettings[controllerPin].controllerPinPin = pin;
    LOG_DEBUG("setControllerPinPin", "Controller pin " << controllerPin << " set to: " << pin);
    return true;
}

bool dewController::setControllerPinMode(int controllerPin, controllerPinModes mode)
{
    if ((controllerPin < CONTROLLER_PIN0) || (controllerPin > CONTROLLER_PIN5))
    {
        LOG_ERROR("setControllerPinMode", "Invalid controller pin: " << controllerPin);
        return false;
    }

    if ((mode < CONTROLLER_PIN_MODE_DISABLED) || (mode > CONTROLLER_PIN_MODE_BOOLEAN))
    {
        LOG_ERROR("setControllerPinMode", "Invalid controller pin mode: " << mode);
        return false;
    }
 
    if (!this->_controllerEnabled) {
        this->_controllerPinSettings[controllerPin].controllerPinMode = mode;
        LOG_DEBUG("setControllerPinMode", "Controller Disabled but Controller pin " << controllerPin << " set to mode: " << mode);
        return true;
    }

    if (this->_controllerPinSettings[controllerPin].controllerPinPin == CONTROLLER_PIN_NOT_CONFIGURED)
    {
        LOG_ERROR("setControllerPinMode", "Controller pin " << controllerPin << " is not configured.");
        return false;
    }

    if ((this->_controllerPinSettings[controllerPin].controllerPinMode != mode) && (mode != CONTROLLER_PIN_MODE_DISABLED) && (this->_controllerPinSettings[controllerPin].controllerPinMode != CONTROLLER_PIN_MODE_DISABLED))
    {
      int savedPinPin = this->_controllerPinSettings[controllerPin].controllerPinPin;
      if (!this->setControllerPinMode(controllerPin, CONTROLLER_PIN_MODE_DISABLED)) 
      {
        LOG_ERROR("setControllerPinMode", "Controller pin " << controllerPin << " not reset to CONTROLLER_PIN_MODE_DISABLED");
        return false;
      }
      this->_controllerPinSettings[controllerPin].controllerPinPin = savedPinPin;
      return true;
    }

    switch (mode)
    {
    case CONTROLLER_PIN_MODE_DISABLED:
        if ((this->_controllerPinSettings[controllerPin].controllerPinMode == CONTROLLER_PIN_MODE_CONTROLLER) || 
            (this->_controllerPinSettings[controllerPin].controllerPinMode == CONTROLLER_PIN_MODE_PWM))
        {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
            if (!ledcDetach(this->_controllerPinSettings[controllerPin].controllerPinPin)) {
              LOG_ERROR("setControllerPinMode", "Could not detach Pin: " << controllerPin << ":" << 
                        this->_controllerPinSettings[controllerPin].controllerPinPin << " from Controller/PWM.");
            }
#else
            ledcDetachPin(this->_controllerPinSettings[controllerPin].controllerPinPin);
#endif // ESP Core version  
        } 
        digitalWrite(_controllerPinSettings[controllerPin].controllerPinPin, 0);
        this->_controllerPinSettings[controllerPin].controllerPinOutput = 0;
        this->_controllerPinSettings[controllerPin].controllerPinPin = CONTROLLER_PIN_NOT_CONFIGURED;
        break;
    case CONTROLLER_PIN_MODE_CONTROLLER:
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    if (!ledcAttachChannel(this->_controllerPinSettings[controllerPin].controllerPinPin, CDC_PWM_FREQUENCY, CDC_PWM_RESOLUTION, CDC_CONTROLLER_PWM_CHANNEL)) {
      LOG_ERROR("setControllerPinMode", "Could not attach Pin: " << controllerPin << ":" << this->_controllerPinSettings[controllerPin].controllerPinPin << 
                " to Controller PWM Channel: " << CDC_CONTROLLER_PWM_CHANNEL);
    }
#else
    ledcAttachPin(this->_controllerPinSettings[controllerPin].controllerPinPin, CDC_CONTROLLER_PWM_CHANNEL);
#endif // ESP Core version
        this->_controllerPinSettings[controllerPin].controllerPinOutput = this->_currentOutput;
        break;
    case CONTROLLER_PIN_MODE_PWM:
        if ((controllerPin == CONTROLLER_PIN0) || (controllerPin == CONTROLLER_PIN1))
        {
            LOG_ERROR("setControllerPinMode", "Controller pin " << controllerPin << " cannot be set to PWM mode.");
            return false;
        }
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        if (!ledcAttachChannel(this->_controllerPinSettings[controllerPin].controllerPinPin, CDC_PWM_FREQUENCY, CDC_PWM_RESOLUTION, controllerPin - 1)) {
        LOG_ERROR("setControllerPinMode", "Could not attach Pin: " << controllerPin << ":" << this->_controllerPinSettings[controllerPin].controllerPinPin << 
                    " to Pin PWM Channel: " << (controllerPin - 1));
        }
#else
        ledcSetup(controllerPin - 1, CDC_PWM_FREQUENCY, CDC_PWM_RESOLUTION);
        ledcAttachPin(this->_controllerPinSettings[controllerPin].controllerPinPin, controllerPin - 1);
#endif // ESP Core version
        break;
    case CONTROLLER_PIN_MODE_BOOLEAN:
        if ((controllerPin == CONTROLLER_PIN0) || (controllerPin == CONTROLLER_PIN1))
        {
            LOG_ERROR("setControllerPinMode", "Controller pin " << controllerPin << " cannot be set to Boolean mode.");
            return false;
        }
        if (this->_controllerPinSettings[controllerPin].controllerPinMode != CONTROLLER_PIN_MODE_DISABLED) {
            int savePinPin = this->_controllerPinSettings[controllerPin].controllerPinPin;
            int saveOutput = this->_controllerPinSettings[controllerPin].controllerPinOutput;
            this->setControllerPinMode(controllerPin, CONTROLLER_PIN_MODE_DISABLED);
            this->_controllerPinSettings[controllerPin].controllerPinPin = savePinPin;
            this->_controllerPinSettings[controllerPin].controllerPinOutput = saveOutput;
        }
        pinMode(_controllerPinSettings[controllerPin].controllerPinPin, OUTPUT);    
        break;
    default:
        break;
    }

    this->_controllerPinSettings[controllerPin].controllerPinMode = mode;
    if ((mode != CONTROLLER_PIN_MODE_DISABLED) && (mode != CONTROLLER_PIN_MODE_CONTROLLER)) {
        if (!this->setControllerPinOutput(controllerPin, this->_controllerPinSettings[controllerPin].controllerPinOutput)) {
            LOG_ERROR("setControllerPinMode", "Failed to set output for controller pin " << controllerPin);
            return false;
        }
    }
    LOG_DEBUG("setControllerPinMode", "Controller pin " << controllerPin << " set to: " << mode);
    return true;
}

bool dewController::setControllerPinOutput(int controllerPin, int output)
{
    if ((controllerPin < CONTROLLER_PIN2) || (controllerPin > CONTROLLER_PIN5))
    {
        LOG_ERROR("setControllerPinOutput", "Invalid controller pin: " << controllerPin);
        return false;
    }

    if (!this->_controllerEnabled) {
        this->_controllerPinSettings[controllerPin].controllerPinOutput = output;
        LOG_DEBUG("setControllerPinOutput", "Controller Disabled but Controller pin " << controllerPin << " output set to: " << output);
        return true;
    }

    switch (_controllerPinSettings[controllerPin].controllerPinMode)
    {
    case CONTROLLER_PIN_MODE_DISABLED:
        LOG_ERROR("setControllerPinOutput", "Controller pin " << controllerPin << " is Disabled.");
        return false;
        break;
    case CONTROLLER_PIN_MODE_CONTROLLER:
        LOG_ERROR("setControllerPinOutput", "Controller pin " << controllerPin << " is in Controller mode.");
        return false;
        break;
    case CONTROLLER_PIN_MODE_PWM:
        if ((output < 0) || (output > 100))
        {
            LOG_ERROR("setControllerPinOutput", "Invalid PWM output value: " << output);
            return false;
        } else {
            int PWMDutyCycle = (output * (pow(2, CDC_PWM_RESOLUTION) - 1)) / 100;
            int PWMChannel = controllerPin - 1;
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
            ledcWriteChannel(PWMChannel, PWMDutyCycle);
#else
            ledcWrite(PWMChannel, PWMDutyCycle);
#endif  // ESP_ARDUINO_VERSION
        }
        break;
    case CONTROLLER_PIN_MODE_BOOLEAN:
        if (output < 0)
        {
            LOG_ERROR("setControllerPinOutput", "Invalid boolean output value: " << output);
            return false;
        }
        digitalWrite(_controllerPinSettings[controllerPin].controllerPinPin, (output > 0) ? HIGH : LOW);
        break;
    default:
        LOG_ERROR("setControllerPinOutput", "Controller pin " << controllerPin << " has an invalid mode.");
        return false;
        break;
    }

    this->_controllerPinSettings[controllerPin].controllerPinOutput = 
            (((output > 0)&&(this->_controllerPinSettings[controllerPin].controllerPinMode==CONTROLLER_PIN_MODE_BOOLEAN)) ? 100 : output);
    LOG_DEBUG("setControllerPinOutput", "Controller pin " << controllerPin << " Output set to: " << output);
    return true;
}

int dewController::getControllerPinOutput(int controllerPin)
{
    if ((controllerPin < CONTROLLER_PIN0) || (controllerPin > CONTROLLER_PIN5))
    {
        LOG_ERROR("getControllerPinOutput", "Invalid controller pin: " << controllerPin);
        return -1;
    }
    return _controllerPinSettings[controllerPin].controllerPinOutput;
}

int dewController::getControllerPinPin(int controllerPin)
{
    if ((controllerPin < CONTROLLER_PIN0) || (controllerPin > CONTROLLER_PIN5))
    {
        LOG_ERROR("getControllerPinPin", "Invalid controller pin: " << controllerPin);
        return -1;
    }
    return _controllerPinSettings[controllerPin].controllerPinPin;
}

controllerPinModes dewController::getControllerPinMode(int controllerPin)
{
    if ((controllerPin < CONTROLLER_PIN0) || (controllerPin > CONTROLLER_PIN5))
    {
        LOG_ERROR("getControllerPinMode", "Invalid controller pin value: " << controllerPin);
        return CONTROLLER_PIN_MODE_DISABLED;
    }
    return _controllerPinSettings[controllerPin].controllerPinMode;
}

int dewController::_calculateOutput(float currentTemperature, float setPoint, float range, float offset)
{
    float delta;

    this->_currentTrackPoint = setPoint + offset;
    delta = currentTemperature - this->_currentTrackPoint;

    if (delta <= 0)
    {
        return this->_maximumOutputSetting;
    }
    else if (delta <= range)
    {
        float powerMax = this->_maximumOutputSetting;
        float powerRange = this->_maximumOutputSetting - this->_minimumOutputSetting;
        return int(roundf(powerMax - ((delta * powerRange) / range)));
    }

    return this->_minimumOutputSetting;
}

void dewController::updateOutput(int output)
{
    int newOutput;
    int PWMDutyCycle;
    int previousOutput = this->_currentOutput;
    float referenceTemperature;
    float setPoint;

    if (this->_controllerEnabled)
    {
        switch (this->_currentControllerMode)
        {

        case AUTOMATIC:
            if (this->_currentTemperatureMode == EXTERNAL_INPUT)
            {
                referenceTemperature = theSetup->getAmbientTemperatureExternal();
            }
            else
            {
                referenceTemperature = theSetup->getAmbientTemperatureWQ();
            }

            switch (this->_currentSetPointMode)
            {
            case DEWPOINT:
                setPoint = theSetup->getDewPoint();
                break;
            case TEMPERATURE:
                setPoint = this->_currentTemperatureSetPoint;
                break;
            case MIDPOINT:
                if (referenceTemperature <= theSetup->getDewPoint())
                    setPoint = theSetup->getDewPoint();
                else
                    setPoint = (referenceTemperature + theSetup->getDewPoint()) / 2;
                break;
            default:
                setPoint = theSetup->getDewPoint();
                break;
            }

            newOutput = this->_calculateOutput(referenceTemperature, setPoint, this->_currentTrackingRange, this->_currentTrackPointOffset);
            break;

        case MANUAL:
            if (output < 0)
                newOutput = previousOutput;
            else
                newOutput = output;
            break;

        case OFF:
            if (output > 0)
                LOG_ALERT("updateOutput", "Controller mode must be set to AUTOMATIC or MANUAL before outputs can be set.");
            newOutput = 0;
            break;

        default:
            newOutput = previousOutput;
            break;
        }

        if (((newOutput < CDC_MINIMUM_CONTROLLER_OUTPUT) || (newOutput > CDC_MAXIMUM_CONTROLLER_OUTPUT)) && (this->_currentControllerMode != OFF))
        {
            LOG_ERROR("updateOutput", "Output value must be >=" << CDC_MINIMUM_CONTROLLER_OUTPUT << " and  <=" << CDC_MAXIMUM_CONTROLLER_OUTPUT << ". Out of range value: " << newOutput);
            newOutput = previousOutput;
        }

        if ((newOutput < this->_minimumOutputSetting) && (this->_currentControllerMode != OFF))
        {
            this->_currentOutput = this->_minimumOutputSetting;
        }
        else if (newOutput > this->_maximumOutputSetting)
        {
            this->_currentOutput = this->_maximumOutputSetting;
        }
        else
        {
            this->_currentOutput = newOutput;
        }

        if (this->_currentOutput != previousOutput)
        {

            PWMDutyCycle = (this->_currentOutput * (pow(2, CDC_PWM_RESOLUTION) - 1)) / 100;
            LOG_DEBUG("updateOutput", "Output set to: " << PWMDutyCycle << " of " << (pow(2, CDC_PWM_RESOLUTION) - 1));
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
            ledcWriteChannel(CDC_CONTROLLER_PWM_CHANNEL, PWMDutyCycle);
#else
            ledcWrite(CDC_CONTROLLER_PWM_CHANNEL, PWMDutyCycle);
#endif

#ifdef CDC_STATUS_LED_BLINK_ON_POWER_CHANGE
            theSetup->statusLEDOn();
#endif
            // Update Pin Settings Output value for pins linked to Controller PWM
            for (int i = CONTROLLER_PIN0; i < MAX_CONTROLLER_PINS; i++)
            {
                if (this->_controllerPinSettings[i].controllerPinMode == CONTROLLER_PIN_MODE_CONTROLLER)
                {
                    this->_controllerPinSettings[i].controllerPinOutput = this->_currentOutput;
                }
            }

            LOG_ALERT("updateOutput", "Power output changed to: " << this->_currentOutput);
        }

        LOG_DEBUG("updateOutput", "Output set to: " << this->_currentOutput << "%");
    }
}

void dewController::setControllerMode(controllerMode mode)
{

    if ((mode < AUTOMATIC) || (mode > OFF))
    {
        LOG_ALERT("setControllerMode", "Invalid controller mode: " << mode << " set to default " << (controllerMode)CDC_DEFAULT_CONTROLLER_MODE);
        this->_currentControllerMode = (controllerMode)CDC_DEFAULT_CONTROLLER_MODE;
    }
    else
    {
        this->_currentControllerMode = mode;
    }

    if (this->_currentControllerMode == OFF)
    {
        this->updateOutput(0);
    }

    LOG_DEBUG("setControllerMode", "Controller mode set to: " << this->_currentControllerMode);
}

void dewController::setTemperatureMode(temperatureMode mode)
{
    if ((mode < WEATHER_QUERY) || (mode > EXTERNAL_INPUT))
    {
        LOG_ALERT("setTemperatureMode", "Invalid temperature mode: " << mode << " set to default " << (temperatureMode)CDC_DEFAULT_TEMPERATURE_MODE);
        this->_currentTemperatureMode = (temperatureMode)CDC_DEFAULT_TEMPERATURE_MODE;
    }
    else
    {

        this->_currentTemperatureMode = mode;
    }
    LOG_DEBUG("setTemperatureMode", "Temperature mode set to: " << this->_currentTemperatureMode);
}
void dewController::setSetPoint(float setPointTemperature)
{
    this->_currentTemperatureSetPoint = setPointTemperature;
    LOG_DEBUG("setSetPoint", "Set point set to: " << this->_currentTemperatureSetPoint);
}

void dewController::setTrackPointOffset(float trackPointOffset)
{
    if ((trackPointOffset < CDC_MINIMUM_TRACK_POINT_OFFSET) || (trackPointOffset > CDC_MAXIMUM_TRACK_POINT_OFFSET))
    {
        LOG_ALERT("setTrackPointOffset", "Track point offset out of range: " << trackPointOffset);
        this->_currentTrackPointOffset = 0;
    }
    else
    {
        this->_currentTrackPointOffset = trackPointOffset;
    }

    LOG_DEBUG("setTrackPointOffset", "Track point offset set to: " << this->_currentTrackPointOffset);
}

void dewController::setSetPointMode(setPointMode mode)
{
    if ((mode < DEWPOINT) || (mode > MIDPOINT))
    {
        LOG_ALERT("setSetPointMode", "Invalid set point mode: " << mode << " set to default " << (setPointMode)CDC_DEFAULT_SET_POINT_MODE);
        this->_currentSetPointMode = (setPointMode)CDC_DEFAULT_SET_POINT_MODE;
    }
    else
    {
        this->_currentSetPointMode = mode;
    }
    LOG_DEBUG("setSetPointMode", "Set point mode set to: " << this->_currentSetPointMode);
}

void dewController::setTrackingRange(float trackingRange)
{
    if ((trackingRange >= CDC_MINIMUM_TRACKING_RANGE) & (trackingRange <= CDC_MAXIMUM_TRACKING_RANGE))
    {
        this->_currentTrackingRange = trackingRange;
    }
    else
    {
        LOG_ERROR("setTrackingRange", "Invalid Controller Range value: " << trackingRange);
    }
    LOG_DEBUG("setTrackingRange", "Set controller range to: " << this->_currentTrackingRange);
}

void dewController::setMinOutput(int output)
{
    if ((output >= CDC_MINIMUM_CONTROLLER_OUTPUT) & (output < this->_maximumOutputSetting))
    {
        this->_minimumOutputSetting = output;
    }
    else
    {
        LOG_ERROR("setMinOutput", "Invalid Minimum Output value: " << output);
    }
    LOG_DEBUG("setMinOutput", "Set controller minimum output to: " << this->_minimumOutputSetting);
}

void dewController::setMaxOutput(int output)
{
    if ((output > this->_minimumOutputSetting) & (output <= CDC_MAXIMUM_CONTROLLER_OUTPUT))
    {
        this->_maximumOutputSetting = output;
    }
    else
    {
        LOG_ERROR("setMaxOutput", "Invalid Maximum Output value: " << output);
    }
    LOG_DEBUG("setMaxOutput", "Set controller maximum output to: " << this->_maximumOutputSetting);
}