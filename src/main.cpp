#include <Arduino.h>
#include <Wire.h>  // Add this include
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define PH_PIN 34

#define POWER 33
#define SIGNAL 32

int value=0;

int level=0;

float ph_level=0;

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

  // set for water level
  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, LOW);
  
  connectToWiFi();
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
  ph_level = readPH();

  String message = "PH Value: "+ String(ph_level) + " Water Level: " + String(level);

  // scrollText(1, message, 320, 16);
  delay(670);
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

  float volatge = adcVal * (3.3/4095.0);

  Serial.printf("Voltage: %.2f\n", volatge);


  float ph = 7 + ( (2.5 - volatge) / 0.18);
  
  return ph;
};