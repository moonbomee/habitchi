#include "Events.h"

namespace {
constexpr uint8_t CAPACITY = 16;
Event queueData[CAPACITY];
uint8_t head = 0;
uint8_t count = 0;
}

bool eventPush(Ev type, int16_t value) {
  if (count == CAPACITY) return false;
  uint8_t tail = (head + count) % CAPACITY;
  queueData[tail] = {type, value};
  count++;
  return true;
}

bool eventPop(Event& out) {
  if (count == 0) return false;
  out = queueData[head];
  head = (head + 1) % CAPACITY;
  count--;
  return true;
}
