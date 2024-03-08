#include <DHT.h>
#include <SoftwareSerial.h>

#define DHTPIN 2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "PLDTHOMEFIBR5vGz3";
const char* password = "PLDTWIFIzr3VV";

SoftwareSerial sim(12, 11);
const String number = "639451722389";

const float minTemperatureForWatering = 25.0;
const float maxHumidityForWatering = 70.0;
const int sensor_pin = A0, relayPin = 8, trig = 9, echo = 10;
float distanceInCm;
int relayOnTimer, lastRelayChangeTime;
unsigned char check_connection = 0;

void setup() {
  Serial.begin(9600);
  delay(100);
  sim.begin(9600);
  
  sim.println("AT+CMGF=1");
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

  delay(2000);
  
  unsigned long currentMillis = millis();
  
  if (getSoilMoistureStatus() == "wet" && digitalRead(relayPin) == LOW) {
    digitalWrite(relayPin, HIGH);
    Serial.println("Water turned off automatically");
  }

  if(measureWaterLevel() < 20) {
    digitalWrite(relayPin, HIGH);
    Serial.println("Warning! Water level is below 20%. Please refill the water tank.");
  }

  if (checkHumidityAndTemp() && getSoilMoistureStatus() == "dry" && measureWaterLevel() > 20) {
    Serial.println("DILIGAN ANG HALAMAN!");
  }

  printStatus();
  
  delay(1000);
}

void printStatus() {
  Serial.print("Soil Moisture: ");
  Serial.println(getSoilMoistureStatus());
  Serial.print("Temperature: ");
  Serial.print(dht.readTemperature());
  Serial.println("°C");
  Serial.print("Humidity: ");
  Serial.print(dht.readHumidity());
  Serial.println("%");
  Serial.print("Water Level: ");
  Serial.print(measureWaterLevel());
  Serial.println("%\n");
}

// ------------------------------------------------------------------------------------------------

void checkSIMStatus() {
  sim.println("AT+CNUM");
  delay(1000);
  while (sim.available()) {
    String response = sim.readStringUntil('\n');
    response.trim();
    Serial.println(response);
  }
}

void receiveMessage() {
  sim.println("AT+CMGF=1");
  delay(1000);

  sim.println("AT+CNMI=2,2,0,0,0");
  delay(1000);

  while (sim.available() > 0) {
    String receivedMessage = sim.readStringUntil("\n\n");
    receivedMessage.trim();
    if (receivedMessage.indexOf("+CMT:") != -1) {
      Serial.println("receivedMessage: " + receivedMessage);
      sim.readStringUntil('+'); 
      String senderNumber = sim.readStringUntil('"');
      senderNumber.trim();
      senderNumber.replace("+", "");

      sim.readStringUntil('"'); 
      sim.readStringUntil('\n'); 
      
      String smsContent = sim.readStringUntil('\n');
      smsContent.trim();

      Serial.println("From: " + senderNumber);
      Serial.println("SMS Content: " + smsContent);

      if (smsContent.indexOf("status") != -1) {
        sendMessage(senderNumber);
      } else if (smsContent.indexOf("on") != -1) {
        if (smsContent.startsWith("on ")) {
          int spaceIndex = smsContent.indexOf(" ");
          float timer = smsContent.substring(spaceIndex + 1).toFloat();
          int relayOnTimer = timer * 60 * 60 * 1000;
          if (timer < 1) {
              relayOnTimer = timer * 60 * 1000; 
          }
          setRelayTimer(String(timer));
          digitalWrite(relayPin, LOW);
          if (timer >= 1) {
              sendSMSConfirmation(senderNumber, "Water will turn on in " + String(timer) + " hour(s)");
          } else {
              sendSMSConfirmation(senderNumber, "Water will turn on in " + String(timer * 60) + " minute(s)");
          }
        } else {
          digitalWrite(relayPin, LOW);
          sendSMSConfirmation(senderNumber, "Water is turned on.");
        }
      } else if (smsContent.indexOf("off") != -1) {
        digitalWrite(relayPin, HIGH);
        sendSMSConfirmation(senderNumber, "Water turned off");
      }
    }
  }
}

void sendMessage(const String& number) {
  String message = "Soil Moisture: " + getSoilMoistureStatus() + "\n";
  message += "Temperature: " + String(dht.readTemperature()) + "°C\n";
  message += "Humidity: " + String(dht.readHumidity()) + "%\n";
  message += "Water Level: " + String(measureWaterLevel()) + "%\n";
  message += digitalRead(relayPin) == HIGH ? "Water Sprinker is OFF" : "Water Sprinker is ON";
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

// ------------------------------------------------------------------------------------------------
void setRelayTimer(String command) {
  if (command.indexOf(" ") != -1) {
    int spaceIndex = command.indexOf(" ");
    String timerString = command.substring(spaceIndex + 1);
    relayOnTimer = timerString.toInt() * 60000;
  }
}

// ------------------------------------------------------------------------------------------------
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

String getSoilMoistureStatus() {
  int sensor_data = analogRead(sensor_pin);
  if (sensor_data >= 1000) {
    return "dry";
  } else if (sensor_data >= 701 && sensor_data <= 999) {
    return "medium";
  } else if (sensor_data <= 700) {
    return "wet";
  }
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
