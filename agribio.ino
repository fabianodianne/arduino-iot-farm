#include <DHT.h>
#include <SoftwareSerial.h>

#define DHTPIN 2
#define DHTTYPE DHT22

#define ARDUINO_RX 0
#define ARDUINO_TX 1

SoftwareSerial arduinoSerial(ARDUINO_RX, ARDUINO_TX);

DHT dht(DHTPIN, DHTTYPE); // Initialize the DHT sensor

SoftwareSerial sim(12, 11); // Initialize software serial for SIM module
const String number = "639451722389"; // Phone number for SMS notifications

const float minTemperatureForWatering = 25.0; // Minimum temperature for watering
const float maxHumidityForWatering = 70.0; // Maximum humidity for watering
const int sensor_pin = A0, relayPin = 8, trig = 9, echo = 10; // Pin assignments
float distanceInCm; // Variable to store distance in centimeters

void setup() {
  Serial.begin(9600);
  delay(100);
  sim.begin(9600);
  delay(100);
  arduinoSerial.begin(9600);

  sim.println("AT+CMGF=1"); // Set SMS mode to text
  delay(200);

  checkSIMStatus();

  dht.begin();
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  pinMode(sensor_pin, INPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
}

void loop() {
  receiveMessage();
  printStatus();
  delay(1000);
  checkSerialCommands();
  
  // Turn off watering if soil is wet
  if (getSoilMoistureStatus() >= 40 && digitalRead(relayPin) == LOW) {
    digitalWrite(relayPin, HIGH);
  }

  // Send warning if water level is low
  if(measureWaterLevel() < 30) { 
    digitalWrite(relayPin, HIGH);
  }
  
  delay(1000);
}

// ------------------------------------------------------------------------------------------------

void checkSerialCommands() {
  if (arduinoSerial.available()) {
    String data = arduinoSerial.readStringUntil('\n');
    data.trim();

    if(data.equals("s")) {
      printStatus();
    } else if (data.equals("on")) {
      digitalWrite(relayPin, LOW);
      sendSMSConfirmation(number, "Water is turned on.");
    } else if (data.equals("off")) {
      digitalWrite(relayPin, HIGH);
      sendSMSConfirmation(number, "Water turned off");
    }
  }
}

// -----------------------------------------------------------------------------------

void printStatus() {
  // Read sensor data
  int soilMoisture = getSoilMoistureStatus();
  float temperature = dht.readTemperature();
  int humidity = dht.readHumidity();
  int waterLevel = measureWaterLevel();
  String waterSwitch = digitalRead(relayPin) == HIGH ? "off" : "on";

  // Construct the data string
  String sensorData = String(soilMoisture) + "," + String(temperature) + "," + String(humidity) + "," + String(waterLevel) + "," + waterSwitch;

  // Send the data to NodeMCU
  Serial.println(sensorData);
}

// ------------------------------------------------------------------------------------------------

void checkSIMStatus() {
  sim.println("AT+CNUM"); // Send command to get SIM card number

  delay(1000);
  while (sim.available()) {
    String response = sim.readStringUntil('\n');
    response.trim();
    Serial.println(response); // Print SIM status response
  }
}

// Function to receive SMS messages
void receiveMessage() {
  sim.println("AT+CMGF=1"); // Set SMS mode to text
  delay(1000);

  sim.println("AT+CNMI=2,2,0,0,0"); // Set module to forward received messages to Arduino
  delay(1000);

  while (sim.available() > 0) {
    String receivedMessage = sim.readStringUntil("\n\n\n\n");
    receivedMessage.trim();
    if (receivedMessage.indexOf("+CMT:") != -1) {
      Serial.println("receivedMessage: " + receivedMessage);

      int start = receivedMessage.indexOf('\"') + 1;
      int end = receivedMessage.indexOf('\"', start);

      // Extract sender's number from received message
      String senderNumber = receivedMessage.substring(start, end);
      senderNumber.trim();
      senderNumber.replace("+", "");

       // Find the starting index of the SMS content
      int contentStart = receivedMessage.indexOf('\n', end) + 1;

      // Extract the SMS content
      String smsContent = receivedMessage.substring(contentStart);
      smsContent.trim();

      Serial.println("\nFrom: " + senderNumber);
      Serial.println("SMS Content: " + smsContent + "\n\n");

      processCommand(senderNumber, smsContent);
    }
  }
}

void processCommand(String senderNumber, String smsContent) {
  if (smsContent.indexOf("status") != -1) {
    sendMessage(senderNumber);
  } else if (smsContent.indexOf("on") != -1) {
    digitalWrite(relayPin, LOW);
    sendSMSConfirmation(senderNumber, "Water is turned on.");
  } else if (smsContent.indexOf("off") != -1) {
    digitalWrite(relayPin, HIGH);
    sendSMSConfirmation(senderNumber, "Water turned off");
  }
}

// ------------------------------------------------------------------------------------------------

void sendMessage(const String& number) {
  // Compose SMS message with sensor readings
  String moisture = getSoilMoistureStatus() >= 40 ? "wet" : "dry";

  String message = "Soil Moisture: " + moisture + "\n";
  message += "Temperature: " + String(dht.readTemperature()) + " degrees\n";
  message += "Humidity: " + String(dht.readHumidity()) + "%\n";
  message += "Water Level: " + String(measureWaterLevel()) + "%\n";

  message += checkStatus() == 1 ? "\nBased on current conditions, it's a good time to water your plant." : "\nNo watering is required at the moment.";
  
  message += digitalRead(relayPin) == HIGH ? "\nWater Sprinker is OFF" : "Water Sprinker is ON";


  sendSMS(number, message);
  Serial.println("SMS Sent to: " + number);
  Serial.println("Message:\n" + message);
}

void sendSMS(String recipientNumber, String message) {
  sim.println("AT+CMGF=1");
  delay(200);
  sim.println("AT+CMGS=\"" + recipientNumber + "\"\r");
  delay(200);
  sim.print(message);
  sim.println((char)26);
  delay(200);
}

void sendSMSConfirmation(String number, String confirmation) {
  Serial.println(confirmation);
  sim.println("AT+CMGF=1");
  delay(200);
  sim.println("AT+CMGS=\"" + number + "\"\r");
  delay(200);
  
  sim.print(confirmation);
  sim.println((char)26);
  delay(200);
}

// ---------------------------------------------------------------------------------------------------

int checkStatus() {
  // Check humidity, temperature, soil moisture, and water level to determine if watering is needed
  if (checkHumidityAndTemp() && getSoilMoistureStatus() < 40 && measureWaterLevel() > 20 && digitalRead(relayPin) == HIGH) {
    return 1;
  } else {
    return 0;
  }
}

int checkHumidityAndTemp() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Check if it's best to water the plants based on temperature and humidity
  if (temperature >= minTemperatureForWatering && humidity <= maxHumidityForWatering) {
    return 1;
  } else {
    return 0;
  }
}

int getSoilMoistureStatus() {
  int sensor_data = analogRead(sensor_pin);

  int mappedData = map(sensor_data, 0, 1023, 100, 0);

  return mappedData;
}

int measureWaterLevel() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  int timeInMicro = pulseIn(echo, HIGH);
  distanceInCm = timeInMicro / 29.0 / 2.0;
  int percentage = max(0.0, min(100.0, 100.0 - (distanceInCm / 50.0 * 100.0)));
  return percentage;
}
