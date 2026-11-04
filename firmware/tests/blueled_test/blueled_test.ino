const uint8_t ANODE_PIN = 30;
const uint8_t CATHODE_PIN = 31;
const unsigned long TIMEOUT_US = 900000UL;
bool measuring = false;
unsigned long startUs = 0;
unsigned long lastStartMs = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ANODE_PIN, OUTPUT);
  digitalWrite(ANODE_PIN, LOW);
  pinMode(CATHODE_PIN, OUTPUT);
  digitalWrite(CATHODE_PIN, LOW);
}

void startMeasure() {
  digitalWrite(CATHODE_PIN, HIGH);
  pinMode(CATHODE_PIN, INPUT);
  startUs = micros();
  lastStartMs = millis();
  measuring = true;
}

void finishMeasure(unsigned long elapsedUs, bool timedOut) {
  pinMode(CATHODE_PIN, OUTPUT);
  digitalWrite(CATHODE_PIN, LOW);
  measuring = false;
  Serial.print("discharge_us:");
  Serial.print(elapsedUs);
  Serial.println(timedOut ? " (timeout = dark)" : "");
}

void loop() {
  if (!measuring) {
    if (millis() - lastStartMs >= 1000) startMeasure();
    return;
  }
  unsigned long elapsedUs = micros() - startUs;
  if (digitalRead(CATHODE_PIN) == LOW) finishMeasure(elapsedUs, false);
  else if (elapsedUs >= TIMEOUT_US) finishMeasure(TIMEOUT_US, true);
}
