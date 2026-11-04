#pragma once
#include <Arduino.h>

namespace ClockCfg {
constexpr unsigned long RTC_READ_MS = 500;
constexpr uint32_t SYNC_TOLERANCE_S = 2;
constexpr uint32_t FALLBACK_START_UNIX = 1767268800UL;

constexpr unsigned long DEBOUNCE_MS = 30;
constexpr unsigned long LONG_PRESS_MS = 800;
constexpr int16_t DEMO_STEP_BIG_MIN = 60;
constexpr int16_t DEMO_STEP_SMALL_MIN = 10;
}
