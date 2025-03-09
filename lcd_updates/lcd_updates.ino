#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define TRIG_PIN 9 
#define ECHO_PIN 8

LiquidCrystal_I2C lcd(0x27,16,2);

const unsigned long lcd_update_rate=5000;
unsigned long pre_time = 0;
unsigned int iteration_variable=0;

float rain = 0.0;
int flood_level = 0;
int landslide_level = 0;
// Previous warning levels to track changes
int prevUltrasonicLevel = -1;
int prevWaterLevel = -1;
int prevFlexLevel = -1;

int ultrasonicLevel = 0;
int waterWarningLevel = 0;
int flexWarningLevel = 0;


void setup() {
  Serial.begin(9600);
  lcd.init();
  pinMode(A2,INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("---------------Start!---------------");

}

void loop() {
  unsigned long current_time = millis();

  // Read sensors
  int ultrasonicDist = getDistance();
  int waterLevel = analogRead(A1);
  int flexValue = analogRead(A2);
  
Serial.print("--ultrasonicDist: ");
Serial.print(ultrasonicDist);
Serial.print(" --waterLevel: ");
Serial.print(waterLevel);
Serial.print(" --flexValue: ");
Serial.println(flexValue);

  // Convert readings to warning levels (0-3)
  int ultrasonicLevel = getUltrasonicLevel(ultrasonicDist);
  int waterWarningLevel = getWaterLevel(waterLevel);
  int flexWarningLevel = getFlexLevel(flexValue);

  checkAndSendWarning("Ultrasonic", ultrasonicLevel, prevUltrasonicLevel);
  checkAndSendWarning("Water", waterWarningLevel, prevWaterLevel);
  checkAndSendWarning("Flex", flexWarningLevel, prevFlexLevel);

  prevUltrasonicLevel = ultrasonicLevel;
  prevWaterLevel = waterWarningLevel;
  prevFlexLevel = flexWarningLevel;
  delay(1000);


  


  if (current_time-pre_time > lcd_update_rate){

    // Serial.println("Updating LCD screen");
    if (iteration_variable >= 3) {
    iteration_variable = 0;
    } else {
        iteration_variable++;
    }
    display_with_options(iteration_variable);
    pre_time=current_time;
  }

}
void display_with_options(int iteration_num){
  lcd.clear();
  // Serial.print("iteration_num:");
  // Serial.println(iteration_num);
    switch (iteration_num) {
    case 0:  
      Serial.print("Rain Level-1: ");
      Serial.println(ultrasonicLevel);
      lcd.setCursor(2,0);
      lcd.print("Rain Level: ");
      lcd.setCursor(0,1);
      lcd.print(ultrasonicLevel);
      break;
    case 1: 
      Serial.print("Flood Level-2: ");
      Serial.println(waterWarningLevel);
      lcd.setCursor(2,0);
      lcd.print("Flood Level: ");
      lcd.setCursor(0,1);
      lcd.print("Not Detected");
      break;
    case 2:  
      
      Serial.print("Landslide-3: ");
      Serial.println(flexWarningLevel);
      lcd.setCursor(0,0);
      lcd.print("Landslide Level: ");
      lcd.setCursor(0,1);
      lcd.print("level: 0");
      break;
    case 3:  
      Serial.println("------------------------------");
      lcd.setCursor(0,0);
      lcd.print("Earthquake: ");
      lcd.setCursor(0,1);
      lcd.print("level: 0");
      break;
  }
}

int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  return pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;
}


int getUltrasonicLevel(int distance) {
  if (distance < 10) return 3;
  if (distance < 20) return 2;
  if (distance < 30) return 1;
  return 0;
}

int getWaterLevel(int value) {
  if (value > 750) return 3;
  if (value > 500) return 2;
  if (value > 250) return 1;
  return 0;
}

int getFlexLevel(int value) {
  if (value > 900) return 3;
  if (value > 700) return 2;
  if (value > 500) return 1;
  return 0;
}

void checkAndSendWarning(const char* sensor, int currentLevel, int previousLevel) {
  if (currentLevel != previousLevel && previousLevel != -1) {
    Serial.print(sensor);
    Serial.print(" Warning Level Changed to: ");
    Serial.println(currentLevel);
    Serial.println("Sending Warning SMS...");
    delay(2000);
    Serial.println("Message has been sent!");
    Serial.println("-----------------------------------------");
  }
}
