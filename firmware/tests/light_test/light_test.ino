const uint8_t LIGHT_PIN = A2;
unsigned long lastPrintMs = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  if (millis() - lastPrintMs < 200) return;
  lastPrintMs = millis();
  analogRead(LIGHT_PIN);
  Serial.print("light:");
  Serial.println(analogRead(LIGHT_PIN));
}
