/*
 * Smart Street Lighting (standalone subsystem, ultrasonic variant)
 * Automatic Street & Traffic Light System with Over-Speed Detection
 *
 * Cleaned-up version of historical/04_smart_lighting_ultrasonic.ino.
 * Behavior is intended to match the original. Changes are limited to naming,
 * comments and grouping the repeated analogWrite calls into one helper.
 *
 * This sketch is a separate experiment: its pins overlap with the speed
 * detection sketch (pin 9), and ultrasonic sensors do not appear in the
 * project's Tinkercad schematic or component list. The integrated system
 * uses the IR sensors for vehicle detection instead.
 *
 * Hardware (as referenced in the source):
 *   - Arduino
 *   - 3x ultrasonic distance sensors with TRIG/ECHO pins
 *     (module model not named in the source)
 *   - LDR on A0
 *   - 3x LEDs on PWM pins 9, 10, 11
 */

// ---- Pins ----
#define TRIG1 2
#define ECHO1 3
#define TRIG2 4
#define ECHO2 5
#define TRIG3 6
#define ECHO3 7

#define LDR_PIN A0

const int LED1_PIN = 9;
const int LED2_PIN = 10;
const int LED3_PIN = 11;

// ---- Thresholds (from original code) ----
const int  LDR_BRIGHT_THRESHOLD = 800;   // ldrValue > 800 treated as daylight -> lights off
const int  DIM_LEVEL            = 51;    // ~20% of 255
const int  FULL_LEVEL           = 255;
const long DETECT_DISTANCE_CM   = 20;    // Object within 20 cm -> brighten that LED
const unsigned long HOLD_MS     = 5000;  // Keep the brightened LED on for 5 s

void setLeds(int l1, int l2, int l3) {
  analogWrite(LED1_PIN, l1);
  analogWrite(LED2_PIN, l2);
  analogWrite(LED3_PIN, l3);
}

// Trigger one ultrasonic reading and return distance in cm.
// distance = echo_time_us * 0.034 cm/us / 2 (round trip)
long readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);   // blocking; default 1 s timeout
  long distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  Serial.begin(9600);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  pinMode(TRIG3, OUTPUT);
  pinMode(ECHO3, INPUT);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
}

void loop() {
  long distance1 = readUltrasonic(TRIG1, ECHO1);
  long distance2 = readUltrasonic(TRIG2, ECHO2);
  long distance3 = readUltrasonic(TRIG3, ECHO3);

  int ldrValue = analogRead(LDR_PIN);

  Serial.print("Distance 1: ");
  Serial.print(distance1);
  Serial.print(" cm | Distance 2: ");
  Serial.print(distance2);
  Serial.print(" cm | Distance 3: ");
  Serial.print(distance3);
  Serial.print(" cm | LDR: ");
  Serial.println(ldrValue);

  // Daylight: all lights off, skip the rest of this loop iteration.
  // (Direction of the LDR reading depends on how the voltage divider is wired.)
  if (ldrValue > LDR_BRIGHT_THRESHOLD) {
    setLeds(0, 0, 0);
    return;
  }

  // Dark: brighten the LED next to the first sensor that sees an object.
  // Only one LED is brightened per pass (if / else if priority: 1, 2, 3).
  if (distance1 <= DETECT_DISTANCE_CM) {
    setLeds(FULL_LEVEL, DIM_LEVEL, DIM_LEVEL);
    delay(HOLD_MS);
  } else if (distance2 <= DETECT_DISTANCE_CM) {
    setLeds(DIM_LEVEL, FULL_LEVEL, DIM_LEVEL);
    delay(HOLD_MS);
  } else if (distance3 <= DETECT_DISTANCE_CM) {
    setLeds(DIM_LEVEL, DIM_LEVEL, FULL_LEVEL);
    delay(HOLD_MS);
  }

  // Return to the dim default level.
  setLeds(DIM_LEVEL, DIM_LEVEL, DIM_LEVEL);
  delay(200);
}
