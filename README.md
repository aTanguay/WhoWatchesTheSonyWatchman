# Sony Watchman ESP32 Retro Media Player

A vintage Sony Watchman portable TV converted into a modern ESP32-powered media player that plays classic TV shows on a continuous loop.

![Project Status](https://img.shields.io/badge/Status-Phase%201-yellow)
![Build](https://img.shields.io/badge/Build-Not%20Tested-lightgrey)
![Platform](https://img.shields.io/badge/Platform-ESP32%20%7C%20ESP32--S3-blue)

## 📺 Project Overview

Transform a vintage 1980s Sony Watchman into a pocket-sized retro media player powered by ESP32. Turn the dial to switch between channels (TV shows), each playing episodes on continuous loop. Shake to shuffle episodes, or just enjoy the nostalgic viewing experience!

## ✨ Features

- **Multi-Channel System** - Each channel plays a different TV series
- **Rotary Encoder Control** - Authentic channel switching via the original tuning knob
- **MJPEG Video Playback** - Optimized for ESP32 hardware
- **I2S Audio** - High-quality sound through the original speaker
- **SD Card Storage** - 32GB+ for your entire video library
- **Battery Powered** - 2x 18650 batteries for 10+ hours of playback
- **Auto Sleep** - Power management for extended battery life
- **Episode Auto-Advance** - Seamless viewing experience

## 🚀 Current Status

**Phase 1 - Proof of Concept: 60% Complete**

### ✅ Completed
- ✅ ESP-IDF project structure
- ✅ Display driver (ST7789VW 240x320)
- ✅ SD card interface with channel management
- ✅ AVI/MJPEG file parser
- ✅ Video playback engine
- ✅ Audio player (I2S)
- ✅ Rotary encoder driver
- ✅ Power management system

### 🚧 In Progress
- 🚧 A/V synchronization
- 🚧 Main application integration
- 🚧 Hardware testing

### 📋 Next Steps
1. Integrate all components
2. Build and flash to hardware
3. Create test content
4. Optimize performance

See [STATUS.md](STATUS.md) for detailed progress tracking.

## 🛠 Hardware Components

| Component | Model | Purpose |
|-----------|-------|---------|
| Microcontroller | ESP32-WROOM-32 / ESP32-S3 | Main processor |
| Display | 2" IPS LCD (ST7789VW) | 240x320 color display |
| Storage | MicroSD Card (32GB+) | Video content |
| Audio | I2S DAC | Audio output |
| Input | KY-040 Rotary Encoder | Channel selection |
| Power | 2x 18650 Batteries | 6.0V - 8.4V |
| Case | Vintage Sony Watchman | Original housing |

### Pin Configuration

```
ESP32 Connections:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Display (SPI):
  MOSI   → GPIO 23
  CLK    → GPIO 18
  CS     → GPIO 5
  DC     → GPIO 16
  RST    → GPIO 4
  BL     → GPIO 15

SD Card (SPI):
  MISO   → GPIO 19
  MOSI   → GPIO 23 (shared)
  CLK    → GPIO 18 (shared)
  CS     → GPIO 17

Audio (I2S):
  BCLK   → GPIO 26
  WS     → GPIO 25
  DOUT   → GPIO 22

Encoder:
  CLK    → GPIO 32
  DT     → GPIO 33
  SW     → GPIO 27

Battery:
  VOLTAGE → GPIO 34 (ADC)
```

## 📦 Software Architecture

```
components/
├── display/          # ST7789 SPI display driver
├── storage/          # SD card + channel management
├── video/            # AVI parser + MJPEG decoder
├── audio/            # I2S audio player
├── input/            # Rotary encoder driver
└── power/            # Battery monitoring + sleep
```

**Framework:** ESP-IDF 5.x
**Video Format:** MJPEG in AVI container
**Audio Format:** PCM 16-bit @ 22.05kHz mono
**Target FPS:** 15-20 FPS
**Memory:** ~288KB heap usage (fits in ESP32)

## 🔧 Quick Start

### Prerequisites

- **Option 1 (Easier):** PlatformIO IDE (recommended for beginners)
- **Option 2:** ESP-IDF v5.x (for advanced users)
- ESP32 development board
- USB cable
- Python 3.8+

### Build Instructions

**PlatformIO (Recommended):**
```bash
# Install PlatformIO
pip install platformio

# Build
pio run

# Flash
pio run -t upload -t monitor
```

**Native ESP-IDF:**
```bash
# Set up ESP-IDF environment
. ~/esp/esp-idf/export.sh

# Build and flash
idf.py set-target esp32
idf.py build flash monitor
```

For detailed instructions:
- **PlatformIO:** see [PLATFORMIO.md](PLATFORMIO.md)
- **ESP-IDF:** see [BUILD.md](BUILD.md)

### Preparing SD Card & Content

**Hardware:** You'll need a MicroSD Card Module (SPI interface)
- See [SD_CARD_SETUP.md](SD_CARD_SETUP.md) for module recommendations and wiring

**Setup:**
1. **Format SD Card** - FAT32, 16-32GB, Class 10
2. **Create Directory Structure**
   ```
   /sdcard/
   └── channels/
       ├── MST3K/
       │   ├── episode_01.avi
       │   ├── episode_02.avi
       │   └── ...
       └── The_Twilight_Zone/
           ├── episode_01.avi
           └── ...
   ```

3. **Encode Videos**
   ```bash
   ffmpeg -i input.mp4 \
     -vf "scale=240:240:force_original_aspect_ratio=decrease,\
          pad=240:240:(ow-iw)/2:(oh-ih)/2" \
     -r 15 \
     -q:v 8 \
     -vcodec mjpeg \
     -acodec pcm_s16le \
     -ar 22050 \
     -ac 1 \
     output.avi
   ```

## 📚 Documentation

- **[PLATFORMIO.md](PLATFORMIO.md)** - PlatformIO setup and usage (recommended)
- **[BUILD.md](BUILD.md)** - Native ESP-IDF build instructions
- **[SD_CARD_SETUP.md](SD_CARD_SETUP.md)** - SD card module selection and wiring
- **[STATUS.md](STATUS.md)** - Current development status
- **[PROJECTPLAN.MD](PROJECTPLAN.MD)** - Project vision and architecture
- **[TASKS.MD](TASKS.MD)** - Detailed task breakdown (156 tasks)
- **[CLAUDE.MD](CLAUDE.MD)** - Development guidelines for AI assistance

## 🎯 Success Criteria

- [x] Video plays at 15+ FPS
- [ ] Audio synchronized within 100ms
- [ ] Channel switching under 2 seconds
- [ ] Battery life exceeds 10 hours
- [ ] Fits in original Watchman case
- [ ] All controls functional

## 🗺 Roadmap

### Phase 1: Proof of Concept ✅ 60%
- Core video/audio playback
- Basic hardware drivers
- SD card content management

### Phase 2: Core Features 🔄 Next
- A/V synchronization
- State persistence
- Episode management

### Phase 3: Enhanced Features
- Shake detection
- UI overlays
- Battery optimization

### Phase 4: Physical Integration
- Case retrofitting
- Final assembly
- Polish and testing

### Phase 5: Content & Polish
- Video encoding pipeline
- Content library creation
- Documentation

## 🤝 Contributing

This is a personal hobby project, but suggestions are welcome! Please refer to:
- Task tracking in [TASKS.MD](TASKS.MD)
- Architecture guidelines in [CLAUDE.MD](CLAUDE.MD)

## 📝 License

This project is open source for educational and hobby purposes.

## 🙏 Acknowledgments

- Espressif for the amazing ESP32 platform
- The open-source community for ESP-IDF and libraries
- Vintage electronics enthusiasts keeping retro tech alive

## 📧 Contact

Have questions or want to share your own build? Open an issue on GitHub!

---

**Last Updated:** 2025-11-05
**ESP-IDF Version:** 5.x
**Hardware:** ESP32-WROOM-32 / ESP32-S3
