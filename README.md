# 🌿 Smart Tropical Garden - Automated Water Control System

This Arduino project is an **automated water control system** for plant care in a tropical environment. It monitors **temperature**, **humidity**, **soil moisture**, and **water level**, and automatically controls **rain and waterfall pumps** to irrigate plants. It also includes a **buzzer alarm** for low water levels to protect the system.

> Ideal for smart gardens, indoor greenhouses, or educational automation projects.

---

## 🔧 Features

✅ Monitors environmental parameters:  
- 🌡️ Temperature (via DHT22)  
- 💧 Humidity (via DHT22)  
- 🌱 Soil moisture (analog sensor)  
- 🚰 Water tank level (digital input)

✅ Controls actuators:  
- 🌊 Relay for waterfall pump  
- 🌧️ Relay for rain sprinkler  
- 🚨 Buzzer alarm when water is low

✅ Intelligent behavior:  
- Watering based on sensor thresholds  
- Scheduled watering every 5 hours  
- Automatic pump shutdown if water tank is empty  
- Real-time sensor data via Serial Monitor  

---

## Hardware Components

| Component             | Function                                |
|----------------------|------------------------------------------|
| Arduino UNO/Nano     | Main microcontroller                     |
| DHT22 Sensor         | Measures temperature and humidity        |
| Soil Moisture Sensor | Monitors soil condition                  |
| Water Level Sensor   | Detects if tank is empty (LOW = empty)   |
| Relay Module (x2)    | Controls waterfall and rain pumps        |
| Buzzer               | Alerts when the water tank is empty      |
| Jumper Wires         | Connections                              |
| Power Source         | 5V for Arduino and connected devices     |

---

## 🔌 Pin Connections

| Signal             | Arduino Pin |
|--------------------|-------------|
| DHT22 Data         | D2          |
| Soil Moisture      | A0          |
| Water Level Sensor | A1 (INPUT_PULLUP) |
| Relay - Waterfall  | D4          |
| Relay - Rain       | D5          |
| Buzzer             | D6          |

> 📝 Make sure the water level sensor outputs LOW when the tank is **empty**.

---

## 📊 Sensor Thresholds & Logic

| Condition                                  | Action                              |
|-------------------------------------------|-------------------------------------|
| Soil Moisture < 30%                       | Start watering                      |
| Temp > 30°C and Humidity < 90%            | Start watering                      |
| 5 Hours Elapsed Since Last Watering       | Start watering                      |
| Tank Empty (Sensor LOW)                   | Stop pumps, turn on buzzer          |
| Tank Full (Sensor HIGH)                   | Continue normal logic               |

---

## 🧠 Control Logic Flow

1. **Read sensors** every 2 seconds.
2. **Display sensor values** every 10 seconds on the Serial Monitor.
3. **Check for watering conditions:**
   - If soil is dry or it's hot & dry or interval expired → activate pumps.
   - If tank is empty → stop pumps and sound buzzer.
4. **Buzzer alarm** toggles every 500ms if water is low.
5. **Pumps** are turned off once conditions return to normal.

---

## 📟 Serial Output Example

```text
--- Sensor Readings ---
Temperature: 32.1°C [HIGH]
Humidity: 45.0% [LOW]
Soil Moisture: 25% (Raw: 750) [DRY]
Soil Sensor Status: Dry
Water Tank: FULL
Pumps Status - Waterfall: ON, Rain: ON
----------------------
```

## 🛠️ Getting Started

- Clone or download this project.
- Wire your components as shown above.
- Open the .ino file in the Arduino IDE.
- Install the required library:
- DHT Sensor Library
- Upload the sketch to your Arduino.
- Open the Serial Monitor at 9600 baud to view real-time logs.

##  Calibration Tips
- Adjust soil moisture thresholds according to your sensor model.
- Ensure your DHT22 has a 10K pull-up resistor on the data line.
- If using different relays or sensors, update pin numbers in the code.

## 📌 To-Do / Future Improvements
- Add LCD/TFT display for offline data visualization.
- Connect to Wi-Fi (ESP32/ESP8266) for remote monitoring.
- Log data to SD card or cloud platform.
- Add manual mode (buttons or touch interface).
- Create a mobile app or web dashboard for control.

## 📜 License
This project is open-source and free to modify and use for personal or academic purposes.

## 👤 Author
Eyad Qasim Raheem
Smart Systems Developer | IoT & Embedded Enthusiast
GitHub: @Eyad6789

## 🌟 Show Your Support
If this project helped you, feel free to ⭐️ star the repository and share your feedback!
