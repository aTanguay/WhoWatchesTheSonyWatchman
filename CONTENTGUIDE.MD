# Content Encoding Guide for ESP32 Retro Media Player

This guide covers how to convert your video content (TV shows, movies) into a format optimized for playback on the ESP32 with a small 1.7" IPS display.

## Table of Contents
- [Display Specifications](#display-specifications)
- [Video Format Requirements](#video-format-requirements)
- [Tool Options](#tool-options)
- [Handbrake Batch Processing](#handbrake-batch-processing)
- [FFmpeg Batch Processing](#ffmpeg-batch-processing)
- [Quality Settings Guide](#quality-settings-guide)
- [Testing Your Encoded Videos](#testing-your-encoded-videos)
- [Storage Planning](#storage-planning)

---

## Display Specifications

### Common 1.7" Display Resolutions
- **ST7735**: 128x160 pixels (common, affordable)
- **ST7789**: 240x240 or 240x280 pixels (higher quality)

### Target Specifications
- **Resolution**: 128x160 or 240x280 (choose based on your display)
- **Frame Rate**: 15 FPS (balance of smoothness and performance)
- **Aspect Ratio**: Content will be letterboxed/pillarboxed to fit
- **Color Depth**: 16-bit (RGB565) - handled by display

---

## Video Format Requirements

### Recommended Format: MJPEG in AVI Container

**Why MJPEG?**
- Simple decoder (just sequential JPEG images)
- No inter-frame dependencies (easy seeking)
- Well-supported libraries for ESP32
- Predictable performance

**Container**: AVI (simple structure, easy to parse)

### Video Specifications
- **Codec**: MJPEG (Motion JPEG)
- **Resolution**: 128x160 or 240x280 (match your display)
- **Frame Rate**: 15 FPS
- **Bitrate**: 400-800 kbps (adjust for quality/size)
- **Quality**: JPEG quality 6-8 (lower number = better quality)

### Audio Specifications
- **Codec**: MP3 (widely supported) or PCM (uncompressed)
- **Sample Rate**: 22050 Hz (half of CD quality, adequate)
- **Channels**: Mono (saves space, small speaker)
- **Bitrate**: 64 kbps for MP3

### File Size Estimates
- **22-minute episode at 128x160**: ~80-120 MB
- **22-minute episode at 240x280**: ~150-250 MB
- **44-minute episode at 128x160**: ~160-240 MB

---

## Tool Options

### Option 1: FFmpeg (Recommended - Most Control)
- **Type**: Command-line tool
- **Cost**: Free and open-source
- **Best for**: Power users, automation, batch processing
- **Pros**: Maximum control, scriptable, fast
- **Cons**: Steeper learning curve, command-line only
- **Download**: https://ffmpeg.org/download.html

### Option 2: Handbrake (Great for Beginners)
- **Type**: GUI application with CLI option
- **Cost**: Free and open-source
- **Best for**: Visual workflow, queue-based batch processing
- **Pros**: User-friendly interface, presets, built-in queue
- **Cons**: Less control over MJPEG settings, may need custom presets
- **Download**: https://handbrake.fr/downloads.php

### Option 3: Hybrid (Advanced)
- **Type**: GUI front-end for multiple encoders including FFmpeg
- **Cost**: Free
- **Best for**: Users who want GUI with FFmpeg's power
- **Download**: http://www.selur.de/

---

## Handbrake Batch Processing

### Important Note About MJPEG
Handbrake primarily focuses on H.264/H.265 encoding. For MJPEG, you'll need to use **FFmpeg** or create a custom preset. However, you can use Handbrake for initial testing with H.264, then switch to FFmpeg for final MJPEG encoding.

### Using Handbrake GUI for Batch Processing

#### Step 1: Create a Preset

1. Open Handbrake
2. Load a sample video file
3. Configure settings:
   - **Dimensions**: Set width to 128 (or 240) - height will auto-calculate
   - **Filters**: Add "Crop and Scale" if needed
   - **Video Codec**: H.264 (x264) - since MJPEG isn't well supported
   - **Frame Rate**: Set to 15 FPS constant
   - **Quality**: Use constant quality ~28 (experiment)
   - **Audio**: AAC codec, Mono, 64kbps, 22.05kHz

4. Click "Presets" → "Save New Preset"
5. Name it "ESP32_128x160" or similar

#### Step 2: Add Files to Queue

1. Click "Open Source" → "Folder" (select folder with episodes)
2. Or add files individually
3. Select your preset from the preset list
4. Click "Add to Queue" for each file
5. Set output folder

#### Step 3: Process Queue

1. Click "Queue" button (top toolbar)
2. Review all items
3. Click "Start Encode" (green play button)
4. Handbrake will process all files sequentially

### Using Handbrake CLI for Batch Processing

Handbrake has a command-line interface (`HandBrakeCLI`) for scripting.

#### Linux/Mac Batch Script
```bash
#!/bin/bash
# Process all MP4 files in a directory

INPUT_DIR="/path/to/episodes"
OUTPUT_DIR="/path/to/encoded"
mkdir -p "$OUTPUT_DIR"

for file in "$INPUT_DIR"/*.mp4; do
    filename=$(basename "$file" .mp4)
    HandBrakeCLI -i "$file" \
        -o "$OUTPUT_DIR/${filename}_esp32.mp4" \
        --preset "Very Fast 480p30" \
        --width 128 \
        --rate 15 \
        --audio 1 \
        --aencoder copy:aac \
        --ab 64 \
        --mixdown mono \
        --arate 22.05
    echo "Completed: $filename"
done
```

#### Windows Batch Script
```batch
@echo off
setlocal enabledelayedexpansion

set INPUT_DIR=C:\Videos\Episodes
set OUTPUT_DIR=C:\Videos\Encoded
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

for %%f in ("%INPUT_DIR%\*.mp4") do (
    echo Processing: %%~nf
    HandBrakeCLI.exe -i "%%f" ^
        -o "%OUTPUT_DIR%\%%~nf_esp32.mp4" ^
        --preset "Very Fast 480p30" ^
        --width 128 ^
        --rate 15 ^
        --audio 1 ^
        --aencoder copy:aac ^
        --ab 64 ^
        --mixdown mono ^
        --arate 22.05
)
echo All files processed!
pause
```

---

## FFmpeg Batch Processing

FFmpeg gives you complete control and can create MJPEG files directly.

### Single File Encoding Example

#### For 128x160 Display
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

#### For 240x280 Display
```bash
ffmpeg -i input.mp4 \
  -vf "scale=240:280:force_original_aspect_ratio=decrease,pad=240:280:(ow-iw)/2:(oh-ih)/2" \
  -r 15 \
  -q:v 6 \
  -vcodec mjpeg \
  -acodec libmp3lame \
  -ar 22050 \
  -ac 1 \
  -b:a 64k \
  output.avi
```

### Parameter Explanation

- `-i input.mp4`: Input file
- `-vf "scale=..."`: Video filter to resize and pad
  - `scale=128:160`: Target dimensions
  - `force_original_aspect_ratio=decrease`: Shrink to fit, maintain aspect
  - `pad=128:160:(ow-iw)/2:(oh-ih)/2`: Add black bars to center image
- `-r 15`: Frame rate (15 FPS)
- `-q:v 8`: JPEG quality (2=best, 31=worst; 6-8 is good balance)
- `-vcodec mjpeg`: Use MJPEG video codec
- `-acodec libmp3lame`: Use MP3 audio codec
- `-ar 22050`: Audio sample rate (22.05 kHz)
- `-ac 1`: Audio channels (1=mono)
- `-b:a 64k`: Audio bitrate (64 kbps)
- `output.avi`: Output file (AVI container)

### Batch Processing with FFmpeg

#### Linux/Mac Bash Script
```bash
#!/bin/bash
# Batch encode all MP4 files in a directory to MJPEG AVI for ESP32

INPUT_DIR="/path/to/source/episodes"
OUTPUT_DIR="/path/to/encoded/episodes"
RESOLUTION="128:160"  # Change to "240:280" for larger display
QUALITY="8"           # Lower = better quality (6-8 recommended)

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Counter for progress
count=0
total=$(ls -1 "$INPUT_DIR"/*.mp4 2>/dev/null | wc -l)

echo "Found $total video files to process"
echo "Starting batch encoding..."

for input_file in "$INPUT_DIR"/*.mp4; do
    # Skip if no files found
    [ -e "$input_file" ] || continue

    # Get filename without extension
    filename=$(basename "$input_file" .mp4)
    output_file="$OUTPUT_DIR/${filename}.avi"

    # Increment counter
    ((count++))

    echo ""
    echo "[$count/$total] Processing: $filename"
    echo "----------------------------------------"

    # Run FFmpeg
    ffmpeg -i "$input_file" \
        -vf "scale=$RESOLUTION:force_original_aspect_ratio=decrease,pad=$RESOLUTION:(ow-iw)/2:(oh-ih)/2" \
        -r 15 \
        -q:v "$QUALITY" \
        -vcodec mjpeg \
        -acodec libmp3lame \
        -ar 22050 \
        -ac 1 \
        -b:a 64k \
        "$output_file"

    if [ $? -eq 0 ]; then
        echo "✓ Successfully encoded: $filename"

        # Show file size
        size=$(du -h "$output_file" | cut -f1)
        echo "  Output size: $size"
    else
        echo "✗ Error encoding: $filename"
    fi
done

echo ""
echo "========================================="
echo "Batch encoding complete!"
echo "Processed $count files"
echo "Output directory: $OUTPUT_DIR"
```

#### Windows PowerShell Script
```powershell
# Batch encode all MP4 files to MJPEG AVI for ESP32

$INPUT_DIR = "C:\Videos\Source"
$OUTPUT_DIR = "C:\Videos\Encoded"
$RESOLUTION = "128:160"  # Change to "240:280" for larger display
$QUALITY = "8"           # Lower = better quality (6-8 recommended)

# Create output directory if it doesn't exist
if (!(Test-Path $OUTPUT_DIR)) {
    New-Item -ItemType Directory -Path $OUTPUT_DIR | Out-Null
}

# Get all MP4 files
$files = Get-ChildItem -Path $INPUT_DIR -Filter *.mp4
$total = $files.Count
$count = 0

Write-Host "Found $total video files to process"
Write-Host "Starting batch encoding..."

foreach ($file in $files) {
    $count++
    $filename = $file.BaseName
    $outputFile = Join-Path $OUTPUT_DIR "$filename.avi"

    Write-Host ""
    Write-Host "[$count/$total] Processing: $filename"
    Write-Host "----------------------------------------"

    # Build FFmpeg command
    $ffmpegArgs = @(
        "-i", $file.FullName,
        "-vf", "scale=$RESOLUTION:force_original_aspect_ratio=decrease,pad=$RESOLUTION`:(ow-iw)/2:(oh-ih)/2",
        "-r", "15",
        "-q:v", $QUALITY,
        "-vcodec", "mjpeg",
        "-acodec", "libmp3lame",
        "-ar", "22050",
        "-ac", "1",
        "-b:a", "64k",
        $outputFile
    )

    # Run FFmpeg
    & ffmpeg $ffmpegArgs

    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Successfully encoded: $filename" -ForegroundColor Green

        # Show file size
        $size = (Get-Item $outputFile).Length / 1MB
        Write-Host "  Output size: $([math]::Round($size, 2)) MB"
    } else {
        Write-Host "✗ Error encoding: $filename" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "========================================="
Write-Host "Batch encoding complete!"
Write-Host "Processed $count files"
Write-Host "Output directory: $OUTPUT_DIR"
```

#### Windows Batch Script (CMD)
```batch
@echo off
setlocal enabledelayedexpansion

set INPUT_DIR=C:\Videos\Source
set OUTPUT_DIR=C:\Videos\Encoded
set RESOLUTION=128:160
set QUALITY=8

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

set count=0
set total=0

rem Count total files
for %%f in ("%INPUT_DIR%\*.mp4") do set /a total+=1

echo Found %total% video files to process
echo Starting batch encoding...

for %%f in ("%INPUT_DIR%\*.mp4") do (
    set /a count+=1
    set filename=%%~nf

    echo.
    echo [!count!/%total%] Processing: !filename!
    echo ----------------------------------------

    ffmpeg -i "%%f" ^
        -vf "scale=%RESOLUTION%:force_original_aspect_ratio=decrease,pad=%RESOLUTION%:(ow-iw)/2:(oh-ih)/2" ^
        -r 15 ^
        -q:v %QUALITY% ^
        -vcodec mjpeg ^
        -acodec libmp3lame ^
        -ar 22050 ^
        -ac 1 ^
        -b:a 64k ^
        "%OUTPUT_DIR%\!filename!.avi"

    if !errorlevel! equ 0 (
        echo Successfully encoded: !filename!
    ) else (
        echo Error encoding: !filename!
    )
)

echo.
echo =========================================
echo Batch encoding complete!
echo Processed %count% files
echo Output directory: %OUTPUT_DIR%
pause
```

### Advanced: Two-Pass Encoding for Better Quality

If you want even better quality control, you can use two-pass encoding:

```bash
# First pass - analyze
ffmpeg -i input.mp4 \
  -vf "scale=128:160:force_original_aspect_ratio=decrease,pad=128:160:(ow-iw)/2:(oh-ih)/2" \
  -r 15 \
  -vcodec mjpeg \
  -b:v 600k \
  -pass 1 \
  -an \
  -f avi /dev/null

# Second pass - encode
ffmpeg -i input.mp4 \
  -vf "scale=128:160:force_original_aspect_ratio=decrease,pad=128:160:(ow-iw)/2:(oh-ih)/2" \
  -r 15 \
  -vcodec mjpeg \
  -b:v 600k \
  -pass 2 \
  -acodec libmp3lame \
  -ar 22050 \
  -ac 1 \
  -b:a 64k \
  output.avi
```

---

## Quality Settings Guide

### Finding the Right Balance

The main trade-off is **quality vs. file size**. Here's how to adjust:

#### Video Quality (`-q:v` for MJPEG)
- **2-5**: Excellent quality, large files (~200-300 MB/episode)
- **6-8**: Good quality, reasonable size (~80-150 MB/episode) ⭐ **Recommended**
- **9-12**: Acceptable quality, small files (~50-80 MB/episode)
- **13+**: Poor quality, very small files (not recommended)

#### Audio Bitrate (`-b:a`)
- **96k**: Better audio quality (adds ~16 MB per 22-min episode)
- **64k**: Good balance ⭐ **Recommended**
- **48k**: Acceptable, saves space
- **32k**: Noticeably worse quality

#### Frame Rate (`-r`)
- **20 FPS**: Smoother motion (may stress ESP32)
- **15 FPS**: Good balance ⭐ **Recommended**
- **12 FPS**: Slightly choppy but works
- **10 FPS**: Noticeably choppy (only for desperate optimization)

### Testing Methodology

1. **Encode a 30-second clip** at different quality levels:
```bash
# Test clip extraction
ffmpeg -i episode.mp4 -t 30 -c copy test_clip.mp4

# Encode at different qualities
for q in 6 7 8 9; do
    ffmpeg -i test_clip.mp4 \
        -vf "scale=128:160:force_original_aspect_ratio=decrease,pad=128:160:(ow-iw)/2:(oh-ih)/2" \
        -r 15 -q:v $q -vcodec mjpeg \
        -acodec libmp3lame -ar 22050 -ac 1 -b:a 64k \
        test_q${q}.avi
done
```

2. **Test on ESP32**: Load each test file and evaluate:
   - Playback smoothness
   - Visual quality on small screen
   - File size

3. **Choose your setting** based on results

### Resolution Decision

**128x160 (ST7735)**
- Pros: Smaller files, faster decoding, longer battery life
- Cons: Less detail (not noticeable on 1.7" screen)
- Best for: Cartoons, sitcoms, longer battery life

**240x280 (ST7789)**
- Pros: Better clarity, more detail
- Cons: Larger files, slower decoding, more power
- Best for: Live-action shows, films, if you have the display

---

## Testing Your Encoded Videos

### Quick Test with VLC Media Player

Before loading onto the ESP32, verify your encoded videos play correctly:

1. Open the AVI file in VLC
2. Check: Tools → Codec Information
   - Video Codec should show "MJPEG" or "Motion JPEG"
   - Audio Codec should show "MP3"
   - Resolution should match your target (128x160 or 240x280)
   - Frame rate should be 15 FPS

### ESP32 Test Checklist

When testing on actual hardware:

- [ ] Video plays without freezing
- [ ] Frame rate is smooth (15 FPS)
- [ ] Audio is in sync with video (within 100ms)
- [ ] No audio crackling or skipping
- [ ] SD card read speed is sufficient
- [ ] Battery life is acceptable
- [ ] Can seek/skip without issues
- [ ] File switching works smoothly

---

## Storage Planning

### SD Card Requirements

**Minimum**: 16 GB (can hold ~5-10 seasons depending on settings)
**Recommended**: 32 GB (holds ~13-20 seasons)
**Maximum useful**: 64-128 GB (limited by ESP32 addressing)

### Example Storage Calculation

**Assumptions:**
- Episode length: 22 minutes
- Quality setting: `-q:v 8` (good quality)
- Resolution: 128x160
- File size per episode: ~100 MB

**Capacity:**
- 16 GB card: ~150 episodes (6-7 full seasons)
- 32 GB card: ~310 episodes (13-15 full seasons)
- 64 GB card: ~620 episodes (26-30 full seasons)

### SD Card Recommendations

**Best Brands:**
- SanDisk Extreme/Ultra
- Samsung EVO/PRO
- Kingston Canvas

**Specifications:**
- Class 10 minimum
- UHS-I (U1 or U3) for better performance
- Avoid cheap no-name brands (unreliable)

**Format**: FAT32 (ESP32 compatibility)
- Windows: Use "FAT32 Format" tool for >32GB cards
- Mac: Disk Utility, select MS-DOS (FAT)
- Linux: `mkfs.vfat -F 32 /dev/sdX1`

---

## Organizing Your Content

### Recommended Folder Structure

```
/SD_CARD/
├── channels.json
├── channel_01/
│   ├── metadata.json
│   ├── episode_01.avi
│   ├── episode_02.avi
│   └── ...
├── channel_02/
│   ├── metadata.json
│   ├── episode_01.avi
│   └── ...
└── system/
    ├── settings.json
    └── state.json
```

### Naming Convention

**Episodes**: Use zero-padded numbers for proper sorting
- ✓ Good: `episode_01.avi`, `episode_02.avi`, ..., `episode_23.avi`
- ✗ Bad: `episode_1.avi`, `episode_2.avi`, ..., `episode_23.avi`
  (episode_2.avi would sort after episode_23.avi)

### Metadata Example

**metadata.json** (in each channel folder):
```json
{
  "channel_id": 1,
  "show_name": "The Simpsons",
  "season": 5,
  "episode_count": 22,
  "description": "Season 5 of The Simpsons",
  "thumbnail": "thumb.jpg"
}
```

---

## Troubleshooting

### Video doesn't play on ESP32
- **Check codec**: Make sure it's MJPEG, not H.264
- **Check container**: Should be AVI
- **Check SD card**: Format as FAT32
- **Check file size**: Very large files may cause issues

### Video is choppy/stutters
- **Reduce quality**: Try `-q:v 10` (lower quality, faster decode)
- **Reduce frame rate**: Try 12 FPS instead of 15
- **Reduce resolution**: Use 128x160 instead of 240x280
- **Check SD card speed**: Use Class 10 or better

### Audio out of sync
- **Check encoding**: Make sure audio bitrate isn't too high
- **Simplify audio**: Use PCM instead of MP3 if issues persist
- **Check ESP32 code**: May need A/V sync adjustments

### Files are too large
- **Increase compression**: Use `-q:v 10` or higher
- **Reduce audio bitrate**: Try 48k or 32k
- **Reduce frame rate**: Try 12 FPS
- **Reduce resolution**: Use 128x160 if you're on 240x280

### Encoding is very slow
- **Use faster preset** (Handbrake): Select "Very Fast" preset
- **Reduce input resolution**: Pre-scale input files if they're 4K
- **Use hardware acceleration** (FFmpeg): `-hwaccel auto` flag
- **Upgrade hardware**: Encoding is CPU-intensive

---

## Quick Reference Commands

### Extract a single episode from a TV series file
```bash
# Extract episode 1 (starts at 0:00, 22 minutes long)
ffmpeg -i tvshow.mp4 -ss 00:00:00 -t 00:22:00 -c copy episode_01.mp4
```

### Crop black bars before encoding
```bash
# Auto-detect and crop black bars
ffmpeg -i input.mp4 \
  -vf "cropdetect=24:16:0" \
  -f null - 2>&1 | grep crop=

# Use detected crop values
ffmpeg -i input.mp4 \
  -vf "crop=1920:800:0:140,scale=128:160:force_original_aspect_ratio=decrease,pad=128:160:(ow-iw)/2:(oh-ih)/2" \
  -r 15 -q:v 8 -vcodec mjpeg \
  -acodec libmp3lame -ar 22050 -ac 1 -b:a 64k \
  output.avi
```

### Check video properties
```bash
# Quick info
ffprobe -hide_banner input.mp4

# Detailed info
ffmpeg -i input.mp4
```

### Batch rename files with zero-padding
```bash
# Linux/Mac - rename files to episode_01.avi format
count=1
for file in *.mp4; do
    printf -v padded "%02d" $count
    mv "$file" "episode_${padded}.avi"
    ((count++))
done
```

---

## Next Steps

1. **Install FFmpeg** or Handbrake on your computer
2. **Test encode a single episode** with recommended settings
3. **Load it onto your ESP32** and test playback
4. **Adjust quality settings** if needed
5. **Batch encode your first full season**
6. **Organize content** on SD card with proper structure

For ESP32 playback code examples, see the project documentation in PROJECTPLAN.MD and TASKS.MD.

Good luck encoding your retro TV library! 📺✨
