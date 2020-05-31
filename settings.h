#define HOSTNAME "p1meter"
#define OTA_PASSWORD "admin"

#define BAUD_RATE 115200
#define P1_SERIAL_RX RX
#define P1_MAXLINELENGTH 1050

#define MQTT_MAX_RECONNECT_TRIES 100
#define MQTT_ROOT_TOPIC "sensors/power/p1meter"

long LAST_RECONNECT_ATTEMPT = 0;

char WIFI_SSID[32] = "";
char WIFI_PASS[32] = "";

char MQTT_HOST[64] = "";
char MQTT_PORT[6]  = "";
char MQTT_USER[32] = "";
char MQTT_PASS[32] = "";

char telegram[P1_MAXLINELENGTH];

// If the CRC check is not working (see below), insert the last known value here 
// for DQ checks initialization. The last three integers, are the three decimal 
// places (e.g. 10.123 = 10123). For more info, see the README.md
long CONSUMPTION_HIGH_TARIF = 0;
long CONSUMPTION_HIGH_TARIF_PREV;
long CONSUMPTION_LOW_TARIF = 0;
long CONSUMPTION_LOW_TARIF_PREV;
long DELIVERED_HIGH_TARIF = 0;
long DELIVERED_HIGH_TARIF_PREV;
long DELIVERED_LOW_TARIF = 0;
long DELIVERED_LOW_TARIF_PREV;
long GAS_METER_M3 = 0;
long GAS_METER_M3_PREV;

long ACTUAL_TARIF;
long ACTUAL_CONSUMPTION;
long INSTANT_POWER_CURRENT;
long INSTANT_POWER_USAGE;

long SHORT_POWER_OUTAGES;
long LONG_POWER_OUTAGES;
long SHORT_POWER_DROPS;
long SHORT_POWER_PEAKS;

// The CRC check didn't work for me: all data was supposedly corrupt and thus nothing was send
// To solve the DQ issues without CRC, checks were implemented using _PREV variables
unsigned int currentCRC = 0;
bool useCRC = false;
