#include <dht.h>
#include <SoftwareSerial.h>

dht DHT;
#define DHT11_PIN 7

SoftwareSerial sim(10, 11); 
String number = "+639385166917";

int sensor_pin = A0;
int relayPin = 8;
int trig = 9;
int echo = 10;
int timeInMicro;
float distanceInCm;
float percentage;
unsigned long lastRelayChangeTime = 0;
unsigned long relayOnDuration = 0;

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  Serial.begin(9600);
  sim.begin(9600);
  pinMode(sensor_pin, INPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
}

void loop() {
  Serial.println("\n---------------------------------------------------------------------");

  receiveMessage(); // Check for incoming SMS commands
  checkSerialCommands(); // Check for serial commands
  
  readSoilMoisture();
  readTemperatureAndHumidity();
  measureWaterLevel();
  
  // Check if it's time to turn off the relay
  if (millis() - lastRelayChangeTime >= relayOnDuration && digitalRead(relayPin) == LOW) {
    digitalWrite(relayPin, HIGH);
    Serial.println("Water turned off automatically");
  }
}

void receiveMessage() {
  sim.println("AT+CMGF=1");
  delay(200);
  sim.println("AT+CNMI=1, 2, 0, 0, 0");
  delay(200);
  
  if (sim.available() > 0) {
    String receivedMessage = sim.readStringUntil('\n');
    receivedMessage.trim();
    
    if (receivedMessage.equals("status")) {
      sendMessage();
    } else if (receivedMessage.startsWith("water on")) {
      setRelayDuration(receivedMessage);
      digitalWrite(relayPin, LOW);
      lastRelayChangeTime = millis(); // Reset the timer
      sendMessageConfirmation("Water turned on for " + String(relayOnDuration / 60000) + " mins");
      Serial.println("Water turned on");
    } else if (receivedMessage.equals("water off")) {
      digitalWrite(relayPin, HIGH);
      sendMessageConfirmation("Water turned off");
      Serial.println("Water turned off");
    }
  }
}

void checkSerialCommands() {
  if (Serial.available() > 0) {
    String serialCommand = Serial.readStringUntil('\n');
    serialCommand.trim();
    
    if (serialCommand.equals("s")) {
      sendMessage();
    } else if (serialCommand.equals("on")) {
      setDefaultRelayDuration();
      digitalWrite(relayPin, LOW);
      lastRelayChangeTime = millis(); // Reset the timer
      sendMessageConfirmation("Water turned on for " + String(relayOnDuration / 60000) + " mins");
    } else if (serialCommand.equals("off")) {
      digitalWrite(relayPin, HIGH);
      sendMessageConfirmation("Water turned off");
      Serial.println("Water turned off");
    } else if (serialCommand.startsWith("on ")) {
      setRelayDuration(serialCommand);
      digitalWrite(relayPin, LOW);
      lastRelayChangeTime = millis(); // Reset the timer
      sendMessageConfirmation("Water turned on for " + String(relayOnDuration / 60000) + " mins");
    }
  }
}

void sendMessage() {
  sim.println("AT+CMGF=1");
  delay(200);
  sim.println("AT+CMGS=\"" + number + "\"\r");
  delay(200);
  
  String message = "Soil Moisture: " + getSoilMoistureStatus() + "\n";
  message += "Temperature: " + String(DHT.temperature) + "C\n";
  message += "Humidity: " + String(DHT.humidity) + "%\n";
  message += "Water Level: " + String(percentage) + "%\n";
  
  sim.print(message);
  sim.println((char)26);
  delay(200);

  Serial.println("SMS Received:\n" + message);
}


void sendMessageConfirmation(String confirmation) {
  Serial.println(confirmation); // Print confirmation message to Serial
  sim.println("AT+CMGF=1");
  delay(200);
  sim.println("AT+CMGS=\"" + number + "\"\r");
  delay(200);
  
  sim.print(confirmation);
  sim.println((char)26);
  delay(200);
}

String getSoilMoistureStatus() {
  int sensor_data = analogRead(sensor_pin);
  if (sensor_data >= 1000) {
    return "No moisture";
  } else if (sensor_data >= 701 && sensor_data <= 999) {
    return "Medium moisture";
  } else {
    return "Soil is wet";
  }
}

void readTemperatureAndHumidity() {
  int chk = DHT.read11(DHT11_PIN);
  Serial.print("Temperature = ");
  Serial.println(DHT.temperature);
  Serial.print("Humidity = ");
  Serial.println(DHT.humidity);
  delay(3000);
}

void readSoilMoisture() {
  int sensor_data = analogRead(sensor_pin);
  Serial.print("Soil Moisture: ");
  Serial.println(getSoilMoistureStatus());
}

void measureWaterLevel() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  timeInMicro = pulseIn(echo, HIGH);
  distanceInCm = timeInMicro / 29.0 / 2.0;
  percentage = 100.0 - (distanceInCm / 50.0 * 100.0);
  Serial.print("Water percentage: ");
  Serial.print(percentage);
  Serial.println("%");
  delay(1000);
}

void setRelayDuration(String command) {
  if (command.indexOf(" ") != -1) { // Check if duration is provided
    int spaceIndex = command.indexOf(" ");
    String durationString = command.substring(spaceIndex + 1); // Extract duration substring
    relayOnDuration = durationString.toInt() * 60000; // Convert duration to milliseconds
  } else {
    // Set default duration to 3 minutes (180000 milliseconds) if no duration is provided
    relayOnDuration = 180000;
  }
}

void setDefaultRelayDuration() {
  // Set default duration to 3 minutes (180000 milliseconds)
  relayOnDuration = 180000;
}
