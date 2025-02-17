#include "wifi_config.h"
WiFiManager wm;

void WifiConfig::setup() {
  bool res;
  //Webserver IP: 192.168.4.1
  res = wm.autoConnect("SmartGrowtent","@smartGrowTent"); // password protected ap
  wm.setWiFiAutoReconnect(true);
  wm.setConnectRetries(4);
  wm.setConnectTimeout(120);
  wm.setConfigPortalBlocking(false);

  if(!wm.getWiFiIsSaved() || WiFi.status() != WL_CONNECTED) {
    if(wm.getWiFiIsSaved()) {
      ESP.restart();
    }
    else {
      wm.setEnableConfigPortal(true);
    }
  }

  if(!res) {
    Serial.println("Failed to connect");
    ESP.restart();
  } 
  else {  
    Serial.println("Connected...");
  }

  //Wait for WiFi to connect to AP
  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi Connected.");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void WifiConfig::process() {
  wm.process();
}