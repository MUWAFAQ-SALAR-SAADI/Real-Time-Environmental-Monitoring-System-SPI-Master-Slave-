/*
  Real-Time Environmental Monitoring System — SLAVE 1 (Sensors)
  ------------------------------------------------------------
  Systems Engineering for Real-time Systems — Individual Project
  Al-Nahrain University | Graded Excellent — first submission in section

  ROLE: Slave 1 — Sensor Node
  - Continuously reads a TMP36 temperature sensor and a gas/smoke sensor.
  - Responds to the Master's SPI polling: command 1 -> return temperature,
    command 2 -> return gas level.

  WIRING:
    SCK  -> D13 (input, from Master)
    MISO -> D11 (input, from Master's MOSI line — see note below)
    MOSI-out (this node's data to Master) -> D12
    SS   -> D10 (input, active LOW, chip-select from Master)
    TMP36 sensor  -> A0
    Gas sensor    -> A1

  Note: pin roles here mirror the bit-banged protocol used by the Master
  (this is software SPI, not hardware SPI, so pin naming follows the
  Master's wiring rather than the standard Arduino SPI library pinout).
*/

byte tempVal = 0, smokeVal = 0;

void setup() {
  pinMode(13, INPUT); pinMode(11, INPUT);
  pinMode(12, OUTPUT); pinMode(10, INPUT_PULLUP);
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

void sendSPI(byte data) {
  for (int i = 7; i >= 0; i--) {
    digitalWrite(12, bitRead(data, i) ? HIGH : LOW);
    while (digitalRead(13) == LOW && digitalRead(10) == LOW);
    while (digitalRead(13) == HIGH && digitalRead(10) == LOW);
  }
}

void loop() {
  // Read TMP36 temperature sensor
  int rawT = analogRead(A0);
  float v = rawT * (5.0 / 1024.0);
  tempVal = (byte)constrain(((v - 0.5) * 100), 0, 150);

  // Read gas sensor
  int rawG = analogRead(A1);
  smokeVal = (byte)map(rawG, 0, 1023, 0, 100);

  if (digitalRead(10) == LOW) {
    byte cmd = handleSPI();
    if (cmd == 1) sendSPI(tempVal);
    else if (cmd == 2) sendSPI(smokeVal);
  } else {
    digitalWrite(12, LOW);
  }
}
