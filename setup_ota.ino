void setup_ota()
{
  ArduinoOTA.setPort(8266);

  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onError([](ota_error_t error)
  {
    blinkLed(3, 2000); // Blink 3 times slowly to indicate OTA error
  });

  ArduinoOTA.begin();
}
