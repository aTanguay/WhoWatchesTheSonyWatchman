# PlatformIO Setup Guide

This project is fully compatible with PlatformIO! You can use either the native ESP-IDF build system or PlatformIO.

## Why PlatformIO?

- **Easier Setup** - No need to manually install ESP-IDF
- **VS Code Integration** - Excellent IDE support
- **Library Management** - Simple dependency handling
- **Multi-Platform** - Works on Windows, macOS, Linux

## Installation

### 1. Install PlatformIO

**VS Code (Recommended):**
1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Install the "PlatformIO IDE" extension
3. Restart VS Code

**Command Line:**
```bash
pip install platformio
```

### 2. Open Project

**VS Code:**
- Open the project folder in VS Code
- PlatformIO will auto-detect the project

**Command Line:**
```bash
cd WhoWatchesTheSonyWatchman
pio run
```

## Building

### VS Code
1. Click PlatformIO icon in sidebar
2. Select "Build" under your environment

### Command Line

**Build for ESP32:**
```bash
pio run -e esp32dev
```

**Build for ESP32-S3:**
```bash
pio run -e esp32-s3-devkitc-1
```

**Clean build:**
```bash
pio run -t clean
pio run
```

## Flashing

### Auto-detect Port

**VS Code:** Click "Upload" in PlatformIO sidebar

**Command Line:**
```bash
pio run -t upload
```

### Specify Port

Edit `platformio.ini` or use command line:
```bash
# Linux/macOS
pio run -t upload --upload-port /dev/ttyUSB0

# macOS (UART adapter)
pio run -t upload --upload-port /dev/cu.usbserial-*

# Windows
pio run -t upload --upload-port COM3
```

## Serial Monitor

### VS Code
Click "Monitor" in PlatformIO sidebar

### Command Line
```bash
pio device monitor
```

**With auto-reset on upload:**
```bash
pio run -t upload -t monitor
```

## Configuration

### Change Target Board

Edit `platformio.ini` and change the default environment:
```ini
[platformio]
default_envs = esp32-s3-devkitc-1  # Change this
```

Or specify on command line:
```bash
pio run -e esp32-s3-devkitc-1
```

### Supported Boards

The project is configured for:
- `esp32dev` - Generic ESP32 (WROOM-32)
- `esp32-s3-devkitc-1` - ESP32-S3 DevKit

To add more boards, add new `[env:...]` sections in `platformio.ini`.

### Custom Upload Port

Edit `platformio.ini`:
```ini
upload_port = /dev/ttyUSB0  # Change to your port
monitor_port = /dev/ttyUSB0
```

### Increase Upload Speed

Already set to 921600 baud. If experiencing issues, reduce:
```ini
upload_speed = 460800  # or 115200
```

## ESP-IDF Configuration

### Using sdkconfig

The project uses `sdkconfig.defaults` for configuration. To customize:

1. **Generate full sdkconfig:**
   ```bash
   pio run -t menuconfig
   ```

2. **Edit settings** in the terminal UI

3. **Save and rebuild**

### Common Settings

Already configured in `sdkconfig.defaults`:
- CPU @ 240 MHz
- Optimized for performance
- FreeRTOS @ 1000 Hz
- WiFi/Bluetooth disabled (saves RAM)

## Component Structure

PlatformIO automatically recognizes the ESP-IDF component structure:

```
components/
├── display/
├── storage/
├── video/
├── audio/
├── input/
└── power/
```

No changes needed! Each component's `CMakeLists.txt` works with PlatformIO.

## Debugging

### Serial Debugging

Monitor with debug output:
```bash
pio device monitor --filter esp32_exception_decoder
```

### GDB Debugging (ESP-PROG required)

Add to `platformio.ini`:
```ini
debug_tool = esp-prog
debug_init_break = tbreak app_main
```

Then in VS Code: Run → Start Debugging (F5)

## Troubleshooting

### "Platform espressif32 not installed"
```bash
pio platform install espressif32
```

### "Component not found"
Make sure all `CMakeLists.txt` files are present in component folders.

### Build errors
Clean and rebuild:
```bash
pio run -t clean
pio run
```

### ESP-IDF version mismatch
PlatformIO uses its own ESP-IDF. To use specific version:
```ini
platform = espressif32@X.Y.Z
```

Check versions: https://registry.platformio.org/platforms/platformio/espressif32

## VS Code Tasks

PlatformIO adds tasks automatically. Use `Ctrl+Shift+P` (Cmd+Shift+P on macOS):
- "PlatformIO: Build"
- "PlatformIO: Upload"
- "PlatformIO: Clean"
- "PlatformIO: Monitor"

## Comparing with Native ESP-IDF

| Feature | Native ESP-IDF | PlatformIO |
|---------|---------------|------------|
| Setup | Manual install | Automatic |
| Build Command | `idf.py build` | `pio run` |
| Flash Command | `idf.py flash` | `pio run -t upload` |
| Monitor | `idf.py monitor` | `pio device monitor` |
| IDE | Any | VS Code (best) |
| Component System | Native | Compatible |

**Both work with this project!** Use whichever you prefer.

## Recommended Workflow

1. **Development:**
   - Use PlatformIO in VS Code
   - Quick iteration and debugging

2. **Production builds:**
   - Optionally use native `idf.py` for fine control
   - Both produce identical binaries

## Additional Resources

- [PlatformIO ESP-IDF Guide](https://docs.platformio.org/en/latest/frameworks/espidf.html)
- [PlatformIO ESP32 Platform](https://docs.platformio.org/en/latest/platforms/espressif32.html)
- [PlatformIO VS Code Extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)

## Quick Reference

```bash
# Build
pio run

# Upload
pio run -t upload

# Monitor
pio device monitor

# Upload + Monitor
pio run -t upload -t monitor

# Clean
pio run -t clean

# Menuconfig
pio run -t menuconfig

# List devices
pio device list
```

---

**You're all set!** PlatformIO makes ESP32 development much easier, especially if you're coming from Arduino.
