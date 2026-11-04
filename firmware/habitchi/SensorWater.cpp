#include "SensorWater.h"
#include <CapacitiveSensor.h>
#include "Pins.h"
#include "Facts.h"
#include "Events.h"
#include "SensorsConfig.h"

namespace SensorWater {
namespace {
CapacitiveSensor foil(Pins::WATER_SEND, Pins::WATER_RECEIVE);
unsigned long lastReadMs = 0;
long recent[SensorCfg::WATER_STABLE_COUNT];
uint8_t recentCount = 0;
uint8_t nextSlot = 0;
bool hasLevel = false;
bool waitingForReturn = false;

void clearRecent() {
  recentCount = 0;
  nextSlot = 0;
}

void remember(long raw) {
  recent[nextSlot] = raw;
  nextSlot = (nextSlot + 1) % SensorCfg::WATER_STABLE_COUNT;
  if (recentCount < SensorCfg::WATER_STABLE_COUNT) recentCount++;
}

bool isStable() {
  if (recentCount < SensorCfg::WATER_STABLE_COUNT) return false;
  long low = recent[0];
  long high = recent[0];
  for (uint8_t i = 1; i < recentCount; i++) {
    if (recent[i] < low) low = recent[i];
    if (recent[i] > high) high = recent[i];
  }
  return high - low <= SensorCfg::WATER_STABLE_SPREAD;
}

long recentAverage() {
  long sum = 0;
  for (uint8_t i = 0; i < recentCount; i++) sum += recent[i];
  return sum / recentCount;
}

uint8_t percentFromRaw(long raw) {
  long percent = (raw - SensorCfg::WATER_EMPTY_RAW) * 100 /
                 (SensorCfg::WATER_FULL_RAW - SensorCfg::WATER_EMPTY_RAW);
  return (uint8_t)constrain(percent, 0L, 100L);
}

void updatePresence(long raw) {
  bool wasPresent = facts.bottlePresent;
  if (!wasPresent && raw > SensorCfg::WATER_PRESENT_ABOVE) facts.bottlePresent = true;
  if (wasPresent && raw < SensorCfg::WATER_ABSENT_BELOW) facts.bottlePresent = false;
  if (wasPresent && !facts.bottlePresent && hasLevel) waitingForReturn = true;
}

void checkDrank(uint8_t percent) {
  if (!waitingForReturn) return;
  waitingForReturn = false;
  if (facts.waterPercent < percent) return;
  uint8_t drop = facts.waterPercent - percent;
  if (drop >= SensorCfg::WATER_DRANK_MIN_DROP) eventPush(Ev::Drank, drop);
}

void acceptLevel(uint8_t percent) {
  checkDrank(percent);
  facts.waterPercent = percent;
  facts.waterValid = true;
  hasLevel = true;
}

bool isTouched(long raw) {
  return raw < 0 || raw >= SensorCfg::WATER_TOUCH_RAW;
}
}

void setup() {
  foil.set_CS_AutocaL_Millis(0xFFFFFFFF);
  foil.set_CS_Timeout_Millis(SensorCfg::WATER_TIMEOUT_MS);
  facts.bottlePresent = false;
  facts.waterPercent = 0;
  facts.waterValid = false;
}

void update() {
  unsigned long now = millis();
  if (now - lastReadMs < SensorCfg::WATER_INTERVAL_MS) return;
  lastReadMs = now;

  long raw = foil.capacitiveSensorRaw(SensorCfg::WATER_SAMPLES);
  if (isTouched(raw)) {
    facts.waterValid = false;
    clearRecent();
    return;
  }
  updatePresence(raw);
  if (!facts.bottlePresent) {
    facts.waterValid = false;
    clearRecent();
    return;
  }
  remember(raw);
  if (!isStable()) {
    facts.waterValid = false;
    return;
  }
  acceptLevel(percentFromRaw(recentAverage()));
}
}
