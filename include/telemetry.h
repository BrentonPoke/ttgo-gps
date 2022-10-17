//
// Created by Brenton on 10/19/2022.
//
#ifndef TELEMETRY_H
#define TELEMETRY_H
#include <coords.h>
#include <TinyGPS++.h>
#include <ArduinoJson.h>
std::string deviceId("MAMAGPS2");
TinyGPSPlus tgps;
HardwareSerial GPS(1);
std::time_t tmConvert_t(int YYYY, int MM, byte DD, byte hh, byte mm, byte ss)
{
    std::tm tmSet{};
    tmSet.tm_year = YYYY - 1900;
    tmSet.tm_mon = MM - 1;
    tmSet.tm_mday = DD;
    tmSet.tm_hour = hh;
    tmSet.tm_min = mm;
    tmSet.tm_sec = ss;
    std::time_t t = std::mktime(&tmSet);
    return mktime(std::gmtime(&t));
}

// Getting GPS data
String getGPSData(std::pair<double,double> gpsPair ) {

    // Printing the GPS data
    Serial.println("--- GPS ---");
    Serial.print("Latitude  : ");
    Serial.println(tgps.location.lat(), 5);
    Serial.print("Longitude : ");
    Serial.println(tgps.location.lng(), 4);
    Serial.print("Altitude  : ");
    Serial.print(tgps.altitude.meters());
    Serial.println("M");
    Serial.print("Satellites: ");
    Serial.println(tgps.satellites.value());
    Serial.print("Raw Date  : ");
    Serial.println(tgps.date.value());
    Serial.print("Epoch     : ");
    Serial.println(tmConvert_t(
            tgps.date.year(),
            tgps.date.month(),
            tgps.date.day(),
            tgps.time.hour(),
            tgps.time.minute(),
            tgps.time.second()));
    Serial.print("Speed     : ");
    Serial.println(tgps.speed.kmph());
    Serial.println("**********************");

    //test of EMS ideas...

    std::string arr[3] = {"NB","M","W"};

    DynamicJsonDocument nestdoc(229);
    JsonObject ems  = nestdoc.createNestedObject("EMS");
    ems["G"] = arr[esp_random() % 3];
    ems["Device"] = deviceId;
    ems["GPS"]["lon"] = gpsPair.first;
    ems["GPS"]["lat"] = gpsPair.second;
    ems["GPS"]["satellites"] = tgps.satellites.value();
    ems["GPS"]["time"] = tmConvert_t(
            tgps.date.year(),
            tgps.date.month(),
            tgps.date.day(),
            tgps.time.hour(),
            tgps.time.minute(),
            tgps.time.second());
    ems["GPS"]["alt"] = tgps.altitude.meters();
    ems["GPS"]["speed"] = tgps.speed.kmph();

    String jsonstat;
    serializeJson(ems,jsonstat);

    Serial.println("Payload: " + jsonstat);
    Serial.print("Payload Size: ");
    Serial.println(jsonstat.length());

    /*
     char buff[229];
    unishox2_compress_simple(jsonstat.c_str(), int(jsonstat.length()), buff);
     */
    // Creating a message of the Latitude and Longitude
    // Check to see if GPS data is being received
    if (millis() > 5000 && tgps.charsProcessed() < 10)
    {
        Serial.println(F("No GPS data received: check wiring"));
    }

    return jsonstat;
}

#endif TELEMETRY_H
