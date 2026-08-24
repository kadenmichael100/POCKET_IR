// --- DISABLE UNUSED IR DECODERS BEFORE INCLUDING LIBRARY ---
#define DECODE_NEC          // Common for cheap/generic TVs & budget brands
#define DECODE_SAMSUNG      // Samsung
#define DECODE_SONY         // Sony
#define DECODE_PANASONIC    // Panasonic
#define DECODE_LG           // LG

// --- LIBRARIES ---
#include <Arduino.h>
#include <SPI.h>
#include <IRremote.hpp>
#include <U8g2lib.h>        
#include <EEPROM.h>
#include <LowPower.h>

// --- PIN SETUP ---
#define PIN_LEFT    0       // D0 = NAV LEFT
#define PIN_RIGHT   1       // D1 = NAV RIGHT
#define PIN_IR_RX   2       // D2 = IR RECEIVER
#define PIN_IR_SEND 3       // D3 = IR Sender
#define PIN_CLICK   4       // D4 = NAV CLICK
#define PIN_UP      5       // D5 = NAV UP
#define PIN_DOWN    6       // D6 = NAV DOWN
#define BUZZER_PIN  7       // D7 = BUZZER PIN

// --- HARDWARE SPI DISPLAY PINS ---
#define OLED_CS    10       // D10 = OLED Chip Select
#define OLED_DC     9       // D9  = OLED Data/Command
#define OLED_RST    8       // D8  = OLED Reset
// Hardware SPI automatically uses Pin 11 (MOSI) and Pin 13 (SCK)

// IR Command Storage Logic
struct IRSlot {
  uint8_t protocol;
  uint32_t address;
  uint32_t command;
};

// Universal Command Dictionary
enum StandardCommand {
  CMD_NONE,
  CMD_CH_UP, CMD_CH_DOWN, CMD_VOL_UP, CMD_VOL_DOWN,
  CMD_MUTE,
  CMD_NAV_UP, CMD_NAV_DOWN, CMD_NAV_LEFT, CMD_NAV_RIGHT,
  CMD_SELECT
};

void wakeUpRoutine() {
  // Left blank intentionally; needed for deepSleep.
}

ISR(PCINT2_vect) {
  // Interrupt Service Routine for Port D waking CPU
}

ISR(PCINT0_vect) {
  // Interrupt Service Routine for Port B waking CPU
}

// --- U8g2 DISPLAY CONSTRUCTOR (HARDWARE SPI) ---
// 4-wire Hardware SPI using your assigned OLED CS, DC, and RST pins
U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI u8g2(U8G2_R0, /* cs= */ OLED_CS, /* dc= */ OLED_DC, /* reset= */ OLED_RST);

const int EEPROM_START_ADR = 100;

// TV BRAND LIST
const char* brands[] = {"TVBGONE!", "SAMSUNG", "SONY", "LG", "ROKU/TCL", "VIZIO", "PANASONIC", "TOSHIBA", "HISENSE", "APPLE TV", "PONG GAME", "LEARNED", "[PROGRAM]"};
int totalBrands = 13; 
int currentBrand = 0;

// Global variables
bool inRemoteMode = false;
bool isSmartMode = false;
bool isScreenDimmed = false;
unsigned long exitTimer = 0;
unsigned long lastUserActivityTime = 0;
unsigned long lastBatteryCheck = 0;
const unsigned long BATTERY_INTERVAL = 15000;
int backupBrandBeforeToggle = 1;
bool presetSmartState = false;
bool learnedSmartState = false;

// --------------------------------------------
// SETUP FUNCTION
// --------------------------------------------

void setup() {
  // 1. Initialize Nav switch pins
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_CLICK, INPUT_PULLUP);
  
  // 2. Set buzzer pin to OUTPUT mode immediately on boot
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Keep transistor OFF at boot

  // 3. Configure IR Receiver & Sender
  // Receiver: Disable raw feedback to prevent DC hum & transistor overheating
  IrReceiver.begin(PIN_IR_RX, DISABLE_LED_FEEDBACK);
  
  // Sender: Pass (Send Pin, Enable Feedback Flag, Feedback Pin)
  IrSender.begin(PIN_IR_SEND, ENABLE_LED_FEEDBACK, BUZZER_PIN);
  
  // 4. Initialize OLED over Hardware SPI
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tr);
  
  drawMenu();
  updateBatteryDisplay();
  playWakeUpTone();
}

// --------------------------------------------
// MAIN LOOP
// --------------------------------------------

void loop() {
  bool isClicked = (digitalRead(PIN_CLICK) == LOW);
  bool isUp      = (digitalRead(PIN_UP) == LOW);
  bool isDown    = (digitalRead(PIN_DOWN) == LOW);

  unsigned long currentMillis = millis();
  if (currentMillis - lastBatteryCheck >= BATTERY_INTERVAL) {
    lastBatteryCheck = currentMillis;
    updateBatteryDisplay(); 
  }

  // --- SCREEN DIMMING LOGIC ---
  if (millis() - lastUserActivityTime > 10000) {
    if (!isScreenDimmed) {
      u8g2.setContrast(1);
      isScreenDimmed = true;
    }
  } else {
    if (isScreenDimmed) {
      u8g2.setContrast(207);
      isScreenDimmed = false;
    }
  }

  if (millis() - lastUserActivityTime > 30000) { 
    goToDeepSleep();
  }

  if (!inRemoteMode) {
    // --- MENU NAVIGATION ---
    if (isDown) { 
      currentBrand++; 
      if(currentBrand >= totalBrands) currentBrand = 0; 
      playNavTick(); 
      drawMenu(); 
      delay(250);
      lastUserActivityTime = millis();
    }
    else if (isUp) { 
      currentBrand--; 
      if(currentBrand < 0) currentBrand = totalBrands - 1; 
      playNavTick(); 
      drawMenu(); 
      delay(250);
      lastUserActivityTime = millis();
    }
    
    if (isClicked) {
      playSelectSound();
      lastUserActivityTime = millis();
  
      if (currentBrand == 12) { // "[PROGRAM]"
        startLearningMode();
        drawMenu();
        lastUserActivityTime = millis(); 
      }
      else if (currentBrand == 0) { // "TVBGONE!"
        triggerUniversalOff(); 
      }   
      else if (currentBrand == 10) { // "PONG GAME"
        runPongGame();
        drawMenu();
        lastUserActivityTime = millis();
      }
      else {
        while(digitalRead(PIN_CLICK) == LOW) { 
          delay(10);
        }
        delay(100);
    
        drawRemoteScreen();
        exitTimer = millis();
        inRemoteMode = true;
      }
    } 
  } else {
    // --- REMOTE PLAYBACK MODE ---
    handleRemoteInput(); 
  }
}