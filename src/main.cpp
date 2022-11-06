#include <string>
#include <arduino-timer.h>
#include <MamaDuck.h>
#include <Arduino.h>
#include <unishox2.h>
#include <ctime>


#ifdef SERIAL_PORT_USBVIRTUAL
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

//GPS
#include<telemetry.h>
const int btnPin = 37;
MamaDuck duck;

auto timer = timer_create_default();
const int INTERVAL_MS = 20000;
char message[32];
int counter = 1;
#define MESSEGES 300

static void smartDelay(unsigned long ms)
{
    unsigned long start = millis();
    do
    {
        while (GPS.available())
            tgps.encode(GPS.read());
    } while (millis() - start < ms);
}


bool runSensor(void*) {
    // Encoding the GPS
    smartDelay(10000);
    //make sure there is at least one point generated
    std::pair<double,double> gpsPair = getLocation(tgps.location.lng(),tgps.location.lat(),15);
    //a max of 3 packets per transmission session
    unsigned int count = esp_random() % 4;
    byte seqID[6];
    std::uniform_int_distribution<int> distribution(0,35);
    std::default_random_engine eng{esp_random()};
    const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < sizeof(seqID) ; i++) {
        seqID[i] = digits[distribution(eng)];
    }
    for (int i = 0; i < count+1; i++) {
        String sensorVal = getGPSData(gpsPair, seqID, i);
        //Serial.println(sensorVal);
        //Send gps data
        duck.sendData(topics::location, sensorVal);
        counter++;
        sleep(1);
    }
    return true;
}

void setup() {
    // We are using a hardcoded device id here, but it should be retrieved or
    // given during the device provisioning then converted to a byte vector to
    // setup the duck NOTE: The Device ID must be exactly 8 bytes otherwise it
    // will get rejected

    std::vector<byte> devId;
    devId.insert(devId.end(), deviceId.begin(), deviceId.end());

    // Use the default setup provided by the SDK
    duck.setupWithDefaults(devId);
    Serial.println("MAMA-DUCK...READY!");

    pinMode(btnPin, INPUT);
    GPS.begin(9600, SERIAL_8N1, 34, 12);   //17-TX 18-RX

    // initialize the timer. The timer thread runs separately from the main loop
    // and will trigger sending a counter message.
    //timer.every(INTERVAL_MS, runSensor);
    runSensor(new void*);
}

void loop() {
    timer.tick();
    if(counter < MESSEGES) {
        if (timer.empty())
            timer.at(millis() + esp_random() % 300000, runSensor);
    }
    else{
        Serial.println("Done sending messages");
        while(true) {
            delay(1000);
        }
    }
    // Use the default run(). The Mama duck is designed to also forward data it receives
    // from other ducks, across the network. It has a basic routing mechanism built-in
    // to prevent messages from hoping endlessly.
    duck.run();
}