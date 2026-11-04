#include "GameClock.h"
#include <RTClib.h>
#include "Timing.h"
#include "ClockConfig.h"

namespace {
RTC_DS3231 rtc;
bool rtcFound = false;
bool rtcTimeGood = false;

uint32_t fallbackStartUnix = ClockCfg::FALLBACK_START_UNIX;
unsigned long fallbackStartMs = 0;

uint32_t realUnix = ClockCfg::FALLBACK_START_UNIX;
unsigned long lastReadMs = 0;

bool demoOn = false;
uint32_t demoStartUnix = 0;
unsigned long demoStartMs = 0;
int32_t demoOffsetSeconds = 0;

bool usingRtc() {
  return rtcFound && rtcTimeGood;
}

void readRealTime() {
  if (usingRtc()) {
    realUnix = rtc.now().unixtime();
  } else {
    realUnix = fallbackStartUnix + (millis() - fallbackStartMs) / 1000UL;
  }
  lastReadMs = millis();
}

void setFallback(uint32_t unixTime) {
  fallbackStartUnix = unixTime;
  fallbackStartMs = millis();
}
}

namespace GameClock {
void setup() {
  rtcFound = rtc.begin();
  rtcTimeGood = rtcFound && !rtc.lostPower();
  setFallback(ClockCfg::FALLBACK_START_UNIX);
  readRealTime();
}

void update() {
  if (millis() - lastReadMs >= ClockCfg::RTC_READ_MS) {
    readRealTime();
  }
}

bool demoMode() {
  return demoOn;
}

void setDemoMode(bool on) {
  if (on == demoOn) return;
  if (on) {
    readRealTime();
    demoStartUnix = realUnix;
    demoStartMs = millis();
    demoOffsetSeconds = 0;
  }
  demoOn = on;
}

uint32_t now() {
  if (!demoOn) return realUnix;
  unsigned long elapsedMs = millis() - demoStartMs;
  uint32_t fastSeconds = elapsedMs / 1000UL * Cfg::DEMO_SPEED + elapsedMs % 1000UL * Cfg::DEMO_SPEED / 1000UL;
  return demoStartUnix + fastSeconds + demoOffsetSeconds;
}

uint8_t hour() {
  return (now() / 3600UL) % 24;
}

uint8_t minute() {
  return (now() / 60UL) % 60;
}

void addMinutes(int16_t minutes) {
  if (!demoOn) return;
  demoOffsetSeconds += (int32_t)minutes * 60L;
}

void syncFromPc(uint32_t unixTime) {
  if (demoOn) return;
  readRealTime();
  uint32_t difference = unixTime > realUnix ? unixTime - realUnix : realUnix - unixTime;
  if (usingRtc() && difference <= ClockCfg::SYNC_TOLERANCE_S) return;
  if (rtcFound) {
    rtc.adjust(DateTime(unixTime));
    rtcTimeGood = true;
  }
  setFallback(unixTime);
  readRealTime();
}
}
