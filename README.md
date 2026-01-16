# ESP32-C3 HM-10 Compatible BLE UART Emulator

This project turns an **ESP32-C3** into a **drop-in replacement for the HM-10 BLE module**.
It emulates the HM-10 BLE UART behavior and is compatible with **existing Android applications**
that were originally designed to work with HM-10 — **no app changes required**.

The firmware provides a **binary-transparent BLE ↔ UART bridge** with HM-10 compatible UUIDs,
AT commands, and stable operation without echo loops.

---

## ✨ Features

- ✅ HM-10 compatible BLE service and characteristic (FFE0 / FFE1)
- ✅ Binary-transparent BLE ↔ UART bridge
- ✅ Compatible with legacy Android apps made for HM-10
- ✅ UART packet buffering with timeout (single BLE notify per packet)
- ✅ AT commands support (`AT`, `AT+NAME?`, `AT+NAME=...`)
- ✅ Settings stored in NVS (Preferences)
- ✅ Factory flashing via `esptool.exe`
- ✅ Fast firmware update using `.bin` files

---

## 📡 BLE Profile (HM-10 Compatible)

| Item | Value |
|----|----|
| Service UUID | `0000FFE0-0000-1000-8000-00805F9B34FB` |
| Characteristic UUID | `0000FFE1-0000-1000-8000-00805F9B34FB` |
| Properties | WRITE, NOTIFY |
| Descriptor | CCCD (0x2902) |

> CCCD is explicitly added to ensure compatibility with legacy Android applications.

---

## 🔌 UART Configuration

| Parameter | Value |
|--------|------|
| Baud rate | 9600 (default, HM-10 compatible) |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| Flow control | None |

### Pin mapping (ESP32-C3)

| ESP32-C3 | Function |
|--------|---------|
| GPIO21 | UART TX |
| GPIO20 | UART RX |
| GND | GND |
| 3.3V | VCC |

---

## 🖼️ Hardware Comparison

*(Add images here)*

- ESP32-C3 module
- HM-10 module
- Pin mapping comparison

---

## 🛠 Development Environment

This project is developed and tested with the following tools:

### Arduino IDE

- **Arduino IDE 1.8.19**
- Chosen for its stable build system and reliable incremental compilation
- Recommended over Arduino IDE 2.x for large ESP32 projects

### ESP32 Arduino Core

- **Espressif ESP32 Arduino Core v2.0.18**
- Fully supports ESP32-C3
- Stable BLE stack behavior
- Compatible with this project without patches

#### Board Manager URL

https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json


---

## 📦 Firmware Structure

- `BLE ↔ UART` is **binary-transparent**
- No `\r` / `\n` added automatically
- UART data is buffered and sent as a single BLE notification
- Echo loops are avoided
- AT commands are processed only for ASCII input

---

## 🧪 Tested With

- Android HM-10 compatible apps (no modification)
- nRF Connect
- External MCU via UART
- ESP32-C3 USB CDC

---

## 🔥 Flashing (Factory / Production)

###  Files

bootloader.bin
partitions.bin
firmware_hm10_esp32c3.bin
esptool.exe
flash.bat


### Factory flash (new module)

flash.bat

Full erase + bootloader + partitions + application

Log saved to flash_log.txt
