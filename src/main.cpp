#include <Arduino.h>
#include <Wire.h>          
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define PH_PIN 34

#define WATER_LEVEL_POWER 33
#define WATER_LEVEL_SIGNAL 32

#define RELAY_PIN 18 

int value = 0;
int level = 0;
float ph_level = 0;

const char* ssid = "G Unit";
const char* password = "hinokamikagurass";
String serverURL = "https://www.google.com";

LiquidCrystal_I2C lcd(0x27, 16, 2);
int WaterSensor();
void connectToWiFi();
float readPH();
void scrollText(int row, String message, int delayTime, int lcdColumns);


void setup()
{
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Configure I2C pins - THIS IS CRUCIAL
  Wire.begin(I2C_SDA, I2C_SCL);

  // Configure water level sensor power pin
  pinMode(WATER_LEVEL_POWER, OUTPUT);
  digitalWrite(WATER_LEVEL_POWER, LOW); // Keep water level sensor power off initially

  // --- **IMPORTANT EDIT HERE:** Configure RELAY_PIN as an OUTPUT ---
  pinMode(RELAY_PIN, OUTPUT);
  // --- **IMPORTANT EDIT HERE:** Set initial state of the relay (pump OFF) ---
  // Assuming a common active-low relay module (LOW = ON, HIGH = OFF)
  digitalWrite(RELAY_PIN, HIGH); // Set HIGH to keep the pump OFF when the ESP32 starts

  connectToWiFi();

  // Initialize LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // Add a small delay to ensure LCD is ready
  delay(100);

  Serial.println("LCD initialized");
  Serial.println("Relay pin configured.");
}


void loop()
{
  lcd.setCursor(2, 0);
  lcd.print("Gang Gang/|?");

  level = WaterSensor(); // Read water level
  ph_level = readPH(); // Read PH level (currently commented out)

  Serial.print("Water Level Sensor Value: ");
  Serial.println(level);
  Serial.print("PH Value: ");
  Serial.println(ph_level);


  // --- **IMPORTANT EDIT HERE:** Logic to control the pump based on water level ---
  // You'll need to adjust the threshold '500' based on your water level sensor's readings.
  // Test your sensor and find the value that indicates "low water."
  int lowWaterThreshold = 500; // Example threshold - ADJUST THIS VALUE!

  if (level < lowWaterThreshold) { // If water level is below the threshold
    digitalWrite(RELAY_PIN, LOW);   // Turn the pump ON (for active-low relays)
    Serial.println("Pump Status: ON (Water level too low)");
    lcd.clear();
    lcd.setCursor(0, 0);            // Second line of LCD
    lcd.print("Pump: Off        "); // Display status on LCD (padded with spaces to clear old text)
  } else { // If water level is at or above the threshold
    digitalWrite(RELAY_PIN, HIGH);  // Turn the pump OFF (for active-low relays)
    Serial.println("Pump Status: OFF (Water level OK)");
    lcd.setCursor(0, 0);            // Second line of LCD
    lcd.print("Pump: On       "); // Display status on LCD
  }

  // lcd.printf("Water Level: %d", level);
  
  // --- END OF RELAY CONTROL LOGIC ---

  lcd.setCursor(0, 1);
  String message = "PH Value: "+ String(ph_level) + " Water Level: " + String(level);
  // scrollText(1, message, 320, 16); // Currently commented out
  lcd.print(message);
  lcd.scrollDisplayLeft();
  delay(670); // Small delay for loop iteration
}


int WaterSensor()
{
  digitalWrite(WATER_LEVEL_POWER, HIGH); // Power the water level sensor
  delay(10);                             // Short delay for sensor to stabilize
  value = analogRead(WATER_LEVEL_SIGNAL); // Read analog value from sensor
  delay(10);                             // Short delay
  digitalWrite(WATER_LEVEL_POWER, LOW);  // Turn off water level sensor power to save energy
  return value;
}


void scrollText(int row, String message, int delayTime, int lcdColumns)
{
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


// Connects the ESP32 to the specified WiFi network.
void connectToWiFi()
{
  // Check if already connected to avoid unnecessary attempts.
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("✅ Already connected to WiFi.");
    return;
  }

  // Explicitly configure DNS servers to ensure reliable hostname resolution
  // Google Public DNS servers are used as a reliable fallback.
  WiFi.disconnect(); // Disconnect before setting config to apply new settings
  WiFi.config(INADDR_NONE, INADDR_NONE, IPAddress(8, 8, 8, 8), IPAddress(8, 8, 4, 4));

  WiFi.begin(ssid, password);
  Serial.print("🔌 Connecting to WiFi");

  int attempts = 0;
  // Wait for connection with a timeout.
  while (WiFi.status() != WL_CONNECTED && attempts < 40)
  { // Wait up to 20 seconds (40 * 500ms)
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\n✅ WiFi connected");
    Serial.print("📶 IP address: ");
    Serial.println(WiFi.localIP());

    // Optional: Test DNS resolution here to confirm it works
    IPAddress resolvedIP;
    Serial.print("🔍 Resolving hostname for server: ");
    // Extract just the hostname part from the serverURL for DNS lookup
    String host = serverURL;
    int schemeEnd = host.indexOf("://");
    if (schemeEnd != -1)
    {
      host = host.substring(schemeEnd + 3);
    }
    int pathStart = host.indexOf("/");
    if (pathStart != -1)
    {
      host = host.substring(0, pathStart);
    }
    Serial.print(host);
    if (WiFi.hostByName(host.c_str(), resolvedIP))
    {
      Serial.print(" -> Resolved to IP: ");
      Serial.println(resolvedIP);
    }
    else
    {
      Serial.println(" -> ❌ DNS Resolution FAILED! This might prevent HTTPS connection.");
    }
  }
  else
  {
    Serial.println("\n❌ WiFi connection failed! Status: " + String(WiFi.status()));
  }
}


// Read PH val from sensor
float readPH()
{
  int adcVal = analogRead(PH_PIN);

  float volatge = adcVal * (3.3 / 4095.0);

  Serial.printf("Voltage: %.2f\n", volatge);

  float ph = 7 + ((2.5 - volatge) / 0.18);

  return ph;
};