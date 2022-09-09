#include <Arduino.h>
#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4
uint16_t adc;
uint16_t vbat;

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

char pin[5] = {1, 2, 3, 4};

// Send sms interval (1 hour here)
long interval = 3600000;

void setup() {
    while (! Serial);

    Serial.begin(115200);
    Serial.println(F("Initializing FONA... (May take a few seconds)"));

    fonaSerial->begin(4800);
    if (! fona.begin(*fonaSerial)) {
        Serial.println(F("Couldn't find FONA"));
        while (1);
    }
    Serial.println(F("FONA is OK"));
    Serial.println(F("Enabling GPS..."));
    fona.enableGPS(true);

    Serial.println(F("disabling GPRS..."));
    fona.enableGPRS(false);

    Serial.print(F("Unlocking SIM card: "));
    Serial.println(fona.unlockSIM(pin) ? F("OK") : F("FAIL"));

    Serial.print(F("VBat = ")); Serial.print(vbat); Serial.println(F(" mV"));
    Serial.print(F("VPct = ")); Serial.print(vbat); Serial.println(F("%"));

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
}

void loop() {
    float latitude, longitude, speed_kph, heading, speed_mph, altitude;
    boolean gps_success = fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude);

    if (gps_success) {

        String lat = String(latitude, 6);
        String lon = String(longitude, 6);

        Serial.println("Latitude: " + lat);
        Serial.println("Longitude: " + lon);

        String message = "https://www.google.com/maps/search/?api=1&query=" + lat + "%2C" + lon;
        String sendTo = "0033650065421";

        Serial.println("Sending SMS to " + sendTo + " with content '" + message + "'");

        if (!fona.sendSMS(const_cast<char*>(sendTo.c_str()), const_cast<char*>(message.c_str()))) {
            Serial.println(F("Failed"));
        } else {
            Serial.println(F("Sent!"));
        }

        delay(interval);
    }
}