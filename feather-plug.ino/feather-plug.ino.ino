#include <MQTTClient.h>
#include <system.h>

#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RCSwitch.h>
#include <WiFi101.h>

#include "secrets.h"

Adafruit_SSD1306 display = Adafruit_SSD1306();

RCSwitch rc = RCSwitch();

// Wi-Fi
const char wifi_ssid[] = WIFI_SSID;
const char wifi_pass[] = WIFI_PASS;

// MQTT
WiFiClient net;
MQTTClient client;
const char mqttBroker[] = MQTT_BROKER;
const char mqttTopic[] = MQTT_TOPIC;
const char mqttSpotifyTopic[] = MQTT_SPOTIFY_TOPIC;
const int mqttPort = MQTT_PORT;
const char mqttClientName[] = MQTT_CLIENT_NAME;
unsigned long lastMillis = 0;

void setup() {
  // Set correct WiFi pins before we do anything else
  WiFi.setPins(8, 7, 4, 2);
  Serial.begin(9600);
  // Ensure serial is connected for debug
  // while (!Serial){}
  Serial.println("Beginning...");

  // Start the MQTT broker
  client.begin(mqttBroker, mqttPort, net);
  client.onMessage(messageReceived);

  // Set the parameters for controlling the plugs
  rc.enableTransmit(10);
  rc.setPulseLength(399);

  // Start outputting to the display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Booting...\n");
  connect();
}

void connect() {
  display.display();
  if (WiFi.status() == WL_NO_SHIELD) {
    display.print("No Wi-Fi Shield\n");
    display.display();
    while(true);
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Connecting to ");
  display.print(wifi_ssid);
  display.print("...\n");
  display.display();
  Serial.println("Attempting to connect to: ");
  Serial.println(wifi_ssid);
  display.setCursor(0, 16);
  int status;
  while (!WiFi.SSID()) {
    display.print("X ");
    display.display();
    status = WiFi.begin(wifi_ssid, wifi_pass);
    Serial.print("Status: ");
    Serial.println(status);
    // Wait 5 seconds for it to connect.
    delay(5000);
  }
  printCurrentNet();
  printWiFiData();
  display.clearDisplay();
  display.display();
  display.setCursor(0, 0);
  display.print("MQTT: ");
  display.print(mqttBroker);
  display.print("\n");
  display.display();
  while (!client.connect(mqttClientName)) {
    delay(1000);
  }
  client.subscribe(mqttTopic);
  client.subscribe(mqttSpotifyTopic);
}

void loop() {
  client.loop();
  if (!client.connected()) {
    connect();
  }

//  if (millis() - lastMillis > 1000) {
//    lastMillis = millis();
//    //client.publish(mqttTopic, "alive");
//  }
}

void messageReceived(String &topic, String &payload) {
  Serial.println("Incoming: " + topic + " - " + payload);
  if (topic == mqttTopic) {
    if (payload == "on") {
      lightsOn();
    }
    else if (payload == "off") {
      lightsOff();
    }
    else {
      Serial.println("Did not understand message");
    }
  }
  else if (topic == mqttSpotifyTopic) {
    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);
    if (root.success()) {
      display.clearDisplay();
      display.setCursor(0, 0);
      if (root["playing"] == true) {
        const char* artist = root["artist"];
        const char* track = root["track"];
        display.print(artist);
        display.print(" - ");
        display.print(track);
      }
      else {
        display.print("No music");
      }
      display.display();
    }
    else {
      Serial.println("Parsing JSON Failed");
    }
  }
}

void lightsOff() {
  Serial.println("Off");
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Switching off\n");
  display.display();
  rc.send("010001010100010101010100");
}

void lightsOn() {
  Serial.println("On");
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Switching on\n");
  display.display();
  rc.send("010001010100010101011100");
}

void printWiFiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

