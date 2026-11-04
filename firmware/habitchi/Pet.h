#pragma once
#include <Arduino.h>
#include "PetView.h"

namespace Pet {
void setup();
void update();
const PetView& view();
}
