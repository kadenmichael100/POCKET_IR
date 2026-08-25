// --------------------------------------------
// PONG MINI-GAME (SPI / SUB-PIXEL PHYSICS)
// --------------------------------------------

void runPongGame() {

  // Use 'float' for sub-pixel physics.
  float playerY = 24.0, aiY = 24.0;
  float ballX = 64.0, ballY = 36.0;
  
  float ballVx = 2.0;          // Base horizontal speed
  float ballVy = 1.0;          // Base vertical speed
  float speedMult = 0.7;       // Starts at 0.7x speed
  const float MAX_SPEED = 4.0; // Caps out at 4x speed

  uint8_t playerScore = 0, aiScore = 0;
  
  // Non-blocking button & pause state
  unsigned long buttonPressTime = 0;
  bool isPressing = false;
  bool gamePaused = false;

  u8g2.setFont(u8g2_font_5x7_tr);

  // Wait for initial button release
  while (digitalRead(PIN_CLICK) == LOW);
  delay(100);

  while (true) {

    // --- 1. TOGGLE PAUSE & HOLD-TO-EXIT CHECK ---
    if (digitalRead(PIN_CLICK) == LOW) {
      if (!isPressing) {
        isPressing = true;
        buttonPressTime = millis();
      } else if (millis() - buttonPressTime > 800) {
        playExitSound();
        drawMenu();
        while (digitalRead(PIN_CLICK) == LOW); 
        return; // Exit game on long press
      }
    } else if (isPressing) {
      // Releasing button before 800ms triggers a pause toggle
      gamePaused = !gamePaused;
      
      if (gamePaused) {
        playPowerDownTone(); // Lower tone on pause
      } else {
        playWakeUpTone(); // Higher tone on unpause
      }
      
      isPressing = false;
    }

    // --- 2. RUN PHYSICS (ONLY WHEN UNPAUSED) ---
    if (!gamePaused) {
      // Player controls
      if (digitalRead(PIN_UP) == LOW)   playerY -= 3.0;
      if (digitalRead(PIN_DOWN) == LOW) playerY += 3.0;
      if (playerY < 12.0) playerY = 12.0;
      else if (playerY > 48.0) playerY = 48.0;

      // AI Logic & Bounds
      float aiSpeed = 2.0 + (speedMult * 0.50);
      float aiCenter = aiY + 7.0;
      if (ballVx > 0) {
        if (aiCenter < ballY - 1.0)      aiY += aiSpeed; 
        else if (aiCenter > ballY + 1.0) aiY -= aiSpeed;
      } else {
        if (aiCenter < 30.0)      aiY += 2.0;
        else if (aiCenter > 34.0) aiY -= 2.0;
      }
      if (aiY < 12.0) aiY = 12.0;
      else if (aiY > 48.0) aiY = 48.0;

      // Ball Movement
      ballX += (ballVx * speedMult);
      ballY += (ballVy * speedMult);

      // Top and Bottom Wall Collisions
      if (ballY <= 13.0 || ballY >= 61.0) {
        ballVy = -ballVy;
        if (ballY <= 13.0) ballY = 13.0; 
        if (ballY >= 61.0) ballY = 61.0;
        safeTone(1800, 10);
      }

      // Paddle Hits
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

      // Scoring
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
    } // End of !gamePaused block

    // --- 3. PRE-CALCULATE DRAW DATA ---
    uint8_t drawPlayerY = playerY;
    uint8_t drawAiY     = aiY;
    uint8_t drawBallX   = ballX;
    uint8_t drawBallY   = ballY;

    // Fast zero-division score string formatting
    char scoreBuf[8];
    uint8_t p = 0;
    if (playerScore >= 10) { scoreBuf[p++] = '1'; scoreBuf[p++] = '0' + (playerScore - 10); }
    else { scoreBuf[p++] = '0' + playerScore; }
    scoreBuf[p++] = ':';
    if (aiScore >= 10) { scoreBuf[p++] = '1'; scoreBuf[p++] = '0' + (aiScore - 10); }
    else { scoreBuf[p++] = '0' + aiScore; }
    scoreBuf[p] = '\0';

    // --- 4. RENDER SCREEN ---
    u8g2.firstPage();
    do {
      u8g2.drawStr(54, 8, scoreBuf);
      u8g2.drawLine(0, 10, 128, 10);

      // Game objects remain visible on screen while paused
      u8g2.drawBox(2, drawPlayerY, 2, 14);   
      u8g2.drawBox(124, drawAiY, 2, 14);    
      u8g2.drawDisc(drawBallX, drawBallY, 2);
    } while (u8g2.nextPage());

    delay(2);
  }
}