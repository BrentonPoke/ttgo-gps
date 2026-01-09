//
// Created by Brenton on 10/19/2022.
//
#ifndef TELEMETRY_H
#define TELEMETRY_H
//For random coordinate generation
#include <iostream>
#include <random>
#include <utility>
#include <cmath>

#include <DuckGPS.h>
#include <ArduinoJson.h>
#include <ctime>
#include <Adafruit_SSD1306.h>
//#include <XPowersLib.h>
#include <utils/DuckUtils.h>
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
#define OLED_RESET     -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3c

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

std::string deviceId("MAMAGPSB");
DuckGPS tgps;
//XPowersPMU axp;

// Getting GPS data
std::string getGPSData(std::pair<float,float> gpsPair, std::array<uint8_t,6>& seqid, int count) {

    //test of EMS ideas...

    std::string arr[3] = {"NB","M","W"};
    // not sure why I need to do this now

    JsonDocument nestdoc;
    JsonObject ems  = nestdoc["EMS"].to<JsonObject>();
    ems["G"] = arr[esp_random() % 3];
    ems["Device"] = deviceId;
    ems["seqID"] = duckutils::toString(seqid);
    ems["seqNum"] = count;
   // ems["Voltage"] = axp.getBattVoltage();
   // ems["level"] = axp.getBatteryPercent();
    ems["GPS"]["lon"] = gpsPair.first;
    ems["GPS"]["lat"] = gpsPair.second;
    ems["GPS"]["satellites"] = tgps.satellites();
    ems["GPS"]["time"] = tgps.epoch();
    ems["GPS"]["alt"] = tgps.altitude(DuckGPS::AltitudeUnit::meter);
    ems["GPS"]["speed"] = tgps.speed(DuckGPS::SpeedUnit::kmph);

    std::string jsonstat;
    serializeJson(ems,jsonstat);

    Serial.print("Payload: ");
    Serial.println(jsonstat.c_str());
    Serial.print("Payload Size: ");
    Serial.println(jsonstat.length());

    display.clearDisplay();
    display.setTextSize(1); // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(deviceId.c_str());
    display.print("SeqID: ");
    display.println(ems["seqID"].as<std::string>().c_str());
    display.print("Time: ");
    display.println(ems["GPS"]["time"].as<long>());
    display.print("Satellites: ");
    display.println(ems["GPS"]["satellites"].as<int>());
    display.println("Coordidnates:");
    display.print(ems["GPS"]["lat"].as<float>(), 6);
    display.print(", ");
    display.println(ems["GPS"]["lon"].as<float>(), 6);
    display.print("Voltage: ");
   // display.println(axp.getBattVoltage());
    display.print("Percentage: ");
   // display.println(axp.getBatteryPercent());
    display.display();

    /*
     char buff[229];
    unishox2_compress_simple(jsonstat.c_str(), int(jsonstat.length()), buff);
     */

    return jsonstat;
}

std::pair<float, float> getLocation(float x0, float y0, int radius) {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0,49);
    // Convert radius from meters to degrees
    float radiusInDegrees = radius / 111000.0f;

    float u = dist(rd);
    float v = dist(rd);
    float w = radiusInDegrees * sqrt(u);
    float t = 2 * M_PI * v;
    float x = w * cos(t);
    float y = w * sin(t);

    // Adjust the x-coordinate for the shrinking of the east-west distances
    float new_x = x / cos(M_PI * y0);
    float foundLongitude = new_x + x0;
    float foundLatitude = y + y0;
    //std::cout << "Longitude: " ;
    return std::make_pair(foundLongitude, foundLatitude);
}
#endif