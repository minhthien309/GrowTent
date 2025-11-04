#include "hum_temp_sht31.h"

HumTemp humtempsht31(0, 0); 
bool isSensorConnected = false;
Adafruit_SHT31 sht30 = Adafruit_SHT31();

unsigned long startTimeReadHumTemp;
unsigned long startTimeWaitingReconnect;

HumTempSHT31::HumTempSHT31(uint8_t address){
  _address = address;
}

void HumTempSHT31::setup(){
	isSensorConnected = sht30.begin(_address);

	if(!isSensorConnected){
		if(millis() - startTimeWaitingReconnect > 1000){
			startTimeWaitingReconnect = millis();
			Serial.println("Could not find Sht30");
			isSensorConnected = sht30.begin(_address);
		}
	}
}

void HumTempSHT31::getHumTemp() {
  if(millis() - startTimeReadHumTemp > 1000){
		startTimeReadHumTemp = millis();
		humtempsht31.temp = sht30.readTemperature();
		humtempsht31.hum = sht30.readHumidity();

		Serial.println("Hum: " + (String)humtempsht31.hum + " Temp: " + (String)humtempsht31.temp);
	}
}

void HumTempSHT31::setHeater(bool enable) {
	if(isSensorConnected) {
		sht30.heater(enable);
	}
}

bool HumTempSHT31::getHeaterStatus() {
	if(isSensorConnected) {
		return sht30.isHeaterEnabled();
	}
	return false;
}

HumTemp HumTempSHT31::getHumTempObject() {
  return humtempsht31;
}
