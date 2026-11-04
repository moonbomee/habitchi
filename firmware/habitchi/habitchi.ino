#include "Timing.h"
#include "GameClock.h"
#include "Input.h"
#include "Sensors.h"
#include "PcLink.h"
#include "Pet.h"
#include "Sound.h"
#include "Display.h"

void setup() {
  Serial.begin(Cfg::SERIAL_BAUD);
  GameClock::setup();
  Input::setup();
  Sensors::setup();
  Sound::setup();
  Display::setup();
  PcLink::setup();
  Pet::setup();
}

void loop() {
  GameClock::update();
  Input::update();
  Sensors::update();
  PcLink::update();
  Pet::update();
  Sound::update();
  Display::update();
}
