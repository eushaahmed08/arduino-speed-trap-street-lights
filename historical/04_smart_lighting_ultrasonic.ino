#define TRIG1 2
#define ECHO1 3

#define TRIG2 4
#define ECHO2 5

#define TRIG3 6
#define ECHO3 7

#define LDR_PIN A0  // LDR sensor at A0

int led1 = 9;   // First LED
int led2 = 10;  // Second LED
int led3 = 11;  // Third LED

void setup() {
    Serial.begin(9600);
    
    pinMode(TRIG1, OUTPUT);
    pinMode(ECHO1, INPUT);

    pinMode(TRIG2, OUTPUT);
    pinMode(ECHO2, INPUT);

    pinMode(TRIG3, OUTPUT);
    pinMode(ECHO3, INPUT);

    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
    pinMode(led3, OUTPUT);
}

long readUltrasonic(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH);
    long distance = duration * 0.034 / 2;  // Convert to cm
    return distance;
}

void loop() {
    long distance1 = readUltrasonic(TRIG1, ECHO1);
    long distance2 = readUltrasonic(TRIG2, ECHO2);
    long distance3 = readUltrasonic(TRIG3, ECHO3);

    int ldrValue = analogRead(LDR_PIN);  // Read LDR sensor

    Serial.print("Distance 1: ");
    Serial.print(distance1);
    Serial.print(" cm | Distance 2: ");
    Serial.print(distance2);
    Serial.print(" cm | Distance 3: ");
    Serial.print(distance3);
    Serial.print(" cm | LDR: ");
    Serial.println(ldrValue);

    int defaultBrightness;

    if (ldrValue > 800) {
        // LDR is very bright, turn OFF all LEDs
        analogWrite(led1, 0);
        analogWrite(led2, 0);
        analogWrite(led3, 0);
        return; // Skip further processing
    } else {
        defaultBrightness = 51;  // LDR ≤ 800, all LEDs at 20% brightness
    }

    // Check ultrasonic distances
    if (distance1 <= 20) {
        analogWrite(led1, 255);  // LED 9 full brightness
        analogWrite(led2, defaultBrightness);
        analogWrite(led3, defaultBrightness);
        delay(5000);  // Keep brightness for 5 seconds
    } 
    else if (distance2 <= 20) {
        analogWrite(led1, defaultBrightness);
        analogWrite(led2, 255);  // LED 10 full brightness
        analogWrite(led3, defaultBrightness);
        delay(5000);  // Keep brightness for 5 seconds
    } 
    else if (distance3 <= 20) {
        analogWrite(led1, defaultBrightness);
        analogWrite(led2, defaultBrightness);
        analogWrite(led3, 255);  // LED 11 full brightness
        delay(5000);  // Keep brightness for 5 seconds
    } 

    // After 5 sec, return all LEDs to default brightness
    analogWrite(led1, defaultBrightness);
    analogWrite(led2, defaultBrightness);
    analogWrite(led3, defaultBrightness);

    delay(200);  // Small delay for stability
}
