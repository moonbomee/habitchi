#include "SensorTemp.h"
#include "Pins.h"
#include "Facts.h"
#include "SensorsConfig.h"
#include "SensorAdc.h"

namespace SensorTemp {
namespace {
unsigned long lastReadMs = 0;
float smoothedC = 0.0f;
bool hasValue = false;

float readCelsius() {
  float raw = SensorAdc::readAverage(Pins::LM35, SensorCfg::TEMP_SAMPLES);
  float millivolts = raw * SensorCfg::TEMP_ADC_REF_MV / 1024.0f;
  return millivolts / 10.0f;
}

bool isPlausible(float celsius) {
  return celsius >= SensorCfg::TEMP_VALID_MIN_C && celsius <= SensorCfg::TEMP_VALID_MAX_C;
}
}

void setup() {
  facts.roomTempC = 0.0f;
  facts.tempValid = false;
}

void update() {
  unsigned long now = millis();
  if (now - lastReadMs < SensorCfg::TEMP_INTERVAL_MS) return;
  lastReadMs = now;

  float celsius = readCelsius();
  if (!isPlausible(celsius)) {
    hasValue = false;
    facts.tempValid = false;
    return;
  }
  if (!hasValue) {
    smoothedC = celsius;
    hasValue = true;
  } else {
    smoothedC += (celsius - smoothedC) * SensorCfg::TEMP_SMOOTHING;
  }
  facts.roomTempC = smoothedC;
  facts.tempValid = true;
}
}
