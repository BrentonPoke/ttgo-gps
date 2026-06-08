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
//#include <XPowersLib.h>
#include <utils/DuckUtils.h>
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
#define OLED_RESET     -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3c

#define SDA_PIN 21 //4 heltec v2
#define SLC_PIN 22 //15 heltec v2
AdafruitDisplay driver(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_RESET);

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

    logdbg_ln("Payload: %s",jsonstat);
    logdbg_ln("Payload Size: %i",jsonstat.length());

    driver.display.clearDisplay();
    driver.display.setTextSize(1); // Draw 2X-scale text
    driver.display.setTextColor(SSD1306_WHITE);
    driver.display.setCursor(0, 0);
    driver.display.println(deviceId.c_str());
    driver.display.print("SeqID: ");
    driver.display.println(ems["seqID"].as<std::string>().c_str());
    driver.display.print("Time: ");
    driver.display.println(ems["GPS"]["time"].as<long>());
    driver.display.print("Satellites: ");
    driver.display.println(ems["GPS"]["satellites"].as<int>());
    driver.display.println("Coordidnates:");
    driver.display.print(ems["GPS"]["lat"].as<float>(), 6);
    driver.display.print(", ");
    driver.display.println(ems["GPS"]["lon"].as<float>(), 6);
    driver.display.print("Voltage: ");
   // display.println(axp.getBattVoltage());
    driver.display.print("Percentage: ");
   // display.println(axp.getBatteryPercent());
    driver.display.display();

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