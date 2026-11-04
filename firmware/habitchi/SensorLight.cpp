#include "SensorLight.h"
#include "Pins.h"
#include "Facts.h"
#include "SensorsConfig.h"
#include "SensorAdc.h"

namespace SensorLight {
namespace {
unsigned long lastReadMs = 0;
int smoothed = 0;
bool started = false;

void updateDark() {
  if (!facts.roomDark && smoothed < SensorCfg::LIGHT_DARK_BELOW) {
    facts.roomDark = true;
  } else if (facts.roomDark && smoothed > SensorCfg::LIGHT_BRIGHT_ABOVE) {
    facts.roomDark = false;
  }
}
}

void setup() {
  facts.lightRaw = 0;
  facts.roomDark = false;
}

void update() {
  unsigned long now = millis();
  if (now - lastReadMs < SensorCfg::LIGHT_INTERVAL_MS) return;
  lastReadMs = now;

  int raw = SensorAdc::readSettled(Pins::LIGHT);
  if (!started) {
    smoothed = raw;
    started = true;
  } else {
    smoothed += (raw - smoothed) / SensorCfg::LIGHT_SMOOTH_DIV;
  }
  facts.lightRaw = smoothed;
  updateDark();
}
}
