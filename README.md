# ESP32S3CAMERA: AI Camera with LVGL, OpenAI, and Imgur

## Overview

**ESP32S3CAMERA** is a smart camera project for the ESP32-S3 microcontroller, featuring:
- Live camera preview and image capture
- Touchscreen and physical button UI (LVGL)
- Image upload to Imgur
- AI-powered image analysis via OpenAI API (GPT-4o)
- On-device QR code generation for sharing image links
- Power-saving deep sleep with wake-on-button

This project is ideal for embedded AI, IoT, and smart display applications.

---

## Features
- **Live Camera Feed:** Real-time preview on LCD
- **Touch & Button UI:** Capture images via touchscreen or physical button
- **Prompt Selection:** Choose from multiple AI prompts (scene description, object ID, OCR, food check, etc.)
- **Imgur Upload:** Captured images are uploaded to Imgur for easy sharing
- **OpenAI Integration:** Analyze images using GPT-4o with selectable prompts
- **QR Code Sharing:** Display a QR code for the Imgur image link
- **Deep Sleep:** Power-saving mode with wake-up on BOOT button
- **WiFi Watchdog:** Automatic reconnection for robust operation

---

## Hardware Requirements
- **ESP32-S3 Dev Board** (with PSRAM recommended)
- **OV2640 (or compatible) Camera Module**
- **ST7789 (or compatible) LCD Display**
- **CST816 (or compatible) Touch Panel**
- **Physical Buttons:** BOOT (GPIO0), Capture (GPIO20)
- **Wiring:** See pinout below

## Pinout Summary
| Function         | ESP32-S3 Pin |
|------------------|-------------|
| Camera D0-D7     | 12,13,15,11,14,10,7,2 |
| Camera XCLK      | 8           |
| Camera PCLK      | 9           |
| Camera VSYNC     | 6           |
| Camera HREF      | 4           |
| Camera SIOD/SIOC | 21,16       |
| Camera PWDN      | 17          |
| LCD SCLK/MOSI    | 39,38       |
| LCD MISO         | 40          |
| LCD DC/CS/BL     | 42,45,1     |
| LCD RST          | -1 (unused) |
| Touch SDA/SCL    | 48,47       |
| BOOT Button      | 0           |
| Capture Button   | 20          |

---

## Software Requirements
- **Arduino IDE** (latest, with ESP32 board support)
- **ESP32 Board Package** ([Install instructions](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html))
- **Required Libraries:**
  - [LVGL](https://github.com/lvgl/lvgl) (v8+)
  - [Arduino_GFX](https://github.com/moononournation/Arduino_GFX)
  - [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
  - [WiFi](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi)
  - [HTTPClient](https://github.com/espressif/arduino-esp32/tree/master/libraries/HTTPClient)
  - [WiFiClientSecure](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiClientSecure)
  - [esp_camera](https://github.com/espressif/esp32-camera)
  - [bsp_cst816](https://github.com/moononournation/Arduino_GFX_Touch)
  - [qrcode](included)

---

## Setup Instructions

### 1. Install Arduino IDE and ESP32 Board Support
- Open Arduino IDE > Preferences > Add board URL:
  `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- Tools > Board > Boards Manager > Search "ESP32" > Install

### 2. Install Required Libraries
- Tools > Manage Libraries > Search and install:
  - LVGL
  - Arduino_GFX
  - ArduinoJson
  - WiFi, HTTPClient, WiFiClientSecure (should be included with ESP32 package)
  - esp_camera (may require manual install)
  - bsp_cst816 (for touch panel)

### 3. Clone or Download this Repository
- Place all files in your Arduino sketch folder
- Ensure `qrcode.h` and `qrcode.c` are present in the project directory

### 4. Configure Your Credentials
- Open `ESP32S3CAMERA.ino`
- Set your WiFi SSID and password:
  ```cpp
  const char* ssid = "YOUR_WIFI_SSID";
  const char* password = "YOUR_WIFI_PASSWORD";
  ```
- Set your OpenAI API key (get one from https://platform.openai.com/):
  ```cpp
  const char* openai_api_key = "sk-...";
  ```
- (Optional) Set your Imgur Client ID (get one from https://api.imgur.com/oauth2/addclient):
  ```cpp
  const char* IMGUR_CLIENT_ID = "...";
  ```

**WARNING:** _Never commit your API keys or WiFi credentials to public repositories!_

### 5. Select the Correct Board and Port
- Tools > Board: "ESP32S3 Dev Module"
- Set upload speed, flash size, PSRAM, etc. as per your board specs

### 6. Upload and Run
- Connect your ESP32-S3 via USB
- Click Upload in Arduino IDE
- Open Serial Monitor for debug output

---

## Usage
- On boot, the device connects to WiFi and shows a live camera feed
- Use the touchscreen or the physical capture button to take a photo
- Select an AI prompt from the dropdown (e.g., "Describe Scene", "Read Text")
- The captured image is uploaded to Imgur, and the link is sent to OpenAI for analysis
- The AI response is displayed on the LCD, with a QR code for the image link
- Press "Close" to return to live view
- Press the BOOT button to enter deep sleep (press again to wake)

---

## File Structure
- `ESP32S3CAMERA.ino` — Main application
- `qrcode.h` / `qrcode.c` — QR code generation (MIT License)
- `lv_conf.h` — LVGL configuration
- `apidocs.imgur.com.pem` — Imgur root certificate
- `backup/` — Backup copies of the main sketch

---

## Credits & Acknowledgements
- [LVGL](https://lvgl.io/) — Embedded GUI library
- [OpenAI](https://platform.openai.com/) — AI image analysis
- [Imgur](https://imgur.com/) — Image hosting
- [Arduino_GFX](https://github.com/moononournation/Arduino_GFX) — Display driver
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) — JSON parsing
- [esp_camera](https://github.com/espressif/esp32-camera) — Camera driver
- [bsp_cst816](https://github.com/moononournation/Arduino_GFX_Touch) — Touch panel
- QR code library by Richard Moore (MIT License)

---

## License

This project is licensed under the MIT License. See `qrcode.h` for details.

---

## Disclaimer
- This project is for educational and prototyping use. Use at your own risk.
- API keys and credentials are sensitive—**do not share them publicly**. 