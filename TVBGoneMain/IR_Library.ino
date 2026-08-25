// --------------------------------------------
// PRESET BRANDS
// --------------------------------------------
void sendPresetCode(int currentBrand, StandardCommand cmd) {
  
  // Used the raw char array to avoid dynamic memory fragmentation
  const char* brand = brands[currentBrand];
  uint8_t hex = 0;

  if (strcmp(brand, "SAMSUNG") == 0) {
    switch(cmd) {
      case CMD_CH_UP:    hex = 0x12; break;  case CMD_CH_DOWN:    hex = 0x10; break;
      case CMD_VOL_UP:   hex = 0x07; break;  case CMD_VOL_DOWN:   hex = 0x0B; break;
      case CMD_MUTE:     hex = 0x0F; break;  case CMD_SELECT:     hex = 0x68; break;
      case CMD_NAV_UP:   hex = 0x60; break;  case CMD_NAV_DOWN:   hex = 0x61; break;
      case CMD_NAV_LEFT: hex = 0x65; break;  case CMD_NAV_RIGHT:  hex = 0x62; break;
      default: return;
    }
    IrSender.sendSamsung(0x0707, hex, 0);
  } 
  
  else if (strcmp(brand, "SONY") == 0) {
    switch(cmd) {
      case CMD_CH_UP:    hex = 0x10; break;  case CMD_CH_DOWN:    hex = 0x11; break;
      case CMD_VOL_UP:   hex = 0x12; break;  case CMD_VOL_DOWN:   hex = 0x13; break;
      case CMD_MUTE:     hex = 0x14; break;  case CMD_SELECT:     hex = 0x65; break;
      case CMD_NAV_UP:   hex = 0x74; break;  case CMD_NAV_DOWN:   hex = 0x75; break;
      case CMD_NAV_LEFT: hex = 0x34; break;  case CMD_NAV_RIGHT:  hex = 0x33; break;
      default: return;
    }
    IrSender.sendSony(0x01, hex, 2, 12); // Sony requires repeats
  } 
  
  else if (strcmp(brand, "LG") == 0) {
    switch(cmd) {
      case CMD_CH_UP:    hex = 0x00; break;  case CMD_CH_DOWN:    hex = 0x01; break;
      case CMD_VOL_UP:   hex = 0x02; break;  case CMD_VOL_DOWN:   hex = 0x03; break;
      case CMD_MUTE:     hex = 0x09; break;  case CMD_SELECT:     hex = 0x44; break;
      case CMD_NAV_UP:   hex = 0x40; break;  case CMD_NAV_DOWN:   hex = 0x41; break;
      case CMD_NAV_LEFT: hex = 0x07; break;  case CMD_NAV_RIGHT:  hex = 0x06; break;
      default: return;
    }
    IrSender.sendNEC(0x04, hex, 0);
  } 
  
  else if (strcmp(brand, "ROKU/TCL") == 0) {
    switch(cmd) {
      case CMD_VOL_UP:   hex = 0xCC; break;  case CMD_VOL_DOWN:   hex = 0xCD; break;
      case CMD_MUTE:     hex = 0x8F; break;  case CMD_SELECT:     hex = 0xC7; break;
      case CMD_NAV_UP:   hex = 0xCA; break;  case CMD_NAV_DOWN:   hex = 0xCB; break;
      case CMD_NAV_LEFT: hex = 0xC8; break;  case CMD_NAV_RIGHT:  hex = 0xC9; break;
      default: return; // Roku doesn't traditionally map CH_UP/DOWN
    }
    IrSender.sendNEC(0x5743, hex, 0);
  } 
  
  else if (strcmp(brand, "VIZIO") == 0) {
    switch(cmd) {
      case CMD_CH_UP:    hex = 0x00; break;  case CMD_CH_DOWN:    hex = 0x08; break;
      case CMD_VOL_UP:   hex = 0x40; break;  case CMD_VOL_DOWN:   hex = 0xC0; break;
      case CMD_MUTE:     hex = 0x09; break;  case CMD_SELECT:     hex = 0x59; break;
      case CMD_NAV_UP:   hex = 0x54; break;  case CMD_NAV_DOWN:   hex = 0x55; break;
      case CMD_NAV_LEFT: hex = 0x5A; break;  case CMD_NAV_RIGHT:  hex = 0x5B; break;
      default: return;
    }
    IrSender.sendNEC(0x04FB, hex, 0);
  } 
  
  else if (strcmp(brand, "PANASONIC") == 0) {
    switch(cmd) {
      case CMD_CH_UP:    hex = 0x34; break;  case CMD_CH_DOWN:    hex = 0x35; break;
      case CMD_VOL_UP:   hex = 0x30; break;  case CMD_VOL_DOWN:   hex = 0x31; break;
      case CMD_MUTE:     hex = 0x32; break;  case CMD_SELECT:     hex = 0x56; break;
      case CMD_NAV_UP:   hex = 0x52; break;  case CMD_NAV_DOWN:   hex = 0x53; break;
      case CMD_NAV_LEFT: hex = 0x54; break;  case CMD_NAV_RIGHT:  hex = 0x55; break;
      default: return;
    }
    IrSender.sendPanasonic(0x4004, hex, 0);
  } 
  
  else if (strcmp(brand, "TOSHIBA") == 0) {
    switch(cmd) {
      case CMD_CH_UP:    hex = 0x1A; break;  case CMD_CH_DOWN:    hex = 0x1B; break;
      case CMD_VOL_UP:   hex = 0x16; break;  case CMD_VOL_DOWN:   hex = 0x17; break;
      case CMD_MUTE:     hex = 0x14; break;  case CMD_SELECT:     hex = 0x13; break;
      case CMD_NAV_UP:   hex = 0x4E; break;  case CMD_NAV_DOWN:   hex = 0x4F; break;
      case CMD_NAV_LEFT: hex = 0x50; break;  case CMD_NAV_RIGHT:  hex = 0x51; break;
      default: return;
    }
    IrSender.sendNEC(0x02FD, hex, 0);
  } 
  
  else if (strcmp(brand, "HISENSE") == 0) {
    switch(cmd) {
      case CMD_CH_UP:    hex = 0x12; break;  case CMD_CH_DOWN:    hex = 0x13; break;
      case CMD_VOL_UP:   hex = 0x0A; break;  case CMD_VOL_DOWN:   hex = 0x0B; break;
      case CMD_MUTE:     hex = 0x0D; break;  case CMD_SELECT:     hex = 0x17; break;
      case CMD_NAV_UP:   hex = 0x06; break;  case CMD_NAV_DOWN:   hex = 0x07; break;
      case CMD_NAV_LEFT: hex = 0x08; break;  case CMD_NAV_RIGHT:  hex = 0x09; break;
      default: return;
    }
    IrSender.sendNEC(0xFB04, hex, 0);
  } 
  
  else if (strcmp(brand, "APPLE TV") == 0) {
    switch(cmd) {
      case CMD_VOL_UP:   hex = 0x0B; break;  case CMD_VOL_DOWN:   hex = 0x0D; break;
      case CMD_MUTE:     hex = 0x04; break;  case CMD_SELECT:     hex = 0x05; break; 
      case CMD_NAV_UP:   hex = 0x0A; break;  case CMD_NAV_DOWN:   hex = 0x0C; break;
      case CMD_NAV_LEFT: hex = 0x09; break;  case CMD_NAV_RIGHT:  hex = 0x06; break;
      default: return;
    }
    IrSender.sendApple(0xEE87, hex, 0);
  }
}