# Waveshare ESP32-C6 Mini Display Clock & Ambient HUD

A real-time digital clock and ambient lighting display designed for the **Waveshare ESP32-C6-LCD-1.47** development board. Features anti-aliased font rendering, dynamic glowing warm color gradients, synchronized underglow LED lighting, and USB serial synchronization.

<p align="center">
  <img src="preview.png" alt="Waveshare ESP32-C6 Mini Display Preview" width="480">
  <br>
  <em>Live 320×172 display output showing anti-aliased typography and warm glowing thermal gradient</em>
</p>
---

## Features

- **Anti-Aliased Typography**: Large, smooth 68 px time digits rendered with per-pixel alpha blending against pure pitch black (`#000000`).
- **Dynamic Warm Glow**: Real-time sinusoidal phase calculation creating a shifting orange-to-red thermal gradient across the clock digits.
- **Synchronized WS2812 Underglow**: The onboard addressable RGB LED pulses with the display's thermal color palette through the acrylic casing.
- **180° Display Flip**: Toggle orientation on the fly by pressing the tactile **BOOT** button (`GPIO 9`).
- **USB Serial Synchronization**: Synchronize time and date at 115200 baud over `/dev/ttyACM0`.
- **Modular ESP-IDF / PlatformIO Architecture**: Cleanly separated C modules for timekeeping, graphics, button debouncing, lighting effects, and hardware drivers.

---

## Hardware Specifications

| Component | Pin / Channel | Description |
| :--- | :--- | :--- |
| **MCU** | ESP32-C6FH8 | 32-bit RISC-V @ 160 MHz, 8MB Flash, Wi-Fi 6, BLE 5 |
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
src/
├── main.c           # Application lifecycle & 28 FPS frame render loop
├── clock.h / .c     # Timekeeping state, elapsed tick rollover & serial sync
├── button.h / .c    # GPIO 9 input task with hardware debounce (>50ms)
├── effects.h / .c   # HSV-to-RGB conversion & warm rainbow glow animation
├── ui.h / .c        # Anti-aliased font rasterization & scene composition
├── st7789.h / .c    # SPI2 display driver & 320x172 RGB565 framebuffer
├── ws2812.h / .c    # RMT peripheral driver for onboard addressable RGB LED
└── app_font_data.h  # Anti-aliased bitmap glyph tables (time & date)
```

---

## Getting Started

### Prerequisites & Environment Setup

The repository relies on [PlatformIO Core](https://docs.platformio.org/page/core/index.html) running in Python 3. Both the Python virtual environment (`venv/`) and the build artifact directory (`.pio/`) are deliberately excluded from git tracking via `.gitignore` and are initialized locally:

1. **Create the Python Virtual Environment (`venv/`)**:
   ```bash
   python3 -m venv venv
   ./venv/bin/pip install -r requirements.txt
   ```
   *(Alternatively, if you already have PlatformIO installed globally or through VSCode, you can directly run `pio run`.)*

2. **Automatic Toolchain & Build Provisioning (`.pio/`)**:
   You do **not** need to manually download the RISC-V toolchain or ESP-IDF. On the first build, PlatformIO inspects `platformio.ini` and automatically creates `.pio/`, downloading:
   - `toolchain-riscv32-esp` (RISC-V cross-compiler)
   - `framework-espidf` (Espressif IoT Development Framework)
   - `tool-ninja`, `tool-cmake`, and `tool-esptoolpy`
   - Builds the partition table, bootloader, and final firmware binary into `.pio/build/esp32-c6-devkitc-1/`.

### Build Firmware

To compile the project:

```bash
./venv/bin/pio run
```

### Flash to Board

Connect the board via USB (`/dev/ttyACM0`) and upload:

```bash
./venv/bin/pio run -t upload
```

### Serial Monitor

Open the serial monitor at 115200 baud to view telemetry and send time sync commands:

```bash
./venv/bin/pio device monitor
```

---

## Serial Time & Date Sync Protocol

You can update the display time and date by sending newline-terminated ASCII strings to `/dev/ttyACM0`:

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

---

## Linux Permissions & Serial Setup

If `/dev/ttyACM0` permission is denied:

- **Arch Linux**:
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

*(Log out and log back in for group changes to take effect).*

---

## License

This project is open-source and available under the [MIT License](LICENSE).
