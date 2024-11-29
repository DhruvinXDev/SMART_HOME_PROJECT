#include <SPI.h>
#include <MFRC522.h>

// RFID pins
#define RST_PIN 22
#define SS_PIN 5

// LED and Buzzer pins
#define LED_PIN 2
#define BUZZER_PIN 4

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// Known RFID tag UID (change to match your card/tag UID)
byte knownUID[] = {0x10, 0x0C, 0xB1, 0x56}; // Replace with your UID

void setup() {
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.println("Place your RFID card near the reader...");
}

void loop() {
  // Check for new card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Print UID of the card
  Serial.print("Card UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) Serial.print(":");
  }
  Serial.println();

  // Compare UID with known UID
  if (compareUID(mfrc522.uid.uidByte, mfrc522.uid.size, knownUID, sizeof(knownUID))) {
    Serial.println("Access Granted!");
    blinkLED(5); // Blink LED for 5 seconds
  } else {
    Serial.println("Access Denied!");
    soundBuzzer(5); // Sound buzzer for 5 seconds
  }

  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD
}

bool compareUID(byte *uid1, byte size1, byte *uid2, byte size2) {
  if (size1 != size2) return false;
  for (byte i = 0; i < size1; i++) {
    if (uid1[i] != uid2[i]) return false;
  }
  return true;
}

void blinkLED(int seconds) {
  for (int i = 0; i < seconds; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
}

void soundBuzzer(int seconds) {
  for (int i = 0; i < seconds; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    delay(500);
  }
}
