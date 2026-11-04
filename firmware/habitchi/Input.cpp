#include "Input.h"
#include "Pins.h"
#include "ClockConfig.h"
#include "GameClock.h"
#include "Display.h"
#include "Sound.h"
#include "Events.h"

namespace {
struct Button {
  uint8_t pin;
  bool lastReading;
  bool pressed;
  bool longDone;
  unsigned long changedMs;
  unsigned long pressedMs;
};

constexpr uint8_t BUTTON_COUNT = 3;
constexpr uint8_t INDEX_A = 0;
constexpr uint8_t INDEX_B = 1;
constexpr uint8_t INDEX_C = 2;

Button buttons[BUTTON_COUNT] = {
  {Pins::BUTTON_A, false, false, false, 0, 0},
  {Pins::BUTTON_B, false, false, false, 0, 0},
  {Pins::BUTTON_C, false, false, false, 0, 0}
};

void onShortPress(uint8_t index) {
  if (index == INDEX_A) {
    Display::nextPage();
  } else if (index == INDEX_B && GameClock::demoMode()) {
    GameClock::addMinutes(ClockCfg::DEMO_STEP_BIG_MIN);
  } else if (index == INDEX_C && GameClock::demoMode()) {
    GameClock::addMinutes(ClockCfg::DEMO_STEP_SMALL_MIN);
  } else {
    return;
  }
  Sound::play(SoundId::Click);
}

void onLongPress(uint8_t index) {
  if (index == INDEX_A) {
    GameClock::setDemoMode(!GameClock::demoMode());
  } else if (index == INDEX_B) {
    eventPush(Ev::ExerciseButton);
  } else {
    eventPush(Ev::PhoneModeButton);
  }
  Sound::play(SoundId::Click);
}

void checkButton(uint8_t index) {
  Button& b = buttons[index];
  unsigned long nowMs = millis();
  bool reading = digitalRead(b.pin) == LOW;

  if (reading != b.lastReading) {
    b.lastReading = reading;
    b.changedMs = nowMs;
  }

  if (reading != b.pressed && nowMs - b.changedMs >= ClockCfg::DEBOUNCE_MS) {
    b.pressed = reading;
    if (b.pressed) {
      b.pressedMs = nowMs;
      b.longDone = false;
    } else if (!b.longDone) {
      onShortPress(index);
    }
  }

  if (b.pressed && !b.longDone && nowMs - b.pressedMs >= ClockCfg::LONG_PRESS_MS) {
    b.longDone = true;
    onLongPress(index);
  }
}
}

namespace Input {
void setup() {
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
}

void update() {
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    checkButton(i);
  }
}

bool timeSetActive() {
  return GameClock::demoMode();
}
}
