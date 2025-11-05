# Build Instructions

This guide will help you set up the development environment and build the Sony Watchman ESP32 Retro Media Player firmware.

## Prerequisites

### Required Software

1. **ESP-IDF v5.x** (Espressif IoT Development Framework)
   - Download from: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/
   - Follow the installation guide for your OS (Linux, macOS, or Windows)

2. **Git**
   - Required for cloning ESP-IDF and this project

3. **Python 3.8+**
   - Comes with ESP-IDF installation

### Hardware Requirements

- ESP32 development board (ESP32-WROOM-32 or ESP32-S3 recommended)
- 2" IPS LCD Display with ST7789VW driver (240x320 resolution)
- MicroSD card module (SPI interface)
- USB cable for programming and power
- (Optional) Logic analyzer for debugging

## Installation

### 1. Install ESP-IDF

Follow the official ESP-IDF installation guide:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

**Quick Install (macOS/Linux):**
```bash
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32
. ./export.sh
```

**Windows:**
Use the ESP-IDF Windows installer from the Espressif website.

### 2. Clone This Repository

```bash
git clone https://github.com/yourusername/WhoWatchesTheSonyWatchman.git
cd WhoWatchesTheSonyWatchman
```

### 3. Set Up ESP-IDF Environment

Every time you open a new terminal, activate the ESP-IDF environment:

```bash
# macOS/Linux
. ~/esp/esp-idf/export.sh

# Windows
%userprofile%\esp\esp-idf\export.bat
```

**Tip:** Add an alias to your shell profile for convenience:
```bash
alias get_idf='. ~/esp/esp-idf/export.sh'
```

## Configuration

### Pin Configuration

The default pin configuration is set in `components/display/include/display.h` and `components/storage/include/sd_card.h`.

**Display Pins (ST7789):**
- MOSI: GPIO 23
- CLK: GPIO 18
- CS: GPIO 5
- DC: GPIO 16
- RST: GPIO 4
- BL (Backlight): GPIO 15

**SD Card Pins (SPI):**
- MISO: GPIO 19
- MOSI: GPIO 23 (shared with display)
- CLK: GPIO 18 (shared with display)
- CS: GPIO 17

**To change pin assignments:**
1. Edit the `#define` statements in the header files
2. Or use `idf.py menuconfig` to add custom configuration options

### ESP32 Target Configuration

To configure for a specific ESP32 variant:

```bash
# For standard ESP32
idf.py set-target esp32

# For ESP32-S3 (more RAM, better performance)
idf.py set-target esp32s3
```

### Advanced Configuration

Open the configuration menu:

```bash
idf.py menuconfig
```

**Recommended settings:**
- Component config → ESP32-specific → CPU frequency: 240 MHz
- Component config → FreeRTOS → Tick rate: 1000 Hz
- Component config → FAT Filesystem support → Long filename support: Heap
- Compiler options → Optimization level: Optimize for performance (-O2)

## Building

### Full Build

```bash
idf.py build
```

This will:
1. Configure the build system
2. Compile all components
3. Link the final binary
4. Generate flash images

**Build output location:** `build/`

### Clean Build

If you encounter issues, perform a clean build:

```bash
idf.py fullclean
idf.py build
```

### Build Specific Components

To rebuild only specific components (faster iteration):

```bash
idf.py build
# Then make changes to a component
idf.py app  # Rebuilds only changed components
```

## Flashing

### Flash to ESP32

Connect your ESP32 via USB and flash:

```bash
idf.py -p /dev/ttyUSB0 flash
```

**Note:** Replace `/dev/ttyUSB0` with your actual port:
- Linux: `/dev/ttyUSB0` or `/dev/ttyACM0`
- macOS: `/dev/cu.usbserial-*` or `/dev/cu.SLAB_USBtoUART`
- Windows: `COM3`, `COM4`, etc.

**Auto-detect port:**
```bash
idf.py flash  # Will auto-detect port
```

### Flash and Monitor

Flash and immediately open serial monitor:

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

To exit monitor: Press `Ctrl+]`

### Erase Flash (Factory Reset)

To completely erase the flash:

```bash
idf.py -p /dev/ttyUSB0 erase-flash
```

## Monitoring and Debugging

### Serial Monitor

View serial output:

```bash
idf.py -p /dev/ttyUSB0 monitor
```

**Useful monitor features:**
- `Ctrl+T` → `Ctrl+H`: Help menu
- `Ctrl+T` → `Ctrl+P`: Toggle automatic reset when entering monitor
- `Ctrl+]`: Exit monitor

### View Logs

The firmware outputs detailed logs. Log levels:
- `E` (Error): Critical errors
- `W` (Warning): Warnings
- `I` (Info): General information
- `D` (Debug): Debug information
- `V` (Verbose): Very detailed debug info

### Memory Analysis

Check memory usage:

```bash
idf.py size
idf.py size-components
idf.py size-files
```

### Core Dump Analysis

If the ESP32 crashes, you can analyze core dumps:

```bash
idf.py -p /dev/ttyUSB0 monitor
# After crash, the core dump will be displayed
```

## Preparing SD Card Content

### Format SD Card

1. Format as FAT32 (exFAT not supported)
2. Use 32GB or smaller (for best compatibility)
3. Use Class 10 or UHS-1 for best performance

### Organize Video Files

Create this folder structure on the SD card:

```
/channels/
  ├── MST3K/
  │   ├── episode_01.avi
  │   ├── episode_02.avi
  │   └── ...
  ├── The_Twilight_Zone/
  │   ├── episode_01.avi
  │   └── ...
  └── ...
```

### Encoding Videos for ESP32

Use FFmpeg to encode videos in MJPEG format:

```bash
ffmpeg -i input.mp4 \
  -vf "scale=240:240:force_original_aspect_ratio=decrease,pad=240:240:(ow-iw)/2:(oh-ih)/2" \
  -r 15 \
  -q:v 8 \
  -vcodec mjpeg \
  -acodec libmp3lame \
  -ar 22050 \
  -ac 1 \
  -b:a 64k \
  output.avi
```

**Parameters explained:**
- `scale=240:240`: Resize to 240x240 (adjust for your display)
- `-r 15`: 15 frames per second (good balance for ESP32)
- `-q:v 8`: Quality 5-10 (lower = better quality, larger file)
- `-acodec libmp3lame`: MP3 audio
- `-ar 22050`: 22.05 kHz sample rate (lower CPU usage)
- `-ac 1`: Mono audio

**Test different quality settings:**
- High quality (larger files): `-q:v 5`
- Medium quality (balanced): `-q:v 8`
- Low quality (smaller files): `-q:v 10`

## Troubleshooting

### Build Errors

**"Command not found: idf.py"**
- ESP-IDF environment not activated
- Run `. ~/esp/esp-idf/export.sh`

**"No module named 'esp_idf_kconfig'"**
- ESP-IDF Python dependencies not installed
- Run: `cd ~/esp/esp-idf && ./install.sh`

**CMake errors**
- Try clean build: `idf.py fullclean && idf.py build`

### Flash Errors

**"Failed to connect to ESP32"**
- Check USB cable connection
- Press and hold BOOT button, then press RESET button
- Try different USB cable/port
- Check if device appears: `ls /dev/tty*`

**Permission denied (Linux)**
```bash
sudo usermod -a -G dialout $USER
# Log out and back in
```

### Runtime Issues

**Display shows nothing**
- Check display wiring and power
- Verify pin assignments match your hardware
- Check SPI initialization in logs

**SD card not detected**
- Format as FAT32
- Check CS pin connection
- Verify SD card is inserted correctly
- Try different SD card

**Video playback choppy**
- Reduce video quality (`-q:v 10`)
- Lower frame rate (`-r 10`)
- Check SD card speed (use Class 10)
- Verify SPI clock speed in code

**Out of memory errors**
- Reduce frame buffer size
- Close video file when not in use
- Check for memory leaks: monitor free heap

## Development Workflow

### Typical workflow:

1. **Code changes**
   ```bash
   # Edit files
   vim components/video/video_player.c
   ```

2. **Build**
   ```bash
   idf.py build
   ```

3. **Flash and monitor**
   ```bash
   idf.py -p /dev/ttyUSB0 flash monitor
   ```

4. **Test on hardware**

5. **Iterate**

### Quick rebuild and flash:

```bash
idf.py app-flash monitor
```

This only rebuilds changed components and is much faster.

## Additional Resources

- **ESP-IDF Programming Guide**: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/
- **ESP32 Technical Reference**: https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
- **FFmpeg Documentation**: https://ffmpeg.org/documentation.html
- **Project Documentation**: See `CLAUDE.MD` and `TASKS.MD` for detailed project info

## Getting Help

If you encounter issues:

1. Check the `TROUBLESHOOTING.md` file (if available)
2. Review ESP-IDF documentation
3. Check ESP32 forums: https://esp32.com/
4. Open an issue on GitHub

## Next Steps

After successful build and flash:

1. Insert formatted SD card with video files
2. Connect display hardware
3. Power on and watch the serial output
4. Enjoy your retro media player!

Refer to `TASKS.MD` for implementation roadmap and feature development.
