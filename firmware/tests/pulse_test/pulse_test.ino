const uint8_t PULSE_PIN = A6;
unsigned long lastPrintMs = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  if (millis() - lastPrintMs < 10) return;
  lastPrintMs = millis();
  Serial.print("pulse:");
  Serial.println(analogRead(PULSE_PIN));
}
