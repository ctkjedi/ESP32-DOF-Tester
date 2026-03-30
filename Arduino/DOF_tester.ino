/*
 * ESP32-C3 — DOF TeensyStripController + WS2812B  (v2)
 * =====================================================
 *
 * ARDUINO IDE SETTINGS
 * --------------------
 *   Board              : ESP32C3 Dev Module
 *   USB CDC on Boot    : Enabled
 *   CPU Frequency      : 160MHz
 *   Flash Mode         : DIO

 * CABINET XML
 * -----------
 *   <ComPortDtrEnable>true</ComPortDtrEnable>
 *   <ComPortOpenWaitMs>2000</ComPortOpenWaitMs>
 *   <ComPortHandshakeStartWaitMs>200</ComPortHandshakeStartWaitMs>
 *   <ComPortHandshakeEndWaitMs>200</ComPortHandshakeEndWaitMs>
 *   <ComPortTimeOutMs>200</ComPortTimeOutMs>
 */

#include <FastLED.h>

#define LED_PIN      8
#define NUM_LEDS     1
#define LED_TYPE     WS2812B
#define COLOR_ORDER  GRB
#define BRIGHTNESS   50

CRGB leds[NUM_LEDS];

static const uint16_t MAX_LEDS_PER_CHAN = 1100;
static const uint32_t BYTE_TIMEOUT_MS  = 500;
static uint16_t ledsPerChannel = 0;

// =============================================================================
// DOF PROTOCOL
// =============================================================================

static size_t dofRead(uint8_t* buf, size_t n, uint32_t ms = BYTE_TIMEOUT_MS) {
  uint32_t deadline = millis() + ms;
  size_t got = 0;
  while (got < n && millis() < deadline) {
    if (Serial.available()) buf[got++] = (uint8_t)Serial.read();
  }
  return got;
}

static void dofAck() { Serial.write('A'); }

static void handleM() {
  uint8_t r[3] = {
    (uint8_t)(MAX_LEDS_PER_CHAN >> 8),
    (uint8_t)(MAX_LEDS_PER_CHAN & 0xFF),
    'A'
  };
  Serial.write(r, 3);
}

static void handleL() {
  uint8_t b[2];
  if (dofRead(b, 2) == 2) {
    ledsPerChannel = ((uint16_t)b[0] << 8) | b[1];
    dofAck();
  }
}

static void handleC() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  dofAck();
}

static void handleR() {
  uint8_t hdr[4];
  if (dofRead(hdr, 4) != 4) return;

  uint16_t targetPos = ((uint16_t)hdr[0] << 8) | hdr[1];
  uint16_t nLeds     = ((uint16_t)hdr[2] << 8) | hdr[3];
  uint32_t nBytes    = (uint32_t)nLeds * 3;
  uint32_t deadline  = millis() + BYTE_TIMEOUT_MS + (nBytes / 10);
  uint32_t received  = 0;

  while (received < nBytes && millis() < deadline) {
    if (!Serial.available()) continue;
    uint8_t  b      = Serial.read();
    uint32_t ledIdx = received / 3;
    uint8_t  ch     = received % 3;
    uint16_t dest   = targetPos + ledIdx;
    received++;
    if (dest < NUM_LEDS) {
      switch (ch) {
        case 0: leds[dest].r = b; break;
        case 1: leds[dest].g = b; break;
        case 2: leds[dest].b = b; break;
      }
    }
  }
  if (received == nBytes) dofAck();
}

static void handleO() {
  FastLED.show();
  dofAck();
}

// =============================================================================
// SETUP
// =============================================================================

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
         .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  Serial.begin(115200);

  // Flush ROM bootloader banner and any other garbage from the RX buffer.
  // The ROM prints ~31 bytes. We flush for 150ms to catch it all.
  uint32_t flushEnd = millis() + 150;
  while (millis() < flushEnd) {
    if (Serial.available()) Serial.read();
  }

  // Handshake window: watch for 'M' for up to 3 seconds.
  // DOF sends 'M' after ComPortOpenWaitMs (2000ms). 
  // Once 'M' is answered and 'L' is received, break out to normal loop.
  uint32_t windowEnd  = millis() + 3000;
  bool     handshaked = false;

  while (millis() < windowEnd && !handshaked) {
    if (!Serial.available()) continue;

    if ((char)Serial.read() == 'M') {
      handleM();

      // Wait for 'L' confirmation (DOF sends it right after getting M reply)
      uint32_t lEnd = millis() + 1000;
      while (millis() < lEnd) {
        if (Serial.available() && (char)Serial.read() == 'L') {
          handleL();
          handshaked = true;
          break;
        }
      }
    }
  }

  // Brief green flash to confirm LED conenctivity
  leds[0] = CRGB::Green;
  FastLED.show();
  delay(200);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

// =============================================================================
// LOOP
// =============================================================================

void loop() {
  if (!Serial.available()) return;

  switch ((char)Serial.read()) {
    case 'M': handleM(); break;
    case 'L': handleL(); break;
    case 'C': handleC(); break;
    case 'R': handleR(); break;
    case 'O': handleO(); break;
    default:  break;
  }
}
