#pragma once
#include <Arduino.h>

namespace SensorAdc {
inline int readSettled(uint8_t pin) {
  analogRead(pin);
  return analogRead(pin);
}

inline float readAverage(uint8_t pin, uint8_t count) {
  long sum = readSettled(pin);
  for (uint8_t i = 1; i < count; i++) {
    sum += analogRead(pin);
  }
  return (float)sum / count;
}
}
