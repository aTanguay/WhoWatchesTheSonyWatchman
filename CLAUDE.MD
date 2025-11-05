# Claude AI Development Guide

This document provides context and guidelines for Claude AI to assist effectively with the Sony Watchman ESP32 Retro Media Player project.

## Important: Related Documentation

**Before starting any development work, please review these essential documents:**

1. **PROJECTPLAN.MD** - High-level project overview, architecture, and vision
   - System architecture and component selection
   - Development phases and timeline
   - File structure and video encoding pipeline
   - Success criteria and future enhancements

2. **TASKS.MD** - Detailed task breakdown and implementation checklist
   - Phase 1: Proof of Concept (29 tasks)
   - Phase 2: Core Features (26 tasks)
   - Phase 3: Enhanced Features (30 tasks)
   - Phase 4: Physical Integration (39 tasks)
   - Phase 5: Content & Polish (32 tasks)
   - Phase 6: Optional/Future (13+ tasks)

3. **BUILD.MD** - ESP-IDF build instructions and development workflow
   - ESP-IDF installation and configuration
   - Pin assignments and hardware setup
   - Building, flashing, and debugging procedures
   - Video encoding guide with FFmpeg

**When implementing features:**
- Reference specific task numbers from TASKS.MD (e.g., "Implementing T1.16: MJPEG decoder")
- Follow the phase order outlined in PROJECTPLAN.MD
- Cross-reference technical constraints and architecture details below

## Project Context

This is an embedded systems project combining:
- **Hardware**: ESP32 microcontroller, display, sensors, audio, power management
- **Software**: Real-time video/audio playback with constrained resources
- **Mechanical**: Retrofitting modern electronics into vintage 1980s Sony Watchman case
- **Media**: Video encoding pipeline to optimize content for embedded playback

## Key Technical Constraints

### Hardware Limitations
1. **ESP32 RAM**: Only 520KB SRAM total
   - Heap typically 200-300KB available
   - Must stream video from SD card, cannot load full files
   - Frame buffers must be double-buffered at minimum

2. **Processing Power**: 240MHz dual-core
   - MJPEG decoding must be optimized
   - Aim for 15-20 FPS, not full 30 FPS
   - Use both cores strategically (core 0: video, core 1: audio/IO)

3. **Storage Speed**: SD card SPI typically 1-4 MB/s
   - Video bitrate must stay well under read speed
   - Target 500-800 kbps for combined A/V

4. **Display Interface**: SPI limits frame rate
   - 128x160 or 240x280 resolution
   - 16-bit color (RGB565)
   - Minimize full-screen redraws

### Software Constraints
1. **Arduino Framework**: Easier for beginners but less control
2. **ESP-IDF**: More powerful but steeper learning curve
3. **Libraries**: TFT_eSPI (display), SD (storage), custom audio/video decoders
4. **No hardware video decoder**: Everything in software

## Development Approach

### When Helping with Code

1. **Memory-Conscious Code**
   - Always consider heap usage
   - Use stack allocation where possible
   - Free memory explicitly after use
   - Avoid String class, use char arrays
   - Monitor free heap regularly

2. **Performance Optimization**
   - Profile before optimizing
   - Use compiler optimizations (-O2 or -O3)
   - Consider assembly for critical loops
   - Leverage DMA where available
   - Use FreeRTOS tasks effectively

3. **Error Handling**
   - SD card can fail or be removed
   - Videos can be corrupted
   - Batteries can die unexpectedly
   - Always check return values
   - Implement graceful degradation

4. **Code Structure**
   - Modular design: separate classes for video, audio, UI, controls
   - Header files for interfaces
   - Avoid circular dependencies
   - Use namespaces or class prefixes

### Recommended Code Architecture

```
src/
├── main.cpp                 # Main setup/loop
├── video/
│   ├── VideoPlayer.h/cpp    # Video playback engine
│   ├── MJPEGDecoder.h/cpp   # MJPEG decoder
│   └── FrameBuffer.h/cpp    # Display buffer management
├── audio/
│   ├── AudioPlayer.h/cpp    # Audio playback engine
│   └── AudioDecoder.h/cpp   # MP3/PCM decoder
├── storage/
│   ├── ChannelManager.h/cpp # Channel/episode management
│   ├── SDCard.h/cpp          # SD card interface
│   └── StateManager.h/cpp    # Save/load state
├── input/
│   ├── RotaryEncoder.h/cpp  # Encoder handling
│   └── ShakeSensor.h/cpp    # Accelerometer
├── ui/
│   ├── Display.h/cpp        # Display abstraction
│   └── OSD.h/cpp            # On-screen display
└── power/
    └── PowerManager.h/cpp   # Battery/sleep management
```

### Video Format Recommendations

**Option 1: MJPEG (Recommended for simplicity)**
- Pros: Simple decoder, well-supported, no inter-frame dependencies
- Cons: Larger file sizes than modern codecs
- Format: AVI container with MJPEG video + MP3 audio

**Option 2: Custom chunked format**
- Pros: Optimized for ESP32, faster seeking
- Cons: Requires custom tools
- Format: Binary format with frame index

**Option 3: H.264 (Advanced)**
- Pros: Better compression
- Cons: Complex decoder, may be too slow for ESP32
- Use: Only if MJPEG performance is insufficient

### FFmpeg Encoding Guidelines

For 128x160 display:
```bash
ffmpeg -i input.mp4 \
  -vf "scale=128:160:force_original_aspect_ratio=decrease,pad=128:160:(ow-iw)/2:(oh-ih)/2" \
  -r 15 \
  -q:v 8 \
  -vcodec mjpeg \
  -acodec libmp3lame \
  -ar 22050 \
  -ac 1 \
  -b:a 64k \
  output.avi
```

Adjust quality (-q:v) between 5 (higher quality) and 10 (smaller size) as needed.

## Common Challenges & Solutions

### Challenge: Video playback is choppy
**Solutions:**
- Reduce video resolution or frame rate
- Optimize JPEG decoder (use ESP32 optimized libraries)
- Increase SPI clock speed for SD card and display
- Use DMA for SPI transfers
- Preload next frame while displaying current

### Challenge: Audio/video out of sync
**Solutions:**
- Timestamp each frame and audio chunk
- Adjust playback based on measured decode time
- Drop frames if behind, insert silence if ahead
- Measure actual FPS and adjust accordingly

### Challenge: Runs out of memory
**Solutions:**
- Reduce frame buffer size
- Stream audio in smaller chunks
- Don't load full file lists, scan on demand
- Use PROGMEM for constants
- Monitor heap usage with ESP.getFreeHeap()

### Challenge: SD card reads are slow
**Solutions:**
- Use high-quality SD card (Class 10, UHS-1)
- Optimize file format (sequential reads, aligned)
- Increase SPI clock (up to 40MHz if stable)
- Use larger read buffers (512 byte multiples)
- Minimize file seeking

### Challenge: Battery drains quickly
**Solutions:**
- Reduce display brightness
- Lower CPU frequency when possible (80MHz vs 240MHz)
- Use light sleep during playback
- Deep sleep when paused >30 seconds
- Optimize inefficient code loops
- Consider e-paper display for ultra-low power (future)

## Testing Strategies

### Unit Testing
- Test each component in isolation
- Video decoder with known test patterns
- Audio decoder with sine wave
- Encoder with simulated rotation
- Display with test patterns

### Integration Testing
- Video + audio synchronization
- Channel switching during playback
- State persistence across reboots
- Battery voltage monitoring accuracy

### Stress Testing
- Rapid channel switching
- Long-duration playback (12+ hours)
- Corrupt file handling
- SD card removal during playback
- Low battery behavior

## Hardware Recommendations

### Essential Components
- **ESP32**: ESP32-WROOM-32 or ESP32-S3 (more RAM)
- **Display**: 1.7" ST7735 (128x160) or ST7789 (240x240/280)
- **SD Card**: SanDisk or Samsung Class 10, 32GB+
- **Audio Amp**: MAX98357A (I2S, best quality) or PAM8403 (analog)
- **Encoder**: KY-040 rotary encoder module
- **Accelerometer**: MPU6050 (I2C, 6-axis)
- **Power**: TP4056 charger + 2x 18650 batteries + MT3608 boost converter

### Component Pinout Suggestions

```
ESP32 Pin Assignments:
─────────────────────────────────────
GPIO 23   MOSI       (SPI - Display/SD)
GPIO 18   SCK        (SPI - Display/SD)
GPIO 19   MISO       (SPI - SD Card)
GPIO  5   CS_Display (Chip Select - Display)
GPIO 17   CS_SD      (Chip Select - SD)
GPIO 16   DC         (Display Data/Command)
GPIO  4   RST        (Display Reset)
GPIO 21   SDA        (I2C - Accelerometer)
GPIO 22   SCL        (I2C - Accelerometer)
GPIO 25   DAC1/I2S   (Audio Out)
GPIO 26   DAC2/I2S   (Audio Out)
GPIO 32   Encoder A  (Rotary)
GPIO 33   Encoder B  (Rotary)
GPIO 27   Encoder SW (Button)
GPIO 34   Battery V  (ADC - voltage divider)
```

## When to Use ESP-IDF vs Arduino

**Use Arduino if:**
- User is beginner/intermediate with embedded systems
- Want faster initial prototyping
- Need library ecosystem (TFT_eSPI, etc.)
- Project complexity is moderate

**Use ESP-IDF if:**
- Need maximum performance
- Want fine-grained control over hardware
- Require advanced FreeRTOS features
- User is experienced with embedded development

**Recommendation**: Start with Arduino, migrate to ESP-IDF if performance is insufficient.

## Debugging Tips

### Serial Debugging
```cpp
// Performance monitoring
Serial.printf("Frame decode: %dms, Free heap: %d\n",
              decodeTime, ESP.getFreeHeap());

// Memory leak detection
int heap_start = ESP.getFreeHeap();
// ... code to test ...
Serial.printf("Heap delta: %d bytes\n", heap_start - ESP.getFreeHeap());
```

### Logic Analyzer Use Cases
- SPI signal integrity (display/SD)
- I2C communication (accelerometer)
- Timing analysis (frame rate)
- Encoder signal debouncing

### Common Issues
1. **White screen**: Check display pins, SPI initialization, power
2. **Garbage on screen**: Wrong display driver, incorrect pinout
3. **SD card not detected**: Bad connection, wrong CS pin, format as FAT32
4. **Choppy video**: SD card too slow, increase buffer size
5. **Audio clicking**: Buffer underruns, increase audio buffer

## Code Quality Standards

When writing or reviewing code:

1. **Comments**: Explain WHY, not WHAT
2. **Naming**: descriptive variable names (no single letters except loops)
3. **Functions**: Single responsibility, <50 lines
4. **Error Handling**: Check all return codes
5. **Magic Numbers**: Use #define or const
6. **Memory**: Always free what you allocate
7. **Blocking**: Minimize blocking operations, use async where possible

## Testing Before Committing

Run these checks:
1. Does it compile without warnings?
2. Does free heap remain stable over time?
3. Is FPS consistent (no stuttering)?
4. Does it work with multiple test videos?
5. Can it recover from SD card errors?
6. Is battery life acceptable?

## Resources for Reference

### Libraries
- TFT_eSPI: https://github.com/Bodmer/TFT_eSPI
- ESP32-audioI2S: https://github.com/schreibfaul1/ESP32-audioI2S
- ArduinoJson: https://arduinojson.org/
- MPU6050: https://github.com/jrowberg/i2cdevlib

### Documentation
- ESP32 Technical Reference: https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
- Arduino-ESP32: https://docs.espressif.com/projects/arduino-esp32/
- FreeRTOS: https://www.freertos.org/Documentation/

### Video Format
- MJPEG Spec: https://en.wikipedia.org/wiki/Motion_JPEG
- FFmpeg Documentation: https://ffmpeg.org/documentation.html
- AVI Format: http://www.alexander-noe.com/video/documentation/avi.pdf

## Project-Specific Guidelines

### File Organization
- Keep video files organized by channel folders
- Use consistent naming (episode_01.avi, episode_02.avi)
- Include metadata.json in each channel folder
- Document file structure in README

### State Management
- Save state every 30 seconds during playback
- Save immediately on channel switch
- Include timestamp, channel, episode in state
- Handle corrupted state files gracefully

### User Experience
- Channel switch should feel instant (<1s)
- Show feedback for all user actions
- Battery indicator always visible
- Smooth transitions between episodes

### Power Optimization
- Target 15+ hours battery life
- Implement auto-sleep after 5 minutes idle
- Dim display after 2 minutes
- Deep sleep when off, wake on encoder press

## Questions to Ask User When Clarification Needed

1. **Display choice**: Which specific display model? (impacts resolution/code)
2. **Audio method**: I2S (better) or DAC (simpler)?
3. **Shake vs button**: Which is preferred for episode switching?
4. **Content priority**: Which shows are most important?
5. **Build experience**: PCB design skills? Soldering experience?
6. **Dev environment**: Arduino IDE or PlatformIO/VSCode?
7. **Budget**: Any constraint on component costs?
8. **Timeline**: Deadline or working at leisure?

## Success Metrics

The project is successful when:
- [ ] Video plays at 15+ FPS consistently
- [ ] Audio is in sync within 100ms
- [ ] Channel switching takes <2 seconds
- [ ] Battery lasts 10+ hours continuous playback
- [ ] All components fit in original case
- [ ] Controls feel responsive and natural
- [ ] User is happy with the final result!

## Development Philosophy

- **Start simple**: Get basic video working before adding features
- **Test continuously**: Don't build too much before testing
- **Optimize later**: Premature optimization wastes time
- **Document as you go**: Future you will thank present you
- **Ask for help**: Community resources and forums are valuable
- **Have fun**: This is a cool project, enjoy the process!

---

## Notes for Claude

When assisting:
1. **Always consult TASKS.MD** - Reference specific task numbers (e.g., T1.16, T2.5) when implementing features
2. **Follow PROJECTPLAN.MD** - Adhere to the architecture and design decisions outlined
3. **Use BUILD.MD** - Reference pin assignments, build commands, and encoding guidelines
4. Prioritize working code over perfect code
5. Explain trade-offs in approach options
6. Provide complete code snippets, not just fragments
7. Consider user's skill level in explanations
8. Suggest testing steps after each implementation
9. Warn about common pitfalls proactively
10. Be encouraging - embedded projects can be challenging!

### Development Workflow
1. **Before starting**: Check which phase and task number you're working on (TASKS.MD)
2. **During implementation**: Follow architecture from PROJECTPLAN.MD and constraints from this file
3. **After implementation**: Update task status and test according to BUILD.MD
4. **Documentation**: Keep all documentation files (this file, TASKS.MD, BUILD.MD) in sync with code changes

This project combines many disciplines. Don't hesitate to break complex problems into smaller, testable pieces. Always reference the task tracking system to maintain organized progress.
