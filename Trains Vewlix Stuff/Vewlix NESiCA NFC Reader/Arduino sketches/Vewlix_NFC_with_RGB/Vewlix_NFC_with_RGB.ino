// --------------------------
// NFC Console Loading System
// Version 1.0
// Designed by: TheTrain
// --------------------------
//
// Attribution
// 
// The following text must be included in any distribution of derivatives of this code. All links must also be included.
//
// Based on the [NFC Console Loading System](https://github.com/OpenStickCommunity/Hardware) designed by TheTrain.
// 
// Copyright 2024 [TheTrain](https://x.com/TheTrain24)(https://github.com/OpenStickCommunity)
//
// [Licensed under CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)
// 
// Changes from the original design:
//  - list any changes you make here
//
// Original idea from:
//    Tapto - An open source NFC loading system for MiSTer (https://github.com/TapToCommunity)
//
// Special thanks to:
//    [Lucipher](https://github.com/arntsonl)
//    [NickGuyver](https://github.com/NickGuyver)
//    [Wizzomafizzo](https://github.com/wizzomafizzo)

#include <Adafruit_PN532.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

// Define the SDA and SCL pins.
// These must be kept on an I2C pair and in order.
// Reference your board pinout diagram to see which pins are connected to which I2C pairs.
// Please note that these are GPIO pins, not the numbered pins on some boards.
// Please note that GPIO pins start at 0 (GPIO0).


#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

constexpr int led_pin = 0;
constexpr int coin_pin = 6;
constexpr int back_pin = 7;
constexpr int pin_array[] = {coin_pin, back_pin};

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(20, led_pin, NEO_GRB + NEO_KHZ800);

const uint8_t allowedUIDs[][7] = {
  { 0x12, 0x7F, 0x16, 0x3F },
  { 0xC9, 0x3A, 0xD6, 0x05 },
};

void setup(void) {
  Serial.begin(115200);
  for (int pin : pin_array) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  strip.begin();
  strip.show();

  delay(1000);
  Serial.println("Serial connection has been at 115200 baud.");
  nfc.begin();

  Serial.println("NFC board has been found and started.");
  uint32_t nfc_version = nfc.getFirmwareVersion();
  if (!nfc_version) {
    Serial.print("Didn't find PN53x board");
    while (1);
  }

  nfc.SAMConfig();
  Serial.println("Waiting for an NFC card to be tapped...");

  for (int i = 0; i <= 255; i++) {
    setAllLEDs(strip.Color(i, i, i));
    delay(10);
  }
}

void loop(void) {
  bool isAllowedUID = false;
  uint8_t uid[] = {0,0,0,0,0,0,0};
  uint8_t uidLength = 0;
  uint8_t nfc_success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (nfc_success) {
    for (int i = 0; i < sizeof(allowedUIDs)/sizeof(allowedUIDs[0]); i++) {
      if (memcmp(uid, allowedUIDs[i], uidLength) == 0) {
        isAllowedUID = true;
        break;
      }
    }

    Serial.println("--------------------------");
    Serial.print("UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(" 0x"); Serial.print(uid[i], HEX);
    }
    Serial.println("");

    if (isAllowedUID) {
      Serial.println("Found an allowed NFC card!");
      chenillardLEDs(strip.Color(0, 255, 0), strip.Color(127, 127, 127), 4); // Vert + traînée de 3

      if (memcmp(uid, allowedUIDs[0], uidLength) == 0) {
        Serial.println("UID 1 detected - Credit Game");
        writeDelay(coin_pin, 100, 1000);
      } 
      else if (memcmp(uid, allowedUIDs[1], uidLength) == 0) {
        Serial.println("UID 2 detected - Back to Menu");
        writeDelay(back_pin, 5000, 2000);
      }
    } else {
      Serial.println("Found an NFC card, but not allowed");
      chenillardLEDs(strip.Color(255, 0, 0), strip.Color(127, 127, 127), 4); // Rouge + traînée
    }
  }

  delay(1000);
  setAllLEDs(strip.Color(255, 255, 255)); // Retour au blanc
}

void writeDelay(int pin, int delay_low, int delay_high) {
  digitalWrite(pin, LOW);
  delay(delay_low);
  digitalWrite(pin, HIGH);
  delay(delay_high);
}

void chenillardLEDs(uint32_t color, uint32_t baseColor, int tailleQueue) {
  int nbPixels = strip.numPixels();

  for (int round = 0; round < 2; round++) {
    for (int i = 0; i < nbPixels; i++) {
      setAllLEDs(baseColor);

      for (int j = 0; j < tailleQueue; j++) {
        int pixel = (i - j + nbPixels) % nbPixels;
        strip.setPixelColor(pixel, color);
      }

      strip.show();
      delay(20);
    }
  }

  for (int blink = 0; blink < 2; blink++) {
    setAllLEDs(color);
    delay(100);
    setAllLEDs(baseColor);
    delay(100);
  }
}

void setAllLEDs(uint32_t color) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}
