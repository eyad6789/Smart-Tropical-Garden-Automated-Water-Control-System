#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

// TFT Display
MCUFRIEND_kbv tft;

// Touch Screen pins - ADJUST THESE TO MATCH YOUR WIRING
#define YP A2  // Y+ pin (usually connects to analog pin)
#define XM A3  // X- pin (usually connects to analog pin)  
#define YM 8   // Y- pin (usually connects to digital pin)
#define XP 9   // X+ pin (usually connects to digital pin)

// Colors
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0

// Touch reading variables
int touchX, touchY, touchZ;
bool touchDetected = false;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Touch Screen Test Starting..."));
  
  // Initialize TFT
  uint16_t ID = tft.readID();
  if (ID == 0xFFFF) {
    Serial.println(F("TFT not found! Using default ID."));
    ID = 0x9486;
  }
  Serial.print(F("TFT ID: 0x"));
  Serial.println(ID, HEX);
  
  tft.begin(ID);
  tft.setRotation(1); // Landscape mode
  
  showTestScreen();
}

void loop() {
  readTouchRaw();
  
  if (touchDetected) {
    // Show touch info on screen
    updateTouchDisplay();
    
    // Print to serial for debugging
    Serial.print(F("Raw Touch - X: "));
    Serial.print(touchX);
    Serial.print(F(", Y: "));
    Serial.print(touchY);
    Serial.print(F(", Pressure: "));
    Serial.println(touchZ);
    
    delay(100); // Slow down updates
  }
  
  delay(50);
}

void showTestScreen() {
  tft.fillScreen(BLACK);
  
  // Title
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.setCursor(50, 20);
  tft.print(F("TOUCH TEST"));
  
  // Instructions
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 50);
  tft.print(F("Touch anywhere on the screen"));
  tft.setCursor(10, 65);
  tft.print(F("Check Serial Monitor for raw values"));
  
  // Touch info area
  tft.setTextColor(GREEN);
  tft.setCursor(10, 90);
  tft.print(F("Touch Info:"));
  
  // Draw test targets
  tft.drawCircle(50, 150, 20, RED);
  tft.setCursor(25, 175);
  tft.print(F("Touch"));
  
  tft.drawCircle(270, 150, 20, BLUE);
  tft.setCursor(245, 175);
  tft.print(F("Touch"));
  
  tft.drawCircle(160, 200, 20, GREEN);
  tft.setCursor(135, 225);
  tft.print(F("Touch"));
}

void readTouchRaw() {
  // Method 1: Standard resistive touch reading
  touchDetected = false;
  
  // Read X coordinate
  pinMode(YP, INPUT);
  pinMode(YM, INPUT);
  pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  
  digitalWrite(XP, HIGH);
  digitalWrite(XM, LOW);
  delay(1); // Small delay for stability
  
  int xRaw = analogRead(YP);
  
  // Read Y coordinate
  pinMode(XP, INPUT);
  pinMode(XM, INPUT);
  pinMode(YP, OUTPUT);
  pinMode(YM, OUTPUT);
  
  digitalWrite(YP, HIGH);
  digitalWrite(YM, LOW);
  delay(1); // Small delay for stability
  
  int yRaw = analogRead(XM);
  
  // Read pressure (Z)
  pinMode(XP, OUTPUT);
  pinMode(YM, OUTPUT);
  pinMode(XM, INPUT);
  pinMode(YP, INPUT);
  
  digitalWrite(XP, LOW);
  digitalWrite(YM, HIGH);
  delay(1); // Small delay for stability
  
  int z1 = analogRead(XM);
  int z2 = analogRead(YP);
  
  // Calculate pressure
  int pressure = 1023 - abs(z2 - z1);
  
  // Reset pins for TFT
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);
  digitalWrite(YP, LOW);
  digitalWrite(XM, LOW);
  
  // Check if touch is detected (adjust these thresholds as needed)
  if (pressure > 50 && pressure < 950) {  // Very loose pressure thresholds
    touchX = xRaw;
    touchY = yRaw;
    touchZ = pressure;
    touchDetected = true;
  }
  
  // Alternative method if the above doesn't work
  if (!touchDetected) {
    readTouchAlternative();
  }
}

void readTouchAlternative() {
  // Alternative touch reading method
  pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, INPUT_PULLUP);
  pinMode(YM, INPUT_PULLUP);
  
  digitalWrite(XP, HIGH);
  digitalWrite(XM, LOW);
  
  int x = analogRead(YP);
  
  pinMode(YP, OUTPUT);
  pinMode(YM, OUTPUT);
  pinMode(XP, INPUT_PULLUP);
  pinMode(XM, INPUT_PULLUP);
  
  digitalWrite(YP, HIGH);
  digitalWrite(YM, LOW);
  
  int y = analogRead(XM);
  
  // Simple pressure detection
  if (x > 100 && x < 900 && y > 100 && y < 900) {
    touchX = x;
    touchY = y;
    touchZ = 500; // Dummy pressure value
    touchDetected = true;
  }
  
  // Reset pins
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);
  digitalWrite(YP, LOW);
  digitalWrite(XM, LOW);
}

void updateTouchDisplay() {
  // Clear previous touch info
  tft.fillRect(10, 105, 300, 30, BLACK);
  
  // Display current touch values
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 105);
  tft.print(F("X: "));
  tft.print(touchX);
  tft.print(F("  Y: "));
  tft.print(touchY);
  tft.print(F("  P: "));
  tft.print(touchZ);
  
  // Map to screen coordinates (rough estimate)
  int screenX = map(touchX, 100, 900, 0, 320);
  int screenY = map(touchY, 100, 900, 0, 240);
  
  tft.setCursor(10, 120);
  tft.print(F("Screen X: "));
  tft.print(screenX);
  tft.print(F("  Y: "));
  tft.print(screenY);
  
  // Draw a small dot where touch is detected (approximately)
  screenX = constrain(screenX, 5, 315);
  screenY = constrain(screenY, 5, 235);
  
  tft.fillCircle(screenX, screenY, 3, YELLOW);
  delay(50);
  tft.fillCircle(screenX, screenY, 3, BLACK); // Erase it
}