# Claude AI Development Guide

This document provides context and guidelines for Claude AI to assist effectively with the Sony Watchman ESP32 Retro Media Player project.

## Important: Related Documentation

**Before starting any development work, consult these essential documents:**

1. **[PROJECTPLAN.md](PROJECTPLAN.md)** - High-level vision, architecture, component selection, and success criteria
2. **[TASKS.md](TASKS.md)** - Detailed task breakdown with phase tracking and completion status
3. **[BUILD.md](BUILD.md)** - ESP-IDF installation, build commands, flashing procedures
4. **[WIRING_GUIDE.md](WIRING_GUIDE.md)** - Complete hardware wiring diagrams with verified pin assignments
5. **[docs/CONTENTGUIDE.MD](docs/CONTENTGUIDE.MD)** - Video encoding pipeline, FFmpeg commands, quality settings
6. **[STATUS.md](STATUS.md)** - Current development status and component verification

**When implementing features:**
- Reference specific task numbers from TASKS.md (e.g., "Implementing T1.16: MJPEG decoder")
- Follow the phase order outlined in PROJECTPLAN.md
- Consult WIRING_GUIDE.md for all hardware pin assignments
- Use docs/CONTENTGUIDE.MD for video encoding parameters

---

## Project Context

This is an embedded systems project that combines multiple disciplines:

- **Hardware**: ESP32 microcontroller, IPS display, SD card storage, I2S audio, rotary encoder, power management
- **Software**: Real-time video/audio playback with severe resource constraints
- **Mechanical**: Retrofitting modern electronics into vintage 1980s Sony Watchman case
- **Media**: Video encoding pipeline optimized for embedded playback

### Key Challenge
Creating smooth 15 FPS MJPEG video playback on an ESP32 with only 200-300KB available heap, streaming from SD card at ~600-800 kbps.

---

## Key Technical Constraints

### Hardware Limitations

#### 1. ESP32 Memory (CRITICAL)
- **Total SRAM**: 520KB
- **Available heap**: Typically 200-300KB after system overhead
- **Implication**: Cannot load full video files into RAM - must stream everything
- **Strategy**: Double-buffered frame approach, streaming from SD card

#### 2. Processing Power
- **CPU**: 240MHz dual-core Xtensa LX6
- **Target**: 15 FPS MJPEG decode + display
- **Strategy**: Core 0 for video decode, Core 1 for audio/IO/UI
- **Reality**: No hardware video decoder - everything in software

#### 3. Storage Speed
- **SD Card SPI**: Typically 1-4 MB/s real-world throughput
- **Video bitrate**: Must stay under 600-800 kbps total (A/V combined)
- **Strategy**: Optimize file layout for sequential reads, minimize seeking

#### 4. Display Interface
- **Resolution**: 240x320 (RGB565)
- **Interface**: SPI @ 26MHz (display limit)
- **Frame size**: 240×320×2 bytes = 150KB per frame
- **Strategy**: DMA transfers, minimize full-screen redraws

---

## Development Approach

### Framework Choice

**This project uses ESP-IDF 5.x** (not Arduino framework).

**Rationale:**
- Fine-grained hardware control required for video performance
- Advanced FreeRTOS task management essential
- Custom display driver with DMA optimization
- Direct control over memory allocation strategies

### Code Architecture

The codebase follows a modular component structure:

```
components/
├── display/              # ST7789 display driver with DMA
├── video/               # MJPEG decoder and AVI parser
├── audio/               # I2S audio output (22.05kHz mono)
├── storage/             # SD card FAT filesystem and channel management
├── input/               # Rotary encoder with interrupt-based debouncing
├── power/               # Battery monitoring via ADC, sleep modes
└── ui/                  # On-screen display overlays

main/
└── main.c               # Application entry point and event loop
```

### Memory-Conscious Coding Practices

**Critical for this project:**

1. **Stack vs Heap**
   - Prefer stack allocation for temporary buffers
   - Use `malloc()` only for long-lived data
   - Always free allocated memory immediately after use

2. **Avoid Wasteful Patterns**
   - ❌ Arduino `String` class (heap fragmentation)
   - ❌ Loading full files into memory
   - ❌ Unnecessary buffer copies
   - ✅ Use `char[]` arrays on stack
   - ✅ Stream data in fixed-size chunks
   - ✅ Reuse buffers where possible

3. **Monitor Heap Usage**
   ```c
   ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());
   ```

### Performance Optimization

1. **Profile Before Optimizing**
   - Measure actual decode times
   - Identify true bottlenecks with timing logs
   - Don't optimize prematurely

2. **Leverage Hardware**
   - Use DMA for all SPI transfers (display, SD card)
   - Use both CPU cores strategically
   - Enable compiler optimizations (`-O2` or `-O3`)

3. **FreeRTOS Task Design**
   - Separate concerns: video decode, audio playback, UI updates, input handling
   - Use queues for inter-task communication
   - Proper task priorities to prevent starvation

### Error Handling Philosophy

**Graceful degradation is essential:**

- SD card can be removed during playback → detect and show error
- Video files can be corrupted → skip to next episode
- Battery can die unexpectedly → save state frequently
- Memory allocation can fail → reduce quality or abort cleanly

**Always check return values:**
```c
esp_err_t ret = sd_card_read(...);
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "SD read failed: %s", esp_err_to_name(ret));
    // Handle error gracefully
}
```

---

## Common Challenges & Solutions

### Challenge: Video Playback is Choppy

**Symptoms:** Frame rate drops, stuttering, lag

**Common Causes:**
1. SD card read speed too slow
2. JPEG decode taking too long
3. Display SPI transfer not using DMA
4. Insufficient buffering

**Solutions:**
- Use Class 10 or UHS-1 SD card (not Class 4)
- Optimize JPEG decoder settings (lower quality if needed)
- Ensure DMA is enabled for display writes
- Implement triple buffering: read → decode → display pipeline
- Profile decode time: aim for <60ms per frame (15 FPS)

### Challenge: Audio/Video Out of Sync

**Symptoms:** Audio drifts ahead or behind video

**Solutions:**
- Timestamp each video frame and audio chunk
- Measure actual frame decode time and adjust timing
- Drop video frames if falling behind (audio has priority)
- Insert silence if audio buffer runs dry
- Use a PTS (presentation timestamp) system

### Challenge: Out of Memory Crashes

**Symptoms:** Random reboots, heap allocation failures

**Solutions:**
- Reduce frame buffer count (but maintain double buffering minimum)
- Stream audio in smaller chunks (e.g., 2KB instead of 8KB)
- Don't load full directory listings - scan on demand
- Monitor heap with `esp_get_free_heap_size()` in main loop
- Check for memory leaks: heap should stabilize after startup

### Challenge: SD Card Read Errors

**Symptoms:** Files not found, mount failures, corrupted data

**Solutions:**
- Format card as FAT32 (exFAT not supported by ESP-IDF)
- Use high-quality branded SD cards (avoid cheap no-name cards)
- Check wiring: CS pin, MISO/MOSI, clock
- Increase SPI clock gradually: start at 10MHz, test up to 40MHz
- Use 512-byte aligned read buffers
- Check for loose connections

### Challenge: Battery Drains Too Quickly

**Symptoms:** <6 hours runtime on 6000mAh battery

**Solutions:**
- Reduce display brightness (biggest power draw)
- Lower CPU frequency during playback (160MHz instead of 240MHz)
- Use light sleep between frame decodes
- Deep sleep when paused >30 seconds
- Optimize inefficient loops (check for busy-waiting)
- Profile current draw with multimeter

---

## Testing Strategies

### Unit Testing
- Test each component in isolation before integration
- Display: Solid colors, test patterns, DMA transfer timing
- Video: Known MJPEG files with frame counting
- Audio: Sine wave test tones at 22.05kHz
- Encoder: Simulated rotation events with debouncing

### Integration Testing
- Video + audio sync: Check for drift over 5+ minutes
- Channel switching: Should complete in <1 second
- State persistence: Verify resume across reboots
- Battery monitoring: Compare ADC reading to multimeter

### Stress Testing
- Rapid channel switching (10+ times in succession)
- Long-duration playback (12+ hours continuous)
- Corrupt file handling (truncated videos, bad headers)
- SD card removal during active playback
- Low battery behavior (<6.5V on 2S pack)

---

## Hardware Configuration

### Quick Reference

**For complete wiring diagrams, see [WIRING_GUIDE.md](WIRING_GUIDE.md)**

Key points for development:
- **Display**: ST7789VW on SPI2_HOST, GPIO 19 (MOSI), GPIO 18 (CLK), GPIO 5 (CS)
- **SD Card**: SPI2_HOST shared with display, GPIO 17 (CS)
- **Audio**: MAX98357A I2S DAC, GPIO 26 (BCLK), GPIO 25 (LRC), GPIO 22 (DIN)
- **Encoder**: GPIO 32/33 (channels A/B), GPIO 27 (button)
- **Battery**: GPIO 34 (ADC) with 3:1 voltage divider

### Critical Hardware Notes

⚠️ **MUST READ:**
1. **Display VCC MUST be 3.3V** (NOT 5V) - will damage display or cause noise
2. **SPI clock limited to 26MHz** - ESP32 hardware limitation for stability
3. **GPIO 19 is shared** between display MOSI and SD MISO (works because display is write-only)
4. **Battery divider MUST be 3:1 ratio** (not 2:1) to stay under 3.3V ADC limit

---

## Video Format Specification

### Chosen Format: MJPEG in AVI Container

**Rationale:**
- Simple software decoder (sequential JPEG frames)
- No inter-frame dependencies (easy seeking, frame dropping)
- Predictable decode time per frame
- Well-supported by ESP-IDF JPEG library

### Target Specifications
- **Resolution**: 240x320 (matches display exactly)
- **Frame rate**: 15 FPS (balance of smoothness and performance)
- **Video codec**: MJPEG with quality setting 6 (verified optimal)
- **Audio codec**: MP3 mono, 22.05kHz, 64 kbps
- **Container**: AVI (simple structure, easy parsing)
- **File size**: ~150-200MB per 22-minute episode (animation/live-action varies)

**For complete encoding instructions, see [docs/CONTENTGUIDE.MD](docs/CONTENTGUIDE.MD)**

---

## Debugging Tips

### Serial Output Best Practices

```c
// Performance monitoring
ESP_LOGI(TAG, "Frame %d: decode=%dms, display=%dms, free_heap=%lu",
         frame_num, decode_ms, display_ms, esp_get_free_heap_size());

// Memory leak detection
size_t heap_before = esp_get_free_heap_size();
// ... code under test ...
size_t heap_after = esp_get_free_heap_size();
if (heap_before != heap_after) {
    ESP_LOGW(TAG, "Heap changed: %d bytes leaked", heap_before - heap_after);
}

// Timing critical sections
int64_t start = esp_timer_get_time();
// ... operation ...
int64_t elapsed_us = esp_timer_get_time() - start;
ESP_LOGI(TAG, "Operation took %lld µs", elapsed_us);
```

### Common Issues Quick Reference

| Symptom | Likely Cause | Check |
|---------|--------------|-------|
| White/blank screen | Power, reset, or SPI pins | Verify 3.3V, GPIO 4 (RST), GPIO 16 (DC) |
| Multicolored noise | SPI config or voltage | Check VCC is 3.3V, SPI flags correct |
| SD card not detected | Wiring or format | Check CS pin (GPIO 17), format as FAT32 |
| Choppy video | SD speed or decode time | Use Class 10 card, profile decode timing |
| Audio clicking/popping | Buffer underruns | Increase audio buffer size |
| Random reboots | Memory exhaustion | Monitor heap, check for leaks |

---

## Code Quality Standards

When writing or reviewing code for this project:

1. **Comments**: Explain WHY, not WHAT (code shows what)
2. **Naming**: Descriptive names (`frame_decode_time_ms` not `t`)
3. **Functions**: Single responsibility, typically <50 lines
4. **Error Handling**: Check ALL return codes, handle errors gracefully
5. **Magic Numbers**: Use `#define` constants or enums
6. **Memory Management**: Free everything you allocate, no leaks
7. **Blocking Operations**: Minimize blocking, prefer async/event-driven

---

## Pre-Commit Checklist

Before committing code changes:

- [ ] Does it compile without warnings? (`idf.py build`)
- [ ] Does free heap remain stable over 5+ minutes of operation?
- [ ] Is frame rate consistent (no stuttering)?
- [ ] Have you tested with multiple video files?
- [ ] Does it handle SD card errors gracefully?
- [ ] Are all heap allocations paired with frees?
- [ ] Have you updated relevant documentation?

---

## External Resources

### ESP-IDF Documentation
- ESP32 Technical Reference: https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
- ESP-IDF Programming Guide: https://docs.espressif.com/projects/esp-idf/
- FreeRTOS Documentation: https://www.freertos.org/Documentation/

### Video Format References
- MJPEG Overview: https://en.wikipedia.org/wiki/Motion_JPEG
- FFmpeg Documentation: https://ffmpeg.org/documentation.html
- AVI Container Format: http://www.alexander-noe.com/video/documentation/avi.pdf

---

## Project-Specific Guidelines

### SD Card File Organization

See PROJECTPLAN.md for complete structure. Key points:

```
/sdcard/
├── channels/
│   ├── 01_ShowName/
│   │   ├── metadata.json        # Show info: title, episode count
│   │   ├── episode_01.avi
│   │   ├── episode_02.avi
│   │   └── ...
│   └── 02_AnotherShow/
│       └── ...
└── state.dat                    # Playback state (NVS backup)
```

- Use consistent naming: `episode_XX.avi` (01, 02, 03...)
- Keep `metadata.json` small and parseable
- Document file structure changes in PROJECTPLAN.md

### State Management

- **Save frequency**: Every 30 seconds during playback
- **Save triggers**: Channel switch, pause, sleep
- **State data**: Current channel, episode, timestamp (seconds)
- **Storage**: Primary in NVS (Non-Volatile Storage), backup to SD card
- **Recovery**: Handle corrupted state files gracefully (default to channel 1, episode 1)

### User Experience Targets

- **Channel switch**: <1 second from encoder turn to video playing
- **Episode skip**: <2 seconds (shake or button press)
- **Feedback**: Visual indicator for ALL user actions (OSD overlay)
- **Battery indicator**: Always visible, update every 30 seconds
- **Smooth transitions**: Fade to black between episodes (if memory allows)

### Power Optimization Targets

- **Battery life**: 15+ hours continuous playback (stretch goal: 20 hours)
- **Auto-sleep**: Dim display after 2 minutes idle, sleep after 5 minutes
- **Deep sleep**: When powered off, wake on encoder button press
- **Brightness**: Default to 60%, user adjustable (future feature)

---

## Questions to Ask User When Clarification Needed

1. **Hardware selection**: "Which specific audio module did you order? MAX98357A or something else?"
2. **Feature priority**: "Should I implement battery monitoring first, or focus on video performance?"
3. **Build experience**: "Are you comfortable with soldering SMD components, or should we stick to modules?"
4. **Timeline**: "Do you need this working by a specific date, or can we iterate and test thoroughly?"
5. **Content preferences**: "Which TV shows are highest priority for encoding first?"
6. **Budget constraints**: "Is there a budget limit for additional components?"

---

## Development Philosophy

- **Start simple**: Get basic functionality working before adding features
- **Test continuously**: Don't build for days without testing on hardware
- **Optimize later**: Premature optimization wastes time - profile first
- **Document as you go**: Update docs when implementation decisions are made
- **Ask for help**: Clarify requirements instead of making assumptions
- **Be pragmatic**: "Good enough" working code beats perfect unfinished code

---

## Notes for Claude AI Assistant

### Session Startup Checklist

**At the start of EVERY new session, remind the user:**
```
⚠️ Quick reminder: Have you run `git pull` to sync the latest changes?
   This prevents merge conflicts and ensures you're working with current docs.
```

If the user hasn't pulled, pause and suggest they do so before proceeding with tasks.

### Core Workflow

1. **Before starting any task:**
   - Check TASKS.md for the specific task number and requirements
   - Review PROJECTPLAN.md for architectural constraints
   - Verify hardware pin assignments in WIRING_GUIDE.md

2. **During implementation:**
   - Follow the modular component structure in `components/`
   - Adhere to memory constraints (monitor heap usage)
   - Reference this file (CLAUDE.md) for technical constraints
   - Use ESP-IDF patterns (not Arduino patterns)

3. **After implementation:**
   - Test the component in isolation first
   - Update task status in TASKS.md
   - Update relevant documentation if design decisions changed
   - Commit with clear, descriptive messages

### Communication Style

- Prioritize working code over perfect code
- Explain trade-offs when multiple approaches are valid
- Provide complete, tested code snippets (not fragments)
- Gauge user's skill level and adjust explanations accordingly
- Suggest concrete testing steps after each implementation
- Warn proactively about common pitfalls (e.g., memory issues, voltage levels)
- Be encouraging - embedded systems projects are challenging but rewarding!

### Task Reference System

When working on features, always reference task numbers from TASKS.md:

**Example**: "Implementing T1.16: MJPEG decoder with ESP-IDF JPEG library"

This helps track progress and ensures alignment with the project plan.

### Document Hierarchy

1. **PROJECTPLAN.md** - Strategic vision and architecture (rarely changes)
2. **TASKS.md** - Tactical task list with status tracking (frequently updated)
3. **CLAUDE.md** (this file) - Development context and constraints (occasional updates)
4. **WIRING_GUIDE.md** - Hardware wiring reference (stable after verification)
5. **BUILD.md** - Build process and toolchain instructions (stable)
6. **docs/CONTENTGUIDE.MD** - Video encoding reference (stable after testing)

**When in doubt, consult the document hierarchy above to find authoritative information.**

---

This project combines multiple engineering disciplines into one cohesive system. Success requires attention to memory constraints, real-time performance requirements, and user experience. Break complex problems into smaller, testable pieces. Always reference the task tracking system to maintain organized progress. Most importantly: test frequently on real hardware!
