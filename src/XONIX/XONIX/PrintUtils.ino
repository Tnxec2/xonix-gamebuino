
byte popupTimeLeft;
const __FlashStringHelper* popupText;

void printCentered(const __FlashStringHelper* text) {
  gb.display.cursorX = (LCDWIDTH / 2) - (strlen_PF((unsigned long)text) * gb.display.fontSize * gb.display.fontWidth / 2);
  gb.display.print(text);
}

void printCentered(char* text) {
  gb.display.cursorX = (LCDWIDTH / 2) - (strlen(text) * gb.display.fontSize * gb.display.fontWidth / 2);
  gb.display.print(text);
}

void popup(const __FlashStringHelper* text) {
  popup(text, 20);
}
void popup(const __FlashStringHelper* text, uint8_t duration) {
  popupText = text;
  popupTimeLeft = duration + 12;
}

void updatePopup() {
  if (popupTimeLeft) {
    uint8_t yOffset = 0;
    if (popupTimeLeft < 12) {
      yOffset = popupTimeLeft - 12;
    }
    byte width = strlen_PF((unsigned long)popupText) * gb.display.fontSize * gb.display.fontWidth;
    gb.display.fontSize = 1;
    gb.display.setColor(BLACK);
    gb.display.drawRect(LCDWIDTH / 2 - width / 2 - 2, yOffset - 1, width + 2, gb.display.fontHeight + 2);
    gb.display.setColor(WHITE);
    gb.display.fillRect(LCDWIDTH / 2 - width / 2 - 1, yOffset - 1, width + 1, gb.display.fontHeight + 1);
    gb.display.setColor(BLACK);
    gb.display.cursorY = yOffset;
    printCentered(popupText);
    popupTimeLeft--;
  }
}