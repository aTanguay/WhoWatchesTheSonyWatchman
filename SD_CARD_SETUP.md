# SD Card Setup Guide

Complete guide for setting up SD card storage for the Sony Watchman media player.

## 📦 Hardware Shopping List

### SD Card Module
**What to buy:** MicroSD Card SPI Module

| Component | Specs | Price | Where to Buy |
|-----------|-------|-------|--------------|
| MicroSD Module | SPI interface, 3.3V/5V | $2-5 | Amazon, Adafruit, AliExpress |
| MicroSD Card | 16-32GB, Class 10, FAT32 | $8-12 | Amazon, Best Buy |

**Search Terms:**
- "microsd card module spi"
- "micro sd adapter module arduino"
- "sd card breakout board"

### Recommended Modules
1. **HiLetgo/Generic** - $2-3 (Amazon)
   - Most common
   - Works well
   - Sometimes called "Catalex MicroSD"

2. **Adafruit MicroSD Breakout** - $7.50
   - High quality
   - Better documentation
   - Good for beginners

3. **SparkFun microSD Transflash Breakout** - $4.95
   - Reliable
   - Good build quality

## 🔌 Wiring Diagram

```
MicroSD Module              ESP32
┌─────────────┐            ┌──────────┐
│             │            │          │
│  VCC  ──────┼────────────┤ 3.3V     │
│  GND  ──────┼────────────┤ GND      │
│  MISO ──────┼────────────┤ GPIO 19  │
│  MOSI ──────┼────────────┤ GPIO 23  │ (shared with display)
│  SCK  ──────┼────────────┤ GPIO 18  │ (shared with display)
│  CS   ──────┼────────────┤ GPIO 17  │
│             │            │          │
└─────────────┘            └──────────┘
```

**Important Notes:**
- ✅ Use **3.3V** power (NOT 5V, unless module has voltage regulator)
- ✅ MOSI and SCK are **shared** with display (this is normal for SPI bus)
- ✅ Each device (display, SD card) has its own **CS** (chip select) pin
- ✅ Connect GND to both module **and** ESP32 common ground

## 💾 SD Card Selection

### Size
- **Recommended:** 16GB or 32GB
- **Maximum:** 32GB (FAT32 format limit)
- **Minimum:** 8GB

**Storage Capacity:**
- 8GB = ~5 TV show seasons
- 16GB = ~10 seasons
- 32GB = ~20 seasons

(Based on ~1.6GB per 20-episode season at our encoding settings)

### Speed Class
- **Minimum:** Class 10
- **Better:** UHS-I (U1)
- **Best:** UHS-I (U3) - overkill but won't hurt

**Why Class 10?**
Our video bitrate is ~500-800 kbps, so we need:
- Minimum: 0.5 MB/s
- Class 10 guarantees: 10 MB/s
- Plenty of headroom! ✅

### Brand Recommendations
1. **SanDisk** - Most reliable, good warranty
2. **Samsung EVO** - Excellent performance
3. **Lexar** - Good budget option

**Avoid:**
- No-name brands
- Suspiciously cheap cards (likely fake)
- Anything from random sellers

## 🔧 Formatting the SD Card

### Windows
1. Insert SD card in computer
2. Right-click drive → Format
3. File System: **FAT32** (NOT exFAT or NTFS)
4. Allocation size: Default
5. Click "Start"

### macOS
1. Open Disk Utility
2. Select SD card
3. Click "Erase"
4. Format: **MS-DOS (FAT)**
5. Scheme: Master Boot Record
6. Click "Erase"

### Linux
```bash
# Find device (usually /dev/sdb1 or /dev/mmcblk0p1)
lsblk

# Unmount if mounted
sudo umount /dev/sdb1

# Format as FAT32
sudo mkfs.vfat -F 32 /dev/sdb1
```

**IMPORTANT:** Make sure it's FAT32, not FAT16 or exFAT!

## 📁 Creating Directory Structure

After formatting, create this folder structure:

```
SD Card Root/
└── channels/
    ├── MST3K/
    │   ├── episode_01.avi
    │   ├── episode_02.avi
    │   └── episode_03.avi
    ├── The_Twilight_Zone/
    │   ├── episode_01.avi
    │   └── episode_02.avi
    └── Futurama/
        ├── episode_01.avi
        └── episode_02.avi
```

**Steps:**
1. Create folder named `channels` in root
2. Inside `channels`, create one folder per TV show
3. Put encoded `.avi` files inside each show folder

## 🎬 Video File Naming

**Recommended naming:**
- `episode_01.avi`
- `episode_02.avi`
- `s01e01.avi` (if tracking seasons)

**Rules:**
- ✅ Use consistent naming within each show
- ✅ Use leading zeros (episode_01, not episode_1)
- ✅ Lowercase is fine
- ✅ Supported extensions: `.avi`, `.mjpeg`, `.mjpg`
- ❌ Avoid spaces in filenames (use underscores)
- ❌ Avoid special characters

## 🧪 Testing SD Card

### 1. Physical Connection Test
```bash
# Build test program
pio run

# Flash and monitor
pio run -t upload -t monitor
```

Expected output:
```
I (123) SD_CARD: Initializing SD card...
I (234) SD_CARD: SD card mounted successfully
I (235) SD_CARD: SD card: 29.50 GB total, 29.30 GB free
```

### 2. File Operation Test
The test program will:
1. ✅ Mount SD card
2. ✅ Show capacity info
3. ✅ Create test file
4. ✅ Read test file
5. ✅ List channels (if any)

### 3. Channel Detection Test
Add a test video and check logs:
```
I (456) CHANNEL_MGR: Found 3 channels with content
I (457) CHANNEL_MGR: Channel 1: MST3K (5 episodes)
I (458) CHANNEL_MGR: Channel 2: The_Twilight_Zone (12 episodes)
```

## 🐛 Troubleshooting

### "Failed to mount SD card"
**Possible causes:**
1. **Wiring issue**
   - Check all 6 connections
   - Verify 3.3V power (NOT 5V)
   - Check for loose connections

2. **Card not inserted**
   - Push card fully into slot
   - Some modules have a "click" when seated

3. **Wrong format**
   - Re-format as FAT32
   - Try different SD card

4. **Module defective**
   - Test with Arduino first
   - Try different module

### "SD card detected but no channels found"
**Fix:**
1. Check folder structure: `/channels/ShowName/`
2. Verify files have `.avi` extension
3. Check file permissions (should be readable)
4. Re-insert SD card

### "Video files won't play"
**Fix:**
1. Verify encoding (use FFmpeg command from README)
2. Check file size (not too large)
3. Test file on computer first
4. Check SD card read speed (Class 10+)

### Slow reading / choppy playback
**Fix:**
1. Use better quality SD card
2. Reduce video bitrate in encoding
3. Check for fragmentation (reformat and re-copy)
4. Ensure Class 10 or faster card

## 📊 Capacity Planning

| TV Show | Episodes | Avg Episode Size | Season Size |
|---------|----------|------------------|-------------|
| MST3K (90min) | 20 | 120 MB | ~2.4 GB |
| Standard (22min) | 20 | 80 MB | ~1.6 GB |
| Drama (45min) | 20 | 160 MB | ~3.2 GB |

**32GB Card Examples:**
- 20 seasons of standard 22-min shows
- 13 seasons of 45-min shows
- 13 MST3K seasons
- Mix and match!

## 🔄 Updating Content

**To add new shows:**
1. Connect SD card to computer
2. Create new folder in `/channels/`
3. Copy encoded videos
4. Safely eject
5. Insert into Sony Watchman
6. Reboot (it will auto-detect)

**To remove shows:**
1. Delete folder from `/channels/`
2. Reboot device

No special tools needed - just drag and drop!

## ✅ Checklist

Before testing with your Watchman:

- [ ] SD card formatted as FAT32
- [ ] SD card is Class 10 or better
- [ ] `/channels/` folder created
- [ ] At least one show folder created
- [ ] Test video encoded and copied
- [ ] SD module wired to ESP32
- [ ] 3.3V power connected (NOT 5V!)
- [ ] All 6 wires connected properly
- [ ] Card fully inserted in module

## 📚 Additional Resources

- [SD Card Module Tutorial](https://randomnerdtutorials.com/esp32-microsd-card-arduino/)
- [ESP32 SD Card Guide](https://www.electroniclinic.com/sd-card-module-with-esp32-arduino/)
- [FAT32 Formatter](http://ridgecrop.co.uk/index.htm?guiformat.htm) - For cards >32GB

---

**Ready to go!** Once you have the module and SD card, the code is already set up to handle everything automatically.
