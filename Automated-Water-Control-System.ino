#include "DHT.h"

// Pin definitions
#define DHTPIN 2
#define DHTTYPE DHT22
#define SOIL_PIN A0
#define WATER_LEVEL_PIN A1
#define RELAY_WATERFALL 4
#define RELAY_RAIN 5
#define BUZZER_PIN 6

// Initialize sensor
DHT dht(DHTPIN, DHTTYPE);

// System variables
unsigned long lastAutoControl = 0;
unsigned long lastSensorRead = 0;
unsigned long lastBuzzerToggle = 0;
unsigned long lastSerialOutput = 0;
bool buzzerState = false;

// Sensor data
float temperature = 0;
float humidity = 0;
int soilMoisture = 0;
int soilMoisturePercent = 0;
bool waterLevel = true;

// Control thresholds
const int SOIL_MOISTURE_THRESHOLD = 30;  // Percentage
const float TEMP_THRESHOLD = 30.0;       // Celsius
const int HUMIDITY_THRESHOLD = 90;       // Percentage
const unsigned long AUTO_WATER_INTERVAL = 18000000; // 5 hours in milliseconds

void setup() {
  Serial.begin(9600);
  
  // Initialize sensor
  dht.begin();
  
  // Configure pins
  pinMode(SOIL_PIN, INPUT);
  pinMode(WATER_LEVEL_PIN, INPUT_PULLUP);
  pinMode(RELAY_WATERFALL, OUTPUT);
  pinMode(RELAY_RAIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Initialize relays (LOW = OFF for most relay modules)
  digitalWrite(RELAY_WATERFALL, LOW);
  digitalWrite(RELAY_RAIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.println("=================================");
  Serial.println("Smart Tropical Garden Controller");
  Serial.println("=================================");
  Serial.println("System Started - Sensors Only Mode");
  Serial.println("Monitoring: Temperature, Humidity, Soil Moisture, Water Level");
  Serial.println();
  
  // Initial sensor reading
  readSensors();
  printSensorData();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Read sensors every 2 seconds
  if (currentTime - lastSensorRead >= 2000) {
    readSensors();
    lastSensorRead = currentTime;
  }
  
  // Print sensor data every 10 seconds
  if (currentTime - lastSerialOutput >= 10000) {
    printSensorData();
    lastSerialOutput = currentTime;
  }
  
  // Automatic control logic
  automaticControl(currentTime);
  
  // Handle water tank empty alarm
  handleWaterAlarm(currentTime);
  
  delay(100); // Small delay for stability
}

void readSensors() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  // Check for DHT sensor errors
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Warning: DHT sensor read error!");
    Serial.println("Check DHT22 connections:");
    Serial.println("- VCC to 3.3V or 5V");
    Serial.println("- GND to GND");
    Serial.println("- DATA to Pin 2");
    Serial.println("- Add 10K pullup resistor between DATA and VCC");
    temperature = 0;
    humidity = 0;
  }
  
  // Read soil moisture multiple times for better accuracy
  long soilTotal = 0;
  for(int i = 0; i < 5; i++) {
    soilTotal += analogRead(SOIL_PIN);
    delay(10);
  }
  soilMoisture = soilTotal / 5;
  
  // Dynamic calibration based on actual sensor readings
  // If your sensor reads around 181 in normal soil, let's work with that
  if (soilMoisture < 100) {
    // Very wet - sensor in water or very saturated soil
    soilMoisturePercent = 100;
  } else if (soilMoisture < 200) {
    // Wet soil (your current reading ~181 falls here)
    soilMoisturePercent = map(soilMoisture, 100, 200, 100, 80);
  } else if (soilMoisture < 400) {
    // Moist soil
    soilMoisturePercent = map(soilMoisture, 200, 400, 80, 60);
  } else if (soilMoisture < 600) {
    // Normal soil
    soilMoisturePercent = map(soilMoisture, 400, 600, 60, 40);
  } else if (soilMoisture < 800) {
    // Dry soil
    soilMoisturePercent = map(soilMoisture, 600, 800, 40, 20);
  } else {
    // Very dry soil or sensor in air
    soilMoisturePercent = map(soilMoisture, 800, 1023, 20, 0);
  }
  
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);
  
  // Read water level (LOW = empty tank)
  waterLevel = digitalRead(WATER_LEVEL_PIN);
}

void printSensorData() {
  Serial.println("--- Sensor Readings ---");
  Serial.print("Temperature: "); 
  Serial.print(temperature, 1); 
  Serial.print("°C");
  if (temperature > TEMP_THRESHOLD) Serial.print(" [HIGH]");
  Serial.println();
  
  Serial.print("Humidity: "); 
  Serial.print(humidity, 1); 
  Serial.print("%");
  if (humidity < HUMIDITY_THRESHOLD) Serial.print(" [LOW]");
  Serial.println();
  
  Serial.print("Soil Moisture: "); 
  Serial.print(soilMoisturePercent); 
  Serial.print("% (Raw: "); 
  Serial.print(soilMoisture); 
  Serial.print(")");
  if (soilMoisturePercent < SOIL_MOISTURE_THRESHOLD) Serial.print(" [DRY]");
  Serial.println();
  
  // Add soil sensor calibration info
  Serial.print("Soil Sensor Status: ");
  if (soilMoisture > 800) {
    Serial.println("Very Dry - Check sensor placement");
  } else if (soilMoisture > 600) {
    Serial.println("Dry");
  } else if (soilMoisture > 400) {
    Serial.println("Moist");
  } else {
    Serial.println("Very Wet");
  }
  
  Serial.print("Water Tank: "); 
  Serial.println(waterLevel ? "FULL" : "EMPTY");
  
  // Show pump status
  bool waterfallOn = digitalRead(RELAY_WATERFALL);
  bool rainOn = digitalRead(RELAY_RAIN);
  Serial.print("Pumps Status - Waterfall: ");
  Serial.print(waterfallOn ? "ON" : "OFF");
  Serial.print(", Rain: ");
  Serial.println(rainOn ? "ON" : "OFF");
  
  Serial.println("----------------------");
  Serial.println();
}

void automaticControl(unsigned long currentTime) {
  bool needsWater = false;
  String reason = "";
  
  // Skip temperature/humidity checks if DHT sensor is not working
  bool dhtWorking = !isnan(dht.readTemperature()) && !isnan(dht.readHumidity());
  
  // Check conditions for automatic watering
  if (soilMoisturePercent < SOIL_MOISTURE_THRESHOLD) {
    needsWater = true;
    reason = "Low soil moisture (" + String(soilMoisturePercent) + "%)";
  }
  else if (dhtWorking && temperature > TEMP_THRESHOLD && humidity < HUMIDITY_THRESHOLD) {
    needsWater = true;
    reason = "Hot and dry conditions (T:" + String(temperature, 1) + "°C, H:" + String(humidity, 1) + "%)";
  }
  else if (currentTime - lastAutoControl > AUTO_WATER_INTERVAL) {
    needsWater = true;
    reason = "Scheduled watering cycle (5 hours elapsed)";
    lastAutoControl = currentTime;
  }
  
  // If DHT is not working, rely more on soil moisture and scheduled watering
  if (!dhtWorking && humidity > HUMIDITY_THRESHOLD) {
    // Without temperature/humidity data, use more frequent watering schedule
    if (currentTime - lastAutoControl > 7200000) { // 2 hours instead of 5
      needsWater = true;
      reason = "Scheduled watering (DHT sensor offline - shorter interval)";
      lastAutoControl = currentTime;
    }
  }
  
  // Control pumps based on conditions and water availability
  if (needsWater && waterLevel) {
    // Activate both pumps for better coverage
    if (!digitalRead(RELAY_WATERFALL) || !digitalRead(RELAY_RAIN)) {
      Serial.println("*** WATERING ACTIVATED ***");
      Serial.println("Reason: " + reason);
      Serial.println("Activating waterfall and rain pumps...");
      Serial.println();
    }
    digitalWrite(RELAY_WATERFALL, HIGH);
    digitalWrite(RELAY_RAIN, HIGH);
  } 
  else if (needsWater && !waterLevel) {
    // Need water but tank is empty
    if (digitalRead(RELAY_WATERFALL) || digitalRead(RELAY_RAIN)) {
      Serial.println("*** WATERING NEEDED BUT TANK EMPTY ***");
      Serial.println("Reason: " + reason);
      Serial.println("Stopping pumps - refill water tank!");
      Serial.println();
    }
    digitalWrite(RELAY_WATERFALL, LOW);
    digitalWrite(RELAY_RAIN, LOW);
  }
  else {
    // Turn off pumps - conditions are good
    if (digitalRead(RELAY_WATERFALL) || digitalRead(RELAY_RAIN)) {
      Serial.println("*** WATERING STOPPED ***");
      Serial.println("Conditions are now adequate");
      Serial.println();
    }
    digitalWrite(RELAY_WATERFALL, LOW);
    digitalWrite(RELAY_RAIN, LOW);
  }
}

void handleWaterAlarm(unsigned long currentTime) {
  if (!waterLevel) {
    // Blink buzzer every 500ms when tank is empty
    if (currentTime - lastBuzzerToggle >= 500) {
      buzzerState = !buzzerState;
      digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
      lastBuzzerToggle = currentTime;
      
      // Print warning periodically
      if (buzzerState) {
        Serial.println("!!! WATER TANK EMPTY - REFILL IMMEDIATELY !!!");
      }
    }
    
    // Force all pumps off when tank is empty
    digitalWrite(RELAY_WATERFALL, LOW);
    digitalWrite(RELAY_RAIN, LOW);
  } else {
    // Turn off buzzer when tank has water
    digitalWrite(BUZZER_PIN, LOW);
    buzzerState = false;
  }
}