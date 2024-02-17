![CheapoDC Logo](images/logo.png)

The Cheapo Dew Controller, or CheapoDC, is a low cost, easy to build DIY dew controller based on an ESP32-C3 mini. Parts required include the ESP32-C3 mini, one or two MOSFET modules, 12V to 5V buck converter, some proto board, a couple of RCA sockets, a 12V barrel socket and wire. Cost of the parts should be less than $20 for a unit that controls 2 dew strap heaters.

A primary goal was to keep the build simple with minimal parts. This is done by leveraging the ESP32 WiFi capability to query the OpenWeather API for ambient temperature and dew point. This is then used for calculating CheapoDC's power output. No additional components to support temperature or humidity probes. The responsiveness or aggressiveness of the controller can be adjusted through several settings. CheapoDC works best with Internet connectivity supporting the weather queries to OpenWeather but it can also be used in a limited way without Internet access. 


# How the Controller Works

## Controller Power Output Calculation
### Variables:
* **Set Point** = ***SP***
  - as set using the **Set Point** selection.
* **Reference Temperature** = ***RT***
  - as set using the **Temperature Mode** selection.
* **Track Point** = ***TP***
  - the temperature point where the **Tracking Range** starts. A **Reference Temperature** less than or equal 
to the **Track Point** will cause the controller to use **Maximum Output**. 
* **Track Point Offset** = ***TPO***
  - an offset applied to the **Set Point** when determining the **Track Point** relative to the **Set Point**. 
The **Track Point Offset** may be set from -5.0 to 5.0 degrees Celsius. The default is 0.0 degrees Celsius.
* **Tracking Range** = ***TR***
  - the temperature range starting at the **Track Point** where the controller output ramps up from **Minimum Output** at the high end of the range to **Maximum Output** at the low end of the range. The range may be set to values from 
4.0 to 10.0 degrees Celsius. The default is 4.0 degrees celsius.
* **Power Output** = ***PO***
  - the percentage of power the controller is outputting to the dew straps. It varies from the **Minimum Output** setting to the **Maximum Output** setting.
* **Minimum Output** = ***MinO***
  - the minimum percent power setting for the controller to output when not in the **Off** **Controller Mode**. Defaults to 0%.
* **Maximum Output** = ***MaxO***
  - the maximum percent power setting for the controller to output when at full output. Defaults to 100%.

### Calculations
$`TP = SP + TPO`$

$`IF`$ $`(RT <= TP )`$ $`THEN`$ $`PO= MaxO`$

$`IF`$ $`(RT >= (TP + TR))`$ $`THEN`$ $`PO = MinO`$

$`IF`$ $`(RT <(TP + TR))`$ $`THEN`$ $`PO = MinO + (MaxO - MinO) * ( 1 - (RT - TP)/(TR))`$

### Examples

1. Exampe 1 is primarily a reference image to illustrate the variables defined above.
   * Power Output curve is shown relative to the Track Point and the Tracking Range. While the Reference Temperature is greater than the high end of the Tracking Range the Power Output is set to Minimum Output. The Power Output ramps up linearly through the Tracking Range from the Minimum Output to the Maximum Output as the Reference Temperature drops.
   * This example shows a Reference Temperature, RT = 8°C, which is greater than the upper end of the Tracking Range, causing Power Output to be set to Minimum Output. In this case Minimum Output is set to 10%.
   * The upper end of the Tracking Range, at 7°C, is determined from the Set Point (SP = -2°C) plus the Track Point Offset (TPO = 4°C), creating a Tack Point, TP = 2°C, plus the Tracking Range (TR = 5°C).
 
![Example 1](images/example1.jpg)

2. Example 2 shows changing the values of the controller variables can affect the Power Output calculation.
   * The Set Point, SP = -2°C, and the Reference Temperature, RT = 8°C, the same as in Example 1. However changing the Track Point Offset, now TPO = 2°C, and the Tracking Range, now TR = 10°C, has change the Power Output as well as flattening  the Power Output curve.
   * 

![Example 1](images/example2.jpg)

## Dew Controller Settings
The following modes of operation are used in the algorithm that calculates controller output. The first item under each 
mode is the default.

### Controller Mode:
The Controller Mode selects the overall operating mode of the dew controller.
* #### Automatic  
  * Uses the **Set Point Mode** and **Temperature Mode** settings to calculate the controller output based on the Power Output calculations above. In Automatic mode the Power Output calculation will be run periodically based on the Update Output Every setting. This may be set to a value from 1 to 20 minutes. The default is 1 minute. Automatic mode may be used with or without internet access.
* #### Manual
  * Controller output is manually controlled by setting the **Dew Controller Output** either through the Web UI or the API. When in Manual mode the the periodic Power Output calculation is suspended. Using Manual control is an option if internet access for OpenWeather API queries is not available.
* #### Off
  * Controller output is set to Zero. 

### Set Point Mode:
The Set Point Mode selects what will be used as the SetPoint for calculating Power Output.
* #### Dew Point
  * Uses the **Dew Point** as the Set Point used for calculating output. This is the default selection.
* #### Temperature
  * Uses a **Temperature Set Point** value input via the Web UI or API as the Set Point for calculating output. If the CheapoDC is being used without internet access then this mode allows a Set Point to be defined when a the DeW Point cannot be determined through the OpenWeather API.
* #### Midpoint
  * Uses the midpoint between the current **Temperature Mode** temperature value and the Dew Point value as the Set Point for calculating output. The impact of selecting this mode is to effectively double the Tracking Range and further flatten the ramp of the Power Output curve.

### Temperature Mode:
The Temperature Mode selects how the Reference Temperature will be determined for calculating the Power Output.
* #### Weather Query
  * Uses the Ambient Temperature returned by the OpenWeather API weather query as the Reference Temperature for calculating the controller output. This requires that the CheapoDC have internet access.
* #### External Input
  * Use the **External Input** temperature set through the Web UI or API as the reference temperature for calculating the controller output. This may be the preferred mode when using the controller with KStars/Indi. The CheapoDC Indilib driver can use a temperature probe attached to a focuser as the external input. This is also the mode that should be used when the CheapoDC does not have access to the internat and the OpenWeather API.



        




