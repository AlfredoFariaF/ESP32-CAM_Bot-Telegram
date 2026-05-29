# ESP32-CAM — Automatic Photo Sender to Telegram

Firmware for the **ESP32-CAM (AI-Thinker)** module that captures photos at regular intervals and automatically sends them to a **Telegram** chat or group via bot.

---

## Overview

The device connects to a Wi-Fi network and, every **30 seconds** (configurable), takes a photo with the built-in camera and sends it directly to Telegram. It is ideal for simple remote monitoring applications such as:

- Security camera
- Environment monitoring (greenhouse, gate, pet)
- Periodic image logging

---

## Required Hardware

| Component | Description |
|---|---|
| ESP32-CAM | Module with OV2640 camera, AI-Thinker model |
| Power supply | Stable 5V (external supply recommended during operation) |

---

## Creating the Telegram Bot

### 1. Create the bot with BotFather

1. Open Telegram and search for **@BotFather**
2. Send the command `/newbot`
3. Choose a name and username for the bot
4. You will receive the **Bot Token** — save it

### 2. Get the Chat ID

1. Start a conversation with your bot (send any message)
2. Open the URL below in your browser, replacing `YOUR_TOKEN`:
   ```
   https://api.telegram.org/botYOUR_TOKEN/getUpdates
   ```
3. In the returned JSON, find the `"id"` field inside `"chat"` — that is your **Chat ID**

---

## Project Configuration

Edit the following constants at the top of the `.ino` file:

```cpp
// Wi-Fi credentials
const char* ssid     = "YOUR_NETWORK_NAME";
const char* password = "YOUR_NETWORK_PASSWORD";

// Telegram credentials
String botToken = "YOUR_TOKEN_HERE";
String chatId   = "YOUR_CHAT_ID_HERE";

// Send interval (in milliseconds)
unsigned long timerDelay = 30000; // 30 seconds

// Flash LED
bool useFlash = true; // set to 'false' to disable the flash
```

---

## How It Works

```
Initialization
    │
    ├─► Configure Flash pin (off)
    ├─► Connect to Wi-Fi
    └─► Initialize camera

Main loop (every 30 seconds)
    │
    ├─► Turn on flash (if enabled)
    ├─► Capture camera frame
    ├─► Turn off flash
    ├─► Open HTTPS connection to api.telegram.org
    ├─► Send image via POST multipart/form-data
    └─► Wait for and print response to Serial
```

Resolution is set automatically:

- **With PSRAM:** VGA (640×480), JPEG quality 10
- **Without PSRAM:** SVGA (800×600), JPEG quality 12

---

## Troubleshooting

| Symptom | Likely Cause | Solution |
|---|---|---|
| Camera fails to initialize | Insufficient power supply | Use a 5V source with at least 500mA |
| Upload fails | IO0 not connected to GND | Check the connection before uploading |
| Does not connect to Wi-Fi | Wrong credentials | Review `ssid` and `password` |
| Photo not received on Telegram | Invalid token or Chat ID | Double-check the configured values |
| Dark image | Flash disabled in low-light environment | Set `useFlash = true` |

---

## License

This project is available under the **MIT License**. Feel free to use, modify, and distribute it.
