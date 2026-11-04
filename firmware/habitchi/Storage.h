#pragma once
#include <Arduino.h>

namespace Storage {
bool load();
void save();
void requestSave();
void update();
}
