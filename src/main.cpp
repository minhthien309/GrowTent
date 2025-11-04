#include <Arduino.h>
#include "wifi_config/wifi_config.h"
#include "relay/relay.h"

#if USE_HUM_TEMP_MIJIA == 1
  #include "hum_temp_mijia/hum_temp_mijia.h"
#else 
  #include "hum_temp_sht31/hum_temp_sht31.h"
#endif

#include <NTPClient.h>
#include <WiFiUdp.h>
#include "const.h"
#include "mqtt/mqtt.h"
#include "ota_update/ota_update.h"
#include <WiFi.h>
#include "configs/config.h"
#include <esp_task_wdt.h>
#include <ESP32Ping.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>

#define WDT_TIMEOUT_UPDATE 60
#define WDT_TIMEOUT 5

Relay relay;

#if USE_HUM_TEMP_MIJIA == 1
  HumTempMijia humTempClass;
#else 
  HumTempSHT31 humTempClass;
#endif

MQTT mqtt;
OtaUpdate otaUpdate;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "vn.pool.ntp.org", 25200, 60000);
Config config;
float hum, temp = 0;
AsyncWebServer server(80);

unsigned long startOfSecond;

unsigned long watchdogTimer = millis();
unsigned long lastWifiReconnect = millis();

bool isPendingToRestart = false;

const char* ssid = "Minh Trieu";
const char* password = "13142528";

// const char* ssid = "Hong Loan";
// const char* password = "Hl0913991314";

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);

  switch(event) {
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("WiFi connected");
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi got IP");
      Serial.println(WiFi.localIP());
      break;
    case SYSTEM_EVENT_STA_LOST_IP:
      Serial.println("WiFi lost IP address");
      break;
  }
}

bool hasValidIP() {
  IPAddress ip = WiFi.localIP();
  return !(ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0);
}

void setup() {
  Serial.begin(115200);

  esp_task_wdt_init(WDT_TIMEOUT_UPDATE, true); //enable panic so ESP32 restarts

  if(Update.end(true)) {
    esp_task_wdt_init(WDT_TIMEOUT, true);
  }

  esp_task_wdt_add(NULL); //add current thread to WDT watch

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  int waitTime = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if(waitTime >= 4) {
      break;
    }
    Serial.print('.');
    waitTime++;
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  // WiFi.onEvent(WiFiEvent);
  
  otaUpdate.setup();

  // WifiConfig::setup();
  mqtt.setup();
  relay.setup();
  humTempClass.setup();
  
  humTempClass.getHumTemp();
  hum = humTempClass.getHumTempObject().hum;
  temp = humTempClass.getHumTempObject().temp;

  config.setup();
  mqtt.publishConfig();

  WebSerial.begin(&server);
  server.begin();

  int waitForTimeSync = 0;
  while(!timeClient.update()) {
    timeClient.forceUpdate();
    delay(200);
    waitForTimeSync++;
    if(waitForTimeSync >= 10) {
      break;
    }
  }
}

// bool isPendingToRestart = false;

void loop() {
  // WifiConfig::process();
  if (millis() - watchdogTimer >= 4000) {
    Serial.println("Resetting WDT...");
    esp_task_wdt_reset();
    watchdogTimer = millis();
  }

  if(WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    mqtt.handleMqtt();
    otaUpdate.loop();
  }
  
  if(millis() - startOfSecond >= 1000){
    bool isRelay1On = relay.getRelayStatus(RELAY_1_PIN) == 1;
    bool isRelay2On = relay.getRelayStatus(RELAY_2_PIN) == 1;
    bool isRelay3On = relay.getRelayStatus(RELAY_3_PIN) == 1;
    bool isRelay4On = relay.getRelayStatus(RELAY_4_PIN) == 1;

    // if ((timeClient.getHours() == 19 && timeClient.getMinutes() == 0 && timeClient.getSeconds() == 10)) {
    //   if(isRelay1On || isRelay2On || isRelay3On || isRelay4On) {
    //     isPendingToRestart = true;
    //   }
    //   else {
    //     ESP.restart();
    //   }    
    // }

    // if ((!isRelay1On && !isRelay2On && !isRelay3On && !isRelay4On) && isPendingToRestart) {
    //   isPendingToRestart = false;
    //   ESP.restart();
    // }
    
    startOfSecond = millis();

    if(timeClient.getSeconds() % TIME_TO_MEANSURING_HUM_TEMP == 0) {
      humTempClass.getHumTemp();
      hum = humTempClass.getHumTempObject().hum;
      temp = humTempClass.getHumTempObject().temp;

      mqtt.sendMessage(EVENT_MQTT_HUM_TEMP, "{\"hum\": " + String(hum) + ", \"temp\": " + String(temp) + "}");
    }

    relay.handleAirConditioner(timeClient.getHours(), temp);
    relay.handleLight(timeClient.getHours());
    relay.handleHumidityDevice(timeClient.getHours(), timeClient.getMinutes(), hum);

    // if(timeClient.getMinutes() % 30 == 0 && timeClient.getSeconds() <= 30) {
    //   humTempClass.setHeater(true);
    // }
    // else {
    //   humTempClass.setHeater(false);
    // }

    // bool isMatchedRestartTime = timeClient.getMinutes() % 10 == 0 && timeClient.getSeconds() == 0; 
    // if(WiFi.status() == WL_CONNECTED && (!hasValidIP() || !Ping.ping("8.8.8.8")) && relay.getRelayStatus(RELAY_1_PIN) == 0 && isMatchedRestartTime) {
    //   ESP.restart();
    // }
  }
  if(WiFi.status() != WL_CONNECTED && millis() - lastWifiReconnect >= 120000) {
    WiFi.disconnect();
    WiFi.reconnect();
    lastWifiReconnect = millis();
  }
}