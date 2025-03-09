#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <SoftwareSerial.h>

#define WATER_SENSOR A0
#define MOISTURE_SENSOR A1
#define BUZZER 7
#define WATER_THRESHOLD 300
#define MOISTURE_THRESHOLD 400  // Moisture sensor is HIGH by default, LOW when detecting water

// ✅ Individual thresholds for each axis
#define X_THRESHOLD 1.5  // Adjust this for X-axis sensitivity
#define Y_THRESHOLD 1.5  // Adjust this for Y-axis sensitivity
#define Z_THRESHOLD 12.0  // Adjust this for Z-axis sensitivity

SoftwareSerial sim800(10, 11);  // RX, TX for SIM800L
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);  // Sensor ID

String serialMsg = "";
int earthquakeFlag = 0;

const unsigned long updateIntervel = 5000;
unsigned long prevTime = 0;
void setup() {
    Serial.begin(9600);
    sim800.begin(9600);
    
    pinMode(BUZZER, OUTPUT);
    pinMode(WATER_SENSOR, INPUT);
    pinMode(MOISTURE_SENSOR, INPUT);

    // Serial.println("Disaster Monitoring System Initialized...");

    // ✅ Use 0x1D manually (if needed)
    // if (!accel.begin(0x1D)) {  
    //     Serial.println("No ADXL345 detected! Check connections.");
    //     while (1);
    // } else {
    //     Serial.println("ADXL345 detected successfully!");
    // }
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - prevTime >= updateIntervel){
    int waterLevel = analogRead(WATER_SENSOR);
    int moistureLevel = digitalRead(MOISTURE_SENSOR);  // HIGH = Dry, LOW = Wet
    sensors_event_t event;
    accel.getEvent(&event);

    float xAccel = abs(event.acceleration.x);
    float yAccel = abs(event.acceleration.y);
    float zAccel = abs(event.acceleration.z);

    // Print sensor values
    // Serial.println("----- SENSOR DATA -----");
    // Serial.print("Water Level: "); Serial.println(waterLevel);
    // Serial.print("Moisture Level: "); Serial.println(moistureLevel == HIGH ? "Dry" : "Wet");
    // Serial.print("Acceleration X: "); Serial.println(xAccel);
    // Serial.print("Acceleration Y: "); Serial.println(yAccel);
    // Serial.print("Acceleration Z: "); Serial.println(zAccel);
    // Serial.println("----------------------");

    // Flood Alert
    if (waterLevel > WATER_THRESHOLD) {
        // Serial.println("🚨 ALERT: High Water Level!");
        digitalWrite(BUZZER, HIGH);
        sendSMS("Flood Alert: High Water Level Detected!");
    } else {
        digitalWrite(BUZZER, LOW);
    }

    // Landslide Alert (Detects when soil is wet)
    if (moistureLevel == LOW) {  // LOW means wet soil (landslide risk)
        // Serial.println("🚨 ALERT: Landslide Warning!");
        sendSMS("Landslide Warning: High Soil Moisture!");
    }

    // Earthquake Alert with Adjustable Thresholds
    if (xAccel > X_THRESHOLD || yAccel > Y_THRESHOLD || zAccel > Z_THRESHOLD) {
        // Serial.println("🚨 ALERT: Earthquake Detected!");
        earthquakeFlag = 1
        sendSMS("Earthquake Alert: Strong Ground Movement!");
    }else{
      earthquakeFlag =0;
    }
    previousTime = currentTime;
  }
    
    
    serialMsg = String(waterLevel)+","+String(map(moistureLevel, 0, 1, 0, 100))+","+String(earthquakeFlag);
    Serial.println(serialMsg);
    delay(500);  // Delay before next reading
}

void sendSMS(String message) {
    // Serial.println("📤 Sending SMS: " + message);
    sim800.println("AT+CMGF=1");  // Set SMS mode to text
    delay(1000);
    sim800.println("AT+CMGS=\"+918547043528\"");  // Replace with actual phone number
    delay(1000);
    sim800.println(message);
    delay(1000);
    sim800.write(26);  // End SMS with Ctrl+Z
    delay(1000);
}
