#pragma once
#include <Arduino.h>

namespace PcLink {
void setup();
void update();
void log(const char* kind, long value);
}
