void send_mqtt_message(const char *topic, char *payload)
{
  bool result = mqtt_client.publish(topic, payload, false);
}

bool mqtt_reconnect()
{
  int MQTT_RECONNECT_RETRIES = 0;

  while (!mqtt_client.connected() && MQTT_RECONNECT_RETRIES < MQTT_MAX_RECONNECT_TRIES)
  {
    MQTT_RECONNECT_RETRIES++;

    if (mqtt_client.connect(HOSTNAME, MQTT_USER, MQTT_PASS))
    {
      char *message = new char[16 + strlen(HOSTNAME) + 1];
      strcpy(message, "p1 meter alive: ");
      strcat(message, HOSTNAME);
      mqtt_client.publish("hass/status", message);
    }
    else
    {
      delay(5000);
    }
  }

  if (MQTT_RECONNECT_RETRIES >= MQTT_MAX_RECONNECT_TRIES)
  {
    return false;
  }

  return true;
}

void send_metric(String name, long metric)
{
  if (metric > 0) {
    char output[10];
    ltoa(metric, output, sizeof(output));

    String topic = String(MQTT_ROOT_TOPIC) + "/" + name;
    send_mqtt_message(topic.c_str(), output);
  }
}

void send_data_to_broker()
{
  // If statement as "power-loss-reset fix" (see README.md)
  if (CONSUMPTION_HIGH_TARIF > 0) send_metric("consumption_high_tarif", CONSUMPTION_HIGH_TARIF);
  if (CONSUMPTION_LOW_TARIF > 0) send_metric("consumption_low_tarif", CONSUMPTION_LOW_TARIF);
  if (DELIVERED_HIGH_TARIF > 0) send_metric("delivered_high_tarif", DELIVERED_HIGH_TARIF);
  if (DELIVERED_LOW_TARIF > 0) send_metric("delivered_low_tarif", DELIVERED_LOW_TARIF);
  if (GAS_METER_M3 > 0) send_metric("gas_meter_m3", GAS_METER_M3);
  
  send_metric("actual_consumption", ACTUAL_CONSUMPTION);
  send_metric("instant_power_usage", INSTANT_POWER_USAGE);
  send_metric("instant_power_current", INSTANT_POWER_CURRENT);
  
  send_metric("actual_tarif_group", ACTUAL_TARIF);
  send_metric("short_power_outages", SHORT_POWER_OUTAGES);
  send_metric("long_power_outages", LONG_POWER_OUTAGES);
  send_metric("short_power_drops", SHORT_POWER_DROPS);
  send_metric("short_power_peaks", SHORT_POWER_PEAKS);
}
