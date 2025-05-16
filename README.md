💊 Smart Medicine Reminder (ESP32 + C + OLED)

This project is a Smart Medicine Reminder built on the ESP32 platform using pure C with ESP-IDF, without Arduino libraries. It helps patients or caretakers ensure timely medicine intake through visual, audio, and interactive alerts.

🧠 Features

⏰ Automatic reminders at regular intervals.

🔔 Buzzer and LED alert when it's time to take medicine.

🖐️ Push-button to confirm medicine taken.

⚠️ Missed dose warning if not confirmed in time.

🖥️ OLED display shows real-time reminder status.

🔧 Hardware Used
ESP32 Dev Board

SSD1306 128x64 OLED Display (I2C)

Push Button

LED

Passive Buzzer

📦 Software Stack
Language: C (not Arduino-based)

Framework: ESP-IDF

OLED Driver: SSD1306 (via I2C)

RTOS: FreeRTOS

🔁 How It Works
Every 15 seconds (simulated period), the device reminds the user to take medicine.

The OLED displays a message, and the buzzer + LED activate.

The user must press the button within 10 seconds.

If confirmed: alert stops, OLED shows confirmation.

If not: buzzer stops, and a "dose missed" warning is displayed.


LINK TO PROJECT - https://wokwi.com/projects/431114126491837441
