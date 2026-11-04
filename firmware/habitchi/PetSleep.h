#pragma once
#include <Arduino.h>

namespace PetSleep {
void setup();
void reset();
void update();
void minuteTick();
void onLoudNoise();
void onLateSnack();
bool sleeping();
}
