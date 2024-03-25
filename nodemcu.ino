#define BLYNK_TEMPLATE_ID "TMPL6xpKplmNo"
#define BLYNK_TEMPLATE_NAME "Agribio"
#define BLYNK_AUTH_TOKEN "331OHyGcb-NlGazNzk5--QFBut0BuCd-"

#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial
#define BUTTON_PIN V4

const char* ssid = "";
const char* password = "";

void setup() {
  Serial.begin(9600);
  delay(100);

  connectWifi();
}

void loop() {
  Blynk.run();
  delay(1000);

  receiveData();
}


void connectWifi() {
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

// Blynk function to handle changes to the button state
BLYNK_WRITE(BUTTON_PIN) {
  int buttonState = param.asInt(); // Read the button state from Blynk

  if (buttonState == 1) {
    Serial.println("on");
  } else {
    Serial.println("off");
  }
}

void receiveData() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n'); // Read data from Arduino until newline character

    // Split the data string into individual sensor readings
    int index = data.indexOf(',');
    String soilMoisture = data.substring(0, index);
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

    String switchState = data;

    // Send sensor readings to Blynk virtual pins
    Blynk.virtualWrite(V0, soilMoisture.toInt());
    Blynk.virtualWrite(V1, temperature.toFloat());
    Blynk.virtualWrite(V2, humidity.toInt());
    Blynk.virtualWrite(V3, waterLevel.toInt());

    switchState.trim();

    // Update the switch state on Blynk
    if (switchState.equals("on")) {
      Blynk.virtualWrite(V4, 1);
    } else {
      Blynk.virtualWrite(V4, 0);
    }
  }
}