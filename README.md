# Smart Plant Watering System with Arduino Uno R3 and NodeMCU

This project implements a smart plant watering system using an Arduino Uno R3 for sensor data processing and control, with the NodeMCU (ESP8266) board acting as a WiFi module for data transmission to a remote server or application.

## Features

1. **Remote Monitoring**: Monitor plant conditions remotely via the Blynk app, including temperature, humidity, soil moisture, and water level.

2. **Remote Control**: Control the water pump remotely through the Blynk app or SMS command to turn it on/off as needed.

3. **Efficient Water Usage**: Optimize water usage by watering plants only when necessary, based on real-time environmental data.

## Hardware Components

- Arduino Uno R3
- NodeMCU (ESP8266)
- DHT22 temperature and humidity sensor
- Soil moisture sensor
- Ultrasonic sensor for water level measurement
- Relay for water pump control
- 12v Solenoid Water Vavle

## Functionality

1. **Setup**: Connects the Arduino Uno R3 and NodeMCU to the WiFi network and initializes Blynk.
2. **Main Loop**: Runs the Blynk communication loop on the NodeMCU and receives data from the Arduino Uno R3.
3. **Receive Data Function**: Reads sensor data from the Arduino Uno R3, sends it to the NodeMCU via serial communication, and transmits it to the Blynk server.

## Usage

1. Upload the code to both the Arduino Uno R3 and the NodeMCU (ESP8266).
2. Connect sensors to the Arduino Uno R3 and the water pump relay.
3. Ensure both the Arduino Uno R3 and NodeMCU are powered on and properly connected.
4. Open the Blynk app, load the "Agribio" template, and monitor/control the plant watering system remotely.

## Note

Ensure that the Blynk app is configured with the provided template ID and authentication token.

For any issues or improvements, feel free to contact the developer.

**Developer:** DIANNE MAE P. FABIANO
