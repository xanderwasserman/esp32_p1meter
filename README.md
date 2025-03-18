# esp32_p1meter
Software for an ESP32 that reads, decodes, and sends P1 smart meter (DSMR 5.0) data to an MQTT broker (e.g., for Home Assistant). It also supports Over-The-Air (OTA) firmware updates and easy Wi-Fi/MQTT configuration via [WiFiManager](https://github.com/tzapu/WiFiManager).

## About this Fork
This fork is based on an ESP32 dev kit and aims to:
- Optimize the project for reporting DSMR 5.0 data to Home Assistant via MQTT.
- Improve the robustness and readability of the code.
- Fix bugs that prevented the original project from working correctly.

## Features
- **Automatic Wi-Fi and MQTT Configuration**  
  Uses WiFiManager to let you configure your Wi-Fi network and MQTT credentials in a captive portal, so you don’t have to hardcode them in the source code.
- **P1 Smart Meter Reading**  
  Decodes DSMR 5.0 telegrams from the meter and publishes the data to MQTT topics.
- **OTA Updates**  
  Once flashed, you can update firmware wirelessly without physically connecting the ESP32 via USB.

---

## Hardware Requirements
- **ESP32** (DoIT DEVKIT V1 tested)
- **Small Breadboard** (optional, for easy prototyping)
- **10kΩ resistor** (pull-up on the P1 data line for many meters)
- **RJ11 (4-pin)** or **RJ12 (6-pin)** cable for the P1 port  
  - A 4-pin cable is sufficient for data and ground.  
  - A 6-pin cable can also carry power from many DSMR 5+ meters, so the ESP32 can be powered directly from the meter.

### Circuit Diagram
> **Note:** Tested with an `ISKRA AM550`. Other DSMR 5.0 meters should be similar.

| P1 Pin | ESP32 Pin    |
| ------ | ------------ |
| 2 - RTS | 3.3V        |
| 3 - GND | GND         |
| 4 -  –  | (Not used)  |
| 5 - RXD (Data) | RXD2 (GPIO16)  |

A **10kΩ resistor** is typically placed between **3.3V** and **RXD** on the meter side to pull the data line high. Many guides mention 5V on RTS, but on some meters (like the ISKRA AM550), **3.3V** is sufficient to activate P1.

<details>
<summary>Optional: Powering the ESP32 from the P1 Port (DSMR5+)</summary>

If you have a 6-pin cable and a meter that provides 5V, you can power the ESP32 from the meter directly.

| P1 Pin   | ESP32 Pin |
| ----     | --------- |
| 1 - 5V out | 5V or Vin |
| 2 - RTS  | 3.3V (enables data) |
| 3 - GND  | GND  |
| 4 -      |      |
| 5 - RXD (Data) | RXD2 (GPIO16) |
| 6 - GND  | GND  |

However, some ESP32 boards may not properly cold-boot on 5V from the meter alone. You might need to power the ESP32 by USB initially, then the meter power can sustain it afterward.

</details>

---

## Software Setup

1. **Install Required Libraries in Arduino IDE**  
   - **ESP32 Board Support** in the Boards Manager (search for “esp32”).
   - **PubSubClient** (Tools → Manage Libraries → search for “PubSubClient”).
   - **WiFiManager** (Tools → Manage Libraries → search for WiFiManager [tzapu/WiFiManager](https://github.com/tzapu/WiFiManager)).

2. **Open this project in Arduino IDE**  
   Make sure you’ve selected the correct ESP32 board under “Tools → Board”.

3. **First-Time Flash via USB**  
   - Connect the ESP32 to your computer using USB.
   - Compile and upload the sketch.  
   After the first flash, you can do all future flashes Over-the-Air (OTA).

4. **Initial Wi-Fi & MQTT Setup**  
   - Once the ESP32 boots (for the first time or if no credentials are found), it will create a WiFi Access Point named **“p1meter”**.
   - Connect to it from your phone or laptop. A captive portal should appear (or browse to `192.168.4.1` if it doesn’t).
   - **Enter your home Wi-Fi credentials** and the **MQTT server details** (host, port, user, password).
   - Save/submit; the ESP32 will store these in its internal memory and reboot.

5. **OTA Updates**  
   - With Wi-Fi configured, future firmware updates can be done via Arduino IDE’s network port or tools like `espota.py`.
   - Alternatively, you can use the ArduinoOTA library with a password if defined in `settings.h`.

---

## Data Sent
By default, the code sends various metrics to individual MQTT topics under a root topic (e.g., `homeassistant/sensors/power/p1meter`). You can see the exact topics by checking the **Serial Monitor** in debug mode or by browsing your MQTT broker. Some examples:

```
homeassistant/sensors/power/p1meter/consumption_tarif_1
homeassistant/sensors/power/p1meter/consumption_tarif_2
homeassistant/sensors/power/p1meter/received_tarif_1
homeassistant/sensors/power/p1meter/received_tarif_2
homeassistant/sensors/power/p1meter/actual_consumption
homeassistant/sensors/power/p1meter/actual_received
homeassistant/sensors/power/p1meter/instant_power_usage_l1
homeassistant/sensors/power/p1meter/instant_power_usage_l2
homeassistant/sensors/power/p1meter/instant_power_usage_l3
homeassistant/sensors/power/p1meter/instant_power_return_l1
homeassistant/sensors/power/p1meter/instant_power_return_l2
homeassistant/sensors/power/p1meter/instant_power_return_l3
homeassistant/sensors/power/p1meter/instant_power_current_l1
homeassistant/sensors/power/p1meter/instant_power_current_l2
homeassistant/sensors/power/p1meter/instant_power_current_l3
homeassistant/sensors/power/p1meter/instant_voltage_l1
homeassistant/sensors/power/p1meter/instant_voltage_l2
homeassistant/sensors/power/p1meter/instant_voltage_l3
homeassistant/sensors/power/p1meter/gas_meter_m3
homeassistant/sensors/power/p1meter/actual_tarif_group
...
```

You can add or modify metrics in the `setupDataReadout()` function. Refer to the [DSMR 5.0 documentation (PDF)](https://www.netbeheernederland.nl/_upload/Files/Slimme_meter_15_a727fce1f1.pdf) (pages 19–23) for the OBIS codes you want to parse.

---

## Home Assistant Configuration
In Home Assistant, you can define sensors in your `configuration.yaml`. Below is an example snippet:

```
# MQTT sensors configuration
mqtt:
  sensor:    
    - name: "Grid Consumption Low Tariff"
      unit_of_measurement: "kWh"
      state_topic: "homeassistant/sensors/power/p1meter/consumption_tarif_1"
      value_template: "{{ value|float / 1000 }}"
      device_class: "energy"
      state_class: "total"

    - name: "Grid Consumption High Tariff"
      unit_of_measurement: "kWh"
      state_topic: "homeassistant/sensors/power/p1meter/consumption_tarif_2"
      value_template: "{{ value|float / 1000 }}"
      device_class: "energy"
      state_class: "total"

    - name: "Grid Return Delivery High Tariff"
      unit_of_measurement: "kWh"
      state_topic: "homeassistant/sensors/power/p1meter/received_tarif_2"
      value_template: "{{ value|float / 1000 }}"
      device_class: "energy"
      state_class: "total"

    - name: "Grid Return Delivery Low Tariff"
      unit_of_measurement: "kWh"
      state_topic: "homeassistant/sensors/power/p1meter/received_tarif_1"
      value_template: "{{ value|float / 1000 }}"
      device_class: "energy"
      state_class: "total"

    - name: "Grid Actual Power Consumption"
      unit_of_measurement: "kW"
      state_topic: "homeassistant/sensors/power/p1meter/actual_consumption"
      value_template: "{{ value|float / 1000 }}"
      device_class: "energy"

    - name: "Grid Actual Return Delivery"
      unit_of_measurement: "kW"
      state_topic: "homeassistant/sensors/power/p1meter/actual_received"
      value_template: "{{ value|float / 1000 }}"
      device_class: "energy"

    - name: "Grid L1 Instant Power Usage"
      unit_of_measurement: "kW"
      state_topic: "homeassistant/sensors/power/p1meter/instant_power_usage_l1"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid L2 Instant Power Usage"
      unit_of_measurement: "kW"
      state_topic: "homeassistant/sensors/power/p1meter/instant_power_usage_l2"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid L3 Instant Power Usage"
      unit_of_measurement: "kW"
      state_topic: "homeassistant/sensors/power/p1meter/instant_power_usage_l3"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid L1 Instant Power Return"
      unit_of_measurement: "kW"
      state_topic: "homeassistant/sensors/power/p1meter/instant_power_return_l1"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid L2 Instant Power Return"
      unit_of_measurement: "kW"
      state_topic: "homeassistant/sensors/power/p1meter/instant_power_return_l2"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid L3 Instant Power Return"
      unit_of_measurement: "kW"
      state_topic: "homeassistant/sensors/power/p1meter/instant_power_return_l3"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid L1 Instant Power Current"
      unit_of_measurement: "A"
      state_topic: "homeassistant/sensors/power/p1meter/instant_power_current_l1"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid L2 Instant Power Current"
      unit_of_measurement: "A"
      state_topic: "homeassistant/sensors/power/p1meter/instant_power_current_l2"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid L3 Instant Power Current"
      unit_of_measurement: "A"
      state_topic: "homeassistant/sensors/power/p1meter/instant_power_current_l3"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid L1 Voltage"
      unit_of_measurement: "V"
      state_topic: "homeassistant/sensors/power/p1meter/instant_voltage_l1"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid L2 Voltage"
      unit_of_measurement: "V"
      state_topic: "homeassistant/sensors/power/p1meter/instant_voltage_l2"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid L3 Voltage"
      unit_of_measurement: "V"
      state_topic: "homeassistant/sensors/power/p1meter/instant_voltage_l3"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid Gas Usage"
      unit_of_measurement: "m3"
      state_topic: "homeassistant/sensors/power/p1meter/gas_meter_m3"
      value_template: "{{ value|float / 1000 }}"

    - name: "Grid Actual Tariff Group"
      state_topic: "homeassistant/sensors/power/p1meter/actual_tarif_group"
```

## Known limitations and issues
- Cold Boot on Meter Power: Some meters (e.g., ISKRA AM550) provide 5V on pin 1. The ESP32 might boot-loop if started directly from meter power. You may need to power it initially by USB, then the meter power can sustain it afterward.
- Data Variations by Meter: Different DSMR 5.0 meters might provide slightly different OBIS codes or require different resistor values on the RTS/data lines.

## Contributing
Feel free to open issues or pull requests if you find bugs or want to add new features. Contributions are welcome!

Other sources:
- [DSMR 5.0 documentation](https://www.netbeheernederland.nl/_upload/Files/Slimme_meter_15_a727fce1f1.pdf)
