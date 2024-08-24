// LIBRARY CONFIGURATION
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
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

// LCD Sreen Width and Height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// LCD OLED Configuration
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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

  // Inisialisasi OLED
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Sesuaikan alamat I2C jika perlu
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Jika gagal, berhenti di sini
  }

  // Membersihkan tampilan OLED
  oled.clearDisplay();
  oled.display();

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
  oled.clearDisplay();  // Clear the display
  oled.setTextColor(WHITE);

  // Set small text for "Gas Value"
  oled.setTextSize(1);
  oled.setCursor(0, 0);  // Top-left corner
  oled.print("Gas Value");

  // Calculate position for the gas value
  int valueWidth = 12 * String(gasValue).length();      // Width of the gas value based on the number of digits
  int valuePosition = (SCREEN_WIDTH - valueWidth) / 2;  // Center the value horizontally

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