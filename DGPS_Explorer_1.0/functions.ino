void IRAM_ATTR counterReset() {
  anz = 0;
  sumLat = 0.0;
  sumLng = 0.0;
  diffLat = 0.0;
  diffLng = 0.0;
  mwLat = 0.0;
  mwLng = 0.0;
}

void ledAus() {
  digitalWrite(LED_bl, LOW);
  digitalWrite(LED_gn, LOW);
  digitalWrite(LED_rt, LOW);
}

void readGPS() {
  byte timeSec = gps.time.second();
  anzSat = gps.satellites.value();
  if (gps.location.isValid()) {
    latSat = gps.location.lat();
    lngSat = gps.location.lng();
  }

  if (anzSat == 0) ledAus();

  if (anzSat > 0 && anzSat < 3) {
    ledAus();
    digitalWrite(LED_rt, HIGH);
  }

  if (gps.location.isValid() && anzSat >= 3 && timeSec != timeSecAlt) {
    ledAus();
    digitalWrite(LED_bl, HIGH);

    timeSecAlt = timeSec;

    mittelWert(latSat, lngSat);
    diffLat = latSat - mwLat;
    diffLng = lngSat - mwLng;
    Serial.println(diffLat, 9);
    sendData();
    Serial.println(myData.dlat,9);
    Serial.println();
  }
}

void sendData() {
  // set data:
  myData.lat = mwLat;
  myData.lng = mwLng;
  myData.dlat = diffLat;
  myData.dlng = diffLng;
  myData.flag = anz;

  // send data:
  esp_now_send(macAddress, (uint8_t *)&myData, sizeof(myData));
}

void mittelWert(double lat, double lng) {
  if (anz < maxMW) {
    anz++;
    sumLat += lat;
    mwLat = sumLat / anz;
    sumLng += lng;
    mwLng = sumLng / anz;
  } else {
    ledAus();
    digitalWrite(LED_gn, HIGH);
  }
}