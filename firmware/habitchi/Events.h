#pragma once
#include <Arduino.h>

enum class Ev : uint8_t {
  CommitGood,
  CommitJunk,
  CommitBroken,
  LoudNoise,
  PhoneBuzz,
  PhoneLifted,
  PhonePlaced,
  Drank,
  ExerciseButton,
  PhoneModeButton,
  DemoStage,
  DemoSick,
  DemoDie,
  DemoEvolve
};

struct Event {
  Ev type;
  int16_t value;
};

bool eventPush(Ev type, int16_t value = 0);
bool eventPop(Event& out);
