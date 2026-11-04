const uint8_t LM35_PIN = A1;
unsigned long lastPrintMs = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  if (millis() - lastPrintMs < 500) return;
  lastPrintMs = millis();
  analogRead(LM35_PIN);
  long sum = 0;
  for (int i = 0; i < 16; i++) sum += analogRead(LM35_PIN);
  float raw = sum / 16.0f;
  float millivolts = raw * 5000.0f / 1024.0f;
  Serial.print("raw:");
  Serial.print(raw);
  Serial.print(" mV:");
  Serial.print(millivolts);
  Serial.print(" C:");
  Serial.println(millivolts / 10.0f);
}
