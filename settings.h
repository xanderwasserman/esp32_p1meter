#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

#define DEBUG
// Update treshold in milliseconds, messages will only be sent on this interval
#define UPDATE_INTERVAL 1000 // 1 second
//#define UPDATE_INTERVAL 10000 // 10 seconds
//#define UPDATE_INTERVAL 60000  // 1 minute
//#define UPDATE_INTERVAL 300000 // 5 minutes

// Update treshold in milliseconds,
// this will also send values that are more than the tresholds time the same
#define UPDATE_FULL_INTERVAL 600000 // 10 minutes
// #define UPDATE_FULL_INTERVAL 1800000 // 30 minutes
// #define UPDATE_FULL_INTERVAL 3600000 // 1 Hour

#define HOSTNAME "p1meter"
#define OTA_PASSWORD "admin"

#define BAUD_RATE 115200
#define P1_BAUD_RATE 115200
#define RXD2 16
#define TXD2 17
#define P1_MAXLINELENGTH 1050

#define MQTT_MAX_RECONNECT_TRIES 100
#define MQTT_ROOT_TOPIC "homeassistant/sensors/power/p1meter"

#define NUMBER_OF_READOUTS 20

// Store last timestamps
extern long LAST_RECONNECT_ATTEMPT;
extern long LAST_UPDATE_SENT;
extern long LAST_FULL_UPDATE_SENT;

// Allocate space for WiFi & MQTT credentials (start them empty)
extern char WIFI_SSID[32];
extern char WIFI_PASS[32];
extern char MQTT_HOST[64];
extern char MQTT_PORT[6];
extern char MQTT_USER[32];
extern char MQTT_PASS[32];

extern char telegram[P1_MAXLINELENGTH];

struct TelegramDecodedObject
{
  String name;
  long value;
  char code[16];
  char startChar = '(';
  char endChar = ')';
  bool sendData = true;
};

extern struct TelegramDecodedObject telegramObjects[NUMBER_OF_READOUTS];

unsigned int currentCRC = 0;

#endif