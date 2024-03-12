#define BLYNK_TEMPLATE_ID "TMPL6xpKplmNo"
#define BLYNK_TEMPLATE_NAME "Agribio"
#define BLYNK_AUTH_TOKEN "331OHyGcb-NlGazNzk5--QFBut0BuCd-"

#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial
#define NODE_RX D1
#define NODE_TX D2
#define BUTTON_PIN V4

SoftwareSerial nodeSerial(NODE_RX, NODE_TX);

const char* ssid = "PLDTHOMEFIBR5vGz3";
const char* password = "PLDTWIFIzr3VV";

bool isWaterOn = false;

void setup() {
  Serial.begin(9600);
  delay(100);
  nodeSerial.begin(9600);

  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  Serial.println("");
  Serial.print("WiFi connected to ");
  Serial.println(ssid);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  Blynk.run();

  receiveData();

  delay(1000);
}

// Blynk function to handle changes to the button state
BLYNK_WRITE(BUTTON_PIN) {
  int buttonState = param.asInt(); // Read the button state from Blynk

  if (buttonState == HIGH) {
    // Toggle the water state
    isWaterOn = !isWaterOn;

    // Send command to turn water on or off
    if (isWaterOn) {
      nodeSerial.println("on"); // Command to turn water on
      Serial.println("Water pump turned on");
    } else {
      nodeSerial.println("off"); // Command to turn water off
      Serial.println("Water pump turned off");
    }
  }
}

void receiveData() {
  if (nodeSerial.available()) {
    String data = nodeSerial.readStringUntil('\n'); // Read data from Arduino until newline character

    // Split the data string into individual sensor readings
    int index = data.indexOf(',');
    String soilMoisture = data.substring(0, index);
    int moisture = mapSoilMoistureToPercentage(soilMoisture.toInt());
    data = data.substring(index + 1);
    
    index = data.indexOf(',');
    String temperature = data.substring(0, index);
    data = data.substring(index + 1);

    index = data.indexOf(',');
    String humidity = data.substring(0, index);
    data = data.substring(index + 1);

    index = data.indexOf(',');
    String waterLevel = data.substring(0, index);
    data = data.substring(index + 1);

    // Extract the switch state
    String switchState = data;

    // Send sensor readings to Blynk virtual pins
    Blynk.virtualWrite(V0, moisture);
    Blynk.virtualWrite(V1, temperature.toFloat());
    Blynk.virtualWrite(V2, humidity.toInt());
    Blynk.virtualWrite(V3, waterLevel.toInt());

    // Update the switch state on Blynk
    if (switchState == "on") {
      Blynk.virtualWrite(V4, HIGH);
    } else {
      Blynk.virtualWrite(V4, LOW);
    }
  }
}


int mapSoilMoistureToPercentage(int sensorData) {
  // Map the analog value (0-1023) to a percentage range (0-100)
  int mappedValue = map(sensorData, 0, 1023, 100, 0); // Inverted mapping to align with the desired logic
  return mappedValue;
}

