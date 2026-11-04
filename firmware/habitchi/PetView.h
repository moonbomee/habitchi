#pragma once
#include <Arduino.h>

enum class Stage : uint8_t { Egg, Baby, Child, Healthy, Weak, Obese, Dead };

enum class Mood : uint8_t {
  Normal,
  Happy,
  Hungry,
  Sleepy,
  Sleeping,
  SoreEyes,
  Stretch,
  Stomachache,
  Indigestion,
  Full,
  Thirsty,
  Cold,
  Sick,
  Studying,
  Exercising,
  Dead
};

enum class Notice : uint8_t {
  None,
  GoodMorning,
  ColdNight,
  NoisyNight,
  ScreenBeforeSleep,
  StretchNow,
  DrinkWater,
  PhoneDown,
  ExerciseResult,
  MeasureAgain,
  NewGeneration
};

struct PetView {
  Stage stage;
  Mood mood;
  Notice notice;
  uint8_t fullness;
  uint8_t rest;
  uint8_t stamina;
  uint16_t exp;
  uint8_t generation;
  uint8_t sleepScore;
  bool phoneMode;
  int16_t noticeValue;
};
