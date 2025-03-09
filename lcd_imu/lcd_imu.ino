#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_ADXL345_U.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

void setup() {
  // Initialize Serial
  Serial.begin(9600);
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize ADXL345
  if(!accel.begin()) {
    lcd.print("ADXL345 Error!");
    Serial.println("ADXL345 Error!");
    while(1);
  }
  
  accel.setRange(ADXL345_RANGE_4_G);
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);
  
  // Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("X:");
  lcd.print(event.acceleration.x);
  lcd.print(" Y:");
  lcd.print(event.acceleration.y);
  lcd.setCursor(0, 1);
  lcd.print("Z:");
  lcd.print(event.acceleration.z);
  
  // Print to Serial
  Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
  Serial.print("Z: "); Serial.println(event.acceleration.z);
  
  delay(200);
}