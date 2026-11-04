const uint8_t SOUND_PIN = A0;
unsigned long windowStartMs = 0;
int windowMin = 1023;
int windowMax = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int value = analogRead(SOUND_PIN);
  if (value < windowMin) windowMin = value;
  if (value > windowMax) windowMax = value;
  if (millis() - windowStartMs < 50) return;
  Serial.print("min:");
  Serial.print(windowMin);
  Serial.print(" max:");
  Serial.print(windowMax);
  Serial.print(" p2p:");
  Serial.println(windowMax - windowMin);
  windowStartMs = millis();
  windowMin = 1023;
  windowMax = 0;
}
