#include "config.h"

JsonDocument config_doc;

void Config::setup() {
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("setup -> SPIFFS Mount Failed");
  }
  else{
    Serial.println("setup -> SPIFFS mounted successfully");
    if(this->readConfig("/config.json") != "") {
      // if not possible -> save new config file
      String config = this->readConfig("/config.json");
      deserializeJson(config_doc, config);
    }
    else {
      Serial.println("setup -> Could not read Config file -> initializing new file");
      JsonDocument doc;
      doc["default_config_temp"][0] = 22;
      doc["default_config_temp"][1] = 22;
      doc["default_config_temp"][2] = 22;
      doc["default_config_temp"][3] = 21;
      doc["default_config_temp"][4] = 21;
      doc["default_config_temp"][5] = 22;
      doc["default_config_temp"][6] = 23;
      doc["default_config_temp"][7] = 24;
      doc["default_config_temp"][8] = 25;
      doc["default_config_temp"][9] = 25;
      doc["default_config_temp"][10] = 26;
      doc["default_config_temp"][11] = 26;
      doc["default_config_temp"][12] = 27;
      doc["default_config_temp"][13] = 27;
      doc["default_config_temp"][14] = 28;
      doc["default_config_temp"][15] = 27;
      doc["default_config_temp"][16] = 27;
      doc["default_config_temp"][17] = 23;
      doc["default_config_temp"][18] = 22;
      doc["default_config_temp"][19] = 22;
      doc["default_config_temp"][20] = 22;
      doc["default_config_temp"][21] = 22;
      doc["default_config_temp"][22] = 22;
      doc["default_config_temp"][23] = 22;
      doc["default_config_light"]["on"] = 7;
      doc["default_config_light"]["off"] = 17;
      doc["default_config_humidity"][0]["min"] = 80;
      doc["default_config_humidity"][0]["max"] = 82;
      doc["default_config_humidity"][1]["min"] = 80;
      doc["default_config_humidity"][1]["max"] = 82;
      doc["default_config_humidity"][2]["min"] = 80;
      doc["default_config_humidity"][2]["max"] = 82;
      doc["default_config_humidity"][3]["min"] = 80;
      doc["default_config_humidity"][3]["max"] = 82;
      doc["default_config_humidity"][4]["min"] = 80;
      doc["default_config_humidity"][4]["max"] = 82;
      doc["default_config_humidity"][5]["min"] = 80;
      doc["default_config_humidity"][5]["max"] = 82;
      doc["default_config_humidity"][6]["min"] = 78;
      doc["default_config_humidity"][6]["max"] = 80;
      doc["default_config_humidity"][7]["min"] = 78;
      doc["default_config_humidity"][7]["max"] = 80;
      doc["default_config_humidity"][8]["min"] = 78;
      doc["default_config_humidity"][8]["max"] = 80;
      doc["default_config_humidity"][9]["min"] = 78;
      doc["default_config_humidity"][9]["max"] = 80;
      doc["default_config_humidity"][10]["min"] = 78;
      doc["default_config_humidity"][10]["max"] = 80;
      doc["default_config_humidity"][11]["min"] = 78;
      doc["default_config_humidity"][11]["max"] = 80;
      doc["default_config_humidity"][12]["min"] = 78;
      doc["default_config_humidity"][12]["max"] = 80;
      doc["default_config_humidity"][13]["min"] = 78;
      doc["default_config_humidity"][13]["max"] = 80;
      doc["default_config_humidity"][14]["min"] = 78;
      doc["default_config_humidity"][14]["max"] = 80;
      doc["default_config_humidity"][15]["min"] = 78;
      doc["default_config_humidity"][15]["max"] = 80;
      doc["default_config_humidity"][16]["min"] = 78;
      doc["default_config_humidity"][16]["max"] = 80;
      doc["default_config_humidity"][17]["min"] = 78;
      doc["default_config_humidity"][17]["max"] = 80;
      doc["default_config_humidity"][18]["min"] = 80;
      doc["default_config_humidity"][18]["max"] = 82;
      doc["default_config_humidity"][19]["min"] = 80;
      doc["default_config_humidity"][19]["max"] = 82;
      doc["default_config_humidity"][20]["min"] = 80;
      doc["default_config_humidity"][20]["max"] = 82;
      doc["default_config_humidity"][21]["min"] = 80;
      doc["default_config_humidity"][21]["max"] = 82;
      doc["default_config_humidity"][22]["min"] = 80;
      doc["default_config_humidity"][22]["max"] = 82;
      doc["default_config_humidity"][23]["min"] = 80;
      doc["default_config_humidity"][23]["max"] = 82;

      String default_config = "";
      serializeJson(doc, default_config);
      this->saveConfig("/config.json", default_config);
    }
  }
}

String Config::readConfig(String file_name) {
  return this->readFile(SPIFFS, file_name);
}

void Config::saveConfig(String filename, String value, bool is_append)
{
  writeFile(SPIFFS, filename, value, is_append);
  String config = this->readConfig("/config.json");
  deserializeJson(config_doc, value);
}

void Config::writeFile(fs::FS &fs, String filename, String message, bool is_append) {
  File file = fs.open(filename, is_append ? FILE_APPEND : FILE_WRITE);
  if(!file){
    Serial.println("writeFile -> failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("writeFile -> file written");
  } else {
    Serial.println("writeFile -> write failed");
  }
  file.close();
}

String Config::readFile(fs::FS &fs, String filename){
  File file = fs.open(filename);
  if(!file || file.isDirectory()){
    Serial.println("readFile -> failed to open file for reading");
    return "";
  }

  String fileText = "";
  while(file.available()){
    fileText = file.readString();
  }
  Serial.println(fileText);
  file.close();
  return fileText;
}

JsonDocument Config::getConfig() {
  return config_doc;
}

void Config::updateConfig(String key, JsonDocument value, String filename) {
  if(value.is<JsonArray>()) {
    config_doc[key] = value;
  }
  else if (value.is<JsonObject>()) {
    config_doc[key] = value;
  }
  else if (value.is<const char*>()) {
    const char* strValue = value.as<const char*>();
    
    // Kiểm tra xem chuỗi có thể chuyển thành số không
    if (String(strValue).toFloat() != 0 || strcmp(strValue, "0") == 0) {
      config_doc[key] = String(strValue).toFloat();
    } else {
      config_doc[key] = strValue;
    }
  }
  else {
    config_doc[key] = value;
  }
  
  //   JsonArray arr = value.as<JsonArray>();

  //   if(arr.size() > 0 && arr[0].is<JsonObject>()) {
  //     config_doc[key] = value;
  //   }
  //   else {
  //     JsonDocument doc;
  //     JsonArray data = doc.to<JsonArray>();

  //     JsonArray arr = value.as<JsonArray>();
  //     for (JsonVariant value : arr) {
  //       if (value.is<const char*>()) {
  //         const char* strValue = value.as<const char*>();
          
  //         if(String(strValue).toFloat() != 0 || strcmp(strValue, "0") == 0){
  //           data.add(String(strValue).toFloat());
  //         } else {
  //           data.add(strValue);
  //         }
  //       } else if (value.is<float>()) {
  //         data.add(value.as<float>());
  //       } else if (value.is<int>()) {
  //         data.add(value.as<int>());
  //       }
  //     }

  //     config_doc[key] = data;
  //   }
  // }
  // else {
  //   config_doc[key] = value;
  // }
  

  String json_value = "";
  // config_doc[key] = doc;
  serializeJson(config_doc, json_value);
  this->saveConfig(filename, json_value);
  config_doc = this->getConfig();
}

