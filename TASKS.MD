# Project Tasks

## Phase 1: Proof of Concept

### Hardware Setup
- [ ] **T1.1**: Acquire ESP32 development board (ESP32-WROOM-32 or ESP32-S3)
- [ ] **T1.2**: Acquire 1.7" IPS display (ST7735 or ST7789)
- [ ] **T1.3**: Acquire MicroSD card module
- [ ] **T1.4**: Acquire audio amplifier module (PAM8403 or MAX98357A I2S)
- [ ] **T1.5**: Set up breadboard prototype
- [ ] **T1.6**: Wire display to ESP32 (SPI: MOSI, SCK, CS, DC, RST)
- [ ] **T1.7**: Wire SD card module to ESP32 (SPI: MISO, MOSI, SCK, CS)
- [ ] **T1.8**: Wire audio output (DAC or I2S pins)

### Software Foundation
- [ ] **T1.9**: Set up Arduino IDE or PlatformIO environment
- [ ] **T1.10**: Install TFT_eSPI library and configure for display
- [ ] **T1.11**: Install SD library and test SD card access
- [ ] **T1.12**: Create basic project structure
- [ ] **T1.13**: Test display with static images
- [ ] **T1.14**: Test SD card read performance (measure MB/s)

### Video Playback Development
- [ ] **T1.15**: Research MJPEG decoder implementations for ESP32
- [ ] **T1.16**: Implement or port MJPEG decoder
- [ ] **T1.17**: Create test video file (short, low resolution)
- [ ] **T1.18**: Implement basic video player (video only, no audio)
- [ ] **T1.19**: Optimize frame buffer and streaming
- [ ] **T1.20**: Measure and optimize FPS performance
- [ ] **T1.21**: Test with 15+ second video clip

### Audio Playback
- [ ] **T1.22**: Implement audio output (DAC or I2S)
- [ ] **T1.23**: Test audio playback from SD card
- [ ] **T1.24**: Implement audio decoder (MP3 or raw PCM)
- [ ] **T1.25**: Test audio quality and synchronization

### A/V Synchronization
- [ ] **T1.26**: Design A/V sync mechanism (timestamps or frame-locked)
- [ ] **T1.27**: Implement synchronized playback
- [ ] **T1.28**: Test and measure sync accuracy
- [ ] **T1.29**: Optimize for smooth playback

---

## Phase 2: Core Features

### Multi-Channel System
- [ ] **T2.1**: Design channel data structure
- [ ] **T2.2**: Implement JSON parsing for channels.json
- [ ] **T2.3**: Create channel manager class
- [ ] **T2.4**: Implement channel switching logic
- [ ] **T2.5**: Test channel loading and switching
- [ ] **T2.6**: Create test content for 3+ channels

### Rotary Encoder Integration
- [ ] **T2.7**: Acquire rotary encoder module (KY-040 or similar)
- [ ] **T2.8**: Wire encoder to ESP32 (CLK, DT, SW pins)
- [ ] **T2.9**: Implement encoder reading with interrupts
- [ ] **T2.10**: Implement debouncing logic
- [ ] **T2.11**: Map encoder rotation to channel up/down
- [ ] **T2.12**: Test channel switching via encoder

### Episode Management
- [ ] **T2.13**: Implement episode list parsing
- [ ] **T2.14**: Create episode queue/playlist
- [ ] **T2.15**: Implement sequential episode playback
- [ ] **T2.16**: Implement episode switching logic
- [ ] **T2.17**: Test multi-episode playback

### State Persistence
- [ ] **T2.18**: Design state storage format (JSON)
- [ ] **T2.19**: Implement state save/load functions
- [ ] **T2.20**: Save current channel, episode, timestamp
- [ ] **T2.21**: Resume playback from saved state on boot
- [ ] **T2.22**: Test state persistence across reboots

### File System Structure
- [ ] **T2.23**: Create SD card directory structure
- [ ] **T2.24**: Implement file discovery and indexing
- [ ] **T2.25**: Create channel metadata JSON files
- [ ] **T2.26**: Test with organized content library

---

## Phase 3: Enhanced Features

### Episode Selection Input
- [ ] **T3.1**: Decide: Accelerometer vs Button (or both)
- [ ] **T3.2**: If accelerometer: Acquire MPU6050 or ADXL345
- [ ] **T3.3**: If button: Acquire tactile button
- [ ] **T3.4**: Wire accelerometer (I2C: SDA, SCL) or button
- [ ] **T3.5**: Implement accelerometer reading library
- [ ] **T3.6**: Implement shake detection algorithm (threshold tuning)
- [ ] **T3.7**: Implement random episode selection
- [ ] **T3.8**: Test shake-to-shuffle or button functionality

### Power Management
- [ ] **T3.9**: Acquire 2x 18650 batteries and holders
- [ ] **T3.10**: Acquire TP4056 charging modules with protection
- [ ] **T3.11**: Acquire buck-boost converter (5V/3.3V output)
- [ ] **T3.12**: Design power circuit schematic
- [ ] **T3.13**: Implement battery voltage monitoring (ADC)
- [ ] **T3.14**: Implement low battery warning
- [ ] **T3.15**: Implement auto-sleep timer (inactivity detection)
- [ ] **T3.16**: Implement deep sleep mode
- [ ] **T3.17**: Test power consumption and battery life
- [ ] **T3.18**: Optimize power usage (display brightness, CPU frequency)

### User Interface
- [ ] **T3.19**: Design on-screen display (OSD) layout
- [ ] **T3.20**: Implement channel indicator overlay
- [ ] **T3.21**: Implement volume indicator (if volume control added)
- [ ] **T3.22**: Implement battery indicator
- [ ] **T3.23**: Implement boot splash screen
- [ ] **T3.24**: Add channel name display on switch
- [ ] **T3.25**: Test UI responsiveness and visibility

### Error Handling
- [ ] **T3.26**: Implement SD card error detection
- [ ] **T3.27**: Implement missing file error handling
- [ ] **T3.28**: Implement corrupted video detection
- [ ] **T3.29**: Display error messages to user
- [ ] **T3.30**: Test recovery from various error conditions

---

## Phase 4: Physical Integration

### Case Preparation
- [ ] **T4.1**: Disassemble Sony Watchman carefully
- [ ] **T4.2**: Document original layout and dimensions
- [ ] **T4.3**: Remove CRT and old electronics
- [ ] **T4.4**: Clean and prepare case
- [ ] **T4.5**: Measure available space for new components

### Display Integration
- [ ] **T4.6**: Measure display mounting requirements
- [ ] **T4.7**: Design/create display mounting bracket
- [ ] **T4.8**: Test fit display in CRT opening
- [ ] **T4.9**: 3D print or fabricate mounting hardware
- [ ] **T4.10**: Secure display in case
- [ ] **T4.11**: Ensure display is visible through window

### Speaker Connection
- [ ] **T4.12**: Test original speaker impedance and condition
- [ ] **T4.13**: Connect speaker to audio amplifier
- [ ] **T4.14**: Test audio output quality
- [ ] **T4.15**: Secure speaker in original location

### Control Integration
- [ ] **T4.16**: Mount rotary encoder to original tuning knob location
- [ ] **T4.17**: Adapt encoder shaft to original knob (coupler/adapter)
- [ ] **T4.18**: Mount shake sensor or button in accessible location
- [ ] **T4.19**: Test all controls with case assembled

### Power System Integration
- [ ] **T4.20**: Design battery compartment layout
- [ ] **T4.21**: Mount 18650 battery holders in case
- [ ] **T4.22**: Install TP4056 charging module
- [ ] **T4.23**: Install buck-boost converter
- [ ] **T4.24**: Route charging port to case exterior
- [ ] **T4.25**: Install power switch (use original if possible)
- [ ] **T4.26**: Wire complete power system
- [ ] **T4.27**: Test charging functionality

### PCB/Wiring
- [ ] **T4.28**: Decide: Custom PCB vs hand-wired
- [ ] **T4.29**: If PCB: Design compact PCB in KiCad/Eagle
- [ ] **T4.30**: If PCB: Order and receive PCB
- [ ] **T4.31**: Solder components to PCB or protoboard
- [ ] **T4.32**: Test all connections
- [ ] **T4.33**: Mount PCB securely in case
- [ ] **T4.34**: Ensure no shorts or loose wires

### Final Assembly
- [ ] **T4.35**: Route all wires cleanly
- [ ] **T4.36**: Secure all components with appropriate mounting
- [ ] **T4.37**: Test full system in assembled case
- [ ] **T4.38**: Close case and test accessibility
- [ ] **T4.39**: Add strain relief for wired connections

---

## Phase 5: Content & Polish

### Video Encoding Pipeline
- [ ] **T5.1**: Install FFmpeg on development machine
- [ ] **T5.2**: Create encoding script for batch conversion
- [ ] **T5.3**: Test encoding parameters for quality/size balance
- [ ] **T5.4**: Optimize encoding settings for ESP32 playback
- [ ] **T5.5**: Document encoding process

### Content Library Creation
- [ ] **T5.6**: Select TV series for channels (3-5 shows)
- [ ] **T5.7**: Source video files (legal copies)
- [ ] **T5.8**: Encode first test season
- [ ] **T5.9**: Create metadata.json for each channel
- [ ] **T5.10**: Create channels.json configuration
- [ ] **T5.11**: Test playback with full content
- [ ] **T5.12**: Encode remaining content
- [ ] **T5.13**: Organize content on SD card

### Testing & Optimization
- [ ] **T5.14**: Conduct full system testing
- [ ] **T5.15**: Test all channels and episodes
- [ ] **T5.16**: Test extended playback (battery endurance)
- [ ] **T5.17**: Test shake/button functionality
- [ ] **T5.18**: Optimize performance bottlenecks
- [ ] **T5.19**: Fix any bugs discovered
- [ ] **T5.20**: Stress test (rapid channel switching, etc.)

### Documentation
- [ ] **T5.21**: Create user manual
- [ ] **T5.22**: Document hardware assembly process
- [ ] **T5.23**: Document software architecture
- [ ] **T5.24**: Create content encoding guide
- [ ] **T5.25**: Take photos/videos of completed project
- [ ] **T5.26**: Write project blog post or README

### Polish & Final Touches
- [ ] **T5.27**: Add loading animations
- [ ] **T5.28**: Fine-tune UI timing and responsiveness
- [ ] **T5.29**: Calibrate audio volume levels
- [ ] **T5.30**: Adjust display brightness/contrast
- [ ] **T5.31**: Clean and detail case exterior
- [ ] **T5.32**: Create custom labels if desired

---

## Optional/Future Enhancements

### WiFi Features
- [ ] **T6.1**: Implement WiFi configuration mode
- [ ] **T6.2**: Create web-based content manager
- [ ] **T6.3**: Implement OTA (Over-The-Air) firmware updates
- [ ] **T6.4**: Add streaming from network storage

### Advanced Features
- [ ] **T6.5**: Implement volume control (potentiometer + UI)
- [ ] **T6.6**: Add brightness control
- [ ] **T6.7**: Implement subtitle/closed caption support
- [ ] **T6.8**: Add Bluetooth audio output
- [ ] **T6.9**: Create companion mobile app

### Content Management
- [ ] **T6.10**: Build desktop content management tool
- [ ] **T6.11**: Implement watch history tracking
- [ ] **T6.12**: Add favorites system
- [ ] **T6.13**: Implement parental controls

---

## Task Categories Summary
- **Phase 1 (POC)**: 29 tasks
- **Phase 2 (Core)**: 26 tasks
- **Phase 3 (Enhanced)**: 30 tasks
- **Phase 4 (Integration)**: 39 tasks
- **Phase 5 (Content)**: 32 tasks
- **Phase 6 (Optional)**: 13+ tasks

**Total Core Tasks**: 156
