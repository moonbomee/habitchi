const uint8_t FSR_PIN = A4;
const uint8_t PIEZO_PIN = A5;
unsigned long windowStartMs = 0;
int piezoMax = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  analogRead(PIEZO_PIN);
  int piezo = analogRead(PIEZO_PIN);
  if (piezo > piezoMax) piezoMax = piezo;
  if (millis() - windowStartMs < 50) return;
  analogRead(FSR_PIN);
  int fsr = analogRead(FSR_PIN);
  Serial.print("fsr:");
  Serial.print(fsr);
  Serial.print(" piezoMax:");
  Serial.println(piezoMax);
  windowStartMs = millis();
  piezoMax = 0;
}
