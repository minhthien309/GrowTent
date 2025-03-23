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
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200, 300000);
Config config;
float hum, temp = 0;

unsigned long startOfSecond;

unsigned long watchdogTimer = millis();

const char* ssid = "Minh Trieu";
const char* password = "13142528";

// const char* ssid = "Hong Loan";
// const char* password = "Hl0913991314";

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
    if(waitTime > 10) {
      break;
    }
    Serial.print('.');
    waitTime++;
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  
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
}

void loop() {
  // WifiConfig::process();
  if (millis() - watchdogTimer >= 4000) {
    Serial.println("Resetting WDT...");
    esp_task_wdt_reset();
    watchdogTimer = millis();
  }

  if ((timeClient.getHours() == 5 && timeClient.getMinutes() == 0 && timeClient.getSeconds() == 10)) {
    ESP.restart();
  }
  else {
    timeClient.update();
    mqtt.handleMqtt();
  }
  otaUpdate.loop();
  
  if(millis() - startOfSecond > 1000){
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
  }
}