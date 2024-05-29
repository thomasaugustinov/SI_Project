#include <Wire.h>
#include <SFE_BMP180.h>
#include <LiquidCrystal.h>
#include "DHT.h"

// Definitions for existing sensors
SFE_BMP180 bmp180;
float alt = 108.0; // Altitude of current location in meters
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// DHT11 setup
#define DHTPIN A2      // Pin which is connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// Rain sensor setup
const int rainPin = A1; // Use A1 (analog pin 1) as digital input

unsigned long previousMillis = 0;
const long interval = 5000; // Interval at which to print the values (5 seconds)
int displayMode = 0; // To toggle display modes

void setup() {
  pinMode(rainPin, INPUT); // Set rain sensor pin as input
  lcd.begin(16, 2); // Set up the LCD's number of columns and rows
  lcd.clear();
  Serial.begin(9600); // Start serial communication at 9600 baud
  Wire.begin(); 

  dht.begin(); // Initialize DHT11 sensor

  if (bmp180.begin()) {
    lcd.print("Sensor init success");
    Serial.println("Sensor init success");
  } else {
    lcd.print("Sensor init fail");
    Serial.println("Sensor init fail");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    char status;
    double T, P, seaLevelPressure;
    float h = dht.readHumidity();
    int rainDetected = digitalRead(rainPin);

    status = bmp180.startTemperature();
    if (status != 0) {
      delay(1000);
      status = bmp180.getTemperature(T);

      if (status != 0) {
        status = bmp180.startPressure(3);

        if (status != 0) {
          delay(status);
          status = bmp180.getPressure(P, T);

          if (status != 0) {
            seaLevelPressure = bmp180.sealevel(P, alt);
            lcd.clear();

            switch (displayMode) {
              case 0: // Display Temperature and Pressure
                lcd.setCursor(0, 0);
                lcd.print("SLP: ");
                lcd.print(seaLevelPressure);
                lcd.print(" hPa");
                lcd.setCursor(0, 1);
                lcd.print("Temp: ");
                lcd.print(T);
                lcd.print(" C");
                break;
              case 1: // Display Humidity and Rain Status
                lcd.setCursor(0, 0);
                lcd.print("Humidity: ");
                lcd.print(h);
                lcd.print(" %");
                lcd.setCursor(0, 1);
                lcd.print("Rain: ");
                lcd.print((rainDetected == LOW) ? "Detected" : "No Rain");
                break;
            }

            // Toggle display mode
            displayMode = (displayMode + 1) % 2;

            // Create data string to send
            String data = String(seaLevelPressure) + "," + String(T) + "," + String(h) + "," + String(rainDetected == LOW ? 1 : 0);

            // Send data to ESP8266
            Wire.beginTransmission(8); // Address of the ESP8266
            Wire.write(data.c_str());  // Send data as bytes
            Wire.endTransmission();

            // Print to Serial Monitor
            Serial.print("Pressure at sea level: ");
            Serial.print(seaLevelPressure);
            Serial.println(" hPa");
            Serial.print("Temperature: ");
            Serial.print(T);
            Serial.println(" C");
            Serial.print("Humidity: ");
            Serial.print(h);
            Serial.println(" %");
            Serial.print("Rain Status: ");
            Serial.println((rainDetected == LOW) ? "Rain Detected!" : "No Rain");
          }
        }
      }
    }
  }
}
