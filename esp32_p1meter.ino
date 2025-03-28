#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>

#include "settings.h"
#include "read_p1.h"

void sendDataToBroker();
void blinkLed(int times, int delayMs);
bool mqttReconnect();  // your existing method to connect to MQTT
void setupDataReadout();
void setupOTA();

WiFiClient espClient;
PubSubClient mqttClient(espClient);

#define LED_BUILTIN 2

/***********************************
            Main Setup
 ***********************************/
void setup()
{
    // Initialize pins
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.begin(BAUD_RATE);
    Serial2.begin(P1_BAUD_RATE, SERIAL_7E1, RXD2, TXD2, true);
    Serial2.setRxInvert(true);

#ifdef DEBUG
    Serial.println("Booting - DEBUG mode on");
    blinkLed(2, 500);
    delay(500);
    blinkLed(2, 2000);
    // Blinking 2 times fast and two times slower to indicate DEBUG mode
#endif

    // -------------------------------------------------
    //   Load stored MQTT credentials using Preferences
    // -------------------------------------------------
    Preferences preferences;
    preferences.begin("mqtt", false);
    String storedHost = preferences.getString("mqtt_host", "");
    String storedPort = preferences.getString("mqtt_port", "");
    String storedUser = preferences.getString("mqtt_user", "");
    String storedPass = preferences.getString("mqtt_pass", "");
    
    // Copy stored values to the global char arrays if available
    if (storedHost.length() > 0) {
        storedHost.toCharArray(MQTT_HOST, sizeof(MQTT_HOST));
    }
    if (storedPort.length() > 0) {
        storedPort.toCharArray(MQTT_PORT, sizeof(MQTT_PORT));
    }
    if (storedUser.length() > 0) {
        storedUser.toCharArray(MQTT_USER, sizeof(MQTT_USER));
    }
    if (storedPass.length() > 0) {
        storedPass.toCharArray(MQTT_PASS, sizeof(MQTT_PASS));
    }

    // -------------------------------------------------
    //    Use WiFiManager to handle WiFi credentials and 
    //    configure MQTT parameters using custom parameters.
    // -------------------------------------------------
    WiFiManager wm;

    // Create custom parameters for MQTT (server, port, user, pass)
    // ID, Placeholder text, Default (initial) value, length
    WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", MQTT_HOST, 64);
    WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", MQTT_PORT, 6);
    WiFiManagerParameter custom_mqtt_user("user", "MQTT User", MQTT_USER, 32);
    WiFiManagerParameter custom_mqtt_password("pass", "MQTT Password", MQTT_PASS, 32);

    // Add these MQTT parameters to WiFiManager
    wm.addParameter(&custom_mqtt_server);
    wm.addParameter(&custom_mqtt_port);
    wm.addParameter(&custom_mqtt_user);
    wm.addParameter(&custom_mqtt_password);

    // Optionally, you can set a custom config portal name
    // or pass a password as well: wm.autoConnect("p1meter", "ConfigPortalPassword");
    // The simplest form:
    wm.autoConnect(HOSTNAME);

    // If you get here, WiFi is connected and credentials are saved.
    // Retrieve the MQTT values the user entered
    strcpy(MQTT_HOST, custom_mqtt_server.getValue());
    strcpy(MQTT_PORT, custom_mqtt_port.getValue());
    strcpy(MQTT_USER, custom_mqtt_user.getValue());
    strcpy(MQTT_PASS, custom_mqtt_password.getValue());

    // Persistently store the MQTT credentials to Preferences.
    preferences.putString("mqtt_host", String(MQTT_HOST));
    preferences.putString("mqtt_port", String(MQTT_PORT));
    preferences.putString("mqtt_user", String(MQTT_USER));
    preferences.putString("mqtt_pass", String(MQTT_PASS));
    preferences.end();

#ifdef DEBUG
    Serial.println("Wi-Fi connected via WiFiManager.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("MQTT Settings from WiFiManager:");
    Serial.print("  Host: ");   Serial.println(MQTT_HOST);
    Serial.print("  Port: ");   Serial.println(MQTT_PORT);
    Serial.print("  User: ");   Serial.println(MQTT_USER);
    Serial.print("  Pass: ");   Serial.println(MQTT_PASS);
#endif

    // -------------------------------------------------
    //    Proceed with rest of setup logic
    // -------------------------------------------------
    setupDataReadout(); // Prepare data readout structures
    setupOTA();         // Initialize Over-the-Air updates

    // Configure the MQTT client with the user-provided server/port
    mqttClient.setServer(MQTT_HOST, atoi(MQTT_PORT));

    blinkLed(5, 500); // Blink 5 times to indicate end of setup

#ifdef DEBUG
    Serial.println("Ready");
#endif
}

/***********************************
            Main Loop
 ***********************************/
 void loop()
 {
     long now = millis();
 
     // Check Wi-Fi status
     if (WiFi.status() != WL_CONNECTED)
     {
         // Blink fast to indicate lost WiFi
         blinkLed(20, 50);
         // Attempt to reconnect
         WiFi.reconnect();
     }
 
     // Handle OTA updates
     ArduinoOTA.handle();
 
     // Check MQTT connection
     if (!mqttClient.connected())
     {
         if (now - LAST_RECONNECT_ATTEMPT > 5000)
         {
             LAST_RECONNECT_ATTEMPT = now;
 
             if (!mqttReconnect())
             {
 #ifdef DEBUG
                 Serial.println("Connection to MQTT Failed! Rebooting...");
 #endif
                 delay(5000);
                 ESP.restart();
             }
             else
             {
                 LAST_RECONNECT_ATTEMPT = 0;
             }
         }
     }
     else
     {
         mqttClient.loop();
     }
 
     // Check if we want a full update
     if (now - LAST_FULL_UPDATE_SENT > UPDATE_FULL_INTERVAL)
     {
         for (int i = 0; i < NUMBER_OF_READOUTS; i++)
         {
             telegramObjects[i].sendData = true;
         }
         LAST_FULL_UPDATE_SENT = millis();
     }
 
     // Regular update interval
     if (now - LAST_UPDATE_SENT > UPDATE_INTERVAL)
     {
         LAST_UPDATE_SENT = millis();
 
         // If the P1 data was read successfully, publish to MQTT
         if (readP1Serial())
         {
 #ifdef DEBUG
             Serial.println("Successfully read P1 serial");
 #endif
             sendDataToBroker();
         }
     }
 
     delay(1);
 }

/***********************************
            Setup Methods
 ***********************************/

/**
   setupDataReadout()

   This method can be used to create more data readout to mqtt topic.
   Use the name for the mqtt topic.
   The code for finding this in the telegram see
    https://www.netbeheernederland.nl/_upload/Files/Slimme_meter_15_a727fce1f1.pdf for the dutch codes pag. 19 -23
   Use startChar and endChar for setting the boundies where the value is in between.
   Default startChar and endChar is '(' and ')'
   Note: Make sure when you add or remove telegramObject to update the NUMBER_OF_READOUTS accordingly.
*/
void setupDataReadout()
{
    // Consumption Tariffs
    telegramObjects[0].name = "consumption_tarif_1";
    strcpy(telegramObjects[0].code, "1-0:1.8.1");
    telegramObjects[0].endChar = '*';

    telegramObjects[1].name = "consumption_tarif_2";
    strcpy(telegramObjects[1].code, "1-0:1.8.2");
    telegramObjects[1].endChar = '*';

    // Received Tariffs
    telegramObjects[2].name = "received_tarif_1";
    strcpy(telegramObjects[2].code, "1-0:2.8.1");
    telegramObjects[2].endChar = '*';

    telegramObjects[3].name = "received_tarif_2";
    strcpy(telegramObjects[3].code, "1-0:2.8.2");
    telegramObjects[3].endChar = '*';

    // Instantaneous Power
    telegramObjects[4].name = "actual_consumption";
    strcpy(telegramObjects[4].code, "1-0:1.7.0");
    telegramObjects[4].endChar = '*';

    telegramObjects[5].name = "actual_received";
    strcpy(telegramObjects[5].code, "1-0:2.7.0");
    telegramObjects[5].endChar = '*';

    // Instantaneous Power per Phase
    telegramObjects[6].name = "instant_power_usage_l1";
    strcpy(telegramObjects[6].code, "1-0:21.7.0");
    telegramObjects[6].endChar = '*';

    telegramObjects[7].name = "instant_power_usage_l2";
    strcpy(telegramObjects[7].code, "1-0:41.7.0");
    telegramObjects[7].endChar = '*';

    telegramObjects[8].name = "instant_power_usage_l3";
    strcpy(telegramObjects[8].code, "1-0:61.7.0");
    telegramObjects[8].endChar = '*';

    // Instantaneous Power Return per Phase
    telegramObjects[9].name = "instant_power_return_l1";
    strcpy(telegramObjects[9].code, "1-0:22.7.0");
    telegramObjects[9].endChar = '*';

    telegramObjects[10].name = "instant_power_return_l2";
    strcpy(telegramObjects[10].code, "1-0:42.7.0");
    telegramObjects[10].endChar = '*';

    telegramObjects[11].name = "instant_power_return_l3";
    strcpy(telegramObjects[11].code, "1-0:62.7.0");
    telegramObjects[11].endChar = '*';

    // Instantaneous Current
    telegramObjects[12].name = "instant_power_current_l1";
    strcpy(telegramObjects[12].code, "1-0:31.7.0");
    telegramObjects[12].endChar = '*';

    telegramObjects[13].name = "instant_power_current_l2";
    strcpy(telegramObjects[13].code, "1-0:51.7.0");
    telegramObjects[13].endChar = '*';

    telegramObjects[14].name = "instant_power_current_l3";
    strcpy(telegramObjects[14].code, "1-0:71.7.0");
    telegramObjects[14].endChar = '*';

    // Voltage
    telegramObjects[15].name = "instant_voltage_l1";
    strcpy(telegramObjects[15].code, "1-0:32.7.0");
    telegramObjects[15].endChar = '*';

    telegramObjects[16].name = "instant_voltage_l2";
    strcpy(telegramObjects[16].code, "1-0:52.7.0");
    telegramObjects[16].endChar = '*';

    telegramObjects[17].name = "instant_voltage_l3";
    strcpy(telegramObjects[17].code, "1-0:72.7.0");
    telegramObjects[17].endChar = '*';

    // Actual Tarif
    telegramObjects[18].name = "actual_tarif_group";
    strcpy(telegramObjects[18].code, "0-0:96.14.0");
    telegramObjects[18].endChar = ')';

    // Gas Meter Reading
    telegramObjects[19].name = "gas_meter_m3";
    strcpy(telegramObjects[19].code, "0-1:24.2.3");
    telegramObjects[19].endChar = '*'; 

#ifdef DEBUG
    Serial.println("MQTT Topics initialized:");
    for (int i = 0; i < NUMBER_OF_READOUTS; i++)
    {
        Serial.println(String(MQTT_ROOT_TOPIC) + "/" + telegramObjects[i].name);
    }
#endif
}

/**
   Over the Air update setup
*/
void setupOTA()
{
    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("Start updating " + type);
        })
        .onEnd([]() {
            Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });

    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.begin();
}
