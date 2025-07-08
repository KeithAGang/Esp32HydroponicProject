#include <Arduino.h>
#include <Wire.h>          
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>

#define BLYNK_TEMPLATE_ID "TMPL2suXuZlZQ"
#define BLYNK_TEMPLATE_NAME "Iot Hydroponics Tower Monitoring and Automation"
#define BLYNK_AUTH_TOKEN "YsHjjO7hNvNDZVSaqjR5zrDiBreFiE2z"  // Use your actual token

#include <BlynkSimpleEsp32.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define PH_PIN 34

#define WATER_LEVEL_POWER 33
#define WATER_LEVEL_SIGNAL 32

#define RELAY_PIN 18 

// Blynk Virtual Pins
#define VPIN_WATER_LEVEL    V0  // Water level reading
#define VPIN_PH_LEVEL       V1  // pH level reading
#define VPIN_PUMP_STATUS    V2  // Pump status display
#define VPIN_PUMP_CONTROL   V3  // Manual pump control button 
#define VPIN_AUTO_MODE      V4  // Auto/Manual mode switch
#define VPIN_THRESHOLD      V5  // Water level threshold slider

// WiFi credentials
const char* ssid = "TECNO-TR118-638B";
const char* password = "751086D1";

// Global variables
int value = 0;
int level = 0;
float ph_level = 0;
bool autoMode = true;  // Auto mode by default
bool manualPumpState = false;  // Manual pump control
int lowWaterThreshold = 500;  // Default threshold

// Timing variables for non-blocking operations
unsigned long lastSensorRead = 0;
unsigned long lastBlynkUpdate = 0;
unsigned long lastLCDUpdate = 0;
const unsigned long SENSOR_INTERVAL = 2000;    // Read sensors every 2 seconds
const unsigned long BLYNK_INTERVAL = 1000;     // Update Blynk every 1 second
const unsigned long LCD_INTERVAL = 500;        // Update LCD every 500ms

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Function declarations
int WaterSensor();
void connectToWiFi();
float readPH();
void updatePumpControl();
void updateLCD();
void scrollText(int row, String message, int delayTime, int lcdColumns);

// Blynk Virtual Pin Handlers
BLYNK_WRITE(VPIN_PUMP_CONTROL) {
  manualPumpState = param.asInt();
  Serial.println("Manual pump control: " + String(manualPumpState ? "ON" : "OFF"));
  
  if (!autoMode) {
    digitalWrite(RELAY_PIN, manualPumpState ? LOW : HIGH);
    // Update pump status immediately
    Blynk.virtualWrite(VPIN_PUMP_STATUS, manualPumpState ? 1 : 0);
  }
}

BLYNK_WRITE(VPIN_AUTO_MODE) {
  autoMode = param.asInt();
  Serial.println("Mode changed to: " + String(autoMode ? "AUTO" : "MANUAL"));
  
  // Update pump control based on mode
  if (autoMode) {
    updatePumpControl();  // Let auto mode take control
  } else {
    // In manual mode, use the manual pump state
    digitalWrite(RELAY_PIN, manualPumpState ? LOW : HIGH);
  }
}

BLYNK_WRITE(VPIN_THRESHOLD) {
  lowWaterThreshold = param.asInt();
  Serial.println("Water level threshold updated to: " + String(lowWaterThreshold));
}

// Blynk Connected callback
BLYNK_CONNECTED() {
  Serial.println("Blynk Connected!");
  
  // Sync all virtual pins
  Blynk.syncVirtual(VPIN_PUMP_CONTROL);
  Blynk.syncVirtual(VPIN_AUTO_MODE);
  Blynk.syncVirtual(VPIN_THRESHOLD);
  
  // Initialize widgets with current values
  Blynk.virtualWrite(VPIN_THRESHOLD, lowWaterThreshold);
  Blynk.virtualWrite(VPIN_AUTO_MODE, autoMode ? 1 : 0);
  
  // Send initial sensor readings
  level = WaterSensor();
  ph_level = readPH();
  Blynk.virtualWrite(VPIN_WATER_LEVEL, level);
  Blynk.virtualWrite(VPIN_PH_LEVEL, ph_level);
  
  // Send initial pump status
  bool pumpOn = (digitalRead(RELAY_PIN) == LOW);
  Blynk.virtualWrite(VPIN_PUMP_STATUS, pumpOn ? 1 : 0);
  
  Serial.println("Initial Blynk values sent");
}

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Configure I2C pins
  Wire.begin(I2C_SDA, I2C_SCL);

  // Configure water level sensor power pin
  pinMode(WATER_LEVEL_POWER, OUTPUT);
  digitalWrite(WATER_LEVEL_POWER, LOW);

  // Configure relay pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Pump OFF initially

  // Connect to WiFi
  connectToWiFi();

  // Initialize Blynk
  Blynk.config(BLYNK_AUTH_TOKEN);
  
  // Try to connect to Blynk
  Serial.println("Connecting to Blynk...");
  if (Blynk.connect()) {
    Serial.println("Blynk connected successfully!");
  } else {
    Serial.println("Blynk connection failed, but will retry in loop");
  }

  // Initialize LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  delay(100);

  Serial.println("System initialized");
  Serial.println("LCD initialized");
  Serial.println("Relay pin configured");
  Serial.println("Blynk configured");
}

void loop() {
  // Run Blynk
  Blynk.run();

  unsigned long currentTime = millis();

  // Read sensors at intervals
  if (currentTime - lastSensorRead >= SENSOR_INTERVAL) {
    level = WaterSensor();
    ph_level = readPH();
    
    Serial.print("Water Level: ");
    Serial.println(level);
    Serial.print("PH Value: ");
    Serial.println(ph_level);
    
    lastSensorRead = currentTime;
  }

  // Update pump control (auto mode only)
  if (autoMode) {
    updatePumpControl();
  }

  // Update Blynk at intervals
  if (currentTime - lastBlynkUpdate >= BLYNK_INTERVAL) {
    if (Blynk.connected()) {
      // Send sensor data
      Blynk.virtualWrite(VPIN_WATER_LEVEL, level);
      Blynk.virtualWrite(VPIN_PH_LEVEL, ph_level);
      
      // Update pump status (send 1 for ON, 0 for OFF to LED widget)
      bool pumpOn = (digitalRead(RELAY_PIN) == LOW);
      Blynk.virtualWrite(VPIN_PUMP_STATUS, pumpOn ? 1 : 0);
      
      // Update auto mode status
      Blynk.virtualWrite(VPIN_AUTO_MODE, autoMode ? 1 : 0);
      
      // Optional: Send notification for low water
      if (level < lowWaterThreshold && autoMode && pumpOn) {
        Blynk.logEvent("low_water", "Water level is low: " + String(level));
      }
      
      Serial.println("Blynk updated - Water: " + String(level) + ", pH: " + String(ph_level) + ", Pump: " + (pumpOn ? "ON" : "OFF"));
      
      lastBlynkUpdate = currentTime;
    } else {
      Serial.println("Blynk not connected!");
      // Try to reconnect
      Blynk.connect();
    }
  }

  // Update LCD at intervals
  if (currentTime - lastLCDUpdate >= LCD_INTERVAL) {
    updateLCD();
    lastLCDUpdate = currentTime;
  }
}

void updatePumpControl() {
  if (level < lowWaterThreshold) {
    digitalWrite(RELAY_PIN, LOW);   // Turn pump ON
    Serial.println("Auto Pump: ON (Water level low)");
  } else {
    digitalWrite(RELAY_PIN, HIGH);  // Turn pump OFF
    Serial.println("Auto Pump: OFF (Water level OK)");
  }
}

void updateLCD() {
  static String lastLine1 = "";
  static String lastLine2 = "";
  static int scrollPos = 0;
  static unsigned long lastScroll = 0;
  
  // First line: System status
  bool pumpOn = (digitalRead(RELAY_PIN) == LOW);
  String mode = autoMode ? "AUTO" : "MAN";
  String pumpStatus = pumpOn ? "ON" : "OFF";
  String line1 = mode + " Pump:" + pumpStatus;
  
  // Pad line1 to exactly 16 characters
  while (line1.length() < 16) {
    line1 += " ";
  }
  line1 = line1.substring(0, 16);
  
  // Only update line 1 if it changed
  if (line1 != lastLine1) {
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lastLine1 = line1;
  }
  
  // Second line: Sensor readings (no scrolling for now)
  String line2 = "PH:" + String(ph_level, 1) + " H2O:" + String(level);
  
  // Pad line2 to exactly 16 characters
  while (line2.length() < 16) {
    line2 += " ";
  }
  line2 = line2.substring(0, 16);
  
  // Only update line 2 if it changed
  if (line2 != lastLine2) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
    lastLine2 = line2;
  }
}

int WaterSensor() {
  digitalWrite(WATER_LEVEL_POWER, HIGH);
  delay(10);
  value = analogRead(WATER_LEVEL_SIGNAL);
  delay(10);
  digitalWrite(WATER_LEVEL_POWER, LOW);
  return value;
}

void connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ Already connected to WiFi.");
    return;
  }

  WiFi.disconnect();
  WiFi.config(INADDR_NONE, INADDR_NONE, IPAddress(8, 8, 8, 8), IPAddress(8, 8, 4, 4));
  WiFi.begin(ssid, password);
  Serial.print("🔌 Connecting to WiFi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected");
    Serial.print("📶 IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi connection failed! Status: " + String(WiFi.status()));
  }
}

float readPH() {
  int adcVal = analogRead(PH_PIN);
  float voltage = adcVal * (3.3 / 4095.0);
  Serial.printf("Voltage: %.2f\n", voltage);
  float ph = 7 + ((2.5 - voltage) / 0.18);
  return ph;
}

void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i = 0; i < lcdColumns; i++) {
    message = " " + message;
  }
  message = message + " ";
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}