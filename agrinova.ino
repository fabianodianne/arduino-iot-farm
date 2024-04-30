#include <DHT.h>
#include <SoftwareSerial.h>

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

#define SIM_TX 12
#define SIM_RX 13

SoftwareSerial sim(SIM_TX, SIM_RX);

const float minTemperatureForWatering = 25.0;
const float maxHumidityForWatering = 70.0; 
const int sensor_pin = A5, relayPin = 7, trig = 8, echo = 9; // Pin assignments
float distanceInCm;

void setup() {
  Serial.begin(9600);
  delay(100);
  sim.begin(19200);
  delay(100);

  setupSIM();

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
  checkSerialCommands();

  // Turn off watering if soil is wet
  if (getSoilMoistureStatus() >= 40 && digitalRead(relayPin) == LOW) {
    digitalWrite(relayPin, HIGH);
  }

  // Send warning if water level is low
  if(measureWaterLevel() < 30) { 
    digitalWrite(relayPin, HIGH);
  }

  delay(600);
}

// ------------------------------------------------------------------------------------------------

void checkSerialCommands() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim();

    if (data.equals("on")) {
      digitalWrite(relayPin, LOW);
    } else if (data.equals("off")) {
      digitalWrite(relayPin, HIGH);
    }
  }
}

// -----------------------------------------------------------------------------------

void printStatus() {
  // Read sensor data
  int soilMoisture = getSoilMoistureStatus();
  int temperature = dht.readTemperature();
  int humidity = dht.readHumidity();
  int waterLevel = measureWaterLevel();
  String waterSwitch = digitalRead(relayPin) == HIGH ? "off" : "on";

  // Construct the data string
  String sensorData = String(soilMoisture) + "," + String(temperature) + "," + String(humidity) + "," + String(waterLevel) + "," + waterSwitch;

  // Send the data to NodeMCU
  Serial.println(sensorData);
}

// ------------------------------------------------------------------------------------------------

void setupSIM() {
  sim.println("AT+CNUM"); // Send command to get SIM card number

  delay(1000);
  while (sim.available()) {
    String response = sim.readStringUntil('\n');
    response.trim();
    Serial.println(response); // Print SIM status response
  }

  sim.println("AT+CMGF=1"); // Set SMS mode to text
  delay(1000);

  sim.println("AT+CNMI=2,2,0,0,0"); // Set module to forward received messages to Arduino
  delay(1000);
}

// Function to receive SMS messages
void receiveMessage() {
  while (sim.available() > 0) {
    String receivedMessage = sim.readStringUntil("\n\n");
    receivedMessage.trim();
    if (receivedMessage.indexOf("+CMT:") != -1) {
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

      String sender = "Sender: " + senderNumber;
      String sms = "SMS Content: " + smsContent;

      Serial.println(sender);
      Serial.println(sms);

      processCommand(senderNumber, smsContent);
    }
  }
}

void processCommand(String senderNumber, String smsContent) {
  smsContent.toLowerCase(); 

  if (smsContent.indexOf("status") != -1 || smsContent.indexOf("state") != -1 ) {
    String status = sendMessage();
    sendSMS(senderNumber, status);
  } else if (smsContent.indexOf("on") != -1) {
    if(getSoilMoistureStatus() >= 40) {
      sendSMS(senderNumber, "Soil is wet. No watering required at the moment.");
    } else {
      digitalWrite(relayPin, LOW);
      sendSMS(senderNumber, "Sprinkler is now turned on.");
    }
  } else if (smsContent.indexOf("off") != -1) {
    digitalWrite(relayPin, HIGH);
    sendSMS(senderNumber, "Sprinkler has been turned off. Thank you!");
  }
}

// ------------------------------------------------------------------------------------------------

String sendMessage() {  
  int humidity = dht.readHumidity();
  int temperature = dht.readTemperature();

  // Compose SMS message with sensor readings
  String moisture = getSoilMoistureStatus() >= 40 ? "wet" : "dry";

  String message = "Soil Moisture: " + String(getSoilMoistureStatus()) + "%\n";
  message += "Temperature: " + String(temperature) + "C\n";
  message += "Humidity: " + String(humidity) + "%\n";
  message += "Water Level: " + String(measureWaterLevel()) + "%\n";
  message += checkStatus() ? "\nTime to water your plant.\n" : "\nNo watering required at the moment.\n";

  message += digitalRead(relayPin) == HIGH ? "\nWater Sprinker is OFF" : "\nWater Sprinker is ON";

  Serial.println(message);

  return message;
}

void sendSMS(String number, String message) {
  sim.println("AT+CMGF=1");
  delay(200);
  sim.println("AT+CMGS=\"" + number + "\"\r");
  delay(200);
  sim.print(message);
  sim.println((char)26);
  delay(200);
}

// ---------------------------------------------------------------------------------------------------

bool checkStatus() {
  // Check humidity, temperature, soil moisture, and water level to determine if watering is needed
  if (checkHumidityAndTemp() || getSoilMoistureStatus() < 40 && measureWaterLevel() > 20) {
    return true;
  } else {
    return false;
  }
}

bool checkHumidityAndTemp() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return false;
  }

  // Check if it's best to water the plants based on temperature and humidity
  if (temperature >= minTemperatureForWatering && humidity <= maxHumidityForWatering) {
    return true;
  } else {
    return false;
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
