/*
  Real-Time Environmental Monitoring System — SLAVE 2 (Dual Display)
  -------------------------------------------------------------------
  Systems Engineering for Real-time Systems — Individual Project
  Al-Nahrain University | Graded Excellent — first submission in section

  ROLE: Slave 2 — Display Node
  - Drives two independent I2C LCD displays (temperature and gas level).
  - Responds to Master commands:
      200 + mode byte -> switch display mode (1=temp only, 2=gas only, 3=both)
      101 + value      -> update temperature display
      102 + value      -> update gas-level display

  WIRING:
    SCK  -> D13 (input, from Master)
    MISO -> D11 (input, from Master's MOSI line)
    SS   -> D10 (input, active LOW, chip-select from Master)
    LCD1 (temperature) -> I2C address 0x20
    LCD2 (gas level)   -> I2C address 0x27

  LIBRARY REQUIRED: LiquidCrystal_I2C
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd1(0x20, 16, 2);
LiquidCrystal_I2C lcd2(0x27, 16, 2);

byte currentMode = 3;

void setup() {
  lcd1.init(); lcd1.backlight();
  lcd2.init(); lcd2.backlight();
  pinMode(13, INPUT); pinMode(11, INPUT);
  pinMode(10, INPUT_PULLUP);

  lcd1.clear(); lcd1.print("SYSTEM STARTING");
  lcd2.clear(); lcd2.print("PLEASE WAIT...");
  delay(1000);
  lcd1.clear(); lcd2.clear();
}

byte handleSPI() {
  byte incoming = 0;
  for (int i = 7; i >= 0; i--) {
    while (digitalRead(13) == LOW && digitalRead(10) == LOW);
    if (digitalRead(11) == HIGH) bitSet(incoming, i);
    while (digitalRead(13) == HIGH && digitalRead(10) == LOW);
  }
  return incoming;
}

void loop() {
  if (digitalRead(10) == LOW) {
    byte cmd = handleSPI();

    if (cmd == 200) {
      currentMode = handleSPI();
      lcd1.clear(); lcd2.clear();
      if (currentMode == 1) {
        lcd1.print("MODE: TEMP ONLY");
        lcd2.noBacklight();
      }
      if (currentMode == 2) {
        lcd2.print("MODE: GAS ONLY");
        lcd1.noBacklight();
      }
      if (currentMode == 3) {
        lcd1.backlight(); lcd2.backlight();
        lcd1.print("MODE: FULL SCAN");
        lcd2.print("MODE: FULL SCAN");
      }
      delay(500);
      lcd1.clear(); lcd2.clear();
    }
    else if (cmd == 101) {
      byte v = handleSPI();
      if (currentMode == 1 || currentMode == 3) {
        lcd1.backlight();
        lcd1.setCursor(0, 0);
        lcd1.print(">> TEMPERATURE");
        lcd1.setCursor(0, 1);
        lcd1.print("VALUE: "); lcd1.print(v); lcd1.print(" C    ");
      }
    }
    else if (cmd == 102) {
      byte v = handleSPI();
      if (currentMode == 2 || currentMode == 3) {
        lcd2.backlight();
        lcd2.setCursor(0, 0);
        lcd2.print(">> GAS LEVEL");
        lcd2.setCursor(0, 1);
        lcd2.print("VALUE: "); lcd2.print(v); lcd2.print(" %    ");
      }
    }
  }
}
