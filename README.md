1. Project Overview

This project implements a Smart Reminder System using the Seeed Studio XIAO ESP32-C3, FreeRTOS multitasking, a real 0.91″ SSD1306 OLED display, a push-button reminder trigger, LED indicator, and buzzer alert.

The system allows the user to:
Set timed reminders (10 s, 30 s, 60 s, OFF)
Receive visual reminders on an OLED display
Receive audio/visual alerts via buzzer + LED
Simulate cloud push notifications (RainMaker-style event logger)
Cancel reminders using a long press
This system demonstrates real-time task scheduling, software timers, inter-task communication, and IoT-ready architecture suitable for cloud notification integration.

2. Key Features
   
✔ User-Controlled Reminders
Short press cycles through reminder intervals:
10 s → 30 s → 60 s → OFF → repeat
Long press cancels active reminders

✔ OLED Display Output
Shows real-time system messages:
Reminder SET
Reminder ALERT
Reminder CANCEL
Reminder OFF
Countdown information

✔ Alert System
LED ON during reminder alert
Buzzer beeps (200 ms ON / 200 ms OFF)
Alerts continue until long press cancel

✔ Cloud Notification Simulation
When a reminder triggers, a mock cloud event is logged:
[CLOUD] Reminder alert fired (10000 ms)

This mimics ESP RainMaker push-notification behaviour.

✔ FreeRTOS-Based Multitasking
Button detection task (debounced)
Reminder control task
Alert (LED + buzzer) task
OLED update functions
ESP timer callback

3. Hardware Used
Component	Purpose
XIAO ESP32-C3	Main microcontroller
SSD1306 0.91″ OLED	Visual output
Push button	Reminder trigger
LED	System + alert indicator
Active buzzer	Audio alert
Jumper wires & breadboard	Prototyping
4. Pin Mapping (Actual Pins Used)
Function	XIAO Pin	GPIO	Direction
LED	D2	GPIO4	Output
Buzzer	D3	GPIO5	Output
Button	D8	GPIO18	Input (pull-up)
OLED SDA	D4	GPIO6	I²C Data
OLED SCL	D5	GPIO7	I²C Clock
VCC	3V3	—	Power
GND	GND	—	Ground

Button Wiring:
Button connected between GPIO18 and GND.
Internal pull-up enabled.

5. Reminder System Logic
Reminder Options
10 seconds
30 seconds
60 seconds
OFF

Short Press
Moves to next option
Starts/removes reminder timer
Updates OLED display

Example OLED:
Reminder SET
Trigger in 30s
Long Press (> 1.2 s)

Cancels any running reminder:
Reminder CANCEL
Press to set again
Timer Expiry

When the selected time ends:
LED ON permanently
Buzzer beeps repeatedly
OLED displays alert
Cloud event logged
Reminder ALERT
Hold to cancel

6. FreeRTOS Tasks Used
   
  1. button_task
Polls button every 20 ms
Detects short vs long press
Sends events via queue

  2. control_task
Receives button events
Starts/stops esp_timer
Updates OLED messages
Manages reminder modes

  3. alert_task
While alert_active == true
LED ON
Buzzer beep pattern
Otherwise turns outputs OFF

  4. esp_timer callback
Triggers reminder expiration
Activates alert
Simulates cloud notification

7. OLED Display Integration
The project now uses a real I²C SSD1306 OLED.

OLED Functions:
oled_init();
oled_print("Reminder SET", "Trigger in 10s");
oled_print("Reminder ALERT", "Hold to cancel");
oled_print("Reminder CANCEL", "Press to set again");

OLED is updated instantly whenever:
A button changes mode
A reminder expires
A reminder is cancelled

8. Cloud Push Notification Simulation
The function:
cloud_push_event("Reminder alert fired");

simulates ESP-RainMaker-style events.
This satisfies the requirement:
“Smart Reminder System with Push Notifications”
even without using Wi-Fi.

9. Repository Structure
AES-project-Theeva/
│── firmware/
│   └── smart_reminder/
│       ├── main.c
│       ├── oled.c
│       ├── oled.h
│       ├── ssd1306.c
│       ├── ssd1306.h
│       ├── CMakeLists.txt
│       └── sdkconfig
│
└── README.md

10. How to Build & Flash Using ESP-IDF
Set target:
idf.py set-target esp32c3

Build:
idf.py build

Flash:
idf.py -p COMX flash

Monitor:
idf.py monitor

11. Demo Video Requirements

The video should show:

✔ System powering on
✔ OLED displaying: “AES Reminder – System Ready”
✔ Short press cycling 10s → 30s → 60s → OFF
✔ OLED updating accordingly
✔ Timer expiry → buzzer + LED + OLED ALERT
✔ Long press → reminder cancelled
✔ Explanation of simulated push notifications

12. Student Information

Name: Theevashini Thankaraj
Matric Number: 24008939
Course: OBE5083 – Sensors & Systems
Project Title: ESP32-C3 Smart Reminder System with Push Notifications
Semester: September 2025
