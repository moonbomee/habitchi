#pragma once
#include <Arduino.h>

enum class SoundId : uint8_t {
  Click,
  Alarm,
  Warning,
  Eat,
  Happy,
  Sneeze,
  Pain,
  Hatch,
  Evolve,
  Death
};

namespace Sound {
void setup();
void update();
void play(SoundId id);
void stop();
bool busy();
}
