#pragma once
#include <Arduino.h>
#include "Events.h"

namespace PetCommit {
void onCommit(Ev type);
void minuteTick();
}
