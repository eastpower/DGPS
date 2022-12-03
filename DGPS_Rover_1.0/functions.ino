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
  if (anzSat < 3) {      
    ledAus();
    digitalWrite(LED_rt, HIGH);
  }
  
  if (gps.location.isValid() && anzSat >= 3 && timeSec != timeSecAlt) {

    if (rxFlag == 50) {
      ledAus();
      digitalWrite(LED_gn, HIGH);
      lat = latSat + diffLat;
      lng = lngSat + diffLng;
    } else{
      ledAus();
      digitalWrite(LED_bl, HIGH);
      lat = latSat;
      lng = lngSat;
    }

    timeSecAlt = timeSec;
  }
}