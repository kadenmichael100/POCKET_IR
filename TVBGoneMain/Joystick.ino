// --------------------------------------------
// 5-WAY SWITCH NAVIGATION FUNCTION (DEBOUCED)
// --------------------------------------------

// Helper function to call whenever entering remote mode to flush old input state
void resetRemoteInputState() {
  // Reset directional locks and click counters
  // (Gets called inside drawRemoteScreen() or when transitioning into remote mode)
}

void handleRemoteInput() {
  // --- TIMING & DEBOUNCE CONSTANTS ---
  const unsigned long DEBOUNCE_DELAY  = 35;  // Ignore switch contact bounce (<35ms)
  const unsigned long LONG_PRESS_TIME = 1200; // Hold threshold for menu exit (1.2s)
  const unsigned long MULTI_CLICK_GAP = 350;  // Window to wait for subsequent clicks

  // --- PERSISTENT STATE VARIABLES ---
  static bool lastRawClickState = HIGH;
  static bool debouncedClickState = HIGH;
  static unsigned long lastDebounceTime = 0;

  static unsigned long holdStartTime = 0;
  static unsigned long lastReleaseTime = 0;
  static int clickCount = 0;
  static bool longPressHandled = false;

  static StandardCommand lastActiveCmd = CMD_NONE;
  static unsigned long lastSendTime = 0;

  StandardCommand cmd = CMD_NONE;

  // Read raw pin states (Active LOW)
  bool rawClick = digitalRead(PIN_CLICK);
  bool isUp     = (digitalRead(PIN_UP) == LOW);
  bool isDown   = (digitalRead(PIN_DOWN) == LOW);
  bool isLeft   = (digitalRead(PIN_LEFT) == LOW);
  bool isRight  = (digitalRead(PIN_RIGHT) == LOW);

  unsigned long now = millis();

  // =========================================================================
  // 1. DEBOUNCE FILTER FOR CENTER CLICK PIN
  // =========================================================================
  if (rawClick != lastRawClickState) {
    lastDebounceTime = now;
    lastRawClickState = rawClick;
  }

  if ((now - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (rawClick != debouncedClickState) {
      debouncedClickState = rawClick;

      // TRANSITION: BUTTON JUST PRESSED
      if (debouncedClickState == LOW) {
        if (holdStartTime == 0) {
          holdStartTime = now;
          longPressHandled = false;
        }
      } 
      // TRANSITION: BUTTON JUST RELEASED
      else {
        if (holdStartTime != 0 && !longPressHandled) {
          unsigned long pressDuration = now - holdStartTime;
          if (pressDuration < LONG_PRESS_TIME) {
            clickCount++;
            lastReleaseTime = now;
          }
        }
        holdStartTime = 0;
      }
    }
  }

  // =========================================================================
  // 2. LONG PRESS EVALUATION (EXIT TO MENU)
  // =========================================================================
  if (debouncedClickState == LOW && holdStartTime != 0 && !longPressHandled) {
    if (now - holdStartTime >= LONG_PRESS_TIME) {
      longPressHandled = true;
      clickCount = 0;        // Purge click queue
      holdStartTime = 0;
      inRemoteMode = false;

      playExitSound();
      drawMenu();

      // Blocking safety lock until finger lifts off switch
      while (digitalRead(PIN_CLICK) == LOW) { delay(10); }
      return;
    }
  }

  // =========================================================================
  // 3. MULTI-CLICK RESOLUTION ENGINE (SINGLE vs DOUBLE vs TRIPLE)
  // =========================================================================
  if (clickCount > 0 && debouncedClickState == HIGH) {
    if (now - lastReleaseTime > MULTI_CLICK_GAP) {
      
      if (clickCount == 1) {
        // Single Click: Context Action
        cmd = isSmartMode ? CMD_SELECT : CMD_MUTE;
      } 
      else if (clickCount == 2) {
        // Double Click: Toggle Layer (Normal <-> Smart)
        isSmartMode = !isSmartMode; 
        playSmartModeSound(); 
        drawRemoteScreen(); 
      }
      else if (clickCount >= 3) {
        // Triple Click: Swap Brand Preset vs Learned Mode
        if (currentBrand == 11) { 
          // Leaving Learned Mode -> Restoring Preset Brand
          learnedSmartState = isSmartMode;
          currentBrand = backupBrandBeforeToggle; 
          isSmartMode = presetSmartState; 
          playLearnedToggleSound(false); 
        } 
        else {
          // Leaving Preset Brand -> Warping to Learned Mode (Index 10)
          presetSmartState = isSmartMode; 
          backupBrandBeforeToggle = currentBrand; 
          currentBrand = 11; 
          isSmartMode = learnedSmartState; 
          playLearnedToggleSound(true); 
        }
        drawRemoteScreen();
      }

      clickCount = 0; // Reset multi-click counter cleanly
    }
  }

  // =========================================================================
  // 4. DIRECTIONAL MAPPING & AUTO-REPEAT LOCK
  // =========================================================================
  StandardCommand currentDirCmd = CMD_NONE;

  // Read directional switches only when Center Click is not active
  if (debouncedClickState == HIGH && clickCount == 0) {
    if (isSmartMode) {
      if (isUp)        currentDirCmd = CMD_NAV_UP;
      else if (isDown)  currentDirCmd = CMD_NAV_DOWN;
      else if (isLeft)  currentDirCmd = CMD_NAV_LEFT;
      else if (isRight) currentDirCmd = CMD_NAV_RIGHT;
    } else {
      if (isUp)        currentDirCmd = CMD_CH_UP;
      else if (isDown)  currentDirCmd = CMD_CH_DOWN;
      else if (isLeft)  currentDirCmd = CMD_VOL_DOWN; 
      else if (isRight) currentDirCmd = CMD_VOL_UP;   
    }
  }

  // Command fire logic
  if (currentDirCmd == CMD_NONE) {
    lastActiveCmd = CMD_NONE; // Unlock continuous press when released
  } 
  else {
    // CONTINUOUS AUTO-REPEAT ALLOWED FOR VOLUME AND CHANNEL PINS
    if (currentDirCmd == CMD_VOL_UP || currentDirCmd == CMD_VOL_DOWN || 
        currentDirCmd == CMD_CH_UP  || currentDirCmd == CMD_CH_DOWN) {
      if (now - lastSendTime > 180) {
        cmd = currentDirCmd;
      }
    } 
    // SINGLE-SHOT LOCK FOR DIRECTIONAL NAV PINS
    else if (lastActiveCmd == CMD_NONE) { 
      cmd = currentDirCmd;
      lastActiveCmd = currentDirCmd; // Lock directional repeat
    }
  }

  // =========================================================================
  // 5. IR TRANSMISSION DISPATCH
  // =========================================================================
  if (cmd != CMD_NONE) {
    lastUserActivityTime = now; 
    lastSendTime = now; 

    if (strcmp(brands[currentBrand], "LEARNED") == 0 || currentBrand == 11) { 
      sendLearnedCode(cmd); 
    } else {
      sendPresetCode(currentBrand, cmd);
    }
  }
}