/*
  Real-Time Environmental Monitoring System — MASTER
  ----------------------------------------------------
  Systems Engineering for Real-time Systems — Individual Project
  Al-Nahrain University | Graded Excellent — first submission in section

  ROLE: Master
  - Polls Slave 1 (sensor node) for temperature and gas-level readings via
    software-bitbanged SPI.
  - Forwards the latest readings to Slave 2 (display node) alternately.
  - Accepts serial commands from the user to switch display mode:
      [1] Temperature only   [2] Gas only   [3] Full system (both)

  WIRING:
    SCK  -> D13   (shared clock line to both slaves)
    MOSI -> D11   (shared data-out line to both slaves)
    MISO -> D12   (shared data-in line from both slaves)
    SS (Slave 1, sensors) -> D10
    SS (Slave 2, display) -> D9
*/

#define SCK_PIN 13
#define MOSI_PIN 11
#define MISO_PIN 12
#define SS_SLAVE1 10
#define SS_SLAVE2 9

int displayMode = 3;
bool toggle = true;

byte spiTransfer(byte data) {
  byte received = 0;
  for (int i = 7; i >= 0; i--) {
    digitalWrite(MOSI_PIN, bitRead(data, i));
    delayMicroseconds(50);
    digitalWrite(SCK_PIN, HIGH);
    delayMicroseconds(50);
    if (digitalRead(MISO_PIN) == HIGH) bitSet(received, i);
    digitalWrite(SCK_PIN, LOW);
    delayMicroseconds(40);
  }
  return received;
}

void setup() {
  Serial.begin(9600);
  pinMode(SCK_PIN, OUTPUT); pinMode(MOSI_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT); pinMode(SS_SLAVE1, OUTPUT);
  pinMode(SS_SLAVE2, OUTPUT);
  digitalWrite(SS_SLAVE1, HIGH); digitalWrite(SS_SLAVE2, HIGH);

  Serial.println("========================================");
  Serial.println("   ENVIRONMENTAL MONITORING SYSTEM      ");
  Serial.println("========================================");
  Serial.println("Commands:");
  Serial.println("[1] -> MONITOR TEMPERATURE ONLY");
  Serial.println("[2] -> MONITOR GAS LEVELS ONLY");
  Serial.println("[3] -> FULL SYSTEM MONITORING (BOTH)");
  Serial.println("========================================");
}

void loop() {
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == '1' || input == '2' || input == '3') {
      displayMode = input - '0';
      digitalWrite(SS_SLAVE2, LOW);
      spiTransfer(200); spiTransfer(displayMode);
      digitalWrite(SS_SLAVE2, HIGH);

      Serial.print(">> SYSTEM NOTIFICATION: MODE ");
      Serial.print(displayMode); Serial.println(" ACTIVATED");
    }
  }

  // Fetch data from Slave 1 (sensors)
  digitalWrite(SS_SLAVE1, LOW);
  delayMicroseconds(500);
  spiTransfer(1); delay(10); byte t = spiTransfer(0);
  delay(10);
  spiTransfer(2); delay(10); byte s = spiTransfer(0);
  digitalWrite(SS_SLAVE1, HIGH);

  // Forward data to Slave 2 (display), alternating between temp and gas
  digitalWrite(SS_SLAVE2, LOW);
  delay(10);
  if (toggle) {
    spiTransfer(101); delay(10); spiTransfer(t);
  } else {
    spiTransfer(102); delay(10); spiTransfer(s);
  }
  digitalWrite(SS_SLAVE2, HIGH);

  toggle = !toggle;

  Serial.print("[STATUS] Temp: "); Serial.print(t);
  Serial.print(" C | Gas: "); Serial.print(s); Serial.println(" %");
  delay(250);
}
