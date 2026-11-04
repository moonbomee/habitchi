#pragma once
#include <Arduino.h>

namespace GameClock {
void setup();
void update();
bool demoMode();
void setDemoMode(bool on);
uint32_t now();
uint8_t hour();
uint8_t minute();
void addMinutes(int16_t minutes);
void syncFromPc(uint32_t unixTime);
}
