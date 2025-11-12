# Hardware Testing Guide

## Display Test Mode

The firmware includes a **TEST_MODE** that lets you verify your display is working correctly without needing an SD card, audio, encoder, or any other components!

### What It Does

When TEST_MODE is enabled, the firmware runs a continuous loop of animated test patterns:

1. **Solid Colors** - Cycles through R, G, B, Yellow, Cyan, Magenta, White, Black (2 sec each)
2. **Color Bars** - Classic TV test pattern (5 sec)
3. **Bouncing Box** - Cyan square bouncing around the screen (10 sec)
4. **RGB Gradients** - Red, green, and blue gradients (5 sec)
5. **Checkerboard** - Black and white checkerboard pattern (5 sec)
6. **Moving Lines** - Scrolling grid pattern (10 sec)

The sequence repeats forever - perfect for verifying your display wiring!

### How to Enable Test Mode

**Step 1**: Open `main/main.c` and find line 15:

```c
#define TEST_MODE 0  // Change to 1 for hardware testing
```

**Step 2**: Change it to:

```c
#define TEST_MODE 1  // Change to 1 for hardware testing
```

**Step 3**: Save the file, rebuild, and flash:

```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

That's it! The display will start showing test patterns.

### What You Should See

#### Serial Monitor Output:
```
========================================
  Sony Watchman Retro Media Player
  ESP-IDF Version: v5.x
  MODE: Hardware Testing

  ** DISPLAY TEST MODE **
  Testing display without SD card/audio/etc
  Change TEST_MODE to 0 for normal operation
========================================

Initializing display for testing...
Display initialized successfully!
Starting test pattern sequence...

========================================
  Display Hardware Test Suite
  Testing ST7789 240x320
========================================
Test 1: Solid Colors
  RED
  GREEN
  BLUE
  ...
```

#### On the Display:
- You should see bright, solid colors
- Color bars should be crisp and aligned
- The bouncing box should move smoothly (~30 FPS)
- Gradients should be smooth (no banding)
- Checkerboard should have sharp edges
- Moving lines should scroll smoothly

### Troubleshooting

**Problem**: Display stays white/blank
- Check all 6 wire connections (MOSI, CLK, CS, DC, RST, BL)
- Verify 3.3V power to display
- Check serial monitor for initialization errors

**Problem**: Display shows garbage/random pixels
- Wrong display driver (check STATUS.md - should be ST7789)
- Poor connections (re-solder or use shorter wires)
- Interference (add 100nF capacitor between display VCC/GND)

**Problem**: Colors look wrong
- Check RGB565 color format support on your display
- Some displays need BGR instead of RGB (check display.c)

**Problem**: Display is dim
- Backlight not connected (GPIO 15)
- Backlight always-on displays don't need BL pin

**Problem**: "Display initialization failed" error
- Serial monitor will show which pin is likely the problem
- Recheck wiring according to WIRINGGUIDE.MD
- Measure voltage on each pin with multimeter

### Wiring Quick Reference

For TEST_MODE, you only need to wire the display:

```
ESP32 Pin    Display Pin
─────────    ───────────
GPIO 23  →   MOSI/SDA
GPIO 18  →   SCK/CLK
GPIO 5   →   CS
GPIO 16  →   DC/RS
GPIO 4   →   RST/RESET
GPIO 15  →   BL/LED (optional)
3.3V     →   VCC
GND      →   GND
```

**IMPORTANT**: Display must be 3.3V! Do NOT connect 5V!

### Returning to Normal Operation

When you're done testing and ready to add other components:

**Step 1**: Set TEST_MODE back to 0 in `main/main.c`:

```c
#define TEST_MODE 0  // Change to 1 for hardware testing
```

**Step 2**: Rebuild and flash:

```bash
idf.py build flash
```

The firmware will now boot in normal operation mode and expect an SD card, audio, encoder, etc.

### Component-by-Component Testing

The recommended testing sequence:

1. ✅ **Display only** (TEST_MODE) - What you're doing now!
2. ⏳ **Add SD card** - Test with TEST_MODE=0, no video files yet (should show "no channels" error)
3. ⏳ **Add test video** - Create one test video, verify playback
4. ⏳ **Add audio** - Verify synchronized audio/video
5. ⏳ **Add encoder** - Test channel switching
6. ⏳ **Add power** - Test battery monitoring
7. ⏳ **Full integration** - Everything together!

This way, if something breaks, you know exactly which component caused it.

### Tips

- **Leave TEST_MODE=1** until you're 100% sure your display works
- Take a video of the test patterns - useful for debugging later
- If the display works in test mode but not in normal mode, the problem is NOT the display
- The test patterns run forever - you can let it run for hours to check for overheating

---

## Other Component Tests

### SD Card Test (Coming Soon)
Future: Add TEST_SD_ONLY mode to test SD card without display

### Audio Test (Coming Soon)
Future: Add TEST_AUDIO_ONLY mode to test audio output (sine wave generator)

### Full Component Test
Once all components are wired, the normal firmware provides extensive logging:
- Battery voltage and percentage
- SD card detection and capacity
- Channel discovery
- Frame rate monitoring
- Memory usage tracking

Watch the serial monitor (idf.py monitor) to see real-time status!

---

*For full wiring instructions, see [WIRINGGUIDE.MD](WIRINGGUIDE.MD)*
