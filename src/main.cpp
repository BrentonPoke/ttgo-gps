#include <string>
#include <arduino-timer.h>
#include <MamaDuck.h>
#include <Arduino.h>
#include <unishox2.h>
#include <ctime>
#include <coords.h>

#ifdef SERIAL_PORT_USBVIRTUAL
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

//GPS
#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <debug.h>

const int btnPin = 37;

TinyGPSPlus tgps;
HardwareSerial GPS(1);
MamaDuck duck;

auto timer = timer_create_default();
const int INTERVAL_MS = 20000;
char message[32];
int counter = 1;

static void smartDelay(unsigned long ms)
{
    unsigned long start = millis();
    do
    {
        while (GPS.available())
            tgps.encode(GPS.read());
    } while (millis() - start < ms);
}

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
    Serial.print(tgps.altitude.feet() / 3.2808);
    Serial.println("M");
    Serial.print("Satellites: ");
    Serial.println(tgps.satellites.value());
//    Serial.print("Time      : ");
//    Serial.print(tgps.time.hour());
//    Serial.print(":");
//    Serial.print(tgps.time.minute());
//    Serial.print(":");
//    Serial.println(tgps.time.second());
//    Serial.print("Raw Time  : ");
//    Serial.println(tgps.time.value());
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
    Serial.print("Charge    : ");
   // Serial.println(BL.getBatteryChargeLevel());
    Serial.print("Speed     : ");
    Serial.println(tgps.speed.kmph());
    Serial.println("**********************");

    //test of EMS ideas...

    std::string arr[3] = {"NB","M","W"};
    bool disabled[2] = {true,false};
//    std::string gender = arr[rand()%(3)];

    DynamicJsonDocument nestdoc(229);
    JsonObject ems  = nestdoc.createNestedObject("EMS");
    ems["G"] = arr[esp_random() % 3];
    ems["lon"] = gpsPair.first;
    ems["lat"] = gpsPair.second;
    ems["Needs"]["M"] = (esp_random() % 3) + 1;
    ems["Needs"]["F"] = (esp_random() % 3) + 1;
    ems["Needs"]["W"] = (esp_random() % 3) + 1;
    ems["Needs"]["D"]["P"] = disabled[esp_random()%2];
    ems["Needs"]["D"]["M"] = disabled[esp_random()%2];

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

bool runSensor(void *) {
    // Encoding the GPS
    smartDelay(5000);
    //make sure there is at least one point generated
    std::pair<double,double> gpsPair = getLocation(tgps.location.lng(),tgps.location.lat(),15);
    auto limit = esp_random()%20+1;

    info("Running GPS loop %d times", limit);

    for(int i=0; i < limit; i++) {
        String sensorVal = getGPSData(gpsPair);
        Serial.print("[MAMA] sensor data: ");
        Serial.println(sensorVal);

        //Send gps data
        duck.sendData(topics::location, sensorVal);
    }
    return true;
}

void setup() {
    // We are using a hardcoded device id here, but it should be retrieved or
    // given during the device provisioning then converted to a byte vector to
    // setup the duck NOTE: The Device ID must be exactly 8 bytes otherwise it
    // will get rejected
    std::string deviceId("MAMAGPS3");
    std::vector<byte> devId;
    devId.insert(devId.end(), deviceId.begin(), deviceId.end());

    // Use the default setup provided by the SDK
    duck.setupWithDefaults(devId);
    Serial.println("MAMA-DUCK...READY!");


    pinMode(btnPin, INPUT);
    GPS.begin(9600, SERIAL_8N1, 34, 12);   //17-TX 18-RX

    // initialize the timer. The timer thread runs separately from the main loop
    // and will trigger sending a counter message.
    timer.every(INTERVAL_MS, runSensor);
}

void loop() {
    timer.tick();
    // Use the default run(). The Mama duck is designed to also forward data it receives
    // from other ducks, across the network. It has a basic routing mechanism built-in
    // to prevent messages from hoping endlessly.
    duck.run();
}