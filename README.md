# esp32_p1meter
Software for an ESP32 that receives, decodes and sends P1 smart meter (DSMR5.0) data to a MQTT broker for HomeAssistant, with the possibility for Over The Air (OTA) firmware updates.

## About this fork
This fork is based on an ESP32 dev kit and aims to accomplish the following:
- Optimise the project for reporting DSMR 5.0 data to HomeAssistant via MQTT.
- Makes improvements to the robustness and readability of the code.
- Fixes a couple bugs that prevented to project from working correctly.

## Setup
This setup requires:
- An ESP32 (DoIT DEVKIT v1 has been tested)
- Small breadboard
- A 10k ohm resistor
- A 4 pin (RJ11) or [6 pin (RJ12) cable](https://www.tinytronics.nl/shop/nl/kabels/adapters/rj12-naar-6-pins-dupont-jumper-adapter). Both cables work great, but a 6 pin cable can also power the ESP32 on most DSMR5+ meters.

Setting up your Arduino IDE:
- Ensure you have selected the right board (you might need to install your esp32board in the Arduino IDE).
- Using the Tools->Manage Libraries... install `PubSubClient`.
- In the file `Settings.h` change all values accordingly
- Write to your device via USB the first time, you can do it OTA all times thereafter.

### Circuit diagram
_Note: I have only tested this on the `ISKRA AM550`._
The RX02 pin on the ESP32 is used to connect to the Smart Meter, so you can still use the USB port for debugging your ESP32.
Connect the ESP32 to an RJ11 cable/connector following the diagram.

| P1 pin   | ESP32 Pin |
| ----     | ---- |
| 2 - RTS  | 3.3v |
| 3 - GND  | GND  |
| 4 -      |      |
| 5 - RXD (data) | RX02 (gpio16) |

On most models a 10K resistor should be used between the ESP's 3.3v and the p1's DATA (RXD) pin. Many howto's mention RTS requires 5V (VIN) to activate the P1 port, but for me 3V3 suffices.

<details><summary>Optional: Powering the ESP32 using your DSMR5+ meter</summary>
<p>
When using a 6 pin cable you can use the power source provided by the meter.
  
| P1 pin   | ESP32 Pin |
| ----     | ---- |
| 1 - 5v out | 5v or Vin |
| 2 - RTS  | 3.3v |
| 3 - GND  | GND  |
| 4 -      |      |
| 5 - RXD (data) | RX02 (gpio16) |
| 6 - GND  | GND  |

</p>
</details>

### Data Sent

All metrics are send to their own MQTT topic.
The software generates all the topic through the Serial monitor when starting up
Example topics are:

```
sensors/power/p1meter/consumption_low_tarif
sensors/power/p1meter/consumption_high_tarif
sensors/power/p1meter/actual_received
sensors/power/p1meter/instant_power_usage_l1
sensors/power/p1meter/instant_power_usage_l2
sensors/power/p1meter/instant_power_usage_l3
sensors/power/p1meter/instant_power_current_l1
sensors/power/p1meter/instant_power_current_l2
sensors/power/p1meter/instant_power_current_l3
sensors/power/p1meter/instant_voltage_l1
sensors/power/p1meter/instant_voltage_l2
sensors/power/p1meter/instant_voltage_l3
sensors/power/p1meter/actual_tarif_group
sensors/power/p1meter/short_power_outages
sensors/power/p1meter/long_power_outages
sensors/power/p1meter/short_power_drops
sensors/power/p1meter/short_power_peaks
```

But all the metrics you need are easily added using the `setupDataReadout()` method. With the DEBUG mode it is easy to see all the topics you add/create by the serial monitor. To see what your telegram is outputting in the Netherlands see: https://www.netbeheernederland.nl/_upload/Files/Slimme_meter_15_a727fce1f1.pdf for the dutch codes pag. 19 -23

### Home Assistant Configuration

Use this [example](https://raw.githubusercontent.com/daniel-jong/esp8266_p1meter/master/assets/p1_sensors.yaml) for home assistant's `sensor.yaml`

## Known limitations and issues
My ESP32 can use the 5v from the `ISKRA AM550` but you first need to power it on via USB else it will bootloop. After it's booted and connected with the 5v port on the P1 connection you can unplug the ESP32 and it will stay on.

Other sources:
- [DSMR 5.0 documentation](https://www.netbeheernederland.nl/_upload/Files/Slimme_meter_15_a727fce1f1.pdf)
