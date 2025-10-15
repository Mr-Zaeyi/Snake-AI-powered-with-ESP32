# Snake-AI-powered-with-ESP32
A Little project about a Snake game powered by an ESP32-C6-Dev-Kit-1C with a Dashboard and an AI Mode

# 🐍 Snake Game – ESP32-C6 with Real-Time Wi-Fi Dashboard

[![ESP32-C6](https://img.shields.io/badge/ESP32-C6-blue.svg)](https://www.espressif.com/en/products/socs/esp32-c6)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![Build](https://img.shields.io/badge/build-passing-brightgreen)
![Code Size](https://img.shields.io/github/languages/code-size/yourusername/snake-esp32-c6)
![Stars](https://img.shields.io/github/stars/yourusername/snake-esp32-c6?style=social)

---

## 🎮 Overview

**Snake ESP32-C6** is a modern remake of the classic *Snake Game* built for the **ESP32-C6 DevKitC-1**.  
It features:
- 🧠 Touch-based control via **Adafruit MPR121**
- 🖥️ Real-time display using **Adafruit ILI9341 TFT**
- 📶 A **Wi-Fi access point** and **live web dashboard**
- 🤖 Optional **AI mode** for self-playing snake
- 💾 Lightweight, fully self-contained firmware

---

## ⚙️ Features

| Feature | Description |
|----------|--------------|
| 🎨 Display | ILI9341 SPI TFT (320×240) |
| 🧭 Input | MPR121 Capacitive Touch Sensor |
| 🌐 Networking | ESP32-C6 in Wi-Fi AP mode |
| 🕹️ Game Modes | Solo / AI / Game Over menu |
| 📊 Web Dashboard | Real-time JSON API + Live HTML dashboard |
| 💾 Memory Footprint | Under 200 KB firmware |

---

## 📡 Live Dashboard

Once the board is powered on, it creates its own Wi-Fi network:

| SSID | Password |
|------|-----------|
| `YourWI-FI_Name` | `YourPassword` |

Connect to it and open the link given in your browser.

You’ll see:
- 🧩 Game Mode  
- 💀 Status (Alive / Game Over)  
- 🧮 Score and Snake Length  
- ⏱️ Live refresh every second  

📸 Example screenshot:

![Dashboard Screenshot](docs/dashboard_screenshot.png)

---

## 🧰 Hardware Setup

| Component | Pin | Description |
|------------|-----|-------------|
| TFT_CS | 21 | Chip Select |
| TFT_DC | 19 | Data/Command |
| TFT_RST | 20 | Reset |
| TFT_MOSI | 23 | SPI MOSI |
| TFT_MISO | 22 | SPI MISO |
| TFT_SCK | 18 | SPI Clock |
| MPR121 SDA | 5 | I2C Data |
| MPR121 SCL | 4 | I2C Clock |

📘 Wiring Diagram:  
`docs/wiring_diagram.png` *(to be added with Fritzing or hand sketch)*

---

## 🚀 Getting Started

1. **Clone this repository:**
   ```bash
   git clone https://github.com/yourusername/snake-esp32-c6.git
   cd snake-esp32-c6
