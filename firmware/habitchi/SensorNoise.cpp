#include "SensorNoise.h"
#include "Pins.h"
#include "Facts.h"
#include "Events.h"
#include "Sound.h"
#include "SensorsConfig.h"

namespace SensorNoise {
namespace {
unsigned long windowStartMs = 0;
int windowMin = 1023;
int windowMax = 0;
unsigned long lastBusyMs = 0;
unsigned long lastLoudMs = 0;
bool loudBefore = false;

void resetWindow(unsigned long now) {
  windowStartMs = now;
  windowMin = 1023;
  windowMax = 0;
}

void sampleBurst() {
  for (uint8_t i = 0; i < SensorCfg::SOUND_BURST_SAMPLES; i++) {
    int value = analogRead(Pins::SOUND);
    if (value < windowMin) windowMin = value;
    if (value > windowMax) windowMax = value;
  }
}

uint8_t levelFromPeakToPeak(int peakToPeak) {
  long level = (long)(peakToPeak - SensorCfg::SOUND_QUIET_P2P) * 100 /
               (SensorCfg::SOUND_MAX_P2P - SensorCfg::SOUND_QUIET_P2P);
  return (uint8_t)constrain(level, 0L, 100L);
}

void updateNoiseLevel(uint8_t level) {
  if (level >= facts.noiseLevel) {
    facts.noiseLevel = level;
  } else if (facts.noiseLevel - level > SensorCfg::SOUND_RELEASE_PER_WINDOW) {
    facts.noiseLevel -= SensorCfg::SOUND_RELEASE_PER_WINDOW;
  } else {
    facts.noiseLevel = level;
  }
}

void checkLoud(uint8_t level, unsigned long now) {
  if (level < SensorCfg::SOUND_LOUD_LEVEL) return;
  if (loudBefore && now - lastLoudMs < SensorCfg::SOUND_EVENT_GAP_MS) return;
  eventPush(Ev::LoudNoise, level);
  lastLoudMs = now;
  loudBefore = true;
}

void finishWindow(unsigned long now) {
  uint8_t level = levelFromPeakToPeak(windowMax - windowMin);
  updateNoiseLevel(level);
  checkLoud(level, now);
}

bool petSoundNearby(unsigned long now) {
  if (Sound::busy()) {
    lastBusyMs = now;
    return true;
  }
  return now - lastBusyMs < SensorCfg::SOUND_AFTER_BUSY_MS;
}
}

void setup() {
  facts.noiseLevel = 0;
  resetWindow(millis());
}

void update() {
  unsigned long now = millis();
  if (petSoundNearby(now)) {
    resetWindow(now);
    return;
  }
  sampleBurst();
  if (now - windowStartMs >= SensorCfg::SOUND_WINDOW_MS) {
    finishWindow(now);
    resetWindow(now);
  }
}
}
