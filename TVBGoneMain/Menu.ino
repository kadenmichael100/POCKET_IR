void drawMenu() {
  u8g2.firstPage();
  do {
    // --- Top Header ---
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setCursor(0, 9);
    u8g2.print(F("SELECT BRAND"));

    // --- Battery Icon in Top Right Corner ---
    drawBatteryIndicator(106, 1);

    // --- Menu Items ---
    int prevIdx = (currentBrand - 1 + totalBrands) % totalBrands;
    u8g2.setCursor(12, 21);
    u8g2.print(brands[prevIdx]);

    // Selected item (Big text)
    u8g2.setFont(u8g2_font_9x15_tr);
    u8g2.setCursor(0, 39);
    u8g2.print(F(">"));
    u8g2.print(brands[currentBrand]);

    // Items below (Small text)
    u8g2.setFont(u8g2_font_6x10_tr);
    int nextIdx = (currentBrand + 1) % totalBrands;
    u8g2.setCursor(12, 51);
    u8g2.print(brands[nextIdx]);

    int nextIdx2 = (currentBrand + 2) % totalBrands;
    u8g2.setCursor(12, 62);
    u8g2.print(brands[nextIdx2]);

  } while (u8g2.nextPage());
}

void drawRemoteScreen() {
  u8g2.firstPage();
  do {
    // Large Brand Name
    u8g2.setFont(u8g2_font_9x15_tr);
    u8g2.setCursor(0, 18);
    u8g2.print(brands[currentBrand]);

    // Mode Status
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setCursor(0, 36);
    u8g2.print(isSmartMode ? F("MODE: SMART") : F("MODE: NORMAL"));

    // Instruction Text
    u8g2.setCursor(0, 56);
    u8g2.print(F("(Hold Center to Exit)"));

  } while (u8g2.nextPage());
}