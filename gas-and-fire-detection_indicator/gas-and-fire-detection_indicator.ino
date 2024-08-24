#include <WiFi.h>                // INCLUDE THE LIBRARY FOR WIFI FUNCTIONALITY
#include <WiFiClientSecure.h>    // INCLUDE THE LIBRARY FOR SECURE WIFI CLIENT CONNECTIONS
#include <PubSubClient.h>        // INCLUDE THE LIBRARY FOR MQTT PROTOCOL IMPLEMENTATION
#include "time.h"                // INCLUDE THE LIBRARY FOR TIME FUNCTIONALITY

// WIFI CONFIGURATION
const char* ssid = "Zakira Lestari2";
const char* password = "muhammad2003";

// MQTT BROKER CONFIGURATION
const char* mqtt_server = "a3de186b.ala.asia-southeast1.emqxsl.com";
const int mqtt_port = 8883;
const char* mqtt_user = "mentoring";
const char* mqtt_password = "mentoring";

// MQTT TOPIC CONFIGURATION
const char* fireTopic = "gas-fire-detection/indicator/fire";
const char* gasTopic = "gas-fire-detection/indicator/gas";

// MQTT BROKER CERTIFICATE
static const char* root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";

// WIFI AND MQTT CLIENT CONFIGURATION
WiFiClientSecure espClient;
PubSubClient client(espClient);

// TIME SERVER CONFIGURATION
const char* ntpServer = "time.nist.gov";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 25200;

// SENSOR PIN CONFIGURATION
const int fireSensor = 25;
const int gasSensor = 35;

void setup() {
  Serial.begin(115200);

  // CONNECT TO WIFI NETWORK
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi CONNECTED.");

  // SET MQTT CLIENT CERTIFICATE
  espClient.setCACert(root_ca);

  // SET MQTT SERVER AND PORT
  client.setServer(mqtt_server, mqtt_port);

  // INITIALIZE TIME
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // SET SENSOR PIN MODE
  pinMode(fireSensor, INPUT);
  pinMode(gasSensor, INPUT);
}

void loop() {
  // MAIN SENSOR FUNCTION
  sensorMain();

  // WAIT 2 SECONDS BEFORE NEXT LOOP
  delay(2000);
}

void sensorMain() {
  // CHECK MQTT CONNECTION AND RECONNECT IF NOT CONNECTED
  if (!client.connected()) {
    reconnect();
  }

  // GET THE CURRENT TIME IN STRING FORMAT
  char timeStr[64];
  getFormattedTime(timeStr, sizeof(timeStr));

  // READ VALUES FROM SENSORS (IR FLAME SENSOR AND MQ-5 GAS SENSOR)
  bool fireDetect = digitalRead(fireSensor);
  int gasValue = analogRead(gasSensor);

  // PUBLISH SENSOR DATA TO MQTT AND STORE THE STATUS
  bool publishStatus = sensor_publishMQTT(fireDetect, gasValue);

  // PRINT MONITORING RESULTS AND MQTT PUBLISH STATUS
  printResults(fireDetect, gasValue, timeStr, publishStatus);
}

bool sensor_publishMQTT(bool fireDetect, int gasValue) {
  // CONVERT SENSOR DATA TO STRING
  String fireData = String(fireDetect);
  String gasData = String(gasValue);

  // PUBLISH SENSOR DATA TO MQTT TOPICS
  bool firePublished = client.publish(fireTopic, fireData.c_str());
  bool gasPublished = client.publish(gasTopic, gasData.c_str());

  // RETURN PUBLISH STATUS: TRUE IF BOTH DATA WERE PUBLISHED SUCCESSFULLY
  return firePublished && gasPublished;
}

void printResults(bool fireDetect, int gasValue, char* timeStr, bool publishStatus) {
  Serial.printf("==============================================\n");
  Serial.printf("                MONITORING REPORT             \n\n");
  Serial.printf("Date/Time           : %s\n\n", timeStr);
  Serial.printf("Sensor Readings:\n");
  Serial.printf("- Fire Detection    : %s\n", fireDetect ? "NOT DETECTED" : "DETECTED");
  Serial.printf("- Gas Value         : %d ppm\n\n", gasValue);
  Serial.printf("MQTT Publish Status : ");
  if (publishStatus) {
    Serial.printf("SUCCESSFULLY PUBLISHED!\n");
  } else {
    Serial.printf("FAILED TO PUBLISH ONE OR MORE MESSAGES!\n");
  }
  Serial.printf("==============================================\n\n");
}

void getFormattedTime(char* buffer, size_t bufferSize) {
  // GET LOCAL TIME AND FORMAT INTO STRING
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("FAILED TO OBTAIN TIME");
    snprintf(buffer, bufferSize, "N/A");
    return;
  }
  strftime(buffer, bufferSize, "%A, %B %d %Y %H:%M:%S", &timeinfo);
}

void reconnect() {
  // LOOP UNTIL SUCCESSFULLY RECONNECTED TO MQTT BROKER
  while (!client.connected()) {
    Serial.print("ATTEMPTING MQTT CONNECTION...");
    // TRY TO RECONNECT TO MQTT BROKER WITH CREDENTIALS
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("CONNECTED");
      // SUBSCRIBE TO TOPICS OR SEND MESSAGES IF NEEDED
    } else {
      Serial.print("FAILED, RC=");
      Serial.print(client.state());
      Serial.println(" TRY AGAIN IN 5 SECONDS");
      // WAIT 5 SECONDS BEFORE RETRYING
      delay(5000);
    }
  }
}
