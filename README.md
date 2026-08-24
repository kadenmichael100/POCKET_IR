# Pocket-IR: Credit-Card Sized Universal Remote & Embedded Gaming Console

A bare-metal ATmega328P embedded system featuring sub-pixel physics game rendering, raw EEPROM IR pulse learning, a 29-code TV-B-Gone sequence, and ultra-low-power deep sleep management. Designed o[...]

![IMG_7540](./IMG_7540.jpeg)

---

## 📋 System Specifications

| Subsystem | Components & Implementations |
|-----------|------------------------------|
| **Microcontroller** | ATmega328P running @ 8MHz internal clock, 6-pin ISP flashing header |
| **Display** | 1.3" SH1106 OLED (128x64) driven over 4-wire Hardware SPI |
| **IR Output** | Quad-IR LED array (driven via N-MOSFET, 10Ω ballast resistors, 1000µF smoothing cap) |
| **IR Input** | 38kHz active IR receiver module |
| **Audio** | MLT8530 piezo buzzer with software frequency synthesis |
| **Power** | 3x AAA Direct Drive (3.0V – 4.5V range) with internal bandgap voltage sensing |
| **Input** | 5-Way Navigation Switch (Up, Down, Left, Right, Center Click) |
| **Enclosure** | Dual-layer PCB sandwich construction with M3 nylon standoffs (55x85mm) |

---

## ⚙️ Key Engineering Features

![Demo](./IMG_7553%20(1).gif)

### 🎮 Smart Layer Input Engine
Supports Single-Click (Context Action), Double-Click (Smart/Normal Layer toggle), and Triple-Click (Learned vs. Preset swap) on a single physical center switch.

### 🔋 Zero-Pin Battery Gauge
Measures internal supply voltage (VCC) against an internal 1.1V reference, eliminating voltage divider power leakage and saving GPIO pins.

### 🎯 Sub-Pixel Floating-Point Physics
Smooth Pong gaming rendered on a 128x64 canvas via sub-pixel float trajectories, vector reflections, and dynamic speed multipliers.

### 💥 PROGMEM Signal Blaster
Asynchronous execution of 29 IR power routines stored in Flash memory, paired with dynamic progress bar rendering.

### 📚 Non-Volatile Signal Learning
Captures raw IR protocols, address bits, and command structures into non-volatile EEPROM storage.

---

## 🔬 Software Architecture Deep Dives

### 1️⃣ Zero-Pin Bandgap Battery Sensing (readVcc)

Instead of using an external voltage divider—which constantly draws current or requires an extra GPIO pin to switch—the system measures VCC internally. By selecting the internal 1.1V bandgap r[...]

**Formula:**
```
VCC (mV) = (1.1V × 1023 × 1000) / ADC Reading
```

**Code:**
```cpp
long readVcc() {
  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__)
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); // Select 1.1V Bandgap
  #endif   
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // Wait for conversion
  uint8_t low  = ADCL; 
  uint8_t high = ADCH; 
  long result = (high << 8) | low;
  return 1125300L / result; // Returns Vcc in millivolts
}
```

### 2️⃣ Multi-Click & Debounce Input State Machine

The 5-way switch leverages a debouncing and timing window engine inside `handleRemoteInput()`. It calculates differential release times to distinguish between single-clicks, layer switches, and pr[...]

**Code:**
```cpp
// Non-blocking Multi-Click Resolution Engine
if (clickCount > 0 && debouncedClickState == HIGH) {
  if (now - lastReleaseTime > MULTI_CLICK_GAP) {
    if (clickCount == 1)      cmd = isSmartMode ? CMD_SELECT : CMD_MUTE;
    else if (clickCount == 2) { isSmartMode = !isSmartMode; playSmartModeSound(); }
    else if (clickCount >= 3) { toggleLearnedMode(); }
    clickCount = 0; 
  }
}
```

### 3️⃣ Sub-Pixel Ball Physics Engine

To prevent motion stuttering on low-resolution monochrome OLED screens, ball position and velocity are calculated using floating-point operations. The coordinates are cast to integers only at the [...]

**Code:**
```cpp
// Floating-point vector update with speed multiplier
ballX += (ballVx * speedMult);
ballY += (ballVy * speedMult);

// Dynamic Paddle Reflection Math
float hitOffset = ballY - (playerY + 7.0); 
ballVx = 2.0; 
ballVy = hitOffset * 0.25; // Imparts spin based on paddle contact point
```

---

## 🏗️ PCB Hardware & Layout

**Dimensions:** 55mm x 85mm (Credit Card Form Factor)

**Assembly Strategy:** Hybrid approach—JLCPCB surface-mount assembly (SMT) for passives and ICs, coupled with manual hand-soldering for high-stress through-hole (THT) connectors, switch, and OL[...]

**Protective Sandwich:** Decorative faceplate fabricated from standard PCB substrate mounted with four M3 nylon standoffs to protect the screen.

![IMG_7553 (1)](./IMG_7553%20(1).gif)

---

## 🚀 How to Build & Flash

### Hardware Prerequisites
- USBasp or Arduino as ISP Programmer
- 6-Pin ISP Cable

### Software Dependencies
```
- U8g2lib
- IRremote
- LowPower
```

### Compilation Settings

| Setting | Value |
|---------|-------|
| **Board** | ATmega328P (3.3V / 8MHz internal clock) |
| **Clock** | Internal 8 MHz |
| **Programmer** | USBasp (via 6-pin ISP header on board) |

**To Flash:** Connect your ISP programmer to the 6-pin header and flash using your preferred IDE or command-line tools.
