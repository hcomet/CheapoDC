![CheapoDC Logo](images/logo.png)

# CheapoDC
Cheap and easy ESP32-C3 based Dew Controller for Astrophotography

# How the Controller Works

The following modes of operation are used in the algorithm that calculates controller output. The first item under each 
mode is the default.

## Controller Modes
* ### Automatic  
  * Uses the **Set Point Mode** and **Temperature Mode** settings to calculate the controller output based on the algorithm below.
* ### Manual
  * Controller output is manually controlled by setting the **Dew Controller Output** either through the Web UI or the API.
* ### Off
  * Controller output is set to Zero.

## Set Point Modes:
* ### Dew Point
  * Uses the **Dew Point** as the Set Point used for calculating output.
* ### Temperature
  * Uses a **Temperature Set Point** value input via the Web UI or API as the Set Point for calculating output.
* ### Midpoint
  * Use the midpoint between the current **Temperature Mode** temperature value and the Dew Point value as the Set Point for calculating output.

## Temperature Modes:
* ### Weather Query
  * Use the Ambient Temperature returned by the OpenWeather weather query as the reference temperature for calculating the controller output.
* ### External Input
  * Use the **External Input** temperature set through the Web UI or API as the reference temperature for calculating the controller output. This may be the preferred mode when using the controller with KStars/Indi. The CheapoDC Indilib driver can use a temperature probe attached to a focuser as the external input. 


## Controller Power Output Model
### Variables:
* **Set Point** = ***SP*** - as set using the **Set Point** selection.
* **Reference Temperature** = ***RT*** - as set using the **Temperature Mode** selection.
* **Track Point** = ***TP*** - the temperature point where the **Tracking Range** starts. A **Reference Temperature** less than or equal 
to the **Track Point** will cause the controller to use **Maximum Output**. 
* **Track Point Offset** = ***TPO*** - an offset applied to the **Set Point** when determining the **Track Point** relative to the **Set Point**. 
The **Track Point Offset** may be set from -5.0 to 5.0 degrees Celsius. The default is 0.0 degrees Celsius.
* **Tracking Range** = ***TR*** - the temperature range starting at the **Track Point** where the controller output ramps up from **Minimum Output** at the high end of the range to **Maximum Output** at the low end of the range. The range may be set to values from 
4.0 to 10.0 degrees Celsius. The default is 4.0 degrees celsius.
* **Power Output** = ***PO*** - the percentage of power the controller is outputting to the dew straps. It varies from the **Minimum Output** setting to the **Maximum Output** setting.
* **Minimum Output** = ***MinO*** - the minimum percent power setting for the controller to output when not in the **Off** **Controller Mode**. Defaults to 0%.
* **Maximum Output** = ***MaxO*** - the maximum percent power setting for the controller to output when at full output. Defaults to 100%.

### Calculations
$`TP = SP + TP`$

$`IF`$ $`(RT <= TP )`$ $`THEN`$ $`PO= MaxO`$

$`IF`$ $`(( RT - TP ) >= TR)`$ $`THEN`$ $`PO = MinO`$

$`IF`$ $`(( RT - TP) < TR)`$ $`THEN`$ $`PO = MinO + (MaxO - MinO) * ( 1 - (RT - TP)/(TR))`$

### Examples

1. Reference image to illustrate the definitions above.
![Example 1](images/example1.png)

2. Example 2
![Example 1](images/example2.png)

3. Example 3
![Example 1](images/example3.png)
        
        

        




