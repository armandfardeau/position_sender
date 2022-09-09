#include <Arduino.h>
#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4
uint16_t vbat;

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

char pin[5] = {1, 2, 3, 4};


String contact_numbers[] = {"0033650065421", "0033612686923"};

// Send sms interval (1 hour here)
long interval = 3600000;

void setup() {
    while (!Serial);

    Serial.begin(115200);
    Serial.println(F("Initializing FONA... (May take a few seconds)"));

    fonaSerial->begin(4800);
    if (!fona.begin(*fonaSerial)) {
        Serial.println(F("Couldn't find FONA"));
        while (true);
    }
    Serial.println(F("FONA is OK"));
    Serial.println(F("Enabling GPS..."));
    fona.enableGPS(true);

    Serial.println(F("disabling GPRS..."));
    fona.enableGPRS(false);

    Serial.print(F("Unlocking SIM card: "));
    Serial.println(fona.unlockSIM(pin) ? F("OK") : F("FAIL"));

    // Waiting for network fix
    uint8_t n = fona.getNetworkStatus();

    while (n != 1 && n != 5) {
        delay(1000);
        n = fona.getNetworkStatus();
        Serial.print(F("Network status: Not registered. Trying again..."));
    }
    Serial.println(F("Network status: Registered"));

    // Waiting for GPS fix
    uint8_t fix = fona.GPSstatus();
    while (fix != 2 && fix != 3) {
        delay(1000);
        fix = fona.GPSstatus();
        Serial.print(F("GPS status: No fix. Trying again..."));
    }
    Serial.println(F("GPS status: Fix"));

    // Displays contact info
    Serial.println(F("Contacts info:"));
    for (int i = 0; i < sizeof(contact_numbers) / sizeof(contact_numbers[0]); i++) {
        Serial.print("Contact number " + String(i) + ": ");
        Serial.println(contact_numbers[i]);
    }
}

void loop() {
    float latitude, longitude;
    boolean gps_success = fona.getGPS(&latitude, &longitude);

    if (gps_success) {
        String lat = String(latitude, 6);
        String lon = String(longitude, 6);

        // Display the GPS data
        Serial.println("Latitude: " + lat);
        Serial.println("Longitude: " + lon);

        // Display hardware info
        fona.getBattVoltage(&vbat);
        fona.getBattPercent(&vbat);

        // Display the battery info
        Serial.print(F("VBat = "));
        Serial.print(vbat);
        Serial.println(F(" mV"));

        Serial.print(F("VPct = "));
        Serial.print(vbat);
        Serial.println(F("%"));

        String message = "https://www.google.com/maps/search/?api=1&query=" + lat + "%2C" + lon;

        //iterating through the contact_numbers array
        for (int i = 0; i < sizeof(contact_numbers) / sizeof(contact_numbers[0]); i++) {
            Serial.println("Sending SMS to: " + contact_numbers[i] + " with content '" + message + "'");
            boolean sms_status = fona.sendSMS(
                    const_cast<char *>(contact_numbers[i].c_str()),
                    const_cast<char *>(message.c_str())
            );

            if (!sms_status) {
                Serial.println(F("Failed"));
            } else {
                Serial.println(F("Sent!"));
            }
        }

        delay(interval);
    }
}