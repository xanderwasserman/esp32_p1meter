#include "settings.h"

long LAST_RECONNECT_ATTEMPT = 0;
long LAST_UPDATE_SENT = 0;
long LAST_FULL_UPDATE_SENT = 0;

// Leave these empty; WiFiManager will populate them
char WIFI_SSID[32] = "";
char WIFI_PASS[32] = "";
char MQTT_HOST[64] = "";
char MQTT_PORT[6] = "";
char MQTT_USER[32] = "";
char MQTT_PASS[32] = "";

char telegram[P1_MAXLINELENGTH];

struct TelegramDecodedObject telegramObjects[NUMBER_OF_READOUTS];
