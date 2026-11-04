#include <RTClib.h>

const uint8_t BUTTON_PINS[3] = {24, 25, 26};
const char BUTTON_NAMES[3] = {'A', 'B', 'C'};

RTC_DS3231 rtc;
bool rtcFound = false;
bool lastPressed[3] = {false, false, false};
unsigned long lastPrintMs = 0;
char line[32];
uint8_t lineLength = 0;

void printTime() {
  if (!rtcFound) {
    Serial.println("RTC not found");
    return;
  }
  DateTime t = rtc.now();
  char text[32];
  snprintf(text, sizeof(text), "%04d-%02d-%02d %02d:%02d:%02d", t.year(), t.month(), t.day(), t.hour(), t.minute(), t.second());
  Serial.print(text);
  Serial.print("  unix:");
  Serial.println(t.unixtime());
}

void handleLine() {
  if (strncmp(line, "EVT,TIME,", 9) == 0 && rtcFound) {
    rtc.adjust(DateTime(strtoul(line + 9, nullptr, 10)));
    Serial.println("RTC set from serial");
  }
}

void readSerial() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      line[lineLength] = '\0';
      if (lineLength > 0) handleLine();
      lineLength = 0;
    } else if (lineLength < sizeof(line) - 1) {
      line[lineLength] = c;
      lineLength++;
    }
  }
}

void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < 3; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }
  rtcFound = rtc.begin();
  if (rtcFound && rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println("RTC lost power, set to compile time");
  }
}

void loop() {
  readSerial();

  for (uint8_t i = 0; i < 3; i++) {
    bool pressed = digitalRead(BUTTON_PINS[i]) == LOW;
    if (pressed != lastPressed[i]) {
      lastPressed[i] = pressed;
      Serial.print("button ");
      Serial.print(BUTTON_NAMES[i]);
      Serial.println(pressed ? " down" : " up");
    }
  }

  if (millis() - lastPrintMs >= 1000) {
    lastPrintMs = millis();
    printTime();
  }
}
