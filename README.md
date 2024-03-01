### Agribio: Smart Watering System for Farms

Agribio is a project aimed at automating the watering process for farms using an Arduino-based system integrated with GSM communication capabilities.

#### Features:

- **Smart Watering:** Agribio utilizes soil moisture sensors to determine the moisture levels in the soil and automatically waters the crops when needed.
- **Remote Control:** Farmers can remotely control the watering system by sending SMS commands to the system. Commands include turning the water pump on or off and checking the status of soil moisture, temperature, humidity, and water level.
- **Timer Functionality:** The system allows users to specify the duration for which the water pump should remain on. By default, the pump is set to run for 3 minutes, but users can customize the duration via SMS commands.
- **App Integration:** In addition to SMS control, users can also control and check the status of the system using a mobile application connected to a Wi-Fi module. The app provides a user-friendly interface for managing the watering system remotely.

#### Components:

- **Arduino Board:** The brain of the system, responsible for reading sensor data, controlling the water pump, and handling communication with the GSM module.
- **GSM Module:** Enables communication between the Arduino board and the user's mobile phone via SMS.
- **Wi-Fi Module:** Allows integration with a mobile application for remote control and monitoring of the system.
- **DHT11 Sensor:** Measures temperature and humidity levels in the environment.
- **Soil Moisture Sensor:** Detects the moisture content in the soil to determine watering needs.
- **Ultrasonic Sensor:** Measures the water level in the reservoir or tank.
- **Relay Module:** Controls the water pump, allowing it to be turned on or off based on sensor readings and user commands.

#### Code Overview:

The Arduino code provided implements the functionality described above. It reads sensor data, handles SMS commands received from the user, and controls the water pump accordingly. Additionally, it includes timer functionality to automatically turn off the water pump after a specified duration.

#### Usage:

1. **Installation:** Connect the Arduino board to the required sensors, the GSM module, and the Wi-Fi module. Ensure proper wiring and connections.
2. **Setup:** Upload the provided Arduino code to the board using the Arduino IDE.
3. **Operation:** Power on the system. It will automatically start monitoring soil moisture, temperature, humidity, and water level.
4. **Remote Control:** Send SMS commands to the GSM module to control the watering system remotely. Available commands include:
   - `water on`: Turns on the water pump for the default duration of 3 minutes.
   - `water on <duration>`: Turns on the water pump for the specified duration in minutes.
   - `water off`: Turns off the water pump.
   - `status`: Retrieves the current status of the system, including soil moisture, temperature, humidity, and water level.
5. **App Control:** Use the mobile application connected to the Wi-Fi module to control and monitor the system remotely.

#### Contributors:

- Dianne Mae P. Fabiano
