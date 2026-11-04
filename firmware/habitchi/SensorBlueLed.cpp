#include "SensorBlueLed.h"
#include "Pins.h"
#include "Facts.h"
#include "SensorsConfig.h"

namespace SensorBlueLed {
namespace {
bool measuring = false;
unsigned long lastStartMs = 0;
unsigned long startUs = 0;

void rest() {
  pinMode(Pins::BLUE_LED_CATHODE, OUTPUT);
  digitalWrite(Pins::BLUE_LED_CATHODE, LOW);
}

void startMeasure(unsigned long now) {
  digitalWrite(Pins::BLUE_LED_ANODE, LOW);
  pinMode(Pins::BLUE_LED_CATHODE, OUTPUT);
  digitalWrite(Pins::BLUE_LED_CATHODE, HIGH);
  pinMode(Pins::BLUE_LED_CATHODE, INPUT);
  startUs = micros();
  lastStartMs = now;
  measuring = true;
}

void updateGlow(unsigned long dischargeUs) {
  if (!facts.screenGlow && dischargeUs < SensorCfg::BLUE_GLOW_ON_US) {
    facts.screenGlow = true;
  } else if (facts.screenGlow && dischargeUs > SensorCfg::BLUE_GLOW_OFF_US) {
    facts.screenGlow = false;
  }
}

void finishMeasure(unsigned long dischargeUs) {
  rest();
  measuring = false;
  updateGlow(dischargeUs);
}

void checkMeasure() {
  unsigned long elapsedUs = micros() - startUs;
  if (digitalRead(Pins::BLUE_LED_CATHODE) == LOW) {
    finishMeasure(elapsedUs);
  } else if (elapsedUs >= SensorCfg::BLUE_TIMEOUT_US) {
    finishMeasure(SensorCfg::BLUE_TIMEOUT_US);
  }
}
}

void setup() {
  pinMode(Pins::BLUE_LED_ANODE, OUTPUT);
  digitalWrite(Pins::BLUE_LED_ANODE, LOW);
  rest();
  facts.screenGlow = false;
  lastStartMs = millis();
}

void update() {
  if (measuring) {
    checkMeasure();
    return;
  }
  unsigned long now = millis();
  if (now - lastStartMs >= SensorCfg::BLUE_INTERVAL_MS) {
    startMeasure(now);
  }
}
}
