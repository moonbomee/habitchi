#include <CapacitiveSensor.h>

CapacitiveSensor foil(32, 33);
unsigned long lastReadMs = 0;

void setup() {
  Serial.begin(115200);
  foil.set_CS_AutocaL_Millis(0xFFFFFFFF);
  foil.set_CS_Timeout_Millis(10);
}

void loop() {
  if (millis() - lastReadMs < 500) return;
  lastReadMs = millis();
  Serial.print("water_raw:");
  Serial.println(foil.capacitiveSensorRaw(10));
}
