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
    LOG_DEBUG("dewController", "Setup and configure dew controller PWM outputs");
#ifdef CDC_ENABLE_PWM_OUTPUT
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcAttachChannel(CDC_PWM_OUTPUT_PIN1, CDC_PWM_FREQUENCY, CDC_PWM_RESOLUTION, CDC_PWM_CHANNEL);
  #else
    ledcSetup(CDC_PWM_CHANNEL, CDC_PWM_FREQUENCY, CDC_PWM_RESOLUTION);
    ledcAttachPin(CDC_PWM_OUTPUT_PIN1, CDC_PWM_CHANNEL);
  #endif
    LOG_DEBUG("dewController", "Output 1: " << CDC_PWM_OUTPUT_PIN1);

  #ifdef CDC_PWM_OUTPUT_PIN2
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcAttachChannel(CDC_PWM_OUTPUT_PIN1, CDC_PWM_FREQUENCY, CDC_PWM_RESOLUTION, CDC_PWM_CHANNEL);
    #else
    ledcAttachPin(CDC_PWM_OUTPUT_PIN2, CDC_PWM_CHANNEL);
    #endif
    LOG_DEBUG("dewController", "Output 2: " << CDC_PWM_OUTPUT_PIN2);
  #endif
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWriteChannel(CDC_PWM_CHANNEL, CDC_PWM_DUTY_MINIMUM);
  #else
    ledcWrite(CDC_PWM_CHANNEL, CDC_PWM_DUTY_MINIMUM);
  #endif
#endif

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
#ifdef CDC_ENABLE_PWM_OUTPUT
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
            ledcWriteChannel(CDC_PWM_CHANNEL, CDC_PWM_DUTY_MINIMUM);
  #else
            ledcWrite(CDC_PWM_CHANNEL, CDC_PWM_DUTY_MINIMUM);
  #endif
#endif
#ifdef CDC_STATUS_LED_BLINK_ON_POWER_CHANGE
            theSetup->statusLEDOn();
#endif
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