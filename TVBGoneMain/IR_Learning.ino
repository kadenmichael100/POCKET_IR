// --------------------------------------------
// PROGRAM / LEARNING MODE UI HELPERS
// --------------------------------------------

void drawLearningWaitScreen(int animFrame) {
  u8g2.firstPage();
  do {
    // --- Header ---
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setCursor(32, 10);
    u8g2.print(F("[ LEARNING ]"));
    u8g2.drawLine(0, 13, 128, 13);

    // --- Guidance Text ---
    u8g2.setCursor(6, 25);
    u8g2.print(F("Aim remote at sensor"));

    // --- Sleek Radar / Signal Wave Animation ---
    const int centerX = 64;
    const int centerY = 41;

    // Central Sensor Icon / Receiver Dot
    u8g2.drawDisc(centerX, centerY, 2);

    // Radiating signal pulse rings (3 expanding waves)
    for (int i = 0; i < 3; i++) {
      int radius = ((animFrame * 3) + (i * 6)) % 18;
      if (radius > 3) {
        u8g2.drawCircle(centerX, centerY, radius);
      }
    }

    // --- Bottom Exit Instruction ---
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setCursor(5, 62);
    u8g2.print(F("(Hold Center to Exit)"));
  } while (u8g2.nextPage());
}

void drawAssignSlotScreen(const IRSlot& captured, bool isSmartMode) {
  u8g2.firstPage();
  do {
    // --- Header ---
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setCursor(12, 10);
    u8g2.print(F("[ SIGNAL CAPTURED ]"));
    u8g2.drawLine(0, 13, 128, 13);

    // --- Signal Details (Protocol, Address, Command) ---
    u8g2.setCursor(0, 25);
    u8g2.print(F("Type: "));
    // getProtocolString converts the numeric enum to "SAMSUNG", "NEC", "SONY", etc.
    u8g2.print(getProtocolString((decode_type_t)captured.protocol));

    u8g2.setCursor(0, 37);
    u8g2.print(F("Addr: 0x"));
    if (captured.address < 0x1000) u8g2.print('0'); // Single quotes use 1 byte char instead of string object
    if (captured.address < 0x0100) u8g2.print('0');
    if (captured.address < 0x0010) u8g2.print('0');
    u8g2.print(captured.address, HEX);

    u8g2.print(F(" Cmd: 0x"));
    if (captured.command < 0x10) u8g2.print(F("0"));
    u8g2.print(captured.command, HEX);

    // --- Action Prompt & Active Layer ---
    u8g2.setCursor(0, 49);
    u8g2.print(F("Press switch to assign"));

    u8g2.setCursor(0, 62);
    u8g2.print(isSmartMode ? F("Target: [SMART LAYER]") : F("Target: [NORMAL LAYER]"));
  } while (u8g2.nextPage());
}

void drawSavedConfirmationScreen(const char* slotName) {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setCursor(0, 18);
    u8g2.print(F("SAVED TO SLOT:"));

    u8g2.setFont(u8g2_font_9x15_tr);
    u8g2.setCursor(0, 42);
    u8g2.print(slotName);
  } while (u8g2.nextPage());
}


// --------------------------------------------
// MAIN LEARNING ROUTINE
// --------------------------------------------

const char* getSlotName(int slot) {
  switch(slot) {
    case 0: return "CLICK (NRM)";
    case 1: return "UP (NRM)";    
    case 2: return "DOWN (NRM)";
    case 3: return "LEFT (NRM)";  
    case 4: return "RIGHT (NRM)";
    case 5: return "CLICK (SMT)";
    case 6: return "UP (SMT)";    
    case 7: return "DOWN (SMT)";
    case 8: return "LEFT (SMT)";  
    case 9: return "RIGHT (SMT)";
    default: return "UNKNOWN";
  }
}

void startLearningMode() {
  IRSlot captured;
  bool gotSignal = false;
  int animFrame = 0;
  unsigned long startTime = millis();
  unsigned long pressStartTime = 0;
  bool buttonWasPressed = false;

  // Temporarily mute the buzzer so it doesn't scream at ambient IR noise
  disableLEDFeedback(); 
  
  // Ensure receiver is actually listening 
  // (Sending an IR code often disables the receiver automatically)
  IrReceiver.start();

  // 1. Capture Signal Phase
  while (!gotSignal) {
    drawLearningWaitScreen(animFrame);
    animFrame = (animFrame + 1) % 6;

    if (millis() - startTime > 30000) {
      enableLEDFeedback(); // Restore feedback before sleeping
      goToDeepSleep(); 
      return;          
    }

    // Hold Center button to exit
    if (digitalRead(PIN_CLICK) == LOW) {
      if (!buttonWasPressed) {
        pressStartTime = millis();
        buttonWasPressed = true;
      }
      if (millis() - pressStartTime > 1000) {
        enableLEDFeedback(); // Restore feedback before exiting
        playExitSound(); 
        return; 
      }
    } else {
      buttonWasPressed = false; 
    }

    // Check IR receiver
    if (IrReceiver.decode()) {
      // Record signal data if protocol is recognized
      if (IrReceiver.decodedIRData.protocol != UNKNOWN) {
        captured.protocol = (uint8_t)IrReceiver.decodedIRData.protocol;
        captured.address  = IrReceiver.decodedIRData.address;
        captured.command  = IrReceiver.decodedIRData.command;
        gotSignal = true;
        
        //Auditory feedback
        safeTone(2800, 10);
      }
      IrReceiver.resume();
      lastUserActivityTime = millis();
    }
  }

  // Restore the buzzer feedback
  enableLEDFeedback();

  // 2. Assign to 5-Way Switch Phase
  int assignedSlot = -1;
  bool assignSmartMode = false;
  unsigned long lastClickTime = 0;
  
  // Wait for user to release button from capture hold
  while(digitalRead(PIN_CLICK) == LOW); 
  delay(100);

  // Render initial capture specs screen
  drawAssignSlotScreen(captured, assignSmartMode);

  while (assignedSlot == -1) {
    bool isClicked = (digitalRead(PIN_CLICK) == LOW);
    bool isUp      = (digitalRead(PIN_UP) == LOW);
    bool isDown    = (digitalRead(PIN_DOWN) == LOW);
    bool isLeft    = (digitalRead(PIN_LEFT) == LOW);
    bool isRight   = (digitalRead(PIN_RIGHT) == LOW);

    // Double-click detection for Smart Mode layer toggle
    if (isClicked) {
      if (millis() - lastClickTime < 300) {
        assignSmartMode = !assignSmartMode;
        playSmartModeSound();
        drawAssignSlotScreen(captured, assignSmartMode); // Refresh screen with updated layer
        delay(300); // Debounce
      } else {
        lastClickTime = millis();
        delay(300); 
        if (digitalRead(PIN_CLICK) == HIGH) { 
          assignedSlot = assignSmartMode ? 5 : 0; 
        }
      }
    } 
    else if (isUp)    assignedSlot = assignSmartMode ? 6 : 1; 
    else if (isDown)  assignedSlot = assignSmartMode ? 7 : 2; 
    else if (isLeft)  assignedSlot = assignSmartMode ? 8 : 3; 
    else if (isRight) assignedSlot = assignSmartMode ? 9 : 4; 
  }

  // 3. Save to EEPROM & Display Confirmation
  int addr = EEPROM_START_ADR + (assignedSlot * sizeof(IRSlot));
  EEPROM.put(addr, captured);

  drawSavedConfirmationScreen(getSlotName(assignedSlot));
  playSuccessSound(); 
  delay(2000);
}

// --------------------------------------------
// SEND LEARNED CODES
// --------------------------------------------

void sendLearnedCode(StandardCommand cmd) {
  int slotIndex = -1;

  switch(cmd) {
    case CMD_MUTE:        slotIndex = 0; break; // Center Click (Norm)
    case CMD_CH_UP:       slotIndex = 1; break; // Up (Norm)
    case CMD_CH_DOWN:     slotIndex = 2; break; // Down (Norm)
    case CMD_VOL_DOWN:    slotIndex = 3; break; // Left (Norm)
    case CMD_VOL_UP:      slotIndex = 4; break; // Right (Norm)
    
    case CMD_SELECT:      slotIndex = 5; break; // Center Click (Smart)
    case CMD_NAV_UP:      slotIndex = 6; break; // Up (Smart)
    case CMD_NAV_DOWN:    slotIndex = 7; break; // Down (Smart)
    case CMD_NAV_LEFT:    slotIndex = 8; break; // Left (Smart)
    case CMD_NAV_RIGHT:   slotIndex = 9; break; // Right (Smart)
    
    default: return; 
  }

  if (slotIndex != -1) {
    IRSlot stored;
    int addr = EEPROM_START_ADR + (slotIndex * sizeof(IRSlot));
    EEPROM.get(addr, stored);

    if (stored.protocol != 0 && stored.protocol != 255) {
      IrSender.write(stored.protocol, stored.address, stored.command, 1);
    } else {
      disableLEDFeedback();
      safeTone(200, 250); // Low error buzz
      enableLEDFeedback();
    }
  }
}