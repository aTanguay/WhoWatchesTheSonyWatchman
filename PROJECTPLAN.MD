# ESP32 Retro Media Player - Sony Watchman Revival

## Project Overview

Transform a vintage 1980s Sony Watchman portable TV into a modern ESP32-powered mini media player that plays classic TV shows on a continuous loop. Each "channel" represents a different TV series, creating a nostalgic viewing experience with modern technology.

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
- Size: 1.7" diagonal
- Resolution: 128x160 or 240x280 (TBD based on available displays)
- Interface: SPI
- Frame rate: 15-20 FPS target

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
1. ESP32-WROOM-32 or ESP32-S3
2. 1.7" ST7735 or ST7789 IPS display
3. MicroSD card module (SPI)
4. Rotary encoder (KY-040 or similar)
5. MPU6050 or ADXL345 accelerometer (I2C)
6. Audio amplifier (PAM8403 or similar)
7. Battery management (TP4056 + protection)
8. Buck-boost converter (3.3V regulated)

### Software Stack
- **Framework**: Arduino/ESP-IDF
- **Display**: TFT_eSPI or Adafruit GFX
- **Video Decoder**: Custom MJPEG decoder
- **Audio**: ESP32 I2S DAC
- **Storage**: SD/SPI filesystem
- **Power**: ESP32 deep sleep modes

## Development Phases

### Phase 1: Proof of Concept
- Basic video playback on breadboard
- Single video file playback
- Display functionality
- Audio output

### Phase 2: Core Features
- Multi-channel system
- Rotary encoder integration
- Episode switching
- Resume functionality

### Phase 3: Enhanced Features
- Shake detection
- Power management
- UI/OSD overlays
- Channel indicator

### Phase 4: Integration
- Hardware fitting into Watchman shell
- Battery installation
- Speaker connection
- Final assembly

### Phase 5: Content & Polish
- Video encoding pipeline
- Content library creation
- Testing and optimization
- Documentation

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

### Example FFmpeg Command
```bash
ffmpeg -i input.mp4 \
  -vf "scale=128:160:force_original_aspect_ratio=decrease,pad=128:160:(ow-iw)/2:(oh-ih)/2" \
  -r 15 \
  -q:v 8 \
  -acodec libmp3lame \
  -ar 22050 \
  -ac 1 \
  -b:a 64k \
  output.avi
```

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

## Timeline Estimate
- Phase 1: 1-2 weeks
- Phase 2: 2-3 weeks
- Phase 3: 1-2 weeks
- Phase 4: 1-2 weeks
- Phase 5: 1 week

**Total**: 6-10 weeks (part-time development)
