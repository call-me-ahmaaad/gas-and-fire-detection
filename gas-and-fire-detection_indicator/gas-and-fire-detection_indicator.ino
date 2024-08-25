// LIBRARY CONFIGURATION
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

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

// OLED SCREEN CONFIGURATION
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 32;
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// GAS LEDs
const int gasRed = 5;
const int gasGreen = 18;
const int gasBlue = 19;

// FIRE LED
const int fireLED = 2;

// FIRE BUZZER
const int fireBuzzer = 4;

// GLOBAL VARIABLE OF SENSOR DATA
int gasValue;

void setup() {
  Serial.begin(115200);

  // Initializing WiFi and MQTT Connection
  setupWifi();
  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Initializing the OLED display
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  oled.clearDisplay();
  oled.display();

  // Setting GPIO pins as output for LEDs
  pinMode(fireLED, OUTPUT);
  pinMode(gasRed, OUTPUT);
  pinMode(gasGreen, OUTPUT);
  pinMode(gasBlue, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect(); // Reconnecting to MQTT broker if the connection is lost
  }
  client.loop(); // Listening for MQTT messages

  printLCD(); // Updating the OLED display with the latest sensor data
}

// THIS FUNCTION IS CALLED WHEN AN MQTT MESSAGE IS RECEIVED
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");

  String message;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();

  // Handling messages for fire detection
  if (String(topic) == fireTopic) {
    if (message == "1") {
      digitalWrite(fireLED, LOW);
      Serial.println("FIRE NOT DETECTED");
      noTone(fireBuzzer);
    } else if (message == "0") {
      digitalWrite(fireLED, HIGH);
      tone(fireBuzzer, 2000);
      Serial.println("FIRE DETECTED!");
    }
  // Handling messages for gas detection
  } else if (String(topic) == gasTopic) {
    gasValue = message.toInt();
    if (gasValue < 400) {
      digitalWrite(gasRed, HIGH);
      digitalWrite(gasGreen, LOW);
      digitalWrite(gasBlue, HIGH);
    } else if (gasValue >= 400 && gasValue <= 1000) {
      digitalWrite(gasRed, LOW);
      digitalWrite(gasGreen, LOW);
      digitalWrite(gasBlue, HIGH);
    } else {
      digitalWrite(gasRed, LOW);
      digitalWrite(gasGreen, HIGH);
      digitalWrite(gasBlue, HIGH);
    }
  }

  Serial.println();
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

// FUNCTION TO RECONNECT TO THE MQTT BROKER IF THE CONNECTION IS LOST
void reconnect() {
  while (!client.connected()) {
    String client_id = "YOUR_CLIENT_ID";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    Serial.print("Attempting MQTT connection...");
    if (client.connect(client_id.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(fireTopic); // Subscribe to fire detection topic
      client.subscribe(gasTopic);  // Subscribe to gas detection topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// FUNCTION TO UPDATE THE OLED DISPLAY WITH THE GAS SENSOR VALUE
void printLCD() {
  oled.clearDisplay();
  oled.setTextColor(WHITE);

  // Set small text for "Gas Value"
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print("Gas Value");

  // Calculate position for the gas value
  int valueWidth = 12 * String(gasValue).length();
  int valuePosition = (SCREEN_WIDTH - valueWidth) / 2;

  // Set large text for gas value
  oled.setTextSize(2);
  oled.setCursor(valuePosition, 12);  // Position the value
  oled.printf("%d", gasValue);

  // Set small text for "ppm" as subscript
  oled.setTextSize(1);
  oled.setCursor(valuePosition + valueWidth + 2, 22);  // Position "ppm" just to the right of the gas value
  oled.print("ppm");

  oled.display();  // Display the changes
}