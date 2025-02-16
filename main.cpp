#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x20, 16, 2);

const int TMP36_PIN = A0;  // Temperature sensor
const int flexpin = A1;    // Flex sensor
const int bulbPin = 13;    // Light bulb or LED pin
const int humPin = A3;     // Humidity sensor pin
#define smoke A2           // Smoke sensor pin
#define buzz1 3            // Buzzer 1 pin for smoke
#define buzz2 8            // Buzzer 2 pin for door alert
#define fan 6              // Fan pin
Servo servo1;

unsigned long doorOpenTime = 0;
const unsigned long doorOpenThreshold = 20000; // 20 seconds
bool isDoorOpen = false;

void setup() {
  lcd.init();
  lcd.clear();
  lcd.backlight();
  Serial.begin(9600);
  servo1.attach(9);
  servo1.write(0);
  pinMode(smoke, INPUT);
  pinMode(humPin, INPUT);
  pinMode(bulbPin, OUTPUT);
  pinMode(buzz1, OUTPUT);
  pinMode(buzz2, OUTPUT);
  pinMode(fan, OUTPUT);
}

void loop() {
  // Temperature Reading
  int sensorValue = analogRead(TMP36_PIN);
  float voltage = sensorValue * (5.0 / 1023.0);
  float temperatureC = (voltage - 0.5) * 100.0;

  // Display Temperature on LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperatureC, 1);
  lcd.print((char)223);
  lcd.print("C");

  // Humidity Reading
  int humidityValue = analogRead(humPin);
  int humidityPercent = map(humidityValue, 0, 1023, 0, 100); // Map analog value to 0-100% range

  // Display Humidity on LCD
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidityPercent);
  lcd.print("%");

  // Condition: Humidity Threshold Alert
  if (humidityPercent < 30) {
    Serial.println("Humidity too low! Consider adding moisture.");
    lcd.setCursor(0, 1);
    lcd.print("Low Humidity!   ");
    analogWrite(fan, 255); // Turn on fan at full speed
  } else if (humidityPercent > 70) {
    Serial.println("Humidity too high! Consider dehumidifying.");
    lcd.setCursor(0, 1);
    lcd.print("High Humidity!  ");
    analogWrite(fan, 50); // Turn on fan at low speed
  } else {
    analogWrite(fan, 0); // Turn off fan
  }

  // Flex Sensor and Door Logic
  int flexposition = analogRead(flexpin);
  int flex = map(flexposition, 768, 964, 0, 180);
  flex = constrain(flex, 0, 180);

  if (flex > 0) {
    servo1.write(flex);
    digitalWrite(bulbPin, HIGH);
  } else {
    servo1.write(0);
    digitalWrite(bulbPin, LOW);
  }

  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidityPercent);
  Serial.println(" %");

  Serial.print("Flex Position: ");
  Serial.print(flexposition);
  Serial.print(" -> Servo Angle: ");
  Serial.println(flex);

  // Smoke Detection
  int smokevalue = analogRead(smoke);
  Serial.print("Smoke: ");
  Serial.println(smokevalue);

  if (smokevalue > 600) {
    lcd.setCursor(0, 1);
    lcd.print("Gas Detected!");
    tone(buzz1, 500); // Continuous tone for smoke alert
  } else {
    noTone(buzz1); // Turn off buzzer 1
  }

  // Door Open Logic
  if (flex > 3) {
    lcd.setCursor(0, 1);
    lcd.print("Door Opened!    ");
    if (!isDoorOpen) {
      doorOpenTime = millis();
      isDoorOpen = true;
    } else if (millis() - doorOpenTime >= doorOpenThreshold) {
      lcd.setCursor(0, 1);
      lcd.print("Close Door!     ");
      tone(buzz2, 1000, 500); // Pulsing tone for door alert
    }
  } else {
    isDoorOpen = false;
    noTone(buzz2); // Turn off buzzer 2
  }

  delay(500);
}
