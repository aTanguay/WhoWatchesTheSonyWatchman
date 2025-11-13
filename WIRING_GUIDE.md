# Complete Wiring Guide
**Sony Watchman ESP32 Retro Media Player**

This guide provides the complete, verified wiring diagram for all components in the Sony Watchman project.

---

## Table of Contents
1. [Quick Reference](#quick-reference)
2. [Display Module (ST7789VW)](#display-module-st7789vw)
3. [SD Card Module](#sd-card-module)
4. [Audio Module (I2S DAC)](#audio-module-i2s-dac)
5. [Rotary Encoder](#rotary-encoder)
6. [Accelerometer (Optional)](#accelerometer-optional)
7. [Power Management](#power-management)
8. [Important Notes](#important-notes)

---

## Quick Reference

### Complete GPIO Pin Map

| GPIO | Function | Component | Direction | Notes |
|------|----------|-----------|-----------|-------|
| **4** | Display RST | Display | Output | Reset line |
| **5** | Display CS | Display | Output | Chip select |
| **14** | I2C SCL | Accelerometer | Bidir | Clock (changed from GPIO 22) |
| **15** | Display BL | Display | Output | Backlight PWM |
| **16** | Display DC | Display | Output | Data/Command |
| **17** | SD Card CS | SD Card | Output | Chip select |
| **18** | SPI CLK | Display + SD | Output | Shared clock |
| **19** | MOSI + MISO | Display + SD | Bidir | Shared data line |
| **21** | I2C SDA | Accelerometer | Bidir | Data line |
| **22** | I2S DOUT | Audio | Output | Audio data (changed) |
| **23** | SD MOSI | SD Card | Output | SD card write |
| **25** | I2S LRC/WS | Audio | Output | Word select |
| **26** | I2S BCLK | Audio | Output | Bit clock |
| **27** | Encoder SW | Encoder | Input | Button (pullup) |
| **32** | Encoder CLK | Encoder | Input | Channel A (pullup) |
| **33** | Encoder DT | Encoder | Input | Channel B (pullup) |
| **34** | Battery ADC | Power | Input | Voltage divider |

### Power Rails
- **3.3V**: Display, SD Card, Encoder, Accelerometer, ESP32
- **5V** (or 3.3V): Audio module (check datasheet)
- **GND**: Common ground for all components

---

## Display Module (ST7789VW)

**Module:** Waveshare 2" LCD Module (240x320 resolution)
**Status:** ✅ VERIFIED WORKING

### Pin Labels on Module
```
┌─────────────────┐
│  VCC  GND  DIN  │  ← Pin labels as printed on module
│  CLK  CS   DC   │
│  RST  BL        │
└─────────────────┘
```

### Wiring Table

| Module Pin | ESP32 GPIO | Wire Color Suggestion | Function |
|------------|------------|-----------------------|----------|
| **VCC** | 3.3V | Red | Power (⚠️ NOT 5V!) |
| **GND** | GND | Black | Ground |
| **DIN** | GPIO 19 | Orange | SPI MOSI (data in) |
| **CLK** | GPIO 18 | Yellow | SPI clock |
| **CS** | GPIO 5 | Green | Chip select |
| **DC** | GPIO 16 | Blue | Data/Command select |
| **RST** | GPIO 4 | Purple | Reset |
| **BL** | GPIO 15 | White | Backlight (PWM) |

### ⚠️ Critical Notes
1. **MUST use 3.3V** - 5V will damage the display or cause corruption
2. **GPIO 19** is used for display MOSI (not GPIO 23) due to board compatibility
3. **SPI clock limited to 26MHz** for stability
4. Display is write-only (no MISO needed), allowing GPIO 19 to be shared with SD card

### Waveshare Module Photos
```
     VCC ●─────────── 3.3V (RED wire)
     GND ●─────────── GND (BLACK wire)
     DIN ●─────────── GPIO 19 (ORANGE)
     CLK ●─────────── GPIO 18 (YELLOW)
      CS ●─────────── GPIO 5 (GREEN)
      DC ●─────────── GPIO 16 (BLUE)
     RST ●─────────── GPIO 4 (PURPLE)
      BL ●─────────── GPIO 15 (WHITE)
```

---

## SD Card Module

**Module:** Standard SPI SD card adapter (most common type)
**Status:** 🔧 Ready to wire

### Pin Labels on Module
```
┌──────────────────┐
│  CS  SCK  MOSI   │  ← Most common pin order
│  MISO VCC GND    │
└──────────────────┘
```

### Wiring Table

| Module Pin | ESP32 GPIO | Wire Color Suggestion | Function |
|------------|------------|-----------------------|----------|
| **VCC** | 3.3V | Red | Power (⚠️ check if 5V tolerant) |
| **GND** | GND | Black | Ground |
| **CS** | GPIO 17 | Green | Chip select |
| **SCK** | GPIO 18 | Yellow | SPI clock (shared w/ display) |
| **MOSI** | GPIO 23 | Orange | SPI data in (SD write) |
| **MISO** | GPIO 19 | Blue | SPI data out (SD read) |

### ⚠️ Critical Notes
1. **Most SD modules require 3.3V** - check your specific module datasheet
2. **GPIO 18 (SCK) is shared** with the display (this is normal and correct)
3. **GPIO 19 is shared** between display MOSI and SD card MISO (works because display is write-only)
4. **Format SD card as FAT32** before use
5. Use a quality **Class 10 or UHS-1** SD card for best performance

### SPI Bus Sharing Diagram
```
                  ESP32 SPI2 Bus
                        │
                        ├─── GPIO 18 (SCK) ────┬─── Display CLK
                        │                      └─── SD Card SCK
                        │
                        ├─── GPIO 19 ──────────┬─── Display DIN (MOSI)
                        │                      └─── SD Card MISO
                        │
                        ├─── GPIO 23 ──────────┬─── SD Card MOSI
                        │
                        ├─── GPIO 5 (CS) ──────┴─── Display CS
                        │
                        └─── GPIO 17 (CS) ─────┴─── SD Card CS

Note: CS (Chip Select) lines ensure only one device responds at a time
```

---

## Audio Module (I2S DAC)

**Recommended Module:** MAX98357A I2S Class D Amplifier
**Status:** 🔧 Not yet implemented (hardware recommendation)

### Why MAX98357A?
- ✅ True I2S interface (best quality)
- ✅ Built-in 3W Class D amplifier
- ✅ No external components needed
- ✅ 3.3V or 5V operation
- ✅ Low cost (~$5-8)
- ✅ Tiny module size (perfect for case)

### Alternative Options
- **PCM5102**: Better audio quality, requires separate amp
- **UDA1334A**: Good balance, smaller than MAX98357A
- **PAM8403**: Cheap analog amp (requires ESP32 DAC, lower quality)

### MAX98357A Wiring Table

| Module Pin | ESP32 GPIO | Wire Color Suggestion | Function |
|------------|------------|-----------------------|----------|
| **VIN** | 5V or 3.3V | Red | Power (check module) |
| **GND** | GND | Black | Ground |
| **BCLK** | GPIO 26 | Yellow | Bit clock |
| **LRC** (or WS) | GPIO 25 | Blue | Word select / Left-Right clock |
| **DIN** | GPIO 22 | Green | Audio data |
| **SD** | 3.3V | Gray | Shutdown (tie high for always on) |
| **GAIN** | See notes | - | Optional: tie to GND/VIN/float for gain |

### ⚠️ Important Notes
1. **GPIO 22** is used for I2S data (changed from original plan to avoid conflict)
2. **SD pin (Shutdown)**: Tie to 3.3V for always on, or connect to GPIO for software control
3. **GAIN pin**:
   - Floating = 9dB gain
   - Tied to GND = 6dB gain
   - Tied to VIN = 3dB gain
   - Tied to GND through 100K resistor = 15dB gain
4. **Speaker connection**: Connect directly to module's speaker terminals (no polarity concern for speaker)
5. **Sample rate**: 22.05kHz mono, 16-bit (matches video encoding)

### Pin Resolution from Original Conflict
```
❌ Original plan had GPIO 22 for BOTH I2C SCL and I2S!

✅ Resolution:
   - GPIO 22 → I2S DOUT (Audio)
   - GPIO 14 → I2C SCL (Accelerometer - moved from GPIO 22)
```

---

## Rotary Encoder

**Module:** KY-040 or generic rotary encoder with switch
**Status:** 🔧 Ready to wire

### Pin Labels on Module
```
┌─────────────────┐
│  CLK  DT  SW    │  ← Most common pin order
│  +    GND       │
└─────────────────┘
```

### Wiring Table

| Module Pin | ESP32 GPIO | Wire Color Suggestion | Function |
|------------|------------|-----------------------|----------|
| **+** (or VCC) | 3.3V | Red | Power |
| **GND** | GND | Black | Ground |
| **CLK** (or A) | GPIO 32 | Yellow | Encoder channel A |
| **DT** (or B) | GPIO 33 | Blue | Encoder channel B |
| **SW** | GPIO 27 | Green | Push button |

### ⚠️ Important Notes
1. **Internal pullups**: ESP32 code will enable internal pullup resistors
2. **Debouncing**: Handled in software (no external caps needed)
3. **Direction detection**: Clockwise/counter-clockwise determined by phase relationship
4. **Some modules have 5 pins**: Ignore the extra ground pin if present

### Usage in Project
- **Rotate**: Change channels (TV tuner style)
- **Press**: Play/pause
- **Long press**: Settings menu (future)

---

## Accelerometer (Optional)

**Module:** MPU6050 6-axis (gyro + accel)
**Status:** 🔧 Optional feature - not yet implemented

### Why MPU6050?
- ✅ Combined accelerometer + gyroscope
- ✅ I2C interface (only 2 wires)
- ✅ Low cost (~$3-5)
- ✅ Gesture detection for "shake to skip" feature

### Wiring Table

| Module Pin | ESP32 GPIO | Wire Color Suggestion | Function |
|------------|------------|-----------------------|----------|
| **VCC** | 3.3V | Red | Power |
| **GND** | GND | Black | Ground |
| **SDA** | GPIO 21 | Blue | I2C data |
| **SCL** | GPIO 14 | Yellow | I2C clock |
| **INT** | (optional) | Purple | Interrupt (future use) |

### ⚠️ Important Notes
1. **GPIO 14** is used for I2C SCL (changed from original GPIO 22 to avoid audio conflict)
2. **I2C address**: Usually 0x68 (or 0x69 if AD0 is high)
3. **INT pin**: Optional - can be used for motion detection interrupts
4. **XDA/XCL**: Auxiliary I2C pins - leave unconnected

### Planned Feature
- **Shake detection**: Quick shake to skip to next episode
- **Gesture**: Double-tap to play/pause (alternative to button)

---

## Power Management

**Components:** Voltage divider for battery monitoring
**Status:** 🔧 Not yet implemented

### Battery Voltage Monitor

ESP32 ADC can only read 0-3.3V, so we need a voltage divider to safely read battery voltage (6V-8.4V range from 2S 18650 cells).

### Wiring Diagram
```
         Battery Positive (6.0V - 8.4V)
                 │
                 ├───── To boost converter & power system
                 │
               [10kΩ]  ← Resistor R1
                 │
                 ├───────── GPIO 34 (ADC1_CH6)
                 │
               [10kΩ]  ← Resistor R2
                 │
                GND
```

### Component List
- **R1**: 10kΩ resistor (1/4W)
- **R2**: 10kΩ resistor (1/4W)
- **Optional**: 0.1µF capacitor across R2 for noise filtering

### Voltage Calculation
```
V_GPIO34 = V_Battery × (R2 / (R1 + R2))
V_GPIO34 = V_Battery × (10kΩ / 20kΩ)
V_GPIO34 = V_Battery × 0.5

Examples:
- 8.4V (fully charged) → 4.2V at divider → ⚠️ TOO HIGH!
- 7.4V (nominal) → 3.7V at divider → ⚠️ TOO HIGH!
- 6.0V (empty) → 3.0V at divider → ✅ OK
```

### ⚠️ PROBLEM DETECTED!
The 2:1 divider ratio is **too high** for 2S battery packs! We need a 3:1 ratio instead.

### CORRECTED WIRING (Use This!)
```
         Battery Positive (6.0V - 8.4V)
                 │
               [20kΩ]  ← Resistor R1 (CHANGED)
                 │
                 ├───────── GPIO 34 (ADC1_CH6)
                 │
               [10kΩ]  ← Resistor R2
                 │
                GND

V_GPIO34 = V_Battery × (10kΩ / 30kΩ) = V_Battery × 0.333

Examples:
- 8.4V (fully charged) → 2.8V at divider → ✅ SAFE
- 7.4V (nominal) → 2.47V at divider → ✅ SAFE
- 6.0V (empty) → 2.0V at divider → ✅ SAFE
```

### Battery Level Calculation in Code
```c
// Read ADC value
uint32_t adc_value = adc1_get_raw(ADC1_CHANNEL_6);

// Convert to voltage (assuming 11dB attenuation, 0-3.3V range)
float v_gpio = (adc_value / 4095.0) * 3.3;

// Calculate actual battery voltage (3:1 divider)
float v_battery = v_gpio * 3.0;

// Determine battery percentage (6.0V to 8.4V range)
float battery_percent = ((v_battery - 6.0) / 2.4) * 100.0;
if (battery_percent > 100.0) battery_percent = 100.0;
if (battery_percent < 0.0) battery_percent = 0.0;
```

### ⚠️ Important Notes
1. **GPIO 34 is input-only** - perfect for ADC, cannot be used as output
2. **Use 1% tolerance resistors** for accurate voltage readings
3. **Calibrate in software** - ESP32 ADC is not perfectly linear
4. **11dB attenuation** recommended for full 0-3.3V range

---

## Important Notes

### ⚠️ Critical Safety Guidelines

1. **Power Supply Voltages**
   - Display: **3.3V ONLY** - 5V will damage it!
   - SD Card: Most modules are **3.3V ONLY**
   - Audio (MAX98357A): Can use 3.3V or 5V (check datasheet)
   - Encoder, Accelerometer: 3.3V

2. **Shared SPI Bus**
   - Display and SD card share GPIO 18 (clock) - this is normal
   - GPIO 19 serves dual purpose: display MOSI + SD card MISO
   - Works because display is write-only (never reads data back)

3. **Pin Conflicts Resolved**
   - ✅ Audio moved to GPIO 22 (I2S DOUT)
   - ✅ Accelerometer SCL moved to GPIO 14
   - ✅ Original conflict: GPIO 22 was planned for both I2C and audio

4. **Wire Management**
   - Use color coding for easier debugging
   - Keep wires short to reduce noise
   - Twist I2C wires together (SDA + SCL)
   - Keep power and ground wires thick (22-24 AWG)

### 🔧 Testing Sequence

**Test in this order:**

1. **Display** (already working ✅)
   - Test with solid colors and patterns

2. **SD Card** (next step 🔧)
   - Check mounting and file listing

3. **Rotary Encoder** (after SD)
   - Test rotation and button press

4. **Audio** (after encoder)
   - Test I2S output with test tone

5. **Accelerometer** (optional, last)
   - Test I2C communication and motion detection

### 📐 Physical Considerations

1. **Component Placement**
   - Display: Front of case (obviously!)
   - SD card: Accessible slot on side/back
   - Encoder: Top or side (easy to reach)
   - Audio module: Near speaker location
   - ESP32: Central location with short wire runs

2. **Wire Lengths**
   - Keep SPI wires < 6 inches for reliability
   - I2C can be longer (~12 inches ok)
   - Power wires should be shortest path

3. **Strain Relief**
   - Use hot glue on connector joints
   - Avoid sharp bends in wires
   - Secure loose wires with zip ties

### 🎯 Pin Changes Summary

This configuration resolves all conflicts from the original plan:

| Original Plan | Issue | New Assignment |
|---------------|-------|----------------|
| GPIO 22: I2C SCL + Audio | Conflict! | GPIO 22 → Audio only |
| GPIO 22: I2C SCL | Moved | GPIO 14 → I2C SCL |
| Battery divider: 2:1 | Too high voltage | Changed to 3:1 ratio |

---

## Next Steps

1. ✅ Display - Working!
2. 🔧 **Wire SD card module** (you are here!)
3. 🔧 Wire rotary encoder
4. 🔧 Order and wire audio module (MAX98357A recommended)
5. 🔧 Wire battery monitoring (with corrected 3:1 divider)
6. 🔧 Wire accelerometer (optional)

---

## Troubleshooting

### Display Issues
- **Multicolored noise**: Check 3.3V power, verify GPIO 19 for MOSI
- **Blank screen**: Check RST (GPIO 4) and DC (GPIO 16) connections
- **Dim display**: Check BL (GPIO 15) connection

### SD Card Issues
- **Not detected**: Check 3.3V power, CS (GPIO 17)
- **Mount failed**: Format card as FAT32
- **Slow reads**: Use Class 10 or UHS-1 card

### Audio Issues
- **No sound**: Check I2S pins (26, 25, 22)
- **Distorted**: Check power supply noise
- **Too quiet**: Adjust GAIN pin on MAX98357A

### Encoder Issues
- **Backwards rotation**: Swap CLK and DT pins
- **Button not working**: Check GPIO 27, ensure pullup enabled
- **Jittery**: Add debouncing delay in software

### I2C Issues
- **Device not found**: Check SDA (21) and SCL (14)
- **Communication errors**: Add 4.7kΩ pullup resistors
- **Unreliable**: Shorten wires, reduce I2C clock speed

---

## Reference Documents

- [CLAUDE.md](CLAUDE.md) - Complete project context and guidelines
- [BUILD.md](BUILD.md) - ESP-IDF build instructions
- [STATUS.md](STATUS.md) - Current development status
- [TASKS.md](TASKS.md) - Task breakdown and checklist
- [PROJECTPLAN.md](PROJECTPLAN.md) - High-level architecture

---

**Last Updated:** 2025-01-13
**Hardware Status:** Display verified ✅ | SD Card ready 🔧 | Audio pending 🔧
