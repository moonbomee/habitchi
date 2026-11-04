#include "Sensors.h"
#include "SensorLight.h"
#include "SensorNoise.h"
#include "SensorTemp.h"
#include "SensorBlueLed.h"
#include "SensorPhone.h"
#include "SensorWater.h"
#include "SensorPulse.h"

namespace Sensors {
void setup() {
  SensorLight::setup();
  SensorNoise::setup();
  SensorTemp::setup();
  SensorBlueLed::setup();
  SensorPhone::setup();
  SensorWater::setup();
  SensorPulse::setup();
}

void update() {
  SensorPulse::update();
  SensorNoise::update();
  SensorLight::update();
  SensorTemp::update();
  SensorBlueLed::update();
  SensorPhone::update();
  SensorWater::update();
  SensorPulse::update();
}
}
