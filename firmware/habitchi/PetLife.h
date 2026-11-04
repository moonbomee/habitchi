#pragma once
#include <Arduino.h>
#include "PetView.h"

namespace PetLife {
void setup();
void update();
void minuteTick();
void setStage(Stage stage);
void hatchIfEgg();
void evolveAdult();
void becomeSick(uint8_t cause);
void cure();
void cureCold();
void die();
void afterNight();
void demoStage(int16_t value);
void demoSick(int16_t cause);
}
