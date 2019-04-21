#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Ticker.h>

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASS"

#define MQTT_HOST IPAddress(202, 138, 232, 189)
#define MQTT_PORT 1883

String mqtt_user = "YOUR_MQTT_USER";
String mqtt_pass = "YOUR_MQTT_PASS";
String clientId = "YOUR_CLIENT_ID";

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

boolean isConnected = false;
char msg[75];

// Callback
void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach();
  wifiReconnectTimer.once(1, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  isConnected = true;

  // Subscribe Topik
  uint16_t packetIdSub = mqttClient.subscribe("LED", 1);
  Serial.print("Subscribing at QoS 1, packetId: ");
  Serial.println(packetIdSub);

  // Kirim Pesan kalau NodeMCU sudah Connect
  snprintf(msg, 75, "{ \"status\": 1, \"clientId\": \"%s\" }", clientId.c_str());
  mqttClient.publish("LED", 1, false, msg);                  // Parameter (topic, QoS, retain, payload)
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  isConnected = false;
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(1, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.print("["); Serial.print(topic); Serial.print("] "); Serial.println(payload);

  StaticJsonDocument<150> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.println("Error parsing");
    Serial.println(error.c_str());
  } else {
    JsonObject root = doc.as<JsonObject>();
    int data = root["data"];
    Serial.print("Data: ");
    Serial.println(data);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200); 
  
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  mqttClient.setKeepAlive(2).setCleanSession(false).setCredentials(mqtt_user.c_str(), mqtt_pass.c_str()).setClientId(clientId.c_str());

  connectToWifi();
}

void loop() {
  
  
  // Delay disimpan dibawah
  delay(10);
}
