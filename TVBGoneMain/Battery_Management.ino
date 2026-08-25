// --------------------------------------------
// BATTERY MEASUREMENT & DISPLAY
// --------------------------------------------

// This function measures battery supply voltage (Vcc) internally in millivolts 
// without using any external I/O pins or voltage divider circuits.
// The ADC is configured to measure the chip's internal 1.1V bandgap 
// reference against Vcc, then back-calculates Vcc using the ratio:
// Vcc (mV) = (1.1V * 1023 * 1000) / ADC_Reading

long readVcc() {
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__)
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif   

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion

  while (bit_is_set(ADCSRA, ADSC)); // Measuring

  uint8_t low  = ADCL; 
  uint8_t high = ADCH; 

  long result = (high << 8) | low;

  // Calculate Vcc (in mV); 1125300 = 1.1V * 1023 * 1000
  result = 1125300L / result; 
  return result; // Returns millivolts (e.g., 4500 for 4.5V)
}

// Helper function called inside drawMenu() / drawRemoteScreen() page loops
void drawBatteryIndicator(int x, int y) {

  static unsigned long lastBattReadTime = 0;
  static int cachedBattLevel = 100; // Store the percentage here

  // 1. Only do the math/hardware read every 5 seconds (5000 ms)
  if (millis() - lastBattReadTime > 5000 || lastBattReadTime == 0) {
    
    long current_mV = readVcc();

    // --- BATTERY CALIBRATION ---
    // 3x AAA batteries are ~4500mV full, ~3000mV empty.
    long empty_mV = 3000; 
    long full_mV = 4500;  

    // Map the millivolts to a 0-100 percentage
    cachedBattLevel = map(current_mV, empty_mV, full_mV, 0, 100);
    
    // Ensure the percentage never drops below 0 or goes above 100
    cachedBattLevel = constrain(cachedBattLevel, 0, 100);
    
    lastBattReadTime = millis(); // Reset the stopwatch
  }

  // 2. Draw the icon using the 'cachedBattLevel'
  u8g2.drawFrame(x, y, 16, 8); // Outline
  u8g2.drawBox(x + 16, y + 2, 2, 4); // Battery nub
  
  // Draw the fill based on the cached battery percentage (0 to 14 pixels wide)
  int fillWidth = map(cachedBattLevel, 0, 100, 0, 14);
  u8g2.drawBox(x + 1, y + 1, fillWidth, 6);
}

// Non-blocking timer calls this to refresh the current screen with new battery level
void updateBatteryDisplay() {
  if (inRemoteMode) {
    drawRemoteScreen();
  } else {
    drawMenu();
  }
}


// --------------------------------------------
// DEEP SLEEP FUNCTION 
// --------------------------------------------

void goToDeepSleep() {
  playPowerDownTone();
  
  // 1. Silence buzzer and safely pause IR receiver timers before sleeping
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  noTone(BUZZER_PIN);
  IrReceiver.stop(); 
  delay(50); 

  // 2. Deselect OLED CS pin (D10) and put display into 0.5uA sleep mode
  digitalWrite(OLED_CS, HIGH);
  u8g2.setPowerSave(1); 

  // 3. Clear any pending interrupt flags on Port D
  PCIFR |= (1 << PCIF2);

  // 4. Enable Pin Change Interrupts ONLY on Port D (PCIE2)
  // (Port B / PCIE0 is no longer used because D8-D13 are reserved for SPI/OLED)
  PCICR |= (1 << PCIE2); 
  
  // 5. Mask ONLY the 5 Nav Switch pins on Port D:
  // PCINT16 = D0 (PIN_LEFT)
  // PCINT17 = D1 (PIN_RIGHT)
  // PCINT20 = D4 (PIN_CLICK)
  // PCINT21 = D5 (PIN_UP)
  // PCINT22 = D6 (PIN_DOWN)
  PCMSK2 = (1 << PCINT16) | (1 << PCINT17) | (1 << PCINT20) | (1 << PCINT21) | (1 << PCINT22); 

  // 6. Freeze MCU core until a nav button is pressed
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  
  // ===================================================================
  // --------- MICROCONTROLLER PAUSES HERE UNTIL BUTTON PRESS ----------
  // ===================================================================
  
  // 7. Immediately disable wake-up interrupts upon waking
  PCICR &= ~(1 << PCIE2);
  PCMSK2 = 0;

  // 8. Wake up OLED display over Hardware SPI
  u8g2.setPowerSave(0); 
  
  // 9. Restart IR Sender and Receiver cleanly
  IrReceiver.begin(PIN_IR_RX, DISABLE_LED_FEEDBACK);
  IrSender.begin(PIN_IR_SEND, ENABLE_LED_FEEDBACK, BUZZER_PIN);
            
  playWakeUpTone();

  // Reset menu state
  inRemoteMode = false;
  isSmartMode = false;
  drawMenu(); 
  
  // 10. Safety Loop with 1-Second Timeout (Prevents infinite lockup if a pin stays LOW)
  unsigned long releaseTimeout = millis();
  while ((digitalRead(PIN_UP) == LOW    || digitalRead(PIN_DOWN) == LOW || 
          digitalRead(PIN_LEFT) == LOW  || digitalRead(PIN_RIGHT) == LOW || 
          digitalRead(PIN_CLICK) == LOW) && (millis() - releaseTimeout < 1000)) { 
    delay(10); 
  }
  delay(50); // Debounce delay

  lastUserActivityTime = millis(); 
}