# ESP32-C3 Smart Reminder System with Push Notifications  
### Universiti Teknologi PETRONAS — AES Project  
### Developer: Theevashini Thankaraj

## Project Overview
The **ESP32-C3 Smart Reminder System** is a FreeRTOS-based embedded application designed to help users remember tasks using a **timer-driven alert system**.  
It operates with a single push button, LED indicator, buzzer alerts, and optional OLED display.

The user sets a reminder duration (10s, 30s, 60s, or OFF) using the button. When the timer expires, a loud alert is activated until the user long-presses the button to stop it.  
A **simulated cloud notification** demonstrates how the system could be extended into a real IoT application.

## Features
### ✔ FreeRTOS Multitasking  
- **Button task** — handles debounced input + short/long press detection  
- **Control task** — manages reminder logic, timer start/stop, OLED updates  
- **Alert task** — controls LED & buzzer in a non-blocking manner  

### ✔ Reminder Timer Using `esp_timer`  
- Non-blocking, accurate one-shot timer  
- No use of `vTaskDelay` for timing logic — professional embedded design  

### ✔ Button Input Logic  
- **Short press** → Cycles between reminder durations  
- **Long press** → Cancels active reminder & turns off alert  

### ✔ Alert Mechanism  
- LED flashes + buzzer beeps repeatedly  
- Alert continues until acknowledged  

### ✔ Optional OLED Display  
- Modular design — can be enabled anytime  
- Logs messages to console when OLED not connected  

### ✔ Simulated Cloud Notification  
When a reminder triggers, the system sends a **mock push notification** to demonstrate future IoT integration.

## System Architecture

### **FreeRTOS Task Diagram**
+---------------------+
| button_task |
| Reads button input |
| Debounces + short/ |
| long press logic |
+----------+----------+
|
v (queue)
+----------+----------+
| control_task |
| Updates reminder |
| Starts/stops timer |
| Controls OLED |
+----------+----------+
|
v
+----------+----------+
| esp_timer |
| One-shot callback |
| -> Sets alert flag |
+----------+----------+
|
v
+----------+----------+
| alert_task |
| LED + buzzer alert |
+---------------------+

## Repository Structure
/main
├── main.c # Main application logic
├── CMakeLists.txt # Build config for ESP-IDF
/build # Auto-generated
/sdkconfig # ESP-IDF project config
/README.md # Documentation

## 🔧 Hardware Requirements
- **Seeed Studio XIAO ESP32-C3**
- **1× Push Button**
- **1× LED**
- **1× Buzzer**
- (Optional) OLED display: **SSD1306 I2C**

### Wiring Summary:
| Component | ESP32-C3 Pin |
|----------|--------------|
| LED      | D2 (GPIO4)   |
| Buzzer   | D3 (GPIO5)   |
| Button   | D8 (GPIO18 → GND) |
| OLED (optional) | SDA=D4, SCL=D5 |

## Installation & Flashing
1. Install **ESP-IDF 5.x**
2. Configure using:
idf.py set-target esp32c3
idf.py menuconfig
3. Build & flash:
idf.py build flash monitor

## Simulated Cloud Logs
Example output when reminder triggers:
[CLOUD] Reminder alert fired (30000 ms)

## License
This project is developed for academic purposes under UTP AES course.

## Video Demo
A working demo of system behavior is included as part of the project submission.
