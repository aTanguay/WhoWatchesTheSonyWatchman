# Claude AI Development Guide

This document provides context and guidelines for Claude AI to assist effectively with the Sony Watchman ESP32 Retro Media Player project.

## Important: Related Documentation

**Before starting any development work, consult these documents:**

- **[PROJECTPLAN.MD](PROJECTPLAN.MD)** - Vision, architecture, component selection, success criteria
- **[TASKS.MD](TASKS.MD)** - Detailed task breakdown with status tracking
- **[BUILD.MD](BUILD.MD)** - ESP-IDF build instructions and flashing procedures
- **[docs/WIRINGGUIDE.MD](docs/WIRINGGUIDE.MD)** - Complete hardware wiring with pin assignments
- **[docs/CONTENTGUIDE.MD](docs/CONTENTGUIDE.MD)** - Video encoding pipeline and FFmpeg scripts

**When implementing features:**
- Reference specific task numbers from TASKS.MD (e.g., "Implementing T1.16: MJPEG decoder")
- Follow the phase order outlined in PROJECTPLAN.MD
- Consult docs/WIRINGGUIDE.MD for hardware pin assignments

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
   - 240x320 resolution (ST7789)
   - 16-bit color (RGB565)
   - Minimize full-screen redraws

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
   - Leverage DMA where available
   - Use FreeRTOS tasks effectively

3. **Error Handling**
   - SD card can fail or be removed
   - Videos can be corrupted
   - Batteries can die unexpectedly
   - Always check return values
   - Implement graceful degradation

4. **Code Structure**
   - Modular design: separate components for video, audio, UI, controls
   - Header files for interfaces
   - Avoid circular dependencies
   - Follow existing architecture in components/

### Recommended Code Architecture

```
components/
├── display/              # ST7789 display driver
├── video/               # Video playback and MJPEG decoder
├── audio/               # I2S audio output
├── storage/             # SD card and channel management
├── input/               # Rotary encoder with debouncing
├── power/               # Battery monitoring and sleep modes
└── ui/                  # OSD overlays

main/
└── main.c               # Application entry point with event loop
```

### Video Format

**MJPEG in AVI container** (chosen for simplicity)
- Simple decoder, no inter-frame dependencies
- AVI container with MJPEG video + MP3 audio
- Target: 240x320 @ 15 FPS, ~600-800 kbps total

**For encoding details, see [docs/CONTENTGUIDE.MD](docs/CONTENTGUIDE.MD)**

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
- Monitor heap usage and adjust allocations

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

## Hardware Configuration

**See [docs/WIRINGGUIDE.MD](docs/WIRINGGUIDE.MD) for complete wiring instructions.**

Key points for development:
- Display uses SPI on GPIO 19 (MOSI), GPIO 18 (SCK), GPIO 5 (CS)
- **Display VCC MUST be 3.3V** (not 5V)
- SPI clock limited to 26MHz for stability
- Rotary encoder on GPIO 32/33 with interrupt-based debouncing

## Framework: ESP-IDF vs Arduino

**This project uses ESP-IDF 5.x** for maximum performance and control.

**Rationale:**
- Fine-grained hardware control needed
- Advanced FreeRTOS features required
- Better optimization for video/audio streaming
- Custom drivers for ST7789 display and video playback

## Debugging Tips

### Serial Debugging
```c
// Performance monitoring
ESP_LOGI(TAG, "Frame decode: %dms, Free heap: %d",
         decode_time, esp_get_free_heap_size());

// Memory leak detection
int heap_start = esp_get_free_heap_size();
// ... code to test ...
ESP_LOGI(TAG, "Heap delta: %d bytes", heap_start - esp_get_free_heap_size());
```

### Common Issues
1. **White screen**: Check display pins, SPI initialization, power
2. **Garbage on screen**: Wrong display driver, incorrect pinout, voltage issue
3. **SD card not detected**: Bad connection, wrong CS pin, format as FAT32
4. **Choppy video**: SD card too slow, increase buffer size
5. **Audio clicking**: Buffer underruns, increase audio buffer

## Code Quality Standards

When writing or reviewing code:

1. **Comments**: Explain WHY, not WHAT
2. **Naming**: Descriptive variable names (no single letters except loops)
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

### ESP-IDF Documentation
- ESP32 Technical Reference: https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
- ESP-IDF Programming Guide: https://docs.espressif.com/projects/esp-idf/
- FreeRTOS: https://www.freertos.org/Documentation/

### Video Format
- MJPEG Spec: https://en.wikipedia.org/wiki/Motion_JPEG
- FFmpeg Documentation: https://ffmpeg.org/documentation.html
- AVI Format: http://www.alexander-noe.com/video/documentation/avi.pdf

## Project-Specific Guidelines

### File Organization
- See PROJECTPLAN.MD for SD card structure
- Use consistent naming (episode_01.avi, episode_02.avi)
- Include metadata.json in each channel folder

### State Management
- Save state every 30 seconds during playback
- Save immediately on channel switch
- Include timestamp, channel, episode in state
- Handle corrupted state files gracefully

### User Experience Targets
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

1. **Component choices**: If hardware details are unclear
2. **Feature priority**: When multiple approaches are possible
3. **Build experience**: To gauge appropriate solution complexity
4. **Timeline**: To understand urgency and testing availability

## Development Philosophy

- **Start simple**: Get basic functionality working before adding features
- **Test continuously**: Don't build too much before testing
- **Optimize later**: Premature optimization wastes time
- **Document as you go**: Update docs when implementation changes
- **Ask for help**: Clarify requirements rather than assume

---

## Notes for Claude

When assisting:
1. **Always consult TASKS.MD** - Reference specific task numbers (e.g., T1.16, T2.5)
2. **Follow PROJECTPLAN.MD** - Adhere to architecture and design decisions
3. **Use docs/WIRINGGUIDE.MD** - For pin assignments and hardware setup
4. **Refer to docs/CONTENTGUIDE.MD** - For video encoding details
5. Prioritize working code over perfect code
6. Explain trade-offs in approach options
7. Provide complete code snippets, not just fragments
8. Consider user's skill level in explanations
9. Suggest testing steps after each implementation
10. Be encouraging - embedded projects can be challenging!

### Development Workflow
1. **Before starting**: Check which phase and task number (TASKS.MD)
2. **During implementation**: Follow architecture from PROJECTPLAN.MD and constraints above
3. **After implementation**: Test and update task status
4. **Documentation**: Keep docs in sync with code changes

This project combines many disciplines. Break complex problems into smaller, testable pieces. Always reference the task tracking system to maintain organized progress.
