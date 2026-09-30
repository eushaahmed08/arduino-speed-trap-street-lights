#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>

// I2C address of the LCD (typical address is 0x27, but may be 0x3F or others)
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // Set the LCD address to 0x27 for a 16 chars and 2 line display

// MH Flying Fish IR sensor pins
const int IR1_PIN = 8;  // First sensor
const int IR2_PIN = 9;  // Second sensor
const int BUZZER_PIN = 13; // Buzzer pin

// Constants
const float DISTANCE = 0.2;  // Distance between sensors in meters (adjust as needed)
const unsigned long TIMEOUT = 5000;  // 5 seconds timeout for measurement
const float SPEED_THRESHOLD = 5.0; // Speed threshold for buzzer activation

// Variables
volatile unsigned long t1 = 0;
volatile unsigned long t2 = 0;
boolean firstSensorTriggered = false;
boolean measurementComplete = false;
float velocity = 0;

void printWithDelay(const String &text, int col,int row) {
  lcd.setCursor(col, row);
  for (int i = 0; i < text.length(); i++) {
    lcd.print(text[i]);
    delay(1); // Add delay for proper display
  }
}


void setup() {
  // Initialize I2C LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize IR sensors as inputs
  pinMode(IR1_PIN, INPUT);
  pinMode(IR2_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("Vehicle Speed Detection System");
  Serial.println("-------------------------------");
  Serial.println("System initialized and ready");
  Serial.println("Sensor distance: " + String(DISTANCE) + " meters");
  Serial.println("Waiting for vehicle...");
  
  // Display welcome message
  lcd.clear();
  printWithDelay("Vehicle Speed",0, 0);
  printWithDelay("Measurement",0, 1);
  delay(1000);
  
  lcd.clear();
  printWithDelay("Ready...",0, 0);
}

void loop() {
  // First sensor detection
  if (digitalRead(IR1_PIN) == LOW && !firstSensorTriggered) {
    t1 = millis();
    firstSensorTriggered = true;
  }
  
  // Second sensor detection
  if (digitalRead(IR2_PIN) == LOW && firstSensorTriggered && !measurementComplete) {
    t2 = millis();
    measurementComplete = true;

    lcd.clear();
    printWithDelay("Object detected",0, 0);
    delay(1000);
    
    Serial.println("First sensor triggered at " + String(t1) + " ms");
    Serial.println("Second sensor triggered at " + String(t2) + " ms");
    
    // Calculate velocity
    if (t2 > t1) {
      float timeSeconds = (t2 - t1) / 1000.0;
      
      if (timeSeconds > 0) {
        velocity = (DISTANCE / timeSeconds) * 3.6; // Convert to km/h
        
        // Display result
        // Activate buzzer if speed exceeds threshold
        if (velocity > SPEED_THRESHOLD) {
          digitalWrite(BUZZER_PIN, HIGH);
          lcd.clear();
          printWithDelay("Overspeed detected!!",0, 0);
          //lcd.setCursor(0, 1);
          printWithDelay("Speed: ",0, 1);

          String velocityStr = String(velocity, 2);
          printWithDelay(velocityStr,7, 1);



          printWithDelay(" km/h:",11,1 ); // Ensure "km/h" appears correctly
          delay(3000);
          digitalWrite(BUZZER_PIN, LOW);
          Serial.println("WARNING: Speed exceeded threshold! Buzzer activated.");
        } else {
          lcd.clear();
          printWithDelay("Normal Speed.",0, 0);
          printWithDelay("Speed: ",0, 1);
          String velocityStr = String(velocity, 2);
          printWithDelay(velocityStr,7, 1);
           // Print velocity with 1 decimal place
          printWithDelay(" km/h:",11,1 );
        }
        
        // Print to serial monitor
        Serial.println("-------- SPEED CALCULATION --------");
        Serial.println("Time difference: " + String(timeSeconds, 3) + " seconds");
        Serial.println("Distance: " + String(DISTANCE) + " meters");
        Serial.println("Speed: " + String(velocity, 1) + " km/h");
        Serial.println("-----------------------------------");
      }
    }
    
    // Reset after a delay
    delay(3000);
    resetMeasurement();
  }
  
  // Timeout if first sensor triggered but second one doesn't trigger
  if (firstSensorTriggered && !measurementComplete && (millis() - t1 > TIMEOUT)) {
    lcd.clear();
    printWithDelay("Timeout!",0, 0);
    printWithDelay("Try again",0, 1);
    
    Serial.println("TIMEOUT: Second sensor not triggered within " + String(TIMEOUT/1000) + " seconds");
    Serial.println("Resetting system...");
    
    delay(2000);
    resetMeasurement();
  }
}

void resetMeasurement() {
  firstSensorTriggered = false;
  measurementComplete = false;
  digitalWrite(BUZZER_PIN, LOW);
  
  lcd.clear();
  printWithDelay("Ready...",0, 0);
  
  Serial.println("System reset. Ready for new measurement.");
}
