#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize the I2C LCD (Address: 0x27, 16 columns x 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin definitions
const int trigPin = 2;   // Ultrasonic trigger
const int echoPin = 3;   // Ultrasonic echo
const int ldrPin = A0;   // Photoresistor (LDR)
const int flowPin = A1;  // Potentiometer (Flow rate)
const int tempPin = A2;  // Temperature sensor (LM35)
const int buzzerPin = 4; // Buzzer for alarm
const int motorPin = 5;  // Motor control

// Variables
float waterLevel;        // Water level in cm
float flowRate = 0;      // Flow rate controlled by potentiometer
int turbidity = 0;       // Water clarity (0-100%)
float temperature = 0;   // Temperature in °C
bool motorOn = false;    // Track if the motor is active

void setup() {
  lcd.init();            // Initialize the I2C LCD
  lcd.backlight();       // Turn on LCD backlight
  lcd.print("Water Monitor");
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT); // Set buzzer pin as output
  pinMode(motorPin, OUTPUT);  // Set motor pin as output

  // Get initial water level using ultrasonic sensor
  waterLevel = getWaterLevel();

  delay(500);           // Delay for initial setup
}

void loop() {
  // Measure flow rate using the potentiometer
  flowRate = map(analogRead(flowPin), 0, 1023, 0, 10); // Flow rate (0-10 L/min)

  // Simulate continuous water level decrease based on flow rate
  if (flowRate > 0) {
    waterLevel -= (flowRate / 120.0); // Reduce water level every second
    waterLevel = max(waterLevel, 0); // Prevent negative water level
  }

  // Measure temperature using the LM35 sensor
  temperature = analogRead(tempPin) * (5.0 / 1023.0) * 100; // Convert to °C

  // Measure turbidity using the photoresistor
  turbidity = map(analogRead(ldrPin), 0, 1023, 0, 100); // Turbidity (0-100%)

  // Control buzzer and motor based on water level
  if (waterLevel < 10) {
    tone(buzzerPin, 1000);     // Turn on buzzer for low water level
    digitalWrite(motorPin, HIGH); // Turn on the motor
    motorOn = true;            // Mark motor as active
  }

  if (waterLevel >= 10 && motorOn) {
    noTone(buzzerPin);         // Turn off buzzer when level > 10
  }

  if (motorOn) {
    waterLevel += 0.5; // Simulate water level rising while the motor is on
    waterLevel = min(waterLevel, 100); // Cap water level at 100 cm

    // Turn off motor and activate buzzer when the tank is full
    if (waterLevel >= 100) {
      tone(buzzerPin, 2000);    // Turn on buzzer for tank full notification
      digitalWrite(motorPin, LOW); // Turn off the motor
      motorOn = false;          // Mark motor as inactive
    }
  }

  // Display water level, flow rate, turbidity, and temperature on the LCD
  displayLCD(waterLevel, flowRate, turbidity, temperature);

  delay(500); // Faster updates for water flow and level
}

// Function to measure water level using the ultrasonic sensor
float getWaterLevel() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2; // Convert to cm
  return constrain(distance, 0, 100);   // Limit to 0-100 cm
}

// Function to display data on the LCD
void displayLCD(float level, float flow, int turbidity, float temp) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Level: ");
  lcd.print(level);
  lcd.print(" cm");

  lcd.setCursor(0, 1);
  lcd.print("Flow: ");
  lcd.print(flow);
  lcd.print(" L/min");

  lcd.setCursor(9, 0);
  lcd.print("Turb: ");
  lcd.print(turbidity);
  lcd.print("%");

  lcd.setCursor(9, 1);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print((char)223); // Degree symbol
  lcd.print("C");
}
