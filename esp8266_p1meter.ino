#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

#include "settings.h"

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void setup()
{
  // Initialize pins
  pinMode(LED_BUILTIN, OUTPUT); // Inverted on Wemos D1 Mini
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(BAUD_RATE, SERIAL_8N1, SERIAL_RX_ONLY, 3, true);
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    blinkLed(20, 50); // Blink fast to indicate no WiFi connection
    delay(500);
  }

  setup_ota();

  mqtt_client.setServer(MQTT_HOST, atoi(MQTT_PORT));
  blinkLed(5, 500); // Blink 5 times to indicate end of setup
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
    blinkLed(20, 50); // Blink fast to indicate failed WiFi connection
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
  }

  ArduinoOTA.handle();
  long now = millis();

  if (!mqtt_client.connected())
  {
    long now = millis();

    if (now - LAST_RECONNECT_ATTEMPT > 5000)
    {
      LAST_RECONNECT_ATTEMPT = now;

      if (mqtt_reconnect())
      {
        LAST_RECONNECT_ATTEMPT = 0;
      }
    }
  }
  else
  {
    mqtt_client.loop();
  }

  read_p1_serial();
}
