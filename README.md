# Description

**Gas and Fire Detection** is a simple Internet of Things (IoT) project designed to measure gas concentration and detect the presence of fire.

This project consists of two main components: a **sensor component** and an **indicator component**. The sensor component functions as a detection and monitoring tool, using special sensors to measure gas concentration and detect fire. Meanwhile, the indicator component functions as a medium to display measurement results and provide warnings. This indicator consists of a buzzer, LED, and monitor that visually and audibly display the detection results.

These two components are connected via the MQTT protocol, a communication protocol that allows sensor components to connect with various indicators simultaneously, as long as the indicators are connected to the same topic. As long as both components are connected to WiFi or an internet network, the indicator component will display the detection and measurement results in real time wherever the indicator component is located.

## Tech Stack

![Static Badge](https://img.shields.io/badge/C%2B%2B-%23044F88?logo=c%2B%2B&logoColor=white) ![Static Badge](https://img.shields.io/badge/MQTT-purple?logo=mqtt&logoColor=white) ![Static Badge](https://img.shields.io/badge/ESP32-grey?logo=espressif&logoColor=white&label=Espressif&labelColor=%23e4372e)

The microcontrollers I use in this project are three ESP32 DevKit V1.For the IDE I use is Arduino IDE version 2.3.2. 

The protocol I use to connect the sensor component with the indicator component is MQTT with the application to monitor the process of publishing data to the specified topic is using MQTTX.

## Demo

## Features

* Measuring gas concentration levels.
* Detecting the presence of fire.
* Measurement and detection indicators consisting of LEDs and buzzers.
* Display monitor as a device for displaying gas concentration measurement results.

# Installation

# Contributors

<a href="https://github.com/call-me-ahmaaad/gas-and-fire-detection/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=call-me-ahmaaad/gas-and-fire-detection" />
</a>

Made with [contrib.rocks](https://contrib.rocks).