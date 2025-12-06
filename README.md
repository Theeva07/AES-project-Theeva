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

