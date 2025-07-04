#include <Arduino.h>
#include <Wire.h>  // Add this include
#include <LiquidCrystal_I2C.h>

#define I2C_SDA 21
#define I2C_SCL 22

#define POWER 33
#define SIGNAL 32

int value=0;

int level=0;

LiquidCrystal_I2C lcd(0x27, 16, 2);
int WaterSensor();
void scrollText(int row, String message, int delayTime, int lcdColumns);


void setup()
{
  // Initialize Serial for debugging
  Serial.begin(115200);
  
  // Configure I2C pins - THIS IS CRUCIAL
  Wire.begin(I2C_SDA, I2C_SCL);

  // set for water level
  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, LOW);
  
  // Initialize LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  
  // Add a small delay to ensure LCD is ready
  delay(100);
  
  // Display text
  
  
  
  Serial.println("LCD initialized");
}

void loop()
{
  lcd.setCursor(2, 0);
  lcd.print("Gang Gang/|?");

  level = WaterSensor();

  String message = "Water Level: " + String(level);

  scrollText(1, message, 320, 16);
}

int WaterSensor()
{
  digitalWrite(POWER, HIGH);
  delay(10);
  value = analogRead(SIGNAL);
  delay(10);
  digitalWrite(POWER, LOW);
  return value;
}

void scrollText(int row, String message, int delayTime, int lcdColumns)
{
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message; 
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}