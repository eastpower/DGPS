/*
  * Copyright 2022 Jörg Bischof
  *
  * Licensed under the EUPL, version 1.2 or - when
    which have been approved by the European Commission -
    Subsequent versions of the EUPL ("License").
  * You may use this work solely in accordance with this license.
  * A copy of the license can be found here:
  *
  * https://joinup.ec.europa.eu/software/page/eupl
  *
  * Unless required by applicable legislation or
    Agreed in writing, the subject of the license
    distributed software "as is",
    WITHOUT ANY WARRANTY OR CONDITIONS - EXPRESS OR
    tacit - spread.
  * The language-specific permissions and restrictions under the
    license can be found in the license text.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "data/consola7pt7b.h"  // Consolas, 7 pt
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <WiFi.h>
#include <esp_now.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

static const int RXPin = 13;
static const int TXPin = 12;
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

const byte LED_bl = 26;
const byte LED_gn = 27;
const byte LED_rt = 32;
int rxLen = 0;
byte rxFlag = 0;
byte timeSecAlt = 0;
uint32_t anzSat = 0;
double latSat, lngSat, refLat, refLng, lat, lng;
double diffLat, diffLng;

// Structure for sending data
typedef struct struct_message {
  double lat;
  double lng;
  double dlat;
  double dlng;
  byte flag;
} struct_message;
struct_message myData;

//callback received data
void OnDataRev(const uint8_t *mac, const uint8_t *incommingData, int len) {
  memcpy(&myData, incommingData, sizeof(myData));
  rxLen = len;
  refLat = myData.lat;
  refLng = myData.lng;
  diffLat = myData.dlat;
  diffLng = myData.dlng;
  rxFlag = myData.flag;
}

// Declaration SSD1306 as I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void ledAus();
void readGPS();

void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);

  WiFi.mode(WIFI_STA);  // Device as WiFi station
  esp_now_init();       // init ESP-NOW
  esp_now_register_recv_cb(OnDataRev);

  pinMode(LED_bl, OUTPUT);
  pinMode(LED_gn, OUTPUT);
  pinMode(LED_rt, OUTPUT);
  ledAus();

  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // init I2C Adr. 0x3C
  oled.setTextColor(WHITE);
  oled.setFont(&consola7pt7b);
}

void loop() {
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      readGPS();

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected!"));
  }

  oled.clearDisplay();
  oled.setCursor(0, 10);
  oled.print("Rover-Sat: ");
  oled.println(gps.satellites.value());
  oled.print("B: ");
  oled.println(lat, 9);
  oled.print("L: ");
  oled.println(lng, 9);
  oled.display();
  Serial.print("rxLen: ");
  Serial.print(rxLen);
  Serial.print("  Flag: ");
  Serial.print(rxFlag);
  Serial.print("  refLat: ");
  Serial.print(refLat, 9);
  Serial.print("  refLng: ");
  Serial.print(refLng, 9);
  Serial.print("  diffLat: ");
  Serial.print(diffLat, 9);
  Serial.print("   diffLng: ");
  Serial.println(diffLng, 9);

}
