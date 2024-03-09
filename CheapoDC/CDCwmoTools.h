// ******************************************************************
// CheapoDC Open-Meteo WMO code to OpenWeather ICON and description
// mapper routine. Only included when Open-Meteo is used.
// Cheap and easy Dew Controller
// Details at https://github.com/hcomet/CheapoDC
// (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// ******************************************************************
// Information for this mapping routine was derived from the JSON
// description file from stellasphere provided here:
// https://gist.github.com/stellasphere/9490c195ed2b53c707087c8c2db4ec0c
// 
#ifndef MY_CDCWMO
#define MY_CDCWMO
#include <Arduino.h>
#include "CDCdefines.h"
#include <EasyLogger.h>
#include "CDCvars.h"
#include "CDCSetup.h"

String mapWMOCode(int code, bool dayTime, bool descOrIcon)
{
    char * description;
    char * iconURL;

    switch (code)
    {
    case 0:
    {
        if (dayTime)
        {
            description = "Sunny";
        }
        else
        {

            description = "Clear";
        }

        iconURL = "01";
        break;
    }

    case 1:
    {
        if (dayTime)
        {
            description = "Mainly Sunny";
        }
        else
        {
            description = "Mainly Clear";
        }
        iconURL = "01";
        break;
    }
    case 2:
    {

        description = "Partly Cloudy";
        iconURL = "02";
        break;
    }
    case 3:
    {

        description = "Cloudy";
        iconURL = "03";
        break;
    }
    case 45:
    {

        description = "Foggy";
        iconURL = "50";
        break;
    }
    case 48:
    {

        description = "Rime Fog";
        iconURL = "50";
        break;
    }
    case 51:
    {

        description = "Light Drizzle";
        iconURL = "09";
        break;
    }
    case 53:
    {

        description = "Drizzle";
        iconURL = "09";
        break;
    }
    case 55:
    {

        description = "Heavy Drizzle";
        iconURL = "09";
        break;
    }
    case 56:
    {

        description = "Light Freezing Drizzle";
        iconURL = "09";
        break;
    }
    case 57:
    {

        description = "Freezing Drizzle";
        iconURL = "09";
        break;
    }
    case 61:
    {

        description = "Light Rain";
        iconURL = "10";
        break;
    }
    case 63:
    {

        description = "Rain";
        iconURL = "10";
        break;
    }
    case 65:
    {

        description = "Heavy Rain";
        iconURL = "10";
        break;
    }
    case 66:
    {

        description = "Light Freezing Rain";
        iconURL = "10";
        break;
    }
    case 67:
    {

        description = "Freezing Rain";
        iconURL = "10";
        break;
    }
    case 71:
    {

        description = "Light Snow";
        iconURL = "13";
        break;
    }
    case 73:
    {

        description = "Snow";
        iconURL = "13";
        break;
    }
    case 75:
    {

        description = "Heavy Snow";
        iconURL = "13";
        break;
    }
    case 77:
    {

        description = "Snow Grains";
        iconURL = "13";
        break;
    }
    case 80:
    {

        description = "Light Showers";
        iconURL = "09";
        break;
    }
    case 81:
    {

        description = "Showers";
        iconURL = "09";
        break;
    }
    case 82:
    {

        description = "Heavy Showers";
        iconURL = "09";
        break;
    }
    case 85:
    {

        description = "Light Snow Showers";
        iconURL = "13";
        break;
    }
    case 86:
    {

        description = "Snow Showers";
        iconURL = "13";
        break;
    }
    case 95:
    {

        description = "Thunderstorm";
        iconURL = "11";
        break;
    }
    case 96:
    {

        description = "Light Thunderstorms With Hail";
        iconURL = "11";
        break;
    }
    case 99:
    {

        description = "Thunderstorm With Hail";
        iconURL = "11";
        break;
    }
default:
{
    description = "NA";
    iconURL = "";
    if (descOrIcon)
        return String(description);
    else 
        return String(iconURL);
}
}

    if (descOrIcon)
        return String(description);
    else
    {
        if (dayTime)
        return String(iconURL)+ String("d");
        else 
        return String(iconURL)+ String("n");
    }
}
#endif