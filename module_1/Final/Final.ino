#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(3, 2);

#define TRIG_PIN 9 //10 - module2
#define ECHO_PIN 8  //9 - module2

// ADXL345 I2C address
#define ADXL345_ADDR 0x53
// 0x1D  - module2
// 0x53
// ADXL345 registers
#define POWER_CTL 0x2D
#define DATA_FORMAT 0x31
#define DATAX0 0x32
#define DATAX1 0x33
#define DATAY0 0x34
#define DATAY1 0x35
#define DATAZ0 0x36
#define DATAZ1 0x37

LiquidCrystal_I2C lcd(0x27,16,2);

const float THRESHOLD = 3.0;  // Acceleration threshold
const int SAMPLE_WINDOW = 1000;  // Time window in ms
const int MIN_PEAKS = 2;  // Minimum peaks to detect earthquake
const int SERIAL_REFRESH = 500;  // Serial update interval in ms

unsigned long lastTime = 0;
unsigned long lastSerialUpdate = 0;
int peakCount = 0;
bool earthquakeDetected = false;

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
  Wire.begin();
  Serial.begin(9600);
  mySerial.begin(9600);
  delay(1000);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  pinMode(A2,INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize ADXL345
  writeRegister(POWER_CTL, 0x08);  
  writeRegister(DATA_FORMAT, 0x0B); 

  lcd.print("Monitoring...");
  delay(1000);

}

void loop() {
  unsigned long current_time = millis();

  int16_t x, y, z;
  float xg, yg, zg;
  
  // Read accelerometer data
  x = readAxis(DATAX0, DATAX1);
  y = readAxis(DATAY0, DATAY1);
  z = readAxis(DATAZ0, DATAZ1);
  
  // Convert to g force
  xg = x * 0.004;  
  yg = y * 0.004;
  zg = z * 0.004;
  
  // Calculate magnitude
  float magnitude = sqrt(xg*xg + yg*yg + zg*zg);

    if (magnitude > THRESHOLD) {
    peakCount++;
  }
    earthquakeDetected = (peakCount >= MIN_PEAKS);
    
    // lcd.clear();
    if (earthquakeDetected) {
      Serial.println("EARTHQUAKE: DETECTED!");
      
      lcd.clear();
      lcd.print("EARTHQUAKE!");
      lcd.setCursor(4, 1);
      lcd.print("ALERT!");
      // sendSMS("EARTHQUAKE: DETECTED!");
      delay(1000);
      peakCount = 0;
    }
    delay(10);

  // // Check if sample window has elapsed
  if (current_time - lastTime >= SAMPLE_WINDOW) {
    int ultrasonicDist = getDistance();
    ultrasonicLevel = getUltrasonicLevel(ultrasonicDist);
    int waterLevel = analogRead(A1); //A1 - module2
    waterWarningLevel = getWaterLevel(waterLevel);
    int flexValue = analogRead(A0); //A2 - module2
    flexWarningLevel = getFlexLevel(flexValue);

    Serial.print("--ultrasonicDist: ");
    Serial.print(ultrasonicDist);
    Serial.print(" --waterLevel: ");
    Serial.print(waterLevel);
    Serial.print(" --flexValue: ");
    Serial.println(flexValue);

    checkAndSendWarning("Rain", ultrasonicLevel, prevUltrasonicLevel);
    prevUltrasonicLevel = ultrasonicLevel;
    checkAndSendWarning("Flood", waterWarningLevel, prevWaterLevel);
    prevWaterLevel = waterWarningLevel;
    checkAndSendWarning("Landslide", flexWarningLevel, prevFlexLevel);
    prevFlexLevel = flexWarningLevel;
    lastTime = current_time;
  }else if (current_time-pre_time > lcd_update_rate){
    
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
      // // Serial.print(ultrasonicDist);
      // Serial.print(" : ");
      Serial.println(ultrasonicLevel);
      lcd.setCursor(2,0);
      lcd.print("Rain Level: ");
      lcd.setCursor(0,1);
      lcd.print((ultrasonicLevel>0)? "Status: Alert! ":"Status: Normal");
      lcd.setCursor(15,1);
      lcd.print((ultrasonicLevel>0)? String(ultrasonicLevel):" ");
      
      break;
    case 1: 
      
      Serial.print("Flood Level-2: ");
      // Serial.print(waterLevel);
      // Serial.print(" : ");
      Serial.println(waterWarningLevel);
      lcd.setCursor(2,0);
      lcd.print("Water Level : ");
      lcd.setCursor(0,1);
      lcd.print((waterWarningLevel>0)?"Status: Alert! ":"Status: Normal");
      lcd.setCursor(15,1);
      lcd.print((waterWarningLevel>0)? String(waterWarningLevel) :" ");
      
      break;
    case 2:  
      
      Serial.print("Landslide-3: ");
      // Serial.print(flexValue);
      // Serial.print(" : ");
      Serial.println(flexWarningLevel);
      lcd.setCursor(0,0);
      lcd.print("Landslide: ");
      lcd.setCursor(0,1);
      lcd.print((flexWarningLevel>0)?"Status: Alert!":"Status: Normal");
      lcd.setCursor(15,1);
      lcd.print((flexWarningLevel>0)? String(flexWarningLevel) :" ");
      
      break;
    case 3:  
      // Serial.println("------------------------------");
      lcd.setCursor(0,0);
      lcd.print("Earthquake: ");
      lcd.setCursor(0,1);
      lcd.print("Status: Normal");
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
  if (value < 10) return 3;
  if (value < 18) return 2;
  if (value < 22) return 1;
  return 0;
}


void writeRegister(byte reg, byte value) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

byte readRegister(byte reg) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(reg);
  Wire.endTransmission();
  
  Wire.requestFrom(ADXL345_ADDR, 1);
  return Wire.read();
}

int16_t readAxis(byte regLow, byte regHigh) {
  Wire.beginTransmission(ADXL345_ADDR);
  Wire.write(regLow);
  Wire.endTransmission();
  
  Wire.requestFrom(ADXL345_ADDR, 2);
  byte low = Wire.read();
  byte high = Wire.read();
  
  return (high << 8) | low;
}

void checkAndSendWarning(const char* sensor, int currentLevel, int previousLevel) {
  if (currentLevel != previousLevel && previousLevel != -1) {
    lcd.clear();
    lcd.setCursor(3,0);
    lcd.print(sensor);
    lcd.setCursor(0,1);
    lcd.print("Status: Alert!- ");
    lcd.setCursor(15,1);
    lcd.print(currentLevel);

    Serial.print(sensor);
    Serial.print(" Warning Level Changed to: ");
    Serial.println(currentLevel);
    Serial.println("Sending Warning SMS...");
    
    String sms = "Warning!!: " + String(sensor) + " Level changed to "+String(currentLevel);
    // sendSMS(sms);
    delay(2000);
    Serial.println("Message has been sent!");
    Serial.println("-----------------------------------------");
  }
}

void sendSMS(String msg){
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println("AT+CMGS=\"+ZZxxxxxxxxxx\""); 
  updateSerial();
  mySerial.print(msg);
  updateSerial();
  mySerial.write(26);
}
void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}
