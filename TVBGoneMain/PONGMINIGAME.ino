// --------------------------------------------
// UPGRADED PONG MINI-GAME (SPI / SUB-PIXEL PHYSICS)
// --------------------------------------------

void runPongGame() {
  // 1. Use 'float' for sub-pixel physics. This allows smooth gradual speed-ups!
  float playerY = 24.0, aiY = 24.0;
  float ballX = 64.0, ballY = 36.0;
  
  float ballVx = 2.0;       // Base horizontal speed
  float ballVy = 1.0;       // Base vertical speed
  float speedMult = 1.0;    // Starts at 1.0x speed
  const float MAX_SPEED = 2.5; // Caps out at 2.5x speed

  int playerScore = 0, aiScore = 0;
  
  // Non-blocking button timer
  unsigned long buttonPressTime = 0;
  bool isPressing = false;

  // Wait for initial button release
  while (digitalRead(PIN_CLICK) == LOW);
  delay(100);

  while (true) {
    bool isPaused = false;

    // --- 1. NON-BLOCKING EXIT CHECK & PAUSE EFFECT ---
    if (digitalRead(PIN_CLICK) == LOW) {
      if (!isPressing) {
        isPressing = true;
        buttonPressTime = millis(); // Start the stopwatch
      } else if (millis() - buttonPressTime > 800) {
        playExitSound();
        drawMenu();
        while (digitalRead(PIN_CLICK) == LOW); 
        return; // Exit game
      }
      isPaused = true; // Freezes movement while button is held
    } else {
      isPressing = false; // Reset if let go early
    }

    // --- RUN PHYSICS ONLY WHEN NOT PAUSED ---
    if (!isPaused) {
      // --- 2. PLAYER CONTROLS ---
      if (digitalRead(PIN_UP) == LOW && playerY > 12.0)   playerY -= 3.0;
      if (digitalRead(PIN_DOWN) == LOW && playerY < 48.0) playerY += 3.0;

      // --- 3. BEATABLE AI LOGIC ---
      float aiCenter = aiY + 7.0;
      if (ballVx > 0) {
        if (aiCenter < ballY - 3.0 && aiY < 48.0)      aiY += 2.0; 
        else if (aiCenter > ballY + 3.0 && aiY > 12.0) aiY -= 2.0;
      } else {
        if (aiCenter < 30.0)      aiY += 1.0;
        else if (aiCenter > 34.0) aiY -= 1.0;
      }

      // --- 4. BALL MOVEMENT (With Speed Multiplier) ---
      ballX += (ballVx * speedMult);
      ballY += (ballVy * speedMult);

      // Top and Bottom Wall Collisions
      if (ballY <= 13.0 || ballY >= 61.0) {
        ballVy = -ballVy;
        if (ballY <= 13.0) ballY = 13.0; 
        if (ballY >= 61.0) ballY = 61.0;
        safeTone(1800, 10);
      }

      // --- 5. PADDLE HITS & GRADUAL SPEED UP ---
      if (ballX <= 5.0 && ballVx < 0 && ballY >= playerY - 1.0 && ballY <= playerY + 14.0) {
        float hitOffset = ballY - (playerY + 7.0); 
        ballVx = 2.0; 
        ballVy = hitOffset * 0.25; 
        if (speedMult < MAX_SPEED) speedMult += 0.15; 
        ballX = 6.0; 
        safeTone(2400, 15);
      }

      if (ballX >= 122.0 && ballVx > 0 && ballY >= aiY - 1.0 && ballY <= aiY + 14.0) {
        float hitOffset = ballY - (aiY + 7.0);
        ballVx = -2.0; 
        ballVy = hitOffset * 0.25; 
        if (speedMult < MAX_SPEED) speedMult += 0.15;
        ballX = 121.0; 
        safeTone(2000, 15);
      }

      // --- 6. SCORING ---
      if (ballX < 0.0 || ballX > 128.0) {
        if (ballX < 0.0) {
          aiScore++;
          safeTone(400, 120);
          ballVx = 2.0; 
          ballVy = 1.0;
        } else {
          playerScore++;
          safeTone(2800, 120);
          ballVx = -2.0; 
          ballVy = -1.0;
        }
        ballX = 64.0; 
        ballY = 36.0; 
        speedMult = 1.0;
        delay(500); 
      }
    } // End of !isPaused block

    // --- 7. RENDER SCREEN ---
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_5x7_tr);
      
      // Scoreboard Left
      u8g2.setCursor(0, 8);
      u8g2.print(F("YOU:"));
      u8g2.print(playerScore);
      u8g2.print(F(" CPU:"));
      u8g2.print(aiScore);

      // Draw battery indicator 
      drawBatteryIndicator(108, 0);

      u8g2.drawLine(0, 10, 128, 10);

      // Cast floats back to ints for drawing
      u8g2.drawBox(2, (int)playerY, 2, 14);  
      u8g2.drawBox(124, (int)aiY, 2, 14);    
      u8g2.drawDisc((int)ballX, (int)ballY, 2);
    } while (u8g2.nextPage());

    delay(15); // Game loop timing (~60 FPS)
  }
}