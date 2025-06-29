#include <string>
#include <arduino-timer.h>
#include <MamaDuck.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#ifdef SERIAL_PORT_USBVIRTUAL
#define Serial SERIAL_PORT_USBVIRTUAL
#endif
#define CDP_LOG_INFO
#define ARDUINO_TBeam
//GPS
#define LORA_FREQ 915.0 // Frequency Range. Set for US Region 915.0Mhz
#define LORA_TXPOWER 20 // Transmit Power
// LORA HELTEC PIN CONFIG
#define LORA_CS_PIN 18
#define LORA_DIO0_PIN 26
#define LORA_DIO1_PIN -1 // unused
#define LORA_RST_PIN 23
const int btnPin = 37;
MamaDuck duck;

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
#define OLED_RESET     -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3c
#define CDPCFG_WIFI_NONE


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include <telemetry.h>

auto timer = timer_create_default();
const int INTERVAL_MS = 20000;
//int counter = 1;

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
    unsigned int count = esp_random() % 3;
    std::array<uint8_t,6> seqID;
    std::uniform_int_distribution<int> distribution(0,35);
    std::default_random_engine eng{esp_random()};
    std::string digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < seqID.size() ; i++) {
        seqID[i] = digits[distribution(eng)];
    }
    //make sure at least one packet is sent
    for (int i = 0; i <= 3; i++) {
        std::string sensorVal = getGPSData(gpsPair, seqID, i);
        //Serial.println(sensorVal);
        //Send gps data
        duck.sendData(topics::location, sensorVal);
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
    WiFi.mode(WIFI_OFF);
    std::vector<byte> devId;
    devId.insert(devId.end(), deviceId.begin(), deviceId.end());

    // Use the default setup provided by the SDK
    duck.setDeviceId(devId);
    // initialize the serial component with the hardware supported baudrate
    duck.setupSerial(115200);
    duck.setupRadio(LORA_FREQ, LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN, LORA_DIO1_PIN, LORA_TXPOWER);
    Serial.println("MAMA-DUCK...READY!");

    pinMode(btnPin, INPUT);
    GPS.begin(9600, SERIAL_8N1, 34, 12);   //17-TX 18-RX

    display.begin(SSD1306_SWITCHCAPVCC,SCREEN_ADDRESS);
    display.display();

    // initialize the timer. The timer thread runs separately from the main loop
    // and will trigger sending a counter message.
    //timer.every(INTERVAL_MS, runSensor);
    runSensor(new void*);
}

void loop() {
    timer.tick();
    std::mt19937 gen(millis());
    std::uniform_int_distribution<> distrib(1000, 300000);
        if (timer.empty())
            timer.in(distrib(gen), runSensor);
    // Use the default run(). The Mama duck is designed to also forward data it receives
    // from other ducks, across the network. It has a basic routing mechanism built-in
    // to prevent messages from hoping endlessly.
    duck.run();
}
