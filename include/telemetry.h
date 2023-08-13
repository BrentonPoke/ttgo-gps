//
// Created by Brenton on 10/19/2022.
//
#ifndef TELEMETRY_H
#define TELEMETRY_H
#include <coords.h>
#include <ArduinoJson.h>
#include <ctime>
#include <DuckGPS.h>

std::string deviceId("MAMAGPS9");
DuckGPS tgps;
std::time_t tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss);

// Getting GPS data
String getGPSData(byte* seqid, int count, unsigned long timepoint) {
    // Printing the GPS data
    Serial.println("--- GPS ---");
    Serial.print("Latitude  : ");
    Serial.println(tgps.lat(), 5);
    Serial.print("Longitude : ");
    Serial.println(tgps.lng(), 4);
    Serial.print("Altitude  : ");
    Serial.print(tgps.altitude(DuckGPS::AltitudeUnit::meter));
    Serial.println("M");
    Serial.print("Satellites: ");
    Serial.println(tgps.satellites());
    Serial.print("Epoch     : ");
    Serial.println(tgps.epoch());
    Serial.print("Speed     : ");
    Serial.println(tgps.speed(DuckGPS::SpeedUnit::kmph));
    Serial.println("**********************");

    //test of EMS ideas...

    DynamicJsonDocument nestdoc(229);
    JsonObject ems  = nestdoc.createNestedObject("EMS");
    ems["Device"] = deviceId;
    ems["seqID"] = seqid;
    ems["seqNum"] = count;
    ems["MCUdelay"] = millis() - timepoint;
    ems["GPS"]["lon"] = tgps.lat();
    ems["GPS"]["lat"] =  tgps.lng();
    ems["Voltage"] = PMU.getBattVoltage();
    ems["level"] = PMU.getBatteryPercent();
    ems["GPS"]["satellites"] = tgps.satellites();
    ems["GPS"]["time"] = tgps.epoch();
    ems["GPS"]["alt"] = tgps.altitude(DuckGPS::AltitudeUnit::kilo);
    ems["GPS"]["speed"] = tgps.speed(DuckGPS::SpeedUnit::kmph);

    String jsonstat;
    serializeJson(ems,jsonstat);

    Serial.println("Payload: " + jsonstat);
    Serial.print("Payload Size: ");
    Serial.println(jsonstat.length());
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(deviceId.c_str());
    display.print("SeqID: ");
    display.println(ems["seqID"].as<String>());
    display.print("Time: ");
    display.println(ems["GPS"]["time"].as<time_t>());
    display.println("Lat, Long: ");
    display.printf("%f, %f", ems["GPS"]["lat"].as<double>(), ems["GPS"]["lon"].as<double>());
    display.print("Voltage: ");
    display.print(PMU.getBattVoltage());
    display.println(" mV");
    display.print("Percentage: ");
    display.println(PMU.getBatteryPercent());
    display.display();

    /*
     char buff[229];
    unishox2_compress_simple(jsonstat.c_str(), int(jsonstat.length()), buff);
     */
    // Creating a message of the Latitude and Longitude
    // Check to see if GPS data is being received
//    if (millis() > 5000 && tgps.charsProcessed() < 10)
//    {
//        Serial.println(F("No GPS data received: check wiring"));
//    }

    return jsonstat;
}

#endif