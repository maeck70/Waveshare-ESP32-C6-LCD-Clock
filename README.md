# Waveshare ESP32-C6 Mini Display Clock & Ambient HUD

A real-time digital clock and ambient lighting display designed for the **Waveshare ESP32-C6-LCD-1.47** development board. Features anti-aliased font rendering, dynamic glowing warm color gradients, synchronized underglow LED lighting, Wi-Fi 6 SNTP internet time synchronization, and USB serial fallback synchronization.

<p align="center">
  <img src="preview.png" alt="Waveshare ESP32-C6 Mini Display Preview" width="480">
  <br>
  <em>Live 320×172 display output showing anti-aliased typography and warm glowing thermal gradient</em>
</p>

---

## Features

- **Internet Time Synchronization (SNTP & Wi-Fi 6)**: Connects to your local Wi-Fi network and synchronizes atomic clock time using SNTP (`pool.ntp.org` / Google NTP). Automatically applies local timezones and Daylight Saving Time (DST).
- **Private Credentials via `.env`**: Wi-Fi SSID, password, and timezone are stored in a local, git-ignored `.env` file and processed automatically at build time. No credentials are ever committed to git.
- **Anti-Aliased Typography**: Large, smooth 68 px time digits and 28 px date glyphs rendered with per-pixel alpha blending against pure pitch black (`#000000`).
- **75° Angled Rainbow Glow Wave**: Continuous, ultra-smooth red-to-yellowish-orange thermal gradient traveling across the complete time block at an exact 75-degree angle (calculated via optimized 2D integer projection and precomputed cosine palette, eliminating sharp banding).
- **Synchronized WS2812 Underglow**: The onboard addressable RGB LED (`GPIO 8`) pulses with the display's thermal color palette through the acrylic casing.
- **180° Display Flip**: Toggle orientation on the fly by pressing the tactile **BOOT** button (`GPIO 9`).
- **USB Serial Synchronization**: Fallback instant time/date sync at 115200 baud over `/dev/ttyACM0`.
- **Expanded 8MB Flash Partition Layout**: Custom partition table (`partitions.csv`) allocating 3MB for application firmware with massive headroom.
- **Modular ESP-IDF / PlatformIO Architecture**: Cleanly separated C modules for timekeeping, networking, graphics, button debouncing, lighting effects, and hardware drivers.

---

## Hardware Specifications

| Component | Pin / Channel | Description |
| :--- | :--- | :--- |
| **MCU** | ESP32-C6FH8 | 32-bit RISC-V @ 160 MHz, 8MB Flash, Wi-Fi 6 (802.11ax/b/g/n), BLE 5 |
| **LCD Controller** | ST7789 | 1.47" IPS LCD (320 × 172 landscape, 40 MHz SPI) |
| **LCD MOSI** | `GPIO 6` | SPI2 Data Out |
| **LCD SCLK** | `GPIO 7` | SPI2 Clock Out |
| **LCD CS** | `GPIO 14` | Chip Select (Active Low) |
| **LCD DC** | `GPIO 15` | Data / Command Selection |
| **LCD RST** | `GPIO 21` | Hardware Reset (Active Low) |
| **LCD Backlight** | `GPIO 22` | Backlight Enable |
| **RGB LED** | `GPIO 8` | Onboard WS2812 driven via ESP32-C6 RMT TX |
| **BOOT Button** | `GPIO 9` | Tactile Button (Active Low, internal pull-up) |
| **USB Interface** | `/dev/ttyACM0` | Native USB CDC / JTAG Serial (VID:PID `303a:1001`) |

---

## Architecture Overview

```
├── .env                 # Local Wi-Fi credentials & timezone (git-ignored)
├── .env.example         # Template configuration for environment settings
├── partitions.csv       # Custom 8MB Flash layout (3MB factory app partition)
├── platformio.ini       # Build configuration, ESP-IDF framework & pre-scripts
├── scripts/
│   └── load_env.py      # Pre-build script converting .env into src/wifi_config.h
├── src/
│   ├── main.c           # Application lifecycle & 28 FPS frame render loop
│   ├── wifi_time.h / .c # Wi-Fi station stack, event handling & SNTP client
│   ├── clock.h / .c     # Timekeeping state, elapsed tick rollover & serial sync
│   ├── button.h / .c    # GPIO 9 input task with hardware debounce (>50ms)
│   ├── effects.h / .c   # HSV-to-RGB conversion & warm rainbow glow animation
│   ├── ui.h / .c        # Anti-aliased font rasterization & scene composition
│   ├── st7789.h / .c    # SPI2 display driver & 320x172 RGB565 framebuffer
│   ├── ws2812.h / .c    # RMT peripheral driver for onboard addressable RGB LED
│   └── app_font_data.h  # Anti-aliased bitmap glyph tables (time & date)
```

---

## Wi-Fi & Timezone Configuration

1. **Create your `.env` file**:
   ```bash
   cp .env.example .env
   ```

2. **Edit `.env` with your network credentials and timezone**:
   ```env
   # Wi-Fi Credentials
   WIFI_SSID=Your_WiFi_Network
   WIFI_PASSWORD=Your_WiFi_Password

   # Timezone (POSIX format)
   TIMEZONE=PST8PDT,M3.2.0,M11.1.0

   # Optional NTP Server (default: pool.ntp.org)
   NTP_SERVER=pool.ntp.org
   ```

### Common POSIX Timezones

| Region | Timezone String |
| :--- | :--- |
| **US Pacific (PDT / PST)** | `PST8PDT,M3.2.0,M11.1.0` |
| **US Mountain (MDT / MST)** | `MST7MDT,M3.2.0,M11.1.0` |
| **US Central (CDT / CST)** | `CST6CDT,M3.2.0,M11.1.0` |
| **US Eastern (EDT / EST)** | `EST5EDT,M3.2.0,M11.1.0` |
| **UTC (Universal Time)** | `UTC0` |
| **UK / London (GMT / BST)** | `GMT0BST,M3.5.0/1,M10.5.0` |
| **Central Europe (CET / CEST)** | `CET-1CEST,M3.5.0,M10.5.0/3` |
| **Japan (JST)** | `JST-9` |
| **Australia Eastern (AEST / AEDT)** | `AEST-10AEDT,M10.1.0,M4.1.0/3` |

When built, `scripts/load_env.py` automatically compiles your credentials into `src/wifi_config.h`. If `WIFI_SSID` is left empty, the clock will run offline using internal hardware timers and serial synchronization.

---

## Getting Started

### Prerequisites & Environment Setup

1. **Set up Python Virtual Environment**:
   ```bash
   python3 -m venv venv
   ./venv/bin/pip install platformio esptool pillow
   ```

2. **Build Firmware**:
   ```bash
   ./venv/bin/pio run
   ```

3. **Flash to Board**:
   Connect the device via USB (`/dev/ttyACM0`) and upload:
   ```bash
   ./venv/bin/pio run -t upload
   ```

4. **Serial Monitor**:
   View Wi-Fi connection, NTP sync logs, and telemetry at 115200 baud:
   ```bash
   ./venv/bin/pio device monitor
   ```

---

## Serial Time & Date Sync Protocol (Fallback)

If operating offline without Wi-Fi, you can update the time and date directly over `/dev/ttyACM0`:

### Set Time
```bash
echo -e "14:30:00\n" > /dev/ttyACM0
```
*Format: `HH:MM:SS\n` (24-hour clock)*

### Set Date
```bash
echo -e "DATE=September 4, 2026\n" > /dev/ttyACM0
```
*Format: `DATE=<date_str>\n`*

### One-Line Host Time Sync
```bash
./venv/bin/python3 -c "
import serial, datetime, time
with serial.Serial('/dev/ttyACM0', 115200, timeout=1) as ser:
    now = datetime.datetime.now()
    ser.write(now.strftime('%H:%M:%S\n').encode())
    time.sleep(0.1)
    ser.write(now.strftime('DATE=%B %-d, %Y\n').encode())
    print('Synced device to host PC time.')
"
```

---

## Linux Permissions & Serial Setup

If `/dev/ttyACM0` permission is denied:

- **Arch Linux / Manjaro**:
  ```bash
  sudo usermod -aG uucp $USER
  ```
- **Ubuntu / Debian**:
  ```bash
  sudo usermod -aG dialout $USER
  ```
- **Temporary Access**:
  ```bash
  sudo chmod 666 /dev/ttyACM0
  ```

---

## License

This project is open-source and available under the [MIT License](LICENSE).
