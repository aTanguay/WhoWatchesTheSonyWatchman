# ESP32 Retro Media Player - Sony Watchman Revival

**Project Status**: ✅ **Software Development Complete** | ⏳ Awaiting Hardware Delivery

## Project Overview

Transform a vintage 1980s Sony Watchman portable TV into a modern ESP32-powered mini media player that plays classic TV shows on a continuous loop. Each "channel" represents a different TV series, creating a nostalgic viewing experience with modern technology.

### Current Progress
- **Software**: 100% complete - All ESP32 firmware implemented and ready to flash
- **Documentation**: Complete guides for wiring, video encoding, and assembly
- **Hardware**: Awaiting component delivery
- **Testing**: Pending hardware arrival
- **Physical Integration**: Not yet started

## Vision

A pocket-sized retro TV that continuously plays your favorite classic shows. Turn the dial to switch between channels (TV series), and shake or press a button to jump to a random episode. The device maintains the authentic look and feel of the original Sony Watchman while running entirely on modern hardware.

## Core Features

### Hardware
- **Microcontroller**: ESP32 (dual-core, WiFi/BT capable)
- **Display**: 1.7" IPS LCD (replacing original CRT)
- **Input**: Rotary encoder for channel selection
- **Episode Selection**: Accelerometer (shake detection) or physical button
- **Power**: Dual 18650 lithium batteries with charging circuit
- **Audio**: Original internal speaker (driven by ESP32 DAC/I2S)
- **Storage**: MicroSD card (32GB+) for video content

### Software Features
- Multiple "channels" - each playing episodes from one TV series
- Smooth channel switching via rotary encoder
- Random episode selection on shake or button press
- Video format: MJPEG or optimized H.264 chunks
- Resume playback from last position
- Low-power sleep mode when idle
- Battery level indicator

## Technical Specifications

### Display
- Size: 2.0" diagonal
- Resolution: 240x320 (ST7789 IPS LCD) **✅ Implemented**
- Interface: SPI @ 40MHz
- Frame rate: 15 FPS target (20 FPS stretch goal)

### Video Format
- Codec: MJPEG (Motion JPEG) for simplicity
- Resolution: Match display (downscaled from source)
- Bitrate: ~500-800 kbps
- Audio: 22.05kHz mono MP3 or AAC
- Container: Custom format or AVI

### Storage Requirements
- Per episode (22 min): ~80-120 MB
- Full season (20 episodes): ~1.6-2.4 GB
- 32GB card: ~13-20 full seasons across multiple shows

### Power Budget
- Display: ~40-60mA (varies with brightness)
- ESP32 active: ~80-160mA
- Audio amp: ~50-100mA
- SD card: ~50-100mA
- Total active: ~220-420mA
- Battery capacity: 2x 3000mAh = 6000mAh
- Runtime: 14-27 hours continuous playback

## System Architecture

### Hardware Components
1. **ESP32-WROOM-32** or ESP32-S3 (38-pin dev board)
2. **2.0" ST7789** 240x320 IPS display **✅ Implemented**
3. **MicroSD card module** (SPI, Class 10+ card recommended)
4. **Rotary encoder** (KY-040) **✅ Implemented**
5. **MAX98357A I2S audio amplifier** **✅ Implemented**
6. 8Ω speaker (1-3W)
7. 2x 18650 batteries + holders
8. TP4056 charging module(s) with protection
9. Buck-boost converter (MT3608 or similar, 3.3V regulated)
10. Voltage divider (2x 10kΩ resistors for battery monitoring)
11. Power switch (SPST, 2A+)
12. Optional: MPU6050 or ADXL345 accelerometer (future feature)

### Software Stack **✅ ALL IMPLEMENTED**
- **Framework**: **ESP-IDF 5.x** (chosen for performance and control)
- **Display**: **Custom ST7789 driver** with DMA support
- **Video Decoder**: **Custom MJPEG decoder** using ESP-IDF JPEG library
- **Video Container**: **Custom AVI parser**
- **Audio**: **ESP32 I2S** with MAX98357A amplifier
- **Storage**: **ESP-IDF FATFS** for SD card
- **Channel Management**: **Custom implementation** with JSON parsing
- **Input**: **Interrupt-based rotary encoder** with debouncing
- **Power**: **ADC-based battery monitoring** + deep sleep modes
- **State Persistence**: **NVS (Non-Volatile Storage)**

## Development Phases

### Phase 1: Proof of Concept ✅ **SOFTWARE COMPLETE**
- ✅ Basic video playback engine (components/video/)
- ✅ MJPEG decoder and AVI parser
- ✅ Display driver with DMA (components/display/)
- ✅ Audio output via I2S (components/audio/)
- ⏳ Hardware testing (awaiting components)

### Phase 2: Core Features ✅ **SOFTWARE COMPLETE**
- ✅ Multi-channel system (components/storage/channel_manager.c)
- ✅ Rotary encoder integration (components/input/rotary_encoder.c)
- ✅ Episode switching and auto-advance
- ✅ State persistence via NVS (resume functionality)
- ⏳ Hardware testing (awaiting components)

### Phase 3: Enhanced Features ✅ **SOFTWARE COMPLETE**
- ⏳ Shake detection (future feature - accelerometer not yet added)
- ✅ Power management (components/power/power_manager.c)
- ✅ Basic OSD overlays (channel/battery indicators)
- ✅ Auto-dim and sleep modes
- ⏳ Hardware testing (awaiting components)

### Phase 4: Physical Integration ⏳ **AWAITING HARDWARE**
- ⏳ Hardware acquisition and breadboard testing
- ⏳ Hardware fitting into Watchman shell
- ⏳ Battery installation and power system
- ⏳ Speaker connection
- ⏳ Final assembly

### Phase 5: Content & Polish **PARTIALLY COMPLETE**
- ✅ Video encoding pipeline documented (CONTENTGUIDE.MD)
- ✅ FFmpeg scripts and batch processing guides
- ⏳ Content library creation (user task)
- ⏳ Hardware testing and optimization
- ✅ Documentation (WIRINGGUIDE.MD, INTEGRATION_SUMMARY.MD, etc.)

## Video Encoding Pipeline

### Source to Device Workflow
1. **Source**: Original video files (MP4, MKV, etc.)
2. **FFmpeg Processing**:
   - Downscale to display resolution
   - Convert to MJPEG
   - Compress audio to mono 22kHz
   - Optimize for low bitrate
3. **Packaging**: Generate index files for seeking
4. **Transfer**: Copy to SD card with channel metadata

**For detailed encoding instructions including batch scripts and FFmpeg commands, see [docs/CONTENTGUIDE.MD](docs/CONTENTGUIDE.MD)**

## File Structure on SD Card

```
/
├── channels.json          # Channel configuration
├── channel_01/
│   ├── metadata.json      # Show info
│   ├── episode_01.avi
│   ├── episode_02.avi
│   └── ...
├── channel_02/
│   ├── metadata.json
│   ├── episode_01.avi
│   └── ...
└── system/
    ├── settings.json      # Device settings
    └── state.json         # Last playback position
```

## Channel Configuration Format

```json
{
  "channels": [
    {
      "id": 1,
      "name": "The Simpsons",
      "folder": "channel_01",
      "episodes": 23
    },
    {
      "id": 2,
      "name": "The X-Files",
      "folder": "channel_02",
      "episodes": 24
    }
  ]
}
```

## Challenges & Solutions

### Challenge 1: ESP32 Memory Constraints
- **Issue**: Limited RAM for video buffering
- **Solution**: Stream from SD with double-buffering, decode frame-by-frame

### Challenge 2: Synchronization
- **Issue**: Audio/video sync
- **Solution**: Use timestamps, adjust based on frame decode time

### Challenge 3: Power Consumption
- **Issue**: Continuous playback drains battery
- **Solution**: Optimize display brightness, use efficient video format, implement auto-sleep

### Challenge 4: Physical Integration
- **Issue**: Fitting modern components in vintage case
- **Solution**: Careful measurement, custom PCB if needed, 3D printed mounting brackets

## Future Enhancements
- WiFi streaming from network storage
- Web interface for content management
- Multiple audio tracks (language selection)
- Closed captions/subtitles
- Parental controls (channel locking)
- Bluetooth audio output
- IR remote control support

## Success Criteria
1. Plays video smoothly at 15+ FPS
2. Audio synchronized within 100ms
3. Channel switching < 2 seconds
4. Battery life > 10 hours
5. Fits within original Watchman case
6. All original controls functional

## Timeline

### Completed (Phases 1-3 Software)
- **Weeks 1-4**: ✅ Complete ESP32 firmware development
- **Week 4**: ✅ Complete documentation (wiring guide, content guide, etc.)

### Remaining (Hardware-Dependent)
- **Awaiting**: Hardware component delivery
- **Week 5**: Breadboard assembly and initial testing
- **Week 6**: Debug and optimize firmware on real hardware
- **Week 7**: Physical integration into Sony Watchman case
- **Week 8**: Final testing, content creation, polish

**Original Estimate**: 6-10 weeks (part-time)
**Software Completion**: 4 weeks ✅
**Remaining**: 3-4 weeks (after hardware arrives)

---

## 📚 Documentation

**Main Project Docs:**
- **[TASKS.MD](TASKS.MD)** - Task breakdown and progress tracking
- **[BUILD.MD](BUILD.MD)** - ESP-IDF build instructions

**Reference Guides (in docs/):**
- **[docs/WIRINGGUIDE.MD](docs/WIRINGGUIDE.MD)** - Hardware wiring instructions
- **[docs/CONTENTGUIDE.MD](docs/CONTENTGUIDE.MD)** - Video encoding pipeline
- **[docs/INTEGRATION_SUMMARY.MD](docs/INTEGRATION_SUMMARY.MD)** - ESP32 integration details
- **[docs/STATUS.MD](docs/STATUS.MD)** - Component implementation status
- **[docs/SD_CARD_SETUP.MD](docs/SD_CARD_SETUP.MD)** - SD card structure

---

*Last Updated: 2025-11-12*
*Software Status: Complete and ready to flash!*
