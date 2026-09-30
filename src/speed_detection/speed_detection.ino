/*
 * Speed Detection (standalone subsystem)
 * Automatic Street & Traffic Light System with Over-Speed Detection
 *
 * Cleaned-up version of historical/03_speed_lcd_col_row.ino.
 * Behavior is intended to match the original: same pins, constants, LCD text,
 * cursor positions, delays and serial output. Changes are limited to naming,
 * comments, removing duplicated display code, and removing `volatile` (the
 * timestamps are never touched by an interrupt, so it had no effect).
 *
 * Hardware (as referenced in the source):
 *   - Arduino (Uno, per project BOM)
 *   - 2x IR obstacle sensor modules ("MH Flying Fish" per original comment),
 *     output pulled LOW when an object is detected
 *   - 16x2 LCD with I2C backpack (address 0x3F in later versions)
 *   - Buzzer
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// 16x2 I2C LCD. The original sketches used 0x27 first and 0x3F later;
// the correct value depends on the specific backpack module.
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// ---- Pins ----
const int IR1_PIN    = 8;   // Entry sensor
const int IR2_PIN    = 9;   // Exit sensor
const int BUZZER_PIN = 13;

// ---- Constants ----
const float         SENSOR_DISTANCE_M   = 0.2;   // Distance between the two IR sensors (m)
const unsigned long TIMEOUT_MS          = 5000;  // Give up if IR2 isn't reached in 5 s
const float         SPEED_THRESHOLD_KMH = 5.0;   // Above this -> overspeed alert

// ---- Measurement state ----
unsigned long t1 = 0;                // millis() when IR1 triggered
unsigned long t2 = 0;                // millis() when IR2 triggered
bool firstSensorTriggered = false;   // IR1 seen, waiting for IR2
bool measurementComplete  = false;   // IR2 seen, result being shown
float velocity = 0;                  // km/h

// Print a string one character at a time starting at (col, row).
// The 1 ms per-character delay was in the original code ("for proper display").
void printWithDelay(const String &text, int col, int row) {
  lcd.setCursor(col, row);
  for (unsigned int i = 0; i < text.length(); i++) {
    lcd.print(text[i]);
    delay(1);
  }
}

// Show "<headline>" on row 0 and "Speed: <v> km/h:" on row 1.
// Note (preserved from original): the value is printed with 2 decimals at
// column 7 and the unit at column 11, so a 5-character value such as "12.34"
// has its last digit overwritten, and the trailing ':' falls off the 16-column
// display. The overspeed headline (20 chars) is also cut to 16 characters.
void showSpeedOnLcd(const String &headline) {
  lcd.clear();
  printWithDelay(headline, 0, 0);
  printWithDelay("Speed: ", 0, 1);
  printWithDelay(String(velocity, 2), 7, 1);
  printWithDelay(" km/h:", 11, 1);
}

void resetMeasurement() {
  firstSensorTriggered = false;
  measurementComplete  = false;
  digitalWrite(BUZZER_PIN, LOW);

  lcd.clear();
  printWithDelay("Ready...", 0, 0);

  Serial.println("System reset. Ready for new measurement.");
}

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(IR1_PIN, INPUT);
  pinMode(IR2_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.begin(9600);
  Serial.println("Vehicle Speed Detection System");
  Serial.println("-------------------------------");
  Serial.println("System initialized and ready");
  Serial.println("Sensor distance: " + String(SENSOR_DISTANCE_M) + " meters");
  Serial.println("Waiting for vehicle...");

  lcd.clear();
  printWithDelay("Vehicle Speed", 0, 0);
  printWithDelay("Measurement", 0, 1);
  delay(1000);

  lcd.clear();
  printWithDelay("Ready...", 0, 0);
}

void loop() {
  // State 1 -> 2: vehicle reaches the first sensor (active LOW).
  if (digitalRead(IR1_PIN) == LOW && !firstSensorTriggered) {
    t1 = millis();
    firstSensorTriggered = true;
  }

  // State 2 -> 3: vehicle reaches the second sensor.
  if (digitalRead(IR2_PIN) == LOW && firstSensorTriggered && !measurementComplete) {
    t2 = millis();
    measurementComplete = true;

    lcd.clear();
    printWithDelay("Object detected", 0, 0);
    delay(1000);

    Serial.println("First sensor triggered at " + String(t1) + " ms");
    Serial.println("Second sensor triggered at " + String(t2) + " ms");

    if (t2 > t1) {
      float timeSeconds = (t2 - t1) / 1000.0;

      if (timeSeconds > 0) {
        // v [km/h] = (distance [m] / time [s]) * 3.6
        velocity = (SENSOR_DISTANCE_M / timeSeconds) * 3.6;

        if (velocity > SPEED_THRESHOLD_KMH) {
          // Overspeed: buzzer on for 3 s while the result is shown.
          digitalWrite(BUZZER_PIN, HIGH);
          showSpeedOnLcd("Overspeed detected!!");
          delay(3000);
          digitalWrite(BUZZER_PIN, LOW);
          Serial.println("WARNING: Speed exceeded threshold! Buzzer activated.");
        } else {
          showSpeedOnLcd("Normal Speed.");
        }

        Serial.println("-------- SPEED CALCULATION --------");
        Serial.println("Time difference: " + String(timeSeconds, 3) + " seconds");
        Serial.println("Distance: " + String(SENSOR_DISTANCE_M) + " meters");
        Serial.println("Speed: " + String(velocity, 1) + " km/h");
        Serial.println("-----------------------------------");
      }
    }

    // Hold the result on screen, then return to idle.
    delay(3000);
    resetMeasurement();
  }

  // Timeout: IR1 fired but IR2 never did.
  if (firstSensorTriggered && !measurementComplete && (millis() - t1 > TIMEOUT_MS)) {
    lcd.clear();
    printWithDelay("Timeout!", 0, 0);
    printWithDelay("Try again", 0, 1);

    Serial.println("TIMEOUT: Second sensor not triggered within " + String(TIMEOUT_MS / 1000) + " seconds");
    Serial.println("Resetting system...");

    delay(2000);
    resetMeasurement();
  }
}
