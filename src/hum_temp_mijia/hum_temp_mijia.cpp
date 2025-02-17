#include "hum_temp_mijia.h"

HumTemp humtemp(0, 0); 

BLEClient *pClient;
BLEScan *pBLEScan;
bool connected = false;

// The remote service we wish to connect to.
static BLEUUID serviceUUID("ebe0ccb0-7a0a-4b0c-8a1a-6ff2997da3a6");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("ebe0ccc1-7a0a-4b0c-8a1a-6ff2997da3a6");

void HumTempMijia::onConnect(BLEClient *pclient) {
  connected = true;
  Serial.printf(" * Connected %s\n", pclient->getPeerAddress().toString().c_str());
}

void HumTempMijia::onDisconnect(BLEClient *pclient) {
  connected = false;
  Serial.printf(" * Disconnected %s\n", pclient->getPeerAddress().toString().c_str());
}

void HumTempMijia::notifyCallback(
  BLERemoteCharacteristic *pBLERemoteCharacteristic,
  uint8_t *pData,
  size_t length,
  bool isNotify)
{
  float voltage;
  Serial.print(" + Notify callback for characteristic ");
  Serial.println(pBLERemoteCharacteristic->getUUID().toString().c_str());
  humtemp.temp = (pData[0] | (pData[1] << 8)) * 0.01; //little endian
  humtemp.hum = pData[2];

  voltage = (pData[3] | (pData[4] << 8)) * 0.001; //little endian
  Serial.printf("temp = %.1f C ; humidity = %.1f %% ; voltage = %.3f V\n", humtemp.temp, humtemp.hum, voltage);
  pClient->disconnect();
}

void HumTempMijia::registerNotification()
{
  if (!connected)
  {
    Serial.println(" - Premature disconnection");
    return;
  }
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print(" - Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
  }
  Serial.println(" + Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  BLERemoteCharacteristic *pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr)
  {
    Serial.print(" - Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
  }
  Serial.println(" + Found our characteristic");
  // pRemoteCharacteristic->registerForNotify(notifyCallback);

  float voltage;
  Serial.print(" + Notify callback for characteristic ");
  Serial.println(pRemoteCharacteristic->getUUID().toString().c_str());
  std::string pData = pRemoteCharacteristic->readValue();

  humtemp.temp = ((uint8_t)pData[0] | ((uint8_t)pData[1] << 8)) * 0.01; //little endian
  
  humtemp.hum = (uint8_t)pData[2];

  voltage = ((uint8_t)pData[3] | ((uint8_t)pData[4] << 8)) * 0.001; //little endian
  Serial.printf("temp = %.1f C ; humidity = %.1f %% ; voltage = %.3f V\n", humtemp.temp, humtemp.hum, voltage);
  pClient->disconnect();
  
}

void HumTempMijia::connectSensor(BLEAddress htSensorAddress)
{
  pClient->connect(htSensorAddress);
}

void HumTempMijia::getHumTemp() {  
  std::string curAddr = MIJIA_MAC_ADDRESS;
  this->connectSensor(BLEAddress(curAddr));
  this->registerNotification();
}

void HumTempMijia::setup() {
  BLEDevice::init("ESP32");
  
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new HumTempMijia());

  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setActiveScan(true);   //active scan uses more power, but get results faster
  pBLEScan->setInterval(0x50);
  pBLEScan->setWindow(0x30);
}

HumTemp HumTempMijia::getHumTempObject() {
  return humtemp;
}