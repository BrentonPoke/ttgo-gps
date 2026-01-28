#include <string>
#include <algorithm>
#include <array>
#include <arduino-timer.h>
#include "Ducks/MamaDuck.h"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <random>
#include <telemetry.h>

#ifdef SERIAL_PORT_USBVIRTUAL
#define Serial SERIAL_PORT_USBVIRTUAL
#endif
const int btnPin = 37;
MamaDuck<> duck(deviceId);

std::mt19937 gen;

auto timer = timer_create_default();
const int INTERVAL_MS = 20000;

bool runSensor(void*) {
    // Encoding the GPS
    tgps.readData(10000);
    //make sure there is at least one point generated
    std::pair<float,float> gpsPair = getLocation(tgps.lng(),tgps.lat(),15);
    //a max of 3 packets per transmission session

    std::array<uint8_t,6> seqID;
    std::uniform_int_distribution<int> distribution(0,35);
    std::string digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < seqID.size() ; i++) {
        seqID[i] = digits[distribution(gen)];
    }
    //make sure at least one packet is sent
    for (int i = 0; i <= 3; i++) {
        std::string sensorVal = getGPSData(gpsPair, seqID, i);
        //Serial.println(sensorVal);
        //Send gps data
        duck.sendData(topics::gps, sensorVal);
        //counter++;
        delay(1000);
    }

    return true;
}

void setup() {
    // We are using a hardcoded device id here, but it should be retrieved or
    // given during the device provisioning then converted to a byte vector to
    // setup the duck NOTE: The Device ID must be exactly 8 bytes otherwise it
    // will get rejected
    Wire.begin(21, 22);
    // Use the default setup provided by the SDK


    Serial.println("MAMA-DUCK...READY!");

    pinMode(btnPin, INPUT);

    display.begin(SSD1306_SWITCHCAPVCC,SCREEN_ADDRESS);
    display.display();
    tgps.setup();
    gen.seed(analogRead(0));
    duck.setupWithDefaults();

    runSensor(nullptr);
}

void loop() {
    timer.tick();
    std::uniform_int_distribution<> distrib(1000, 300000);
        if (timer.empty())
            timer.in(distrib(gen), runSensor);
    // Use the default run(). The Mama duck is designed to also forward data it receives
    // from other ducks, across the network. It has a basic routing mechanism built-in
    // to prevent messages from hoping endlessly.
    duck.run();
}