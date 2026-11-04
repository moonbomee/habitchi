#pragma once
#include <Arduino.h>

namespace PetCoding {
void reset();
void update();
void minuteTick();
void markAllNighter();
void onPhoneModeButton();
void onPhoneBuzz();
void onPhoneLifted();
void onPhonePlaced();
void onDrank(int16_t dropPercent);
bool stretchDue();
bool thirsty();
bool allNighter();
bool distracted();
bool studying();
bool phoneMode();
}
