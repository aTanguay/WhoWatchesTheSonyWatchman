# macOS Compression Guide - Watchman Video Encoder

This guide provides macOS-specific workflows for encoding videos for the Sony Watchman ESP32 Media Player using native macOS tools (Automator, AppleScript, Finder integration).

## Table of Contents
- [Quick Action (Recommended)](#quick-action-recommended)
- [Drag & Drop Application](#drag--drop-application)
- [Finder Scripts Menu](#finder-scripts-menu)
- [Troubleshooting](#troubleshooting)

---

## Quick Action (Recommended)

The **Quick Action** method integrates directly with Finder's right-click menu, making it the most convenient option.

### Step 1: Create the Quick Action

1. Open **Automator** (`/Applications/Automator.app`)
2. Choose **New Document** → **Quick Action**
3. At the top of the workflow, configure:
   - **Workflow receives current**: `video files`
   - **in**: `Finder`

### Step 2: Add Shell Script Action

1. In the left panel, search for **"Run Shell Script"**
2. Drag **"Run Shell Script"** into the workflow area
3. Configure the action:
   - **Shell**: `/bin/bash`
   - **Pass input**: `as arguments`

### Step 3: Paste the Script

Copy and paste this script into the text area:

```bash
#!/bin/bash

# FFmpeg settings for Waveshare 2" LCD (240x320)
RESOLUTION="240:320"
QUALITY="6"  # Tested and verified optimal quality

for input_file in "$@"; do
    # Get the directory and filename without extension
    dir=$(dirname "$input_file")
    filename=$(basename "$input_file")
    name="${filename%.*}"

    # Output file in the same directory with _watchman suffix
    output_file="${dir}/${name}_watchman.avi"

    # Show start notification
    osascript -e "display notification \"Encoding: $filename\" with title \"Watchman Encoder\""

    # Run FFmpeg encoding
    /opt/homebrew/bin/ffmpeg -i "$input_file" \
        -vf "scale=$RESOLUTION:force_original_aspect_ratio=decrease,pad=$RESOLUTION:(ow-iw)/2:(oh-ih)/2" \
        -r 15 \
        -q:v $QUALITY \
        -vcodec mjpeg \
        -acodec libmp3lame \
        -ar 22050 \
        -ac 1 \
        -b:a 64k \
        "$output_file"

    # Show completion notification
    if [ $? -eq 0 ]; then
        size=$(du -h "$output_file" | cut -f1)
        osascript -e "display notification \"Complete! Size: $size\" with title \"Watchman Encoder\" subtitle \"$name\""
    else
        osascript -e "display notification \"Encoding failed!\" with title \"Watchman Encoder\" subtitle \"$name\""
    fi
done
```

### Step 4: Save the Quick Action

1. Click **File** → **Save** (or press ⌘S)
2. Name it: **"Encode for Watchman"**
3. The Quick Action will be saved to: `~/Library/Services/`

### Step 5: Use It!

1. In Finder, select one or more video files
2. **Right-click** on the selected files
3. Choose **Quick Actions** → **Encode for Watchman**
4. macOS will show notifications as each file is encoded
5. Encoded files will appear in the same folder with `_watchman.avi` suffix

**Example:**
- Input: `Bobs_Burgers_S01E01.mp4`
- Output: `Bobs_Burgers_S01E01_watchman.avi` (in same folder)

---

## Drag & Drop Application

If you prefer a standalone application you can drag files onto:

### Step 1: Create the Script

1. Open **Script Editor** (`/Applications/Utilities/Script Editor.app`)
2. Create a new document
3. Paste this AppleScript:

```applescript
on open droppedItems
    repeat with videoFile in droppedItems
        set videoPath to POSIX path of videoFile
        set videoFolder to do shell script "dirname " & quoted form of videoPath
        set videoName to do shell script "basename " & quoted form of videoPath & " | sed 's/\\.[^.]*$//'"

        set outputPath to videoFolder & "/" & videoName & "_watchman.avi"

        display notification "Encoding: " & videoName with title "Watchman Encoder"

        set ffmpegCommand to "/opt/homebrew/bin/ffmpeg -i " & quoted form of videoPath & " -vf \"scale=240:320:force_original_aspect_ratio=decrease,pad=240:320:(ow-iw)/2:(oh-ih)/2\" -r 15 -q:v 6 -vcodec mjpeg -acodec libmp3lame -ar 22050 -ac 1 -b:a 64k " & quoted form of outputPath

        try
            do shell script ffmpegCommand

            set fileSize to do shell script "du -h " & quoted form of outputPath & " | cut -f1"
            display notification "Complete! Size: " & fileSize with title "Watchman Encoder" subtitle videoName
        on error errMsg
            display alert "Encoding Failed" message "Error: " & errMsg
        end try
    end repeat
end open
```

### Step 2: Save as Application

1. Click **File** → **Export**
2. Configure export settings:
   - **Save As**: `Watchman Encoder`
   - **File Format**: `Application`
   - **Location**: Choose Desktop or Applications folder
3. Click **Save**

### Step 3: Use It!

1. Drag and drop video files onto the **Watchman Encoder** app icon
2. The app will process each file automatically
3. Notifications will show progress and completion

---

## Finder Scripts Menu

For power users who want scripts in the menu bar:

### Step 1: Enable Scripts Menu

1. Open **Script Editor**
2. Go to **Script Editor** → **Preferences**
3. Check **"Show Script menu in menu bar"**

### Step 2: Create the Script

1. In Script Editor, create a new document
2. Paste this code:

```applescript
-- Watchman Video Encoder for Finder Selection
-- Encodes selected files to MJPEG/AVI for ESP32 playback

on run
    tell application "Finder"
        set selectedFiles to selection as alias list
    end tell

    if (count of selectedFiles) is 0 then
        display alert "No Files Selected" message "Please select one or more video files in Finder."
        return
    end if

    repeat with videoFile in selectedFiles
        my encodeVideo(videoFile)
    end repeat
end run

on encodeVideo(videoFile)
    set videoPath to POSIX path of videoFile
    set videoFolder to do shell script "dirname " & quoted form of videoPath
    set videoName to do shell script "basename " & quoted form of videoPath & " | sed 's/\\.[^.]*$//'"

    set outputPath to videoFolder & "/" & videoName & "_watchman.avi"

    display notification "Starting: " & videoName with title "Watchman Encoder"

    set ffmpegCommand to "/opt/homebrew/bin/ffmpeg -i " & quoted form of videoPath & " -vf \"scale=240:320:force_original_aspect_ratio=decrease,pad=240:320:(ow-iw)/2:(oh-ih)/2\" -r 15 -q:v 6 -vcodec mjpeg -acodec libmp3lame -ar 22050 -ac 1 -b:a 64k " & quoted form of outputPath

    try
        do shell script ffmpegCommand

        set fileSize to do shell script "du -h " & quoted form of outputPath & " | cut -f1"
        display notification "Done! " & fileSize with title "Watchman Encoder" subtitle videoName
    on error errMsg
        display notification "Failed!" with title "Watchman Encoder" subtitle videoName
        display alert "Encoding Error" message errMsg
    end try
end encodeVideo
```

### Step 3: Save to Scripts Folder

1. Click **File** → **Save**
2. Navigate to: `~/Library/Scripts/`
   - If the folder doesn't exist, create it
3. Name it: **"Encode for Watchman.scpt"**
4. **File Format**: `Script`

### Step 4: Use It!

1. Select video files in Finder
2. Click the **Script menu** (scroll icon) in menu bar
3. Choose **"Encode for Watchman"**

---

## Troubleshooting

### FFmpeg Path Issues

The scripts assume FFmpeg is installed via Homebrew. Check your FFmpeg location:

```bash
which ffmpeg
```

**Common paths:**
- **Apple Silicon (M1/M2/M3)**: `/opt/homebrew/bin/ffmpeg`
- **Intel Mac**: `/usr/local/bin/ffmpeg`

If your path is different, update the scripts by replacing `/opt/homebrew/bin/ffmpeg` with your actual path.

### Installing FFmpeg via Homebrew

If you don't have FFmpeg installed:

```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install FFmpeg
brew install ffmpeg
```

### Permissions Errors

The first time you run the script, macOS may ask for permissions:

1. **Files and Folders Access**: Allow access to the folders containing your videos
2. **Notifications**: Allow notifications to see encoding progress

If you get permission errors:
1. Go to **System Settings** → **Privacy & Security**
2. Grant access to:
   - **Files and Folders**
   - **Automation** (for AppleScript)

### Script Doesn't Appear in Quick Actions

If "Encode for Watchman" doesn't show in the Quick Actions menu:

1. Go to **System Settings** → **Extensions** → **Finder**
2. Make sure **"Encode for Watchman"** is checked
3. Restart Finder: Option + Right-click Finder icon → **Relaunch**

### Encoding Fails or Hangs

**Check FFmpeg:**
```bash
ffmpeg -version
```

**Test encoding manually:**
```bash
ffmpeg -i /path/to/test.mp4 \
  -vf "scale=240:320:force_original_aspect_ratio=decrease,pad=240:320:(ow-iw)/2:(oh-ih)/2" \
  -r 15 -q:v 6 -vcodec mjpeg \
  -acodec libmp3lame -ar 22050 -ac 1 -b:a 64k \
  /path/to/output.avi
```

### Notifications Don't Appear

Enable notifications for Script Editor or Automator:
1. **System Settings** → **Notifications**
2. Find **Script Editor** or **Automator**
3. Enable **"Allow Notifications"**

---

## Customization Options

### Change Output Filename Suffix

By default, files are saved with `_watchman.avi` suffix. To change this:

**In the shell script, find this line:**
```bash
output_file="${dir}/${name}_watchman.avi"
```

**Change to:**
```bash
output_file="${dir}/${name}_esp32.avi"  # Custom suffix
# or
output_file="${dir}/${name}.avi"        # No suffix (overwrites if input is .avi!)
```

### Change Output Directory

To save encoded files to a different folder:

**Add this near the top of the script (after `QUALITY="6"`):**
```bash
OUTPUT_DIR="$HOME/Movies/Watchman_Encoded"
mkdir -p "$OUTPUT_DIR"
```

**Then change the output_file line to:**
```bash
output_file="${OUTPUT_DIR}/${name}_watchman.avi"
```

### Adjust Quality Settings

To change video quality (affects file size):

**In the script, change:**
```bash
QUALITY="6"  # Current setting (150-200 MB per 22-min episode)
```

**To:**
```bash
QUALITY="5"  # Higher quality, larger files (~220-300 MB)
QUALITY="7"  # Lower quality, smaller files (~130-180 MB)
QUALITY="8"  # Even smaller files (~100-150 MB)
```

See [CONTENTGUIDE.MD](CONTENTGUIDE.MD#quality-settings-guide) for detailed quality information.

### Process Multiple Files Sequentially

By default, the Quick Action processes files one at a time (sequentially). This is intentional to:
- Prevent system overload
- Show clear progress notifications
- Ensure each file completes before starting the next

If you want to speed up batch processing, use the command-line batch script from [CONTENTGUIDE.MD](CONTENTGUIDE.MD#batch-processing-with-ffmpeg) instead.

---

## Performance Tips

### Encoding Speed

Encoding is CPU-intensive. Typical speeds on Apple Silicon:
- **M1/M2/M3**: ~2-4x realtime (10 minutes to encode a 22-min episode)
- **Intel**: ~1-2x realtime (15-20 minutes per episode)

### Hardware Acceleration

FFmpeg can use macOS VideoToolbox for faster encoding, but it doesn't support MJPEG. The current CPU-based encoding is necessary for compatibility.

### Batch Encoding Overnight

For large batches (full seasons), consider:
1. Using the command-line batch script from CONTENTGUIDE.MD
2. Running overnight
3. Using **caffeinate** to prevent sleep:

```bash
caffeinate -i ./encode_batch.sh
```

---

## Expected Results

### File Sizes (Real-World Tested)

At `-q:v 6` quality for 22-minute episodes:
- **Animation** (Bob's Burgers, Simpsons): 150-170 MB
- **Sitcoms** (Friends, Office): 180-220 MB
- **Live-action drama**: 200-250 MB

### Video Specifications

Encoded videos will have:
- **Container**: AVI
- **Video Codec**: MJPEG
- **Resolution**: 240x320 (portrait)
- **Frame Rate**: 15 FPS
- **Audio Codec**: MP3 mono
- **Audio Sample Rate**: 22.05 kHz
- **Audio Bitrate**: 64 kbps

### Verification

Test your encoded files with **VLC Media Player**:
1. Open encoded file in VLC
2. **Tools** → **Codec Information**
3. Verify:
   - Video Codec: `Motion JPEG`
   - Audio Codec: `MPEG Audio layer 1/2/3 (mpga)`
   - Resolution: `240x320`
   - Frame rate: `15 fps`

---

## Next Steps

1. **Test encode a single episode** to verify the workflow
2. **Check output quality** by playing in VLC
3. **Batch encode your first season**
4. **Transfer to SD card** formatted as FAT32
5. **Test on ESP32 hardware** (see [STATUS.MD](STATUS.MD) for next steps)

For complete encoding guidelines and optimization tips, see [CONTENTGUIDE.MD](CONTENTGUIDE.MD).

For ESP32 hardware setup and playback testing, see [BUILD.MD](BUILD.MD) and [STATUS.MD](STATUS.MD).

---

**Enjoy your retro TV library!** 📺✨
