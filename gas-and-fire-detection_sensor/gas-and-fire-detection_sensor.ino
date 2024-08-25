// LIBRARY CONFIGURATION
#include <WiFi.h>            
#include <WiFiClientSecure.h>
#include <PubSubClient.h>    
#include "time.h"            

// WIFI CONFIGURATION
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT BROKER CONFIGURATION
const char* mqtt_server = "YOUR_MQTT_BROKER_SERVER";
const int mqtt_port = YOUR_MQTT_BROKER_PORT;
const char* mqtt_user = "YOUR_MQTT_BROKER_USERNAME";
const char* mqtt_password = "YOUR_MQTT_BROKER_PASSWORD";

// MQTT TOPIC CONFIGURATION
const char* fireTopic = "YOUR_MQTT_TOPICS";
const char* gasTopic = "YOUR_MQTT_TOPICS";

// MQTT BROKER CERTIFICATE
static const char* root_ca PROGMEM = "YOUR_CA_CERTIFICATE";

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

  // Initializing WiFi and MQTT Connection
  setupWifi();
  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);

  // Initializing Time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Setting GPIO pins as input for sensors
  pinMode(fireSensor, INPUT);
  pinMode(gasSensor, INPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect(); // Reconnecting to MQTT broker if the connection is lost
  }

  // Run sensorMain() function
  sensorMain();

  // Delay 2s for each loop
  delay(2000);
}

// FUNCTION TO RUN THE SENSOR MONITORING
void sensorMain() {
  // Get the current time in string format
  char timeStr[64];
  getFormattedTime(timeStr, sizeof(timeStr));

  // Read the value from sensors
  bool fireDetect = digitalRead(fireSensor);
  int gasValue = analogRead(gasSensor);

  // Publish sensor data to topic and get publish status
  bool publishStatus = sensor_publishMQTT(fireDetect, gasValue);

  // Print monitoring results and publish status
  printResults(fireDetect, gasValue, timeStr, publishStatus);
}

// FUNCTION TO SEND SENSOR DATA TO MQTT TOPICS AND GET THE PUBLISH STATUS
bool sensor_publishMQTT(bool fireDetect, int gasValue) {
  // Convert sensor data to string
  String fireData = String(fireDetect);
  String gasData = String(gasValue);

  // Publish sensor data to MQTT topics
  bool firePublished = client.publish(fireTopic, fireData.c_str());
  bool gasPublished = client.publish(gasTopic, gasData.c_str());

  // Return publish status: TRUE if both data were published successfully
  return firePublished && gasPublished;
}

// FUNCTION TO PRINT RESULTS IN SERIAL MONITOR
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

// FUNCTION GET CURRENT LOCAL TIME AND FORMAT TO STRING
void getFormattedTime(char* buffer, size_t bufferSize) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("FAILED TO OBTAIN TIME");
    snprintf(buffer, bufferSize, "N/A");
    return;
  }
  strftime(buffer, bufferSize, "%A, %B %d %Y %H:%M:%S", &timeinfo);
}

// FUNCTION TO RECONNECT TO THE MQTT BROKER IF THE CONNECTION IS LOST
void reconnect() {
  while (!client.connected()) {
    String client_id = "YOUR_CLIENT_ID";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    Serial.print("Attempting MQTT connection...");
    if (client.connect(client_id.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// FUNCTION TO CONNECT TO THE WIFI NETWORK
void setupWifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); // Start connecting to the Wi-Fi network

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // Print the local IP address after successful connection
}