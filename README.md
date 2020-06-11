# esp32_p1meter
Software for the ESP32 (DoIT ESP DEVKIT v1/NodeMcu 32s etc.) that decodes and sends P1 smart meter (DSMR) data to a MQTT broker, with the possibility for Over The Air (OTA) firmware updates.

## About this fork
This fork was based on a ESP8266 and I only had a ESP32 laying around so I'm trying to make this work on my ESP32 DoIT board.

The original project of [fliphess](https://github.com/fliphess/esp8266_p1meter) has issues with DSMR 5.0 meters, which send telegrams every 1 second at a high 115200 baud rate. 
This causes the used SoftwareSerial to struggle to keep up and thus only receives corrupted messages. 

The project of [daniel-jong](https://github.com/daniel-jong/esp8266_p1meter) switches to using the main hardware serial port (RX) for communication with the p1 meter and is tested on the `Landys and Gyr E360` smartmeter (DSMR 5.0).

Then I noticed the project of [WhoSayIn](https://github.com/WhoSayIn/esp8266_dsmr2mqtt), that takes a much more minimalistic approach, which I liked. However, I discovered this project was also designed for the DSMR 4.0 meters.

With this fork, I want to accomplish the following:
- Combine the projects mentioned above in a minimalistic setup for the newer DSMR 5.0 smart meters.
- Separate code in multiple files for readability.
- Add solar panel meter: read out delivered energy.
- Easy to read and add new readouts from a telegram. Used a struct to accomplish this.
- Generate the full MQTT topics based on the array of telegram decode structs.
- Easy to debug the software and able to compile without the debug for a more compact compiled code base. 

I noticed that the other repositories use SoftwareSerial library to readout the P1 port but the ESP32 has multiple RX and TX port to read en write serial streams. This made it easy to debug the code and have the full speed of the hardware serial. Also the ESP32 is a bit faster so it doesn't crash as fast as a ESP8266 when you want to readout every second.

## Setup
This setup requires:
- An ESP32 (DoIT DEVKIT v1 has been tested)
- Small breadboard
- A 10k ohm resistor
- A 4 pin (RJ11) or [6 pin (RJ12) cable](https://www.tinytronics.nl/shop/nl/kabels/adapters/rj12-naar-6-pins-dupont-jumper-adapter). Both cables work great, but a 6 pin cable can also power the ESP8266 on most DSMR5+ meters.

Setting up your Arduino IDE:
- Ensure you have selected the right board (you might need to install your esp32board in the Arduino IDE).
- I have tested this on the 80 MHz and 160 MHz CPU frequency mode, pick either one.
- Using the Tools->Manage Libraries... install `PubSubClient`.
- In the file `Settings.h` change all values accordingly
- Write to your device via USB the first time, you can do it OTA all times thereafter.

### Circuit diagram
_Note: I have only tested this on the `ISKRA AM550`._
I have used the RX02 pin on the ESP32 so u can still use the USB port for debugging you ESP32.
Connect the ESP32 to an RJ11 cable/connector following the diagram.

| P1 pin   | ESP32 Pin |
| ----     | ---- |
| 2 - RTS  | 3.3v |
| 3 - GND  | GND  |
| 4 -      |      |
| 5 - RXD (data) | RX02 (gpio16) |

On most models a 10K resistor should be used between the ESP's 3.3v and the p1's DATA (RXD) pin. Many howto's mention RTS requires 5V (VIN) to activate the P1 port, but for me 3V3 suffices.

<details><summary>Optional: Powering the ESP8266 using your DSMR5+ meter</summary>
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
No known limitations and issues so far.

## Thanks to
I want to Thank [JHockx](https://github.com/jhockx/esp8266_p1meter) because he told me he was working on a project reading out his P1 Meter. It sounded like a fun project but I had somewhat different hardware laying around so I started working with that. 

I also want to thank all the people he mentions in his project: 
- https://github.com/fliphess/esp8266_p1meter
- https://github.com/jantenhove/P1-Meter-ESP8266
- https://github.com/neographikal/P1-Meter-ESP8266-MQTT
- http://gejanssen.com/howto/Slimme-meter-uitlezen/
- https://github.com/rroethof/p1reader/
- http://romix.macuser.nl/software.html
- http://blog.regout.info/category/slimmeter/
- http://domoticx.com/p1-poort-slimme-meter-hardware/

In addition, I'd like thank and refer to the following projects which served as a source of information:
- [https://github.com/daniel-jong/esp8266_p1meter](https://github.com/daniel-jong/esp8266_p1meter)
- [https://github.com/WhoSayIn/esp8266_dsmr2mqtt](https://github.com/WhoSayIn/esp8266_dsmr2mqtt)
- [https://github.com/jhockx/esp8266_p1meter](https://github.com/jhockx/esp8266_p1meter)

Other sources:
- [DSMR 5.0 documentation](https://www.netbeheernederland.nl/_upload/Files/Slimme_meter_15_a727fce1f1.pdf)
