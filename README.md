TL;DR — What to look for

- Compact, battery-powered ATmega328P firmware + hardware that learns 38kHz IR, stores codes to EEPROM, and implements multi-brand remote logic.
- Engineering highlights: zero-pin bandgap battery sensing, quad-IR TX array, SPI-driven SH1106 OLED, and a 29-command universal blaster.
- Quick review path for engineers: 1) Gallery section below for hardware photos & screenshots, 2) README "Software Architecture" section for decoding/storage approach, 3) firmware source (if present) for implementation details.

For hiring managers

- Time to first-pass: Read the TL;DR above and then scroll to the Gallery section for photos and demo media embedded directly in this README.
- Skills demonstrated: embedded C/C++ (AVR), low-power hardware design, PCB layout/assembly, digital signal capture & decoding (IR protocols), system-level tradeoffs (power vs. features).
- Want to run it quickly? See Build & Flash Instructions at the bottom — include avr-gcc or avrdude commands in the repo for quick testing.

---

# Pocket-IR: Credit-Card Sized Universal Remote & Signal Learning System

A bare-metal ATmega328P embedded system capable of learning, decoding, and storing arbitrary 38kHz IR signals into EEPROM, providing full multi-brand TV control (volume, channels, menu navigation)...

---

## 📸 Gallery (embedded)

Front view — Pocket-IR PCB & assembled unit

![Front view](https://raw.githubusercontent.com/kadenmichael100/POCKET_IR/25e26d3089dc017c2fb67af5d3ef29edcceda74c/IMG_7540.jpeg)

Demo (animated GIF)

![Demo Large](https://raw.githubusercontent.com/kadenmichael100/POCKET_IR/25e26d3089dc017c2fb67af5d3ef29edcceda74c/IMG_7553%20(1).gif)

Compact demo (smaller GIF)

![Demo GIF](https://raw.githubusercontent.com/kadenmichael100/POCKET_IR/25e26d3089dc017c2fb67af5d3ef29edcceda74c/IMG_7552.gif)

Screenshots

![Screenshot 120835](https://raw.githubusercontent.com/kadenmichael100/POCKET_IR/25e26d3089dc017c2fb67af5d3ef29edcceda74c/Screenshot%202026-08-24%20120835.png)

![Screenshot 133445](https://raw.githubusercontent.com/kadenmichael100/POCKET_IR/25e26d3089dc017c2fb67af5d3ef29edcceda74c/Screenshot%202026-08-24%20133445.png)

---

## 🛠️ System Specifications

| Subsystem | Components & Implementations |
|-----------|------------------------------|
| **Microcontroller** | ATmega328P running @ 8MHz internal clock, 6-pin ISP flashing header |
| **Display** | 1.3" SH1106 OLED (128x64) driven over 4-wire Hardware SPI |
| **IR Transmitter** | Quad-IR LED array driven via N-MOSFET, 10Ω ballast resistors, 1000µF smoothing cap |
| **IR Receiver** | 38kHz active IR receiver module for signal decoding and learning |
| **Preset Controls** | Full control (Volume, Channel, Mute, Nav, Select) for 9 brand presets |
| **Audio Synthesizer** | MLT8530 piezo buzzer driven via non-interfering PWM frequency synthesizer |
| **Power Supply** | 3x AAA Direct Drive (3.0V – 4.5V operating range) with internal bandgap sensing |
| **User Interface** | 5-Way Navigation Switch (Up, Down, Left, Right, Center Click) |
| **Enclosure** | Dual-layer PCB sandwich with M3 nylon standoffs (55x85mm) |

---

## ⚡ Key Engineering Features

![Demo Large](https://raw.githubusercontent.com/kadenmichael100/POCKET_IR/25e26d3089dc017c2fb67af5d3ef29edcceda74c/IMG_7553%20(1).gif)

### 38kHz Signal Capture & Storage
Receives any 38kHz IR command, decodes the protocol, address bitmask, and command code, then saves it into non-volatile EEPROM across 10 customizable input slots.

### Full Multi-Brand TV Remote Engine
Out-of-the-box support for Samsung, Sony, LG, Roku, Vizio, Panasonic, Toshiba, Hisense, and Apple TV. Drives Volume, Channel, Mute, Directional Navigation, and Selection commands.

### Dual-Layer Input Mapping
Integrated switch engine supports single-clicks, double-clicks (layer toggle between Normal and Smart modes), and holds, expanding the 5-way switch into 10 distinct IR transmission channels.

### 29-Code Universal Blaster
Executes a sequence of 29 PROGMEM-stored power commands across major TV brands, streaming sticks, and cable boxes with an on-screen progress bar.

### Zero-Pin Battery Monitor
Measures supply voltage (VCC) internally against the ATmega328P bandgap reference, avoiding external voltage dividers and saving power/pins.

### Bonus Sub-Pixel Game Engine
Includes an embedded Pong mini-game utilizing floating-point sub-pixel physics for frame rendering at ~60 FPS.

---

## 🧠 Software Architecture Deep Dives

### 1️⃣ 38kHz Signal Capture & EEPROM Storage Engine

When capturing a remote control signal, the active IR receiver samples the incoming 38kHz bursts. The software decodes the protocol enum, address bitmask, and command code, then serializes the payload to EEPROM.

**Code:**
```cpp
// IR Slot Data Structure (6 Bytes total)
struct IRSlot {
  uint8_t protocol;
  uint32_t address;
  uint32_t command;
};

// Serializing captured IR data to EEPROM slot
int addr = EEPROM_START_ADR + (assignedSlot * sizeof(IRSlot));
EEPROM.put(addr, captured);
```

![IMG_7552](https://raw.githubusercontent.com/kadenmichael100/POCKET_IR/25e26d3089dc017c2fb67af5d3ef29edcceda74c/IMG_7552.gif)

### 2️⃣ Multi-Brand Protocol Translator

Rather than storing bloated raw pulse arrays for standard electronics, the system stores protocol-specific hex maps in Flash memory. The execution loop dynamically routes standard commands (CMD_VO...)

**Code:**
```cpp
// Excerpt: Brand-Specific Protocol Routing
if (strcmp(brand, "SAMSUNG") == 0) {
  switch(cmd) {
    case CMD_VOL_UP: hex = 0x07; break;
    case CMD_NAV_UP: hex = 0x60; break;
    // ...
  }
  IrSender.sendSamsung(0x0707, hex, 0);
} else if (strcmp(brand, "SONY") == 0) {
  // SONY SIRCS requires 12-bit / 3-repeat frames
  IrSender.sendSony(0x01, hex, 2, 12);
}
```

### 3️⃣ Zero-Pin Bandgap Battery Sensing (readVcc)

To prevent parasitic battery drain through resistor dividers, supply voltage is computed internally. The internal 1.1V reference (VREF) is connected to the ADC multiplexer while VCC acts as the reference.

**Formula:**
```
VCC (mV) = (1.1V × 1023 × 1000) / ADC Reading
```

**Code:**
```cpp
long readVcc() {
  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__)
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); // 1.1V Bandgap Reference
  #endif   
  delay(2); // Settling delay
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // Wait for completion
  uint8_t low  = ADCL; 
  uint8_t high = ADCH; 
  long result = (high << 8) | low;
  return 1125300L / result; // Calculate mV
}
```

---

## ⚖️ Engineering Trade-Offs & Solutions

### Hardware SPI vs. I2C Bus Bottlenecks
**Problem:** Standard I2C OLED screens operating at 400kHz caused noticeable input lag during menu updates and screen redraws.

**Solution:** Switched to a 4-wire Hardware SPI interface using dedicated MOSI and SCK pins. Frame transfer execution dropped significantly, allowing instant UI redraws and high frame rates for g...

---

## 📦 Build & Flash Instructions

### Hardware Requirements
- USBasp or Arduino as ISP programmer connected to the onboard 6-pin ISP header

### Required Libraries
- U8g2lib
- IRremote
- LowPower

### Board Configuration
- **Target:** ATmega328P
- **Clock:** Internal 8MHz

Compile and flash via ISP header.
