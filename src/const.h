#define RELAY_1_PIN 32 // Air condition device
#define RELAY_2_PIN 33
#define RELAY_3_PIN 25
#define RELAY_4_PIN 26

#define MQTT_BROKER "mqtt.minhthien309.com"
#define MQTT_USERNAME "minhthien309"
#define MQTT_PASSWORD "zd1900cm"
#define MQTT_PORT 309

#define IS_DEV ""

#define MQTT_CLIENT_ID "grow-tent-smart-controller" IS_DEV
#define EVENT_MQTT_PREFIX "grow-tent"
#define EVENT_MQTT_CONNECTION EVENT_MQTT_PREFIX "/connection" IS_DEV
#define EVENT_MESSAGE EVENT_MQTT_PREFIX "/message" IS_DEV
#define EVENT_RELAY EVENT_MQTT_PREFIX "/relay" IS_DEV
#define EVENT_RELAY_CONTROL EVENT_MQTT_PREFIX "/relay-control" IS_DEV
#define EVENT_MQTT_HUM_TEMP EVENT_MQTT_PREFIX "/hum-temp" IS_DEV
#define EVENT_MQTT_CONFIG EVENT_MQTT_PREFIX "/config" IS_DEV
#define EVENT_MQTT_GET_CONFIG EVENT_MQTT_PREFIX "/get-config" IS_DEV
#define EVENT_MQTT_SEND_CONFIG EVENT_MQTT_PREFIX "/send-config" IS_DEV

#define MIJIA_MAC_ADDRESS "a4:c1:38:3e:5c:6d"
// #define MIJIA_MAC_ADDRESS "a4:c1:38:57:18:b1"
#define SCAN_TIME 10 // seconds
#undef CONFIG_BTC_TASK_STACK_SIZE
#define CONFIG_BTC_TASK_STACK_SIZE 32768

#define I2CADDRESS 0x44
#define TIME_TO_MEANSURING_HUM_TEMP 30
#define USE_HUM_TEMP_MIJIA 0

#define OTA_UPDATE_PORT 3232