/*
 * Integrated System: street lighting + speed detection on one Arduino
 * Automatic Street & Traffic Light System with Over-Speed Detection
 *
 * Cleaned-up version of historical/05_integrated_TRUNCATED.ino.
 *
 * RECONSTRUCTION NOTE
 * The salvaged copy stops mid-statement inside loop():
 *     Serial.println("Speed: " + String(
 * Up to that point, the speed-detection code is the same, line for line
 * (ignoring whitespace), as historical/03_speed_lcd_col_row.ino. The rest of
 * this file (marked "RECONSTRUCTED" below) is therefore filled in from that
 * sketch. This is a well-supported inference, not a recovered original.
 *
 * Hardware (as referenced in the source):
 *   - Arduino (Uno, per project BOM)
 *   - 2x IR obstacle sensor modules (active LOW), used both for speed timing
 *     and for vehicle presence for the street lights
 *   - LDR on A0
 *   - 2x LEDs on PWM pins 10, 11
 *   - MOSFET gate on pin 6 (switched on/off only; its load is not stated in
 *     the code)
 *   - 16x2 I2C LCD, buzzer on pin 13
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---- Lighting pins ----
#define LDR_PIN    A0
#define LED1_PIN   10
#define LED2_PIN   11
#define MOSFET_PIN 6

// 16x2 I2C LCD (address depends on the backpack module; 0x3F in this version).
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// ---- Speed-detection pins ----
const int IR1_PIN    = 8;
const int IR2_PIN    = 9;
const int BUZZER_PIN = 13;

// ---- Constants ----
const int           LDR_DARK_THRESHOLD  = 200;   // ldrValue < 200 treated as dark
const int           DIM_LEVEL           = 30;    // PWM duty (~12%) when dark and road empty
const float         SENSOR_DISTANCE_M   = 0.2;
const unsigned long TIMEOUT_MS          = 5000;
const float         SPEED_THRESHOLD_KMH = 5.0;

// ---- Measurement state ----
unsigned long t1 = 0;
unsigned long t2 = 0;
bool firstSensorTriggered = false;
bool measurementComplete  = false;
float velocity = 0;

void printWithDelay(const String &text, int col, int row) {
  lcd.setCursor(col, row);
  for (unsigned int i = 0; i < text.length(); i++) {
    lcd.print(text[i]);
    delay(1);
  }
}

// Same LCD layout as the standalone speed sketch (including its known
// column-overlap issue; see src/speed_detection).
void showSpeedOnLcd(const String &headline) {
  lcd.clear();
  printWithDelay(headline, 0, 0);
  printWithDelay("Speed: ", 0, 1);
  printWithDelay(String(velocity, 2), 7, 1);
  printWithDelay(" km/h:", 11, 1);
}

// ---- Street lighting (runs once at the start of every loop) ----
// Dark + no vehicle  -> LEDs dimmed via PWM, MOSFET on
// Dark + vehicle     -> LEDs full on,        MOSFET on
// Light              -> LEDs off,            MOSFET off
void updateStreetLights() {
  int ldrValue = analogRead(LDR_PIN);
  Serial.println(ldrValue);

  if (ldrValue < LDR_DARK_THRESHOLD) {
    analogWrite(LED1_PIN, DIM_LEVEL);
    analogWrite(LED2_PIN, DIM_LEVEL);
    digitalWrite(MOSFET_PIN, HIGH);

    if (digitalRead(IR1_PIN) == LOW || digitalRead(IR2_PIN) == LOW) {
      digitalWrite(LED1_PIN, HIGH);   // full brightness
      digitalWrite(LED2_PIN, HIGH);
      digitalWrite(MOSFET_PIN, HIGH);
    }
  } else {
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(MOSFET_PIN, LOW);
  }
}

// RECONSTRUCTED (from historical/03_speed_lcd_col_row.ino)
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
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(MOSFET_PIN, OUTPUT);
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
  updateStreetLights();

  // Vehicle reaches the first IR sensor.
  if (digitalRead(IR1_PIN) == LOW && !firstSensorTriggered) {
    t1 = millis();
    firstSensorTriggered = true;
  }

  // Vehicle reaches the second IR sensor.
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
        velocity = (SENSOR_DISTANCE_M / timeSeconds) * 3.6;   // km/h

        if (velocity > SPEED_THRESHOLD_KMH) {
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
        // ---- Salvaged copy ends partway through the next line ----
        // RECONSTRUCTED from here to the end of loop()
        Serial.println("Speed: " + String(velocity, 1) + " km/h");
        Serial.println("-----------------------------------");
      }
    }

    delay(3000);
    resetMeasurement();
  }

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
