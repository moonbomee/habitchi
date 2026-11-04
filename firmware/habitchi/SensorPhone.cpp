#include "SensorPhone.h"
#include "Pins.h"
#include "Facts.h"
#include "Events.h"
#include "Sound.h"
#include "SensorsConfig.h"
#include "SensorAdc.h"

namespace SensorPhone {
namespace {
unsigned long lastFsrReadMs = 0;
int pressure = 0;
bool pressed = false;
unsigned long pressedChangedMs = 0;
unsigned long lastHolderMoveMs = 0;

unsigned long lastBusyMs = 0;
uint8_t spikeCount = 0;
unsigned long windowStartMs = 0;
unsigned long lastSpikeMs = 0;
unsigned long lastBuzzMs = 0;
bool buzzedBefore = false;

int readPressure() {
  int raw = SensorAdc::readSettled(Pins::FSR);
  return SensorCfg::FSR_HIGHER_MEANS_PRESSED ? raw : 1023 - raw;
}

void clearSpikes() {
  spikeCount = 0;
}

void updatePressed(unsigned long now) {
  bool next = pressed;
  if (!pressed && pressure > SensorCfg::FSR_ON_ABOVE) next = true;
  if (pressed && pressure < SensorCfg::FSR_OFF_BELOW) next = false;
  if (next != pressed) {
    pressed = next;
    pressedChangedMs = now;
    lastHolderMoveMs = now;
  }
}

void updatePhonePresent(unsigned long now) {
  if (pressed == facts.phonePresent) return;
  if (now - pressedChangedMs < SensorCfg::FSR_DEBOUNCE_MS) return;
  facts.phonePresent = pressed;
  lastHolderMoveMs = now;
  clearSpikes();
  eventPush(pressed ? Ev::PhonePlaced : Ev::PhoneLifted);
}

void updateFsr(unsigned long now) {
  if (now - lastFsrReadMs < SensorCfg::FSR_INTERVAL_MS) return;
  lastFsrReadMs = now;
  pressure += (readPressure() - pressure) / SensorCfg::FSR_SMOOTH_DIV;
  updatePressed(now);
  updatePhonePresent(now);
}

bool piezoShouldIgnore(unsigned long now) {
  if (Sound::busy()) {
    lastBusyMs = now;
    return true;
  }
  if (now - lastBusyMs < SensorCfg::PIEZO_AFTER_BUSY_MS) return true;
  if (!facts.phonePresent) return true;
  if (pressed != facts.phonePresent) return true;
  return now - lastHolderMoveMs < SensorCfg::PIEZO_IGNORE_AFTER_FSR_MS;
}

bool burstHasSpike() {
  if (SensorAdc::readSettled(Pins::PIEZO_SENSE) >= SensorCfg::PIEZO_SPIKE_RAW) return true;
  for (uint8_t i = 1; i < SensorCfg::PIEZO_BURST_SAMPLES; i++) {
    if (analogRead(Pins::PIEZO_SENSE) >= SensorCfg::PIEZO_SPIKE_RAW) return true;
  }
  return false;
}

void reportBuzz(unsigned long now) {
  clearSpikes();
  if (buzzedBefore && now - lastBuzzMs < SensorCfg::PIEZO_BUZZ_GAP_MS) return;
  eventPush(Ev::PhoneBuzz);
  lastBuzzMs = now;
  buzzedBefore = true;
}

void updatePiezo(unsigned long now) {
  if (piezoShouldIgnore(now)) {
    clearSpikes();
    return;
  }
  if (spikeCount > 0 && now - windowStartMs > SensorCfg::PIEZO_WINDOW_MS) clearSpikes();
  if (spikeCount > 0 && now - lastSpikeMs < SensorCfg::PIEZO_SPIKE_GAP_MS) return;
  if (!burstHasSpike()) return;

  if (spikeCount == 0) windowStartMs = now;
  spikeCount++;
  lastSpikeMs = now;
  if (spikeCount >= SensorCfg::PIEZO_MIN_SPIKES) reportBuzz(now);
}
}

void setup() {
  facts.phonePresent = false;
}

void update() {
  unsigned long now = millis();
  updateFsr(now);
  updatePiezo(now);
}
}
