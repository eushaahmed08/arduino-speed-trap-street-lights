#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust 0x27 if your display has a different address

int IR1 = 8;
int IR2 = 9;
unsigned long t1 = 0;
unsigned long t2 = 0;
float velocity;

void setup()
{
  lcd.init();
  lcd.backlight();
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  Serial.begin(9600);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Vehicle Speed ");
}

void loop()
{
  if (digitalRead(IR1) == 1)
  {
    t1 = millis();
  }
  if (digitalRead(IR2) == 1)
  {
    t2 = millis();
  }

  velocity = t2 - t1;
  velocity = velocity / 1000;       // Convert millisecond to second
  velocity = (0.2 / velocity) * 3.6; // km/h

  lcd.setCursor(2, 1);
  lcd.print(velocity);
  lcd.print(" Km/hr");
  delay(500);
}
