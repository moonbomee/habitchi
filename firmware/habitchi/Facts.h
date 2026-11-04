#pragma once
#include <Arduino.h>

struct Facts {
  uint16_t lightRaw;
  bool roomDark;
  uint8_t noiseLevel;
  float roomTempC;
  bool tempValid;
  bool screenGlow;
  bool phonePresent;
  bool bottlePresent;
  uint8_t waterPercent;
  bool waterValid;
  bool fingerOn;
  uint16_t heartBpm;
  bool heartValid;
  bool pcConnected;
  unsigned long lastTypingMs;
  bool typingSeen;
};

extern Facts facts;
