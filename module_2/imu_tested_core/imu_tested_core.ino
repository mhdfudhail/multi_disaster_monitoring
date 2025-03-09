#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ADXL345 I2C address
#define ADXL345_ADDR 0x1D

// ADXL345 registers
#define POWER_CTL 0x2D
#define DATA_FORMAT 0x31
#define DATAX0 0x32
#define DATAX1 0x33
#define DATAY0 0x34
#define DATAY1 0x35
#define DATAZ0 0x36
#define DATAZ1 0x37

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set I2C address to 0x27

const float THRESHOLD = 5.0;  // Acceleration threshold
const int SAMPLE_WINDOW = 1000;  // Time window in ms
const int MIN_PEAKS = 3;  // Minimum peaks to detect earthquake
const int SERIAL_REFRESH = 500;  // Serial update interval in ms

unsigned long lastTime = 0;
unsigned long lastSerialUpdate = 0;
int peakCount = 0;
bool earthquakeDetected = false;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize ADXL345
  writeRegister(POWER_CTL, 0x08);  // Set measurement mode
  writeRegister(DATA_FORMAT, 0x0B);  // Set data format (±16g range)
  
  lcd.print("Monitoring...");
  Serial.println("ADXL345 Earthquake Monitor");
  Serial.println("-------------------------");
}

void loop() {
  int16_t x, y, z;
  float xg, yg, zg;
  
  // Read accelerometer data
  x = readAxis(DATAX0, DATAX1);
  y = readAxis(DATAY0, DATAY1);
  z = readAxis(DATAZ0, DATAZ1);
  
  // Convert to g force (±16g range)
  xg = x * 0.004;  // 16g range / 2048 LSB/g = 0.004
  yg = y * 0.004;
  zg = z * 0.004;
  
  // Calculate magnitude
  float magnitude = sqrt(xg*xg + yg*yg + zg*zg);
  
  // Update Serial Monitor periodically
  if (millis() - lastSerialUpdate >= SERIAL_REFRESH) {
    Serial.println("\nAccelerometer Readings:");
    Serial.print("X: "); Serial.print(xg); Serial.print(" g\t");
    Serial.print("Y: "); Serial.print(yg); Serial.print(" g\t");
    Serial.print("Z: "); Serial.print(zg); Serial.println(" g");
    Serial.print("Magnitude: "); Serial.print(magnitude); Serial.println(" g");
    Serial.print("Peak Count: "); Serial.println(peakCount);
    Serial.print("Status: "); 
    Serial.println(earthquakeDetected ? "EARTHQUAKE DETECTED!" : "Normal");
    Serial.println("-------------------------");
    
    lastSerialUpdate = millis();
  }
  
  if (magnitude > THRESHOLD) {
    peakCount++;
  }
  
  // Check if sample window has elapsed
  if (millis() - lastTime >= SAMPLE_WINDOW) {
    earthquakeDetected = (peakCount >= MIN_PEAKS);
    
    lcd.clear();
    if (earthquakeDetected) {
      lcd.print("EARTHQUAKE!");
      lcd.setCursor(0, 1);
      lcd.print("ALERT!");
    } else {
      lcd.print("Status: Normal");
      lcd.setCursor(0, 1);
      lcd.print("Mag: ");
      lcd.print(magnitude);
    }
    
    peakCount = 0;
    lastTime = millis();
  }
  
  delay(10);
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