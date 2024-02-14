# CheapoDC
Cheap and easy ESP32C3 based Dew Controller for Astrophotography

Basic Algoritm for Cheapo Dew Controller

Modes:
    Automatic
        Local Temperature - local telescope temperature updated via API or probe (not currently supported)
        No Local Temperature - no telescope temperature. PWM calculations based only on weather query information
    Manual
        PWM control is fully manual through the web interface or the API

General Automated Model
    Variables:
        Ambient Temperature - retrieved from weather query
        Humidity - retrieved from weather query
        Dew Point - calculated from Ambient Temperature & Humidity
        Automated Mode - as above
        Dew Control Mode - one of
            Dew Point based - calculates PWM based on difference between a set point equal to the Dew Point and either Ambient or Telescope temperature
            Temperature based - calculates PWM based on difference between a defined temperature set point (default 0 Celsius) and either Ambient or Telescope temperature.
        Tracking Range - degrees Celsius range for modifying PWM from 0% to 100% - 4 degrees is lowest. 10 degrees is highest. Default is 5.
        Track Point Offset - degrees offset to move the Tracking Range relative to the Set Point temperature.
        Minimum PWM - Minimum PWM setting, default is 0%
        Maximum PWM - Maximum PWM setting, default is 100%


    Algorithm:
        T-Temp = Tracking Temperature in degrees C, default is Ambient Temperature
        SP-Temp = Set Point Temperature in degrees C, default is Dew Point Temperature
        SP-Offset = +/- offset to set point in degrees C, default is 0
        Range = Tracking Range in degrees C, default is 5

        Simple algorithm:
            1 - At regular intervals the delta Temperature is calculated based on D-Temp = T-Temp - (SP-Temp + SP-Offset).
            2 - If the Delta Temperature D-Temp is less than the Range then the PWM value is calculated base on the range. PWM = 100/Range.
            3 - If the PWM result is less than Min-PWM then Min-PWM is used. 
            4 - If the PWM result is more than the Max-PWM then the Max-PWM is used.

        




