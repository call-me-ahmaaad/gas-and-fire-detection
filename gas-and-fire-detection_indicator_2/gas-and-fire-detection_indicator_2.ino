// LIBRARY CONFIGURATION
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

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

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// LCD 16x2 i2c Configuration
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

// GAS LEDs
const int gasRed = 5;
const int gasGreen = 18;
const int gasBlue = 19;

// FIRE LED
const int fireLED = 2;

// FIRE BUZZER
const int fireBuzzer = 4;

// GLOBAL VARIABLES OF SENSOR DATA
// String fireDetect;
int gasValue;

void setup() {
  Serial.begin(115200);

  setupWifi();

  espClient.setCACert(root_ca);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Inisialisasi LCD i2c
  lcd.init();

  // turn on LCD backlight                      
  lcd.backlight();

  pinMode(fireLED, OUTPUT);

  pinMode(gasRed, OUTPUT);
  pinMode(gasGreen, OUTPUT);
  pinMode(gasBlue, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  printLCD();
}

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
  } else if (String(topic) == gasTopic) {
    gasValue = message.toInt();
    if (gasValue < 400) {
      digitalWrite(gasRed, LOW);
      digitalWrite(gasGreen, LOW);
      digitalWrite(gasBlue, HIGH);
    } else if (gasValue >= 400 && gasValue <= 1000) {
      digitalWrite(gasRed, LOW);
      digitalWrite(gasGreen, HIGH);
      digitalWrite(gasBlue, LOW);
    } else {
      digitalWrite(gasRed, HIGH);
      digitalWrite(gasGreen, LOW);
      digitalWrite(gasBlue, LOW);
    }
  }

  Serial.println();
}

void setupWifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    String client_id = "sub_gas-and-fire-detection_indicator";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    Serial.print("Attempting MQTT connection...");
    if (client.connect(client_id.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(fireTopic);
      client.subscribe(gasTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void printLCD() {
  lcd.clear();  // Clear the display

  // Set "Gas Value" text on the first row
  lcd.setCursor(0, 0);  // Top-left corner
  lcd.print("Gas Value:");

  // Calculate position for the gas value on the second row
  int valuePosition = (16 - String(gasValue).length() - 3) / 2;  // Center the value and "ppm" horizontally

  // Set gas value and "ppm" on the second row
  lcd.setCursor(valuePosition, 1);  // Second row
  lcd.print(gasValue);
  lcd.print(" ppm");

  delay(500);
}

// /*********
//   Rui Santos
//   Complete project details at https://randomnerdtutorials.com  
// *********/

// #include <LiquidCrystal_I2C.h>

// // set the LCD number of columns and rows
// int lcdColumns = 16;
// int lcdRows = 2;

// // set LCD address, number of columns and rows
// // if you don't know your display address, run an I2C scanner sketch
// LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

// void setup(){
//   // initialize LCD
//   lcd.init();
//   // turn on LCD backlight                      
//   lcd.backlight();
// }

// void loop(){
//   // set cursor to first column, first row
//   lcd.setCursor(0, 0);
//   // print message
//   lcd.print("Hello, World!");
//   delay(1000);
//   // clears the display to print new message
//   lcd.clear();
//   // set cursor to first column, second row
//   lcd.setCursor(0,1);
//   lcd.print("Hello, World!");
//   delay(1000);
//   lcd.clear(); 
// }