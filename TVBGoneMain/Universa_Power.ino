// --------------------------------------------
// UNIVERSAL OFF (TV-B-GONE WITH PROGRESS BAR)
// --------------------------------------------

struct PowerCode {
  uint8_t protocol;
  uint32_t address;
  uint32_t command;
  uint8_t repeats;   
  uint8_t delayMs; 
};

const PowerCode universalCodes[] PROGMEM = {
  // PROTOCOL,   ADDRESS,  COMMAND, REPEATS, DELAY_MS
  
  // --- SONY BRANDS FIRST (Strict SIRCS Protocol: Requires 3 repeats) ---
  { SONY,        0x01,     0x15,    3,       25 }, // 1. Sony 12-bit Power (Standard Toggle)
  { SONY,        0x01,     0x2E,    3,       25 }, // 2. Sony Discrete Power OFF
  { SONY,        0x10,     0x15,    3,       25 }, // 3. Sony 15-bit Power Variant
  { SONY,        0x01,     0x4A,    3,       25 }, // 4. Sony Audio System / Soundbar Power

  // --- MAJOR TV BRANDS (Responsive, fewer repeats needed) ---
  { SAMSUNG,     0x0707,   0x02,    1,       10 }, // 5. Samsung Power (Standard)
  { SAMSUNG,     0x0707,   0x99,    1,       10 }, // 6. Samsung Discrete Power OFF
  { NEC,         0x04,     0x08,    2,       15 }, // 7. LG / Vizio Power (Standard NEC)
  { NEC,         0x04,     0xC3,    2,       15 }, // 8. LG Discrete Power OFF
  { PANASONIC,   0x01,     0x3D,    2,       15 }, // 9. Panasonic Power
  { SHARP,       0x01,     0x41,    2,       15 }, // 10. Sharp Power
  { NEC,         0x40,     0x12,    2,       15 }, // 11. Toshiba TV Power
  { RC5,         0x00,     0x0C,    2,       15 }, // 12. Philips TV Power (RC5)
  { RC6,         0x00,     0x0C,    2,       15 }, // 13. Philips / Magnavox TV Power (RC6)
  { NEC,         0x7807,   0x4D,    2,       15 }, // 14. TCL / RCA / Proscan Power
  { NEC,         0x02BD,   0x00,    2,       15 }, // 15. Insignia / Westinghouse Power
  { NEC,         0x00,     0x02,    2,       15 }, // 16. Universal / Sanyo TV Power
  { NEC,         0x0000,   0x12,    2,       15 }, // Sceptre TV Power (Standard NEC)
  { NEC,         0x0000,   0x1A,    2,       15 }, // Sceptre TV Alt Power
  { NEC,         0x1D1D,   0x0A,    2,       15 }, // Sceptre Legacy Power

  // --- SATELLITE & CABLE BOXES ---
  { RC6,         0x00,     0x0C,    2,       15 }, // 17. Comcast / Xfinity Cable Box Power
  { NEC,         0x01,     0x0C,    2,       15 }, // 18. DirecTV Receiver Power
  { NEC,         0x08,     0x01,    2,       15 }, // 19. DISH Network Satellite Receiver
  { RC5,         0x05,     0x0C,    2,       15 }, // 20. Motorola / Charter Spectrum Box
  { RC6,         0x02,     0x0C,    2,       15 }, // 21. AT&T U-verse Receiver

  // --- STREAMING BOXES & TV STICKS ---
  { NEC,         0x5743,   0x40,    5,       25 }, // 22. Roku Standard Power
  { NEC,         0x5743,   0x40,    2,       15 }, // 23. Roku TV / Onn. Power
  { NEC,         0x5583,   0x03,    2,       15 }, // 24. Roku Alt Code
  { NEC,         0x04FB,   0x08,    5,       25 }, // 25. Hisense / Onn. Roku Variant
  { NEC,         0x807F,   0x10,    5,       25 }, // 26. Element / Onn. Roku Variant
  { NEC,         0x87EE,   0x04,    2,       15 }, // 27. Apple TV Power / Sleep
  { NEC,         0x87EE,   0x03,    2,       15 }, // 28. Apple TV Play/Pause Hold
  { NEC,         0x00F7,   0x01,    2,       15 }  // 29. Amazon Fire TV IR Adapter Power
};

// Helper function to render progress bar cleanly in U8g2 Page Buffer Mode
void drawBlasterProgress(int current, int total) {
  const int barX = 14;
  const int barY = 26;
  const int barW = 100;
  const int barH = 14;

  int fillWidth = 0;
  if (total > 0) {
    fillWidth = ((barW - 4) * current) / total;
  }

  u8g2.firstPage();
  do {
    // Title
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setCursor(31, 14);
    u8g2.print(F("BLASTING..."));

    // Progress Bar Outline Frame
    u8g2.drawFrame(barX, barY, barW, barH);

    // Filled Inner Bar
    if (fillWidth > 0) {
      u8g2.drawBox(barX + 2, barY + 2, fillWidth, barH - 4);
    }

    // Status Text
    u8g2.setCursor(34, 56);
    u8g2.print(F("CODE "));
    u8g2.print(current);
    u8g2.print(F(" / "));
    u8g2.print(total);
  } while (u8g2.nextPage());
}

void triggerUniversalOff() {
  int totalCodes = sizeof(universalCodes) / sizeof(PowerCode);

  // Audio warning chimes before blasting starts
  disableLEDFeedback(); 
  safeTone(1500, 80);  delay(100);
  safeTone(1200, 80);  delay(100);
  enableLEDFeedback();

  // 1. Render initial empty progress bar state (0 / total)
  drawBlasterProgress(0, totalCodes);

  // 2. IR Blasting Loop
  for (int i = 0; i < totalCodes; i++) {
    PowerCode currentCode;
    memcpy_P(&currentCode, &universalCodes[i], sizeof(PowerCode));

    // TRANSMIT IR SIGNAL using tailored repeat counts
    IrSender.write(currentCode.protocol, currentCode.address, currentCode.command, currentCode.repeats); 

    // UPDATE PROGRESS BAR 
    // (Note: The I2C display drawing inherently takes ~30-40ms, which acts as 
    // a great natural breather for the TV's IR receiver between blasts!)
    drawBlasterProgress(i + 1, totalCodes);

    // Dynamic breather delay based on the specific brand's needs
    delay(currentCode.delayMs); 
  }

  // 3. Sequence Complete Screen
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_9x15_tr);
    u8g2.setCursor(35, 24);
    u8g2.print(F("BLAST"));
    u8g2.setCursor(24, 44);
    u8g2.print(F("COMPLETE"));
  } while (u8g2.nextPage());
  
  playSuccessSound();
  delay(1500); 
  
  inRemoteMode = false;
  drawMenu();
  lastUserActivityTime = millis();
}