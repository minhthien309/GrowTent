#include "relay.h"

MQTT relayMQTT;

std::map<int, int> _allRelays;
std::map<int, int> _automaticMode; // Automatic mode: 0 -> manual; 1 -> auto
Config temp_config;

void Relay::setup() {
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(RELAY_3_PIN, OUTPUT);
  pinMode(RELAY_4_PIN, OUTPUT);
  this->turnOffAllRelays();
  this->updateAllRelays();
  this->parseAndSendRelayStatusMQTT();
  this->turnOnAutoDevices();
}

std::map<int, int> Relay::getAllRelaysStatus() {
  return _allRelays;
}

bool Relay::getRelayStatus(int relay) {
  return digitalRead(relay);
}

void Relay::updateAllRelays() {
  _allRelays[RELAY_1_PIN] = digitalRead(RELAY_1_PIN);
  _allRelays[RELAY_2_PIN] = digitalRead(RELAY_2_PIN);
  _allRelays[RELAY_3_PIN] = digitalRead(RELAY_3_PIN);
  _allRelays[RELAY_4_PIN] = digitalRead(RELAY_4_PIN);
}

void Relay::turnOnAutoDevices() {
  _automaticMode[RELAY_1_PIN] = 1;
  _automaticMode[RELAY_2_PIN] = 1;
  _automaticMode[RELAY_3_PIN] = 1;
  _automaticMode[RELAY_4_PIN] = 1;

  JsonDocument doc;
  doc["auto_humidifier"] = 1;
  doc["auto_light"] = 1;

  String message = "";
  serializeJson(doc, message);
  relayMQTT.sendMessage(EVENT_RELAY, (char *)message.c_str());
}

int Relay::getRelayPin(int relay) {
  if(_allRelays.count(relay)){
    return _allRelays[relay];
  }
  return -1;
}

void Relay::turnOffAllRelays() {
  for (auto itr = _allRelays.begin(); itr != _allRelays.end(); ++itr) {
    digitalWrite(this->getRelayPin(itr->first), 0);
  }
}

void Relay::turnOnAllRelays() {
  for (auto itr = _allRelays.begin(); itr != _allRelays.end(); ++itr) {
    digitalWrite(this->getRelayPin(itr->first), 1);
  }
}

void Relay::turnOffRelay(int relay) {
  digitalWrite(relay, 0);
  this->parseAndSendRelayStatusMQTT();
}

void Relay::turnOnRelay(int relay) {
  digitalWrite(relay, 1);
  this->parseAndSendRelayStatusMQTT();
}

void Relay::parseAndSendRelayStatusMQTT() {
  String relay1Status = String(this->getRelayStatus(RELAY_1_PIN));
  String relay2Status = String(this->getRelayStatus(RELAY_2_PIN));
  String relay3Status = String(this->getRelayStatus(RELAY_3_PIN));
  String relay4Status = String(this->getRelayStatus(RELAY_4_PIN));
  String message = "{\"relay1\": "+relay1Status+", \"relay2\": "+relay2Status+", \"relay3\": "+relay3Status+", \"relay4\": "+relay4Status+"}";

  relayMQTT.sendMessage(EVENT_RELAY, (char *)message.c_str());
}

void Relay::updateRelayStatus(int relay, int status) {
  digitalWrite(relay, status);
  this->parseAndSendRelayStatusMQTT();
}

void Relay::handleRelay(JsonDocument json) {
  bool hasLightControl = !json["light"].isNull();
  if(!_automaticMode[RELAY_2_PIN] && hasLightControl) {
    updateRelayStatus(RELAY_2_PIN, json["light"]);
  }

  bool hasHumidityControl = !json["humidity"].isNull();
  if(!_automaticMode[RELAY_3_PIN] && hasHumidityControl) {
    updateRelayStatus(RELAY_3_PIN, json["humidity"]);
  }
}

void Relay::handleAirConditioner(int hour, float temp) {
  JsonDocument config_doc = temp_config.getConfig();

  JsonDocument config_temp = config_doc["default_config_temp"];
  if(!config_doc["config_temp"].isNull()) {
    config_temp = config_doc["config_temp"];
  }

  //                   0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23
  // float tempSet[24] = {22,22,22,21,21,22,23,24,25,25,26,26,27,27,28,27,27,23,22,22,22,22,22,22};
  // float tempSet[24] = {21,21,21,20,20,21,22,23,24,24,25,25,26,26,27,26,26,22,21,21,21,21,21,21};
  int tempSet = config_temp[hour];
  if(temp > (tempSet + 1)) {
    if(!this->getRelayStatus(RELAY_1_PIN)) {
      this->turnOnRelay(RELAY_1_PIN);
    }
  }
  else if(temp < tempSet){
    if(this->getRelayStatus(RELAY_1_PIN)) {
      this->turnOffRelay(RELAY_1_PIN);
    }
  }
}

void Relay::handleLight(int hour) {
  Config temp_config;
  JsonDocument config_doc = temp_config.getConfig();

  JsonDocument config_light = config_doc["default_config_light"];
  if(!config_doc["config_light"].isNull()) {
    config_light = config_doc["config_light"];
  }

  if(_automaticMode[RELAY_2_PIN]) {
    if(hour >= config_light["on"] && hour <= config_light["off"]) {
      if(!this->getRelayStatus(RELAY_2_PIN)) {
        this->turnOnRelay(RELAY_2_PIN);
      }
    }
    else {
      if(this->getRelayStatus(RELAY_2_PIN)) {
        this->turnOffRelay(RELAY_2_PIN);
      }
    }
  }
}

void Relay::handleHumidityDevice(int hour, int minute, float humidity) {
  if(_automaticMode[RELAY_3_PIN]) {
    Config temp_config;
    JsonDocument config_doc = temp_config.getConfig();

    JsonDocument config_humidity = config_doc["default_config_humidity"];
    if(!config_doc["config_humidity"].isNull()) {
      config_humidity = config_doc["config_humidity"];
    }

    if(humidity <= config_humidity[hour]["min"]) {
      if(!this->getRelayStatus(RELAY_3_PIN)) {
        this->turnOnRelay(RELAY_3_PIN);
      }
    }
    else if(humidity >= config_humidity[hour]["max"]) {
      if(this->getRelayStatus(RELAY_3_PIN)) {
        this->turnOffRelay(RELAY_3_PIN);
      }
    }
    // if(hour >= 6 && hour <= 17) {
    //   if(humidity <= 78) {
    //     if(!this->getRelayStatus(RELAY_3_PIN)) {
    //       this->turnOnRelay(RELAY_3_PIN);
    //     }
    //   }
    //   else if(humidity >= 83) {
    //     if(this->getRelayStatus(RELAY_3_PIN)) {
    //       this->turnOffRelay(RELAY_3_PIN);
    //     }
    //   }
    // }
    // else {
    //   if(humidity <= 80) {
    //     if(!this->getRelayStatus(RELAY_3_PIN)) {
    //       this->turnOnRelay(RELAY_3_PIN);
    //     }
    //   }
    //   else if(humidity >= 85) {
    //     if(this->getRelayStatus(RELAY_3_PIN)) {
    //       this->turnOffRelay(RELAY_3_PIN);
    //     }
    //   }  
    // }
  }
}

void Relay::setAutoMode(int relay, int mode) {
  _automaticMode[relay] = mode;
}