#include <WiFi.h>
#include "time.h"

// KONFIGURASI WIFI USER
const char* ssid     = "Zakira Lestari2";
const char* password = "muhammad2003";

// KONFIGURASI SERVER WAKTU
const char* ntpServer = "time.nist.gov";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 25200;

// PIN UNTUK SENSOR
const int flameSensor = 25;
const int gasSensor = 35;

void setup() {
  Serial.begin(115200);

  // MENGHUBUNGKAN PROYEK KE WIFI
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  // INISIALISASI WAKTU
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // ATUR MODE PIN SENSOR
  pinMode(flameSensor, INPUT);
  pinMode(gasSensor, INPUT);
}

void loop() {
  sensorMain();
  delay(2000);
}

void sensorMain() {
  // DAPATKAN WAKTU TERBARU
  char timeStr[64];
  getFormattedTime(timeStr, sizeof(timeStr));

  // SENSOR API (IR FLAME SENSOR)
  bool flameDetect = digitalRead(flameSensor);

  // SENSOR GAS (MQ-5)
  int gasValue = analogRead(gasSensor);

  // PRINT HASIL MONITORING
  Serial.printf("Date/Time: %s\n\
Sensor Reading:\n\
- Flame Detection : %s\n\
- Gas Value       : %d ppm\n\n",
                timeStr, flameDetect ? "NOT DETECTED" : "DETECTED", gasValue);
}

void getFormattedTime(char* buffer, size_t bufferSize) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    snprintf(buffer, bufferSize, "N/A");
    return;
  }
  strftime(buffer, bufferSize, "%A, %B %d %Y %H:%M:%S", &timeinfo);
}