#include "mqtt.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);
Relay mqttRelay;
JsonDocument mqttDoc;

unsigned long lastReconnectAttempt = 0;
bool isReconnectMqtt = false;
int reconnectAttemptTime = 0;

void MQTT::setup() {
  mqttClient.setKeepAlive(90);
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
  mqttClient.setBufferSize(1024);
  this->connect();

  String restartPayload = this->createMqttMessage("Device restarted. Reason: ", true, 1);
  this->sendMessage(EVENT_MESSAGE, (char *)restartPayload.c_str());
  // this->sendMessage(EVENT_RELAY, this->createMqttPumpStatus());
}

void MQTT::handleMqtt() {
if(!mqttClient.connected()){
    isReconnectMqtt = true;

    // mqttRelay.turnOffAllRelays();

    long now = millis();
    if(now - lastReconnectAttempt > 140000){
      Serial.println("Attempting MQTT connection...");
      lastReconnectAttempt = now;
      mqttClient.setSocketTimeout(3);

      // Attempt to reconnect
      if(this->reconnect()){
        lastReconnectAttempt = 0;
        reconnectAttemptTime = 0;
      }
      else{
        reconnectAttemptTime++;
        Serial.println("Failed to connect to MQTT broker, trying again in 5 seconds...");

        if (reconnectAttemptTime > 12) {
          reconnectAttemptTime = 0;
          Serial.println("Multiple MQTT connection failures - will continue with reduced frequency");
        }
      }
    }
  }
  else{
    if(isReconnectMqtt){
      reconnectAttemptTime = 0;
      // mqttSendWebhook->send("Connected mqtt");
    }
    mqttClient.loop();
  }
}

void MQTT::sendMessage(const char *channel, String message) {
  if(WiFi.status() == WL_CONNECTED) {
    mqttClient.publish(channel, (char*) message.c_str());
  }  
}

bool MQTT::reconnect() {
  return this->connect();
}

void MQTT::callback(char *topic, byte *payload, uint length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");

  DeserializationError error = deserializeJson(mqttDoc, payload);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  MQTT* mqttInstance = new MQTT();
  if(strcmp(topic, EVENT_MQTT_CONFIG) != 0){
    mqttInstance->handleMqttEvent(payload);
  }
  if(strcmp(topic, EVENT_MQTT_CONFIG) == 0) {
    mqttInstance->handleMqttConfig(payload);
  }
  if(strcmp(topic, EVENT_MQTT_GET_CONFIG) == 0) {
    mqttInstance->handleMqttGetConfig(payload);
  }
  if(strcmp(topic, EVENT_MQTT_GET_INFO) == 0) {
    mqttInstance->handleMqttGetInfo();
  }
}

void MQTT::handleMqttEvent(byte *payload) {
  bool hasTest = !mqttDoc["test"].isNull();
  bool hasReset = !mqttDoc["reset"].isNull();

  if(hasTest){
    // HumTemp humTemp = getHumTemp();
    // float temp = humTemp.temp;
    // float hum = humTemp.hum;

    // String ssid = WiFi.SSID();
    // String strength = String(dBmtoPercentage(WiFi.RSSI()));
    // String ip = WiFi.localIP().toString();

    // String content = "Test message: nhiệt độ: "+String(temp)+", độ ẩm: "+String(hum)+", wifi: "+ssid+", strength: "+strength+"%, ip: "+ip;
    // String testPayload = createMqttMessage(content);
    // this->sendMessage(EVENT_RELAY, (char*) testPayload.c_str());
  }

  if(hasReset){
    if(mqttDoc["reset"]){
      ESP.restart();
    }
  }

  bool hasAutoAirCondition = !mqttDoc["auto_ac"].isNull();
  if(hasAutoAirCondition) {
    mqttRelay.setAutoMode(RELAY_1_PIN, mqttDoc["auto_ac"]);
  }

  bool hasAutoLight = !mqttDoc["auto_light"].isNull();
  if(hasAutoLight) {
    mqttRelay.setAutoMode(RELAY_2_PIN, mqttDoc["auto_light"]);
  }

  bool hasAutoHumidity = !mqttDoc["auto_humidity"].isNull();
  if(hasAutoHumidity) {
    mqttRelay.setAutoMode(RELAY_3_PIN, mqttDoc["auto_humidity"]);
  }

  JsonObject json = mqttDoc.as<JsonObject>();

  mqttRelay.handleRelay(mqttDoc); 
}

void MQTT::handleMqttConfig(byte *payload) {
  Config _config;
  if(!mqttDoc["key"].isNull() && !mqttDoc["value"].isNull()) {
    String key = mqttDoc["key"];
    JsonDocument value = mqttDoc["value"];
    _config.updateConfig(key, value, "/config.json");
  }
}

void MQTT::handleMqttGetConfig(byte *payload) {
  if(!mqttDoc["key"].isNull()) {
    Config _config;
    JsonDocument config = _config.getConfig();

    String default_key = "default_" + mqttDoc["key"].as<String>();

    JsonDocument new_config;
    String config_value_string = "";

    if(!config[mqttDoc["key"]].isNull()) {
      new_config["key"] = mqttDoc["key"].as<String>();
      new_config["value"] = config[mqttDoc["key"]];
      new_config["is_default"] = 0;
    }
    else if(!config[default_key].isNull()) {
      new_config["key"] = mqttDoc["key"].as<String>();
      new_config["value"] = config[default_key];
      new_config["is_default"] = 1;
    }
    serializeJson(new_config, config_value_string);
    this->sendMessage(EVENT_MQTT_SEND_CONFIG, config_value_string);
  }
}

void MQTT::publishConfig() {
  Config _config;
  JsonDocument config = _config.getConfig();
  String config_value_string = "";

  serializeJson(config, config_value_string);
  this->sendMessage(EVENT_MQTT_SEND_CONFIG, config_value_string);
}

void MQTT::handleMqttGetInfo() {
  JsonDocument info;
  info["ssid"] = WiFi.SSID();
  info["ip"] = WiFi.localIP().toString();
  info["mac"] = WiFi.macAddress();

  String info_string = "";
  serializeJson(info, info_string);
  this->sendMessage(EVENT_MQTT_SEND_INFO, info_string);
}

bool MQTT::connect() {
  const char *status = "offline";

  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  String deviceId = MQTT_CLIENT_ID + String(chipId, HEX);
  deviceId.toLowerCase();
  // deviceId.c_str()
  
  if(mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, EVENT_MQTT_CONNECTION, 1, false, status)){
    this->sendMessage(EVENT_MESSAGE, "hello emqx");

    this->sendMessage(EVENT_MQTT_CONNECTION, "online");
    mqttClient.subscribe(EVENT_RELAY_CONTROL);
    mqttClient.subscribe(EVENT_MQTT_CONFIG);
    mqttClient.subscribe(EVENT_MQTT_GET_CONFIG);
    mqttClient.subscribe(EVENT_MQTT_GET_INFO);
  }
  return mqttClient.connected();
}

String MQTT::createMqttMessage(String message, bool pushNotification, int isRestarted) {
  String response = "{\
    \"message\": \"Mqtt message\", \
    \"content\": \""+message+"\",\
    \"is_restarted\": \""+isRestarted+"\",\
    \"push\": "+pushNotification+"\
  }";
  return (char*) response.c_str();
}

String createMqttPumpStatus(int type){
  String pumpStatusPayload = "";
  if(type){
    pumpStatusPayload = "{\""+String(type)+"\":"+String(mqttRelay.getRelayStatus(type))+"}";
  }
  else{
    JsonDocument doc;
    JsonObject object = doc.to<JsonObject>();

    for (auto itr = mqttRelay.getAllRelaysStatus().begin(); itr != mqttRelay.getAllRelaysStatus().end(); ++itr) {
      object[String(itr->first)] = String(mqttRelay.getRelayStatus(type));
    }

    serializeJson(doc, pumpStatusPayload);
  }
  return (char*) pumpStatusPayload.c_str();
}