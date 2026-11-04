#pragma once
#include <Arduino.h>
#include "GameClock.h"

struct Timing {
  unsigned long normalMs;
  unsigned long demoMs;
};

inline unsigned long pick(const Timing& t) {
  return GameClock::demoMode() ? t.demoMs : t.normalMs;
}

namespace Cfg {
constexpr unsigned long SERIAL_BAUD = 115200;
constexpr uint8_t DEMO_SPEED = 60;
constexpr Timing STRETCH_AFTER{50UL * 60UL * 1000UL, 30UL * 1000UL};
constexpr Timing PHONE_GRACE{5UL * 60UL * 1000UL, 5UL * 1000UL};
constexpr Timing BREAK_AFTER{5UL * 60UL * 1000UL, 10UL * 1000UL};
constexpr unsigned long TYPING_ACTIVE_MS = 3000;
constexpr unsigned long PC_TIMEOUT_MS = 30000;
}
