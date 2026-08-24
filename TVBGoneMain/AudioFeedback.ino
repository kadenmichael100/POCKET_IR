
// =================================================================================
// IRremote and the standard tone() function both rely on ATmega328P timer 2
// When tone() is called, it reconfigures the Timer 2 registers to generate audio.
// This permanently corrupts the 38kHz carrier wave timing and sampling rate 
// used by IRremote, breaking all IR learning and sending until a system reboot.
//
// The following function (safeTone) creates audio using pure software 
// bit-banging (delayMicroseconds) to toggle the buzzer pin. This produces clean audio 
// feedback without touching hardware timers, keeping the device's IR operations reliable.
// =================================================================================

void safeTone(unsigned int frequency, unsigned long duration) {
  disableLEDFeedback();  if (frequency == 0) return;
  unsigned long period = 1000000L / frequency;
  unsigned long halfPeriod = period / 2;
  unsigned long start = millis();
  
  while (millis() - start < duration) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(halfPeriod);
  }
  enableLEDFeedback();
  }

// --- SOUND LIBRARY ---

// --- MENU NAV UP/DOWN ---
void playNavTick() {
  // A tiny, high-pitched mechanical tick for menu scrolling
  safeTone(3000, 10); 
}
// --- Select ---
void playSelectSound() {
  // A pleasant double-chirp for selecting modes
  safeTone(1500, 50); delay(50);
  safeTone(2000, 80);
}

// --- Error ---
void playErrorSound() {
  // A low, descending buzz for empty slots / unrecognized actions
  safeTone(300, 150); delay(50);
  safeTone(200, 200);
}


// --- SMART MODE SWITCH ---
// A futuristic, sweeping "sci-fi" sound for entering Smart Mode
void playSmartModeSound() {
  for (int freq = 1200; freq < 2500; freq += 50) {
    safeTone(freq, 7);
  }
  delay(20);
  safeTone(2800, 120);
}

// --- EXIT / BACK OUT ---
// A clean, descending sci-fi sweep for leaving Smart Mode or a menu
void playExitSound() {
  for (int freq = 2200; freq > 1000; freq -= 60) {
    safeTone(freq, 6);
  }
  delay(10);
  safeTone(800, 100); // Final low grounding note
}
void playSuccessSound(){
  safeTone(1800, 100); delay(30);
  safeTone(2400, 250); delay(300);
}

void playWakeUpTone() {  
  // Ascending musical triad (Clean and bright)
  safeTone(1047, 60);  // C6 note
  delay(30);
  safeTone(1319, 60);  // E6 note
  delay(30);
  safeTone(1568, 120); // G6 note
  }

void playPowerDownTone() {  
  // Descending musical triad (Falling action)
  safeTone(1568, 60);  // G6 note
  delay(30);
  safeTone(1319, 60);  // E6 note
  delay(30);
  safeTone(1047, 120); // C6 note
  
}

void playLearnedToggleSound(bool enteringLearned) {
  if (enteringLearned) {
    // Ascending triplet chime (Powering up into Learned mode)
    tone(BUZZER_PIN, 880, 50);  // A5
    delay(60);
    tone(BUZZER_PIN, 1318, 50); // E6
    delay(60);
    tone(BUZZER_PIN, 1760, 80); // A6
  } else {
    // Descending triplet chime (Dropping back to Preset mode)
    tone(BUZZER_PIN, 1760, 50); // A6
    delay(60);
    tone(BUZZER_PIN, 1318, 50); // E6
    delay(60);
    tone(BUZZER_PIN, 880, 80);  // A5
  }
}
