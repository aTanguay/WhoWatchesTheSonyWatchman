# Sony Watchman ESP32 Wiring Guide

**Version**: 1.0
**Last Updated**: 2025-11-06
**Print-Friendly**: Yes (optimized for black & white printing)

---

## Table of Contents
1. [Components List](#components-list)
2. [Tools Required](#tools-required)
3. [Pin Configuration Overview](#pin-configuration-overview)
4. [Detailed Wiring Instructions](#detailed-wiring-instructions)
5. [Power Supply Wiring](#power-supply-wiring)
6. [Testing Procedure](#testing-procedure)
7. [Troubleshooting](#troubleshooting)
8. [Safety Notes](#safety-notes)

---

## Components List

### Required Components

| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| **ESP32 Development Board** | ESP32-WROOM-32 or ESP32-DevKitC | 1 | 38-pin version recommended |
| **IPS Display** | ST7789 240x320 2.0" IPS LCD | 1 | SPI interface |
| **MicroSD Card Module** | SPI interface, 3.3V | 1 | Supports FAT32 |
| **MicroSD Card** | Class 10, 32GB | 1 | SanDisk or Samsung recommended |
| **Rotary Encoder** | KY-040 or compatible | 1 | With push button |
| **Audio Amplifier** | MAX98357A (I2S) | 1 | Or PAM8403 (analog) |
| **Speaker** | 8Ω 1-3W | 1 | Original Sony Watchman speaker ideal |
| **18650 Batteries** | 3000mAh or higher | 2 | Protected cells recommended |
| **18650 Battery Holder** | 2-cell series holder | 1 | Wired for series (6-8.4V) |
| **TP4056 Charging Module** | With protection circuit | 2 | One per battery, or 2S version |
| **Buck-Boost Converter** | MT3608 or similar, 3.3V output | 1 | Input: 6-8.4V, Output: 3.3V 2A+ |
| **Voltage Divider Resistors** | 10kΩ resistors | 2 | For battery voltage sensing |
| **Power Switch** | SPST toggle switch | 1 | Rated for 2A+ |
| **USB-C or Micro USB Port** | For charging | 1 | Matches TP4056 module |

### Optional Components

| Component | Specification | Quantity | Purpose |
|-----------|--------------|----------|---------|
| **Accelerometer** | MPU6050 or ADXL345 | 1 | Shake detection (future feature) |
| **LED Indicators** | 3mm LEDs (red, green) | 2 | Power/charging status |
| **Resistors for LEDs** | 220Ω | 2 | Current limiting |
| **Breadboard** | Half-size | 1 | Initial testing |
| **Jumper Wires** | Male-to-male, male-to-female | 40+ | Prototyping |
| **Perfboard/Protoboard** | 7x9cm or larger | 1 | Permanent assembly |

### Connectors & Hardware

- JST connectors (optional, for modular connections)
- Heat shrink tubing (various sizes)
- Solder and flux
- Double-sided tape or mounting brackets
- M2/M3 screws and standoffs

---

## Tools Required

### Essential Tools
- ☐ Soldering iron (adjustable, 300-400°C)
- ☐ Solder (60/40 or lead-free)
- ☐ Wire strippers
- ☐ Flush cutters
- ☐ Multimeter (for testing continuity and voltage)
- ☐ Small Phillips and flathead screwdrivers
- ☐ USB cable (for programming ESP32)

### Helpful Tools
- ☐ Helping hands / PCB holder
- ☐ Desoldering pump or wick
- ☐ Hot glue gun
- ☐ Label maker or tape
- ☐ Tweezers
- ☐ Wire organizers

---

## Pin Configuration Overview

### ESP32 Pin Assignments

```
ESP32-WROOM-32 Pinout (38-pin version)

         ┌─────────────────────┐
    3V3  │ 1               38 │  GND
    EN   │ 2               37 │  GPIO 23  ← MOSI (Display/SD)
    VP   │ 3               36 │  GPIO 22  ← I2S DOUT (Audio)
    VN   │ 4               35 │  TXD0
GPIO 34  │ 5 (ADC)         34 │  RXD0
GPIO 35  │ 6               33 │  GPIO 21
GPIO 32  │ 7 (Enc CLK)     32 │  GPIO 19  ← MISO (SD Card)
GPIO 33  │ 8 (Enc DT)      31 │  GPIO 18  ← SCK (Display/SD)
GPIO 25  │ 9 (I2S WS)      30 │  GPIO 5   ← CS Display
GPIO 26  │10 (I2S BCLK)    29 │  GPIO 17  ← CS SD Card
GPIO 27  │11 (Enc SW)      28 │  GPIO 16  ← DC (Display)
GPIO 14  │12               27 │  GPIO 4   ← RST (Display)
GPIO 12  │13               26 │  GPIO 0
    GND  │14               25 │  GPIO 2
GPIO 13  │15               24 │  GPIO 15  ← BL (Display)
    SD2  │16               23 │  SD1
    SD3  │17               22 │  CMD
    CMD  │18               21 │  CLK
    5V   │19               20 │  GND
         └─────────────────────┘
```

### Quick Reference Pin Table

| Function | ESP32 Pin | Component Pin | Wire Color Suggestion |
|----------|-----------|---------------|----------------------|
| **Display (SPI)** |
| MOSI | GPIO 23 | SDI/MOSI | Blue |
| SCK | GPIO 18 | SCK/CLK | Yellow |
| CS | GPIO 5 | CS | Orange |
| DC | GPIO 16 | DC/RS | Green |
| RST | GPIO 4 | RESET | White |
| BL | GPIO 15 | BL/LED | Purple |
| **SD Card (SPI)** |
| MISO | GPIO 19 | MISO/DO | Brown |
| MOSI | GPIO 23 | MOSI/DI | Blue (shared) |
| SCK | GPIO 18 | SCK/CLK | Yellow (shared) |
| CS | GPIO 17 | CS | Red |
| **Audio (I2S)** |
| BCLK | GPIO 26 | BCLK | Gray |
| WS (LRCLK) | GPIO 25 | LRC/WS | Pink |
| DOUT | GPIO 22 | DIN | Cyan |
| **Rotary Encoder** |
| CLK (A) | GPIO 32 | CLK | Black |
| DT (B) | GPIO 33 | DT | Brown |
| SW | GPIO 27 | SW | White |
| **Power** |
| Battery Sense | GPIO 34 | Voltage Divider | Red |
| 3.3V | 3V3 | Power | Red |
| GND | GND | Ground | Black |

---

## Detailed Wiring Instructions

### Step 1: Display Wiring (ST7789 240x320)

The display is the most critical component. Wire carefully.

**Display Pin Connections:**

```
ESP32           ST7789 Display
─────           ──────────────
GPIO 23  ────────→  SDA (MOSI)
GPIO 18  ────────→  SCL (SCK)
GPIO 5   ────────→  CS
GPIO 16  ────────→  DC (Data/Command)
GPIO 4   ────────→  RST (Reset)
GPIO 15  ────────→  BL (Backlight) *optional
3.3V     ────────→  VCC
GND      ────────→  GND
```

**Important Notes:**
- **Voltage**: ST7789 is 3.3V only! DO NOT connect to 5V.
- **Backlight**: Some displays have built-in backlight driver. If BL pin exists, connect to GPIO 15 for PWM brightness control. If no BL pin, backlight is always on.
- **Wire Length**: Keep wires as short as possible (< 10cm) for SPI reliability at 40MHz.

**Connection Tips:**
1. Solder wires to display first (it's smaller and more delicate)
2. Use 24-26 AWG solid core wire for breadboard, or stranded for final assembly
3. Test display alone before connecting other components

---

### Step 2: SD Card Module Wiring

The SD card shares SPI bus with display but has separate CS pin.

**SD Card Pin Connections:**

```
ESP32           SD Card Module
─────           ──────────────
GPIO 19  ────────→  MISO (DO)
GPIO 23  ────────→  MOSI (DI)  [shared with display]
GPIO 18  ────────→  SCK (CLK)  [shared with display]
GPIO 17  ────────→  CS
3.3V     ────────→  VCC
GND      ────────→  GND
```

**Important Notes:**
- **Voltage**: Most SD modules have onboard 3.3V regulator and work with 5V or 3.3V. Check your module's specs.
- **Card Detect**: If your module has CD (card detect) pin, leave it unconnected (not used in firmware).
- **Shared SPI**: MOSI and SCK are shared with display. Use a breadboard rail or solder multiple wires to same ESP32 pin.

**SD Card Format:**
- Must be formatted as FAT32
- For cards >32GB, use third-party formatting tools (Windows format tool won't allow FAT32 >32GB)

---

### Step 3: Audio Amplifier Wiring (MAX98357A I2S)

**MAX98357A Pin Connections:**

```
ESP32           MAX98357A
─────           ─────────
GPIO 26  ────────→  BCLK
GPIO 25  ────────→  LRC (Word Select)
GPIO 22  ────────→  DIN (Data In)
3.3V     ────────→  VIN
GND      ────────→  GND
             ┌────→  OUT+ ────→ Speaker +
             └────→  OUT- ────→ Speaker -
```

**Gain Setting (Optional):**
- MAX98357A has optional gain pins (usually 9dB, 12dB, 15dB)
- Leave unconnected for default 9dB gain
- For louder output, connect gain pin to VIN for 15dB

**Important Notes:**
- MAX98357A is a Class D amplifier - very efficient for battery use
- No external components needed (capacitors built-in)
- Speaker should be 4Ω or 8Ω, 1-3W rating

**Alternative: PAM8403 (Analog Audio)**

If using PAM8403 (cheaper but requires DAC output instead of I2S):

```
ESP32           PAM8403
─────           ───────
GPIO 25  ────────→  L-IN (via 10kΩ resistor)
GPIO 26  ────────→  R-IN (via 10kΩ resistor) [or bridge to L-IN]
3.3V     ────────→  VCC
GND      ────────→  GND
             ┌────→  L-OUT+ ────→ Speaker +
             └────→  L-OUT- ────→ Speaker -
```

Note: Firmware uses I2S by default. For analog output, code changes required.

---

### Step 4: Rotary Encoder Wiring (KY-040)

**Encoder Pin Connections:**

```
ESP32           KY-040 Encoder
─────           ──────────────
GPIO 32  ────────→  CLK (A phase)
GPIO 33  ────────→  DT (B phase)
GPIO 27  ────────→  SW (button)
3.3V     ────────→  + (VCC)
GND      ────────→  GND
```

**Important Notes:**
- KY-040 has onboard pull-up resistors, no external resistors needed
- Some encoders have 5 pins (extra is usually duplicate GND)
- Direction can be swapped if rotation feels backwards (swap CLK/DT in code or wiring)

**Mechanical Mounting:**
- Encoder will mount where original Sony Watchman tuning knob was
- May need shaft adapter/coupler to fit original knob

---

### Step 5: Power Supply Wiring

This is the most complex part. Follow carefully!

#### Battery Configuration

```
2x 18650 Batteries in Series (6.0V - 8.4V)

  [Battery 1]     [Battery 2]
  │+      -│     │+      -│
  └────┬───┘     └───┬────┘
       │             │
       └─────┬───────┘
             │
          6-8.4V
```

#### Charging Circuit

```
                    ┌─── USB-C/Micro USB (charging input)
                    │
              ┌─────▼──────┐
              │  TP4056 x2  │  (2S charging module OR 2x 1S modules)
              │  Charge     │
              │  Protection │
              └─────┬───────┘
                    │
                6-8.4V
```

#### Voltage Divider (Battery Sensing)

```
Battery +  ──────┐
                 │
               [10kΩ]  ← R1
                 │
                 ├────────→  GPIO 34 (ADC)
                 │
               [10kΩ]  ← R2
                 │
GND  ────────────┘

Voltage divider ratio: 2:1
Max input to ADC: 4.2V (safe for ESP32's 3.3V ADC with headroom)
```

#### Buck-Boost Converter (3.3V Regulated)

```
Battery  ────→  MT3608 Buck-Boost  ────→  3.3V Out
6-8.4V          (Adjust pot for 3.3V)      │
                                           └──→ ESP32 3V3
                                           └──→ All 3.3V components
```

**Adjustment Procedure:**
1. Connect battery to input (through power switch)
2. Disconnect ESP32 and all components from output
3. Use multimeter on output
4. Adjust potentiometer until output reads exactly 3.3V
5. Verify with load: connect 100Ω resistor, should still read 3.3V

#### Complete Power Wiring Diagram

```
┌─────────────┐
│  Battery 1  │ 4.2V
│  (18650)    │
└──────┬──────┘
       │ +
       ├──────────────────────┐
       │                      │
┌──────▼──────┐               │
│  Battery 2  │ 4.2V          │ 6-8.4V Total
│  (18650)    │               │
└──────┬──────┘               │
       │ -                    │
       │                      │
       └──────┬───────────────┘
              │
        ┌─────▼──────┐
        │  TP4056    │ ← USB charging port
        │  Charger   │
        └─────┬──────┘
              │
        ┌─────▼──────┐
        │   Power    │ ← SPST Switch
        │   Switch   │
        └─────┬──────┘
              │
              ├─────────────────────────→ Voltage Divider → GPIO 34
              │
        ┌─────▼──────┐
        │  MT3608    │
        │ Buck-Boost │
        │ 6-8V → 3.3V│
        └─────┬──────┘
              │ 3.3V
              ├────────→ ESP32 3V3 pin
              ├────────→ Display VCC
              ├────────→ SD Card VCC
              ├────────→ Encoder VCC
              ├────────→ Audio Amp VCC
              │
             GND (common ground for all)
```

**Critical Safety Notes:**
- ⚠️ **DO NOT** connect battery directly to ESP32 (max 3.6V!)
- ⚠️ Always use protected 18650 cells
- ⚠️ Double-check polarity before connecting batteries
- ⚠️ Add a fuse (1-2A) between battery and circuit for extra safety

---

### Step 6: Optional Accelerometer (Future Feature)

If you want shake detection, add MPU6050 or ADXL345:

**MPU6050 Pin Connections:**

```
ESP32           MPU6050
─────           ───────
GPIO 21  ────────→  SDA (I2C Data)
GPIO 22  ────────→  SCL (I2C Clock)
3.3V     ────────→  VCC
GND      ────────→  GND
             NC  ←─  INT (interrupt, optional)
```

Note: This is not yet implemented in firmware. Reserve these pins for future use.

---

## Complete Wiring Checklist

Print this section and check off as you wire:

### Display (ST7789)
- [ ] GPIO 23 → SDA (MOSI)
- [ ] GPIO 18 → SCL (SCK)
- [ ] GPIO 5 → CS
- [ ] GPIO 16 → DC
- [ ] GPIO 4 → RST
- [ ] GPIO 15 → BL (if available)
- [ ] 3.3V → VCC
- [ ] GND → GND

### SD Card Module
- [ ] GPIO 19 → MISO
- [ ] GPIO 23 → MOSI (shared with display)
- [ ] GPIO 18 → SCK (shared with display)
- [ ] GPIO 17 → CS
- [ ] 3.3V → VCC
- [ ] GND → GND
- [ ] SD card formatted as FAT32
- [ ] Test content on SD card

### Audio (MAX98357A)
- [ ] GPIO 26 → BCLK
- [ ] GPIO 25 → LRC
- [ ] GPIO 22 → DIN
- [ ] 3.3V → VIN
- [ ] GND → GND
- [ ] Speaker connected (8Ω, 1-3W)

### Rotary Encoder (KY-040)
- [ ] GPIO 32 → CLK
- [ ] GPIO 33 → DT
- [ ] GPIO 27 → SW
- [ ] 3.3V → VCC
- [ ] GND → GND

### Power System
- [ ] 2x 18650 batteries in series
- [ ] TP4056 charging module installed
- [ ] Power switch installed
- [ ] Voltage divider (2x 10kΩ) to GPIO 34
- [ ] Buck-boost converter adjusted to 3.3V
- [ ] 3.3V distributed to all components
- [ ] Common ground verified for all components

---

## Testing Procedure

### Pre-Power Tests (Do These First!)

1. **Visual Inspection**
   - [ ] Check all solder joints for bridges or cold joints
   - [ ] Verify no loose wires
   - [ ] Confirm polarity of batteries and power connections

2. **Continuity Tests** (with multimeter, power OFF)
   - [ ] Check 3.3V rail continuity to all components
   - [ ] Check GND continuity to all components
   - [ ] Verify no shorts between 3.3V and GND
   - [ ] Verify no shorts between signal pins

3. **Resistance Test**
   - [ ] Measure resistance between 3.3V and GND (should be >1kΩ)

### Power-On Tests

4. **Voltage Tests** (batteries disconnected initially)
   - [ ] Set buck-boost output to exactly 3.3V (use multimeter)
   - [ ] Verify voltage divider outputs ~3.3V with 6.6V input

5. **First Power-On** (ESP32 not yet programmed)
   - [ ] Connect batteries (through switch)
   - [ ] Turn on power switch
   - [ ] Check 3.3V rail voltage (should be 3.25-3.35V)
   - [ ] Check ESP32 board LED (should light up or blink)
   - [ ] Display backlight should turn on (if display has always-on backlight)

6. **Program ESP32**
   - [ ] Connect ESP32 to computer via USB
   - [ ] Flash firmware using `idf.py flash`
   - [ ] Monitor serial output: `idf.py monitor`

7. **Component Tests** (watch serial monitor)
   - [ ] Display initialization message appears
   - [ ] Cyan splash screen appears on display
   - [ ] SD card mount successful message
   - [ ] Battery percentage reported correctly
   - [ ] Channels scanned and found
   - [ ] Video playback starts

8. **Interactive Tests**
   - [ ] Rotate encoder: channel switches, serial log confirms
   - [ ] Press encoder button: video pauses, press again to resume
   - [ ] Long-press encoder: skips to next episode
   - [ ] Audio plays through speaker (confirm sync with video)

---

## Troubleshooting

### Display Issues

**Problem: White/blank screen**
- Check 3.3V power to display (use multimeter)
- Verify all 8 connections (especially MOSI, SCK, CS, DC)
- Check wire lengths (keep < 10cm for SPI reliability)
- Verify display model matches ST7789 (not ST7735 or ILI9341)

**Problem: Garbage/random pixels**
- Wrong display driver in code
- Poor SPI connections (re-solder)
- Electrical interference (add 100nF capacitor near display VCC/GND)

**Problem: Display very dim**
- Backlight not connected or not powered
- Check GPIO 15 connection if using PWM backlight control

### SD Card Issues

**Problem: SD card not detected**
- Verify CS pin (GPIO 17)
- Check SD card is formatted as FAT32
- Try different SD card (some are incompatible)
- Ensure card is fully inserted
- Check MISO connection (often forgotten)

**Problem: Files not found**
- Verify folder structure: /sdcard/channels/channel_XX/
- Check file naming: episode_01.avi, episode_02.avi (zero-padded)
- Confirm files are MJPEG format (see CONTENTGUIDE.MD)

### Audio Issues

**Problem: No audio**
- Check speaker polarity (try reversing if no sound)
- Verify speaker impedance (4-8Ω)
- Check all 3 I2S pins (BCLK, WS, DIN)
- Verify MAX98357A power (3.3V)

**Problem: Audio distorted/crackling**
- Speaker impedance too high or too low
- Adjust gain on MAX98357A (lower gain)
- Check for loose connections
- Verify SD card read speed (use Class 10+ card)

**Problem: Audio out of sync with video**
- This requires firmware tuning (A/V sync parameters)
- Report FPS achieved vs. target 15 FPS
- May need to adjust frame timing in video_player.c

### Encoder Issues

**Problem: Encoder not responding**
- Check 3.3V power to encoder
- Verify all 3 pins (CLK, DT, SW)
- Try rotating slowly and steadily
- Check serial monitor for encoder events

**Problem: Encoder direction backwards**
- Swap CLK and DT pins (either in wiring or code)

**Problem: Encoder too sensitive/skips**
- Check debouncing in firmware (should be handled)
- Try different encoder (KY-040 quality varies)

### Power Issues

**Problem: ESP32 resets/reboots randomly**
- Insufficient current from buck-boost (use 2A+ rated converter)
- Add 470µF-1000µF capacitor across 3.3V rail
- Check battery voltage (charge if <6.5V)

**Problem: Battery drains quickly**
- Measure actual current draw (should be 200-400mA during playback)
- Display brightness too high (adjust in code)
- SD card read issues (fragmented card, slow card)

**Problem: Battery percentage incorrect**
- Check voltage divider resistors (should both be exactly 10kΩ)
- Verify connection to GPIO 34
- Calibrate in firmware if needed

### General Debugging

**Use Serial Monitor:**
```bash
idf.py -p /dev/ttyUSB0 monitor
```

Watch for:
- Initialization messages from each component
- Heap memory reports (should stay stable)
- Error messages
- Frame rate reports

**Check Logs:**
- "Free heap" should be >100KB
- "Battery: XX%" should match actual charge
- "Found X channels" should match SD card content
- FPS should be 12-15 (target 15)

---

## Common Mistakes to Avoid

❌ **Connecting 5V to 3.3V components** → Instant damage to ESP32, display, or SD card
✅ Use only 3.3V regulated power

❌ **Forgetting common ground** → Nothing works, erratic behavior
✅ Connect GND from all components together

❌ **Long/loose SPI wires** → Display flickers, SD card errors
✅ Keep wires short (<10cm) and secure

❌ **Wrong CS pins** → Display and SD card conflict
✅ Display CS = GPIO 5, SD CS = GPIO 17 (different pins!)

❌ **Reversed battery polarity** → Smoke, fire, damage
✅ Double-check + and - before connecting

❌ **Over-tightening screws on ESP32** → Cracks PCB
✅ Snug but not tight

❌ **No current limiting for LEDs** → Burn out LEDs or GPIO pins
✅ Always use 220Ω resistors with LEDs

---

## Safety Notes

### Battery Safety
⚠️ **18650 lithium cells can be dangerous if mishandled**

- ✅ Only use protected cells (with built-in protection circuit)
- ✅ Check polarity before connecting (+ and -)
- ✅ Never short-circuit batteries
- ✅ Charge in a safe location (fireproof container recommended)
- ✅ Stop using if battery gets hot, swollen, or damaged
- ✅ Store batteries at 40-60% charge if not using for extended periods

### Soldering Safety
- ✅ Use ventilation (fume extractor or open window)
- ✅ Don't touch iron tip (400°C!)
- ✅ Use proper stand for iron
- ✅ Wash hands after soldering (lead exposure)

### Electrical Safety
- ✅ Disconnect power before making wiring changes
- ✅ Check for shorts with multimeter before powering on
- ✅ Add fuse for overcurrent protection
- ✅ Use heat shrink tubing to insulate exposed connections

---

## Next Steps After Wiring

1. **Initial Test** (on breadboard)
   - Verify all components work
   - Test for at least 30 minutes
   - Check for overheating components

2. **Performance Testing**
   - Measure actual FPS achieved
   - Test battery life (aim for 10+ hours)
   - Verify A/V sync quality
   - Test channel switching speed

3. **Physical Integration** (into Sony Watchman case)
   - Carefully disassemble Watchman
   - Plan component layout
   - Create mounting brackets (3D printed or DIY)
   - Route wiring neatly
   - Test fit before final assembly

4. **Final Assembly**
   - Solder permanent connections (replace breadboard)
   - Add strain relief for wires
   - Secure all components
   - Test in case before closing
   - Enjoy your retro mini TV! 📺

---

## Quick Reference: GPIO Summary

```
Display:   23, 18, 5, 16, 4, 15
SD Card:   19, 23, 18, 17  (shares 23, 18 with display)
Audio:     26, 25, 22
Encoder:   32, 33, 27
Battery:   34 (ADC input only)
Reserved:  21, 22 (I2C for future accelerometer)
```

---

## Revision History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2025-11-06 | Initial release |

---

**Questions or Issues?**
Check TROUBLESHOOTING section above, or see project documentation in PROJECTPLAN.MD and BUILD.MD.

**Good luck with your build! 🔧📺✨**

---

*This guide is part of the Sony Watchman ESP32 Retro Media Player project.*
*For more information, see: [github.com/aTanguay/WhoWatchesTheSonyWatchman](https://github.com/aTanguay/WhoWatchesTheSonyWatchman)*
