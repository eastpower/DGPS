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


#define SCHWELLWERT_TOUCH 80
#define TOUCH_PIN T3  // GPI15
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

byte timeSecAlt = 0;
uint32_t anzSat = 0;
byte maxMW = 50;
double latSat, lngSat;
double sumLat, sumLng, mwLat, mwLng, diffLat, diffLng = 0.0;

volatile byte anz = 0;

// Declaration SSD1306 as I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// *** Begin ESP-NOW ***
byte macAddress[] = { 0x24, 0x6F, 0x28, 0x7A, 0x1C, 0xC4 };

// Structure for sending data
typedef struct struct_message {
  double lat;
  double lng;
  double dlat;
  double dlng;
  byte flag;
} struct_message;
struct_message myData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const byte *mac_addr, esp_now_send_status_t status) {}
// *** End ESP-NOW ***

void ledAus();
void readGPS();
void mittelWert(double lat, double lng);
void IRAM_ATTR counterReset();
void sendData();

void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);
  WiFi.mode(WIFI_STA);      // Device as WiFi station
  esp_now_init();           // init ESP-NOW
  esp_now_register_send_cb(OnDataSent);     // status
  // Register peer
  memcpy(peerInfo.peer_addr, macAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);  // Add peer
  
  touchAttachInterrupt(TOUCH_PIN, counterReset, SCHWELLWERT_TOUCH);
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
  oled.print("Referenz-Sat: ");
  oled.println(gps.satellites.value());
  oled.print("B: ");
  oled.println(mwLat, 9);
  oled.print("L: ");
  oled.println(mwLng, 9);
  oled.print("Anz. MW: ");
  oled.println(anz);
  oled.display();

}
