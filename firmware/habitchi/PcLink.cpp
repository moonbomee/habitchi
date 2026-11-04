#include "PcLink.h"
#include <stdlib.h>
#include <string.h>
#include "Events.h"
#include "Facts.h"
#include "GameClock.h"
#include "Pet.h"
#include "PetView.h"
#include "Timing.h"

namespace PcLink {
namespace {
constexpr uint8_t LINE_SIZE = 48;
constexpr uint8_t READ_PER_UPDATE = 64;
constexpr unsigned long STATE_EVERY_MS = 60000UL;

char line[LINE_SIZE];
uint8_t lineLen = 0;
bool lineTooLong = false;
unsigned long lastRxMs = 0;
unsigned long lastStateMs = 0;

void handleCommit(const char* kind) {
  if (strcmp(kind, "GOOD") == 0) {
    eventPush(Ev::CommitGood);
  } else if (strcmp(kind, "JUNK") == 0) {
    eventPush(Ev::CommitJunk);
  } else if (strcmp(kind, "BROKEN") == 0) {
    eventPush(Ev::CommitBroken);
  }
}

bool readNumber(const char* text, unsigned long& out) {
  char* end = nullptr;
  out = strtoul(text, &end, 10);
  return end != text && *end == '\0';
}

void handleTime(const char* text) {
  unsigned long seconds = 0;
  if (readNumber(text, seconds)) {
    GameClock::syncFromPc(seconds);
  }
}

void handleDemo(const char* command) {
  if (!GameClock::demoMode()) {
    return;
  }
  unsigned long number = 0;
  if (strncmp(command, "STAGE,", 6) == 0) {
    if (readNumber(command + 6, number) && number <= static_cast<unsigned long>(Stage::Obese)) {
      eventPush(Ev::DemoStage, static_cast<int16_t>(number));
    }
  } else if (strncmp(command, "SICK,", 5) == 0) {
    if (readNumber(command + 5, number) && number >= 1 && number <= 4) {
      eventPush(Ev::DemoSick, static_cast<int16_t>(number));
    }
  } else if (strcmp(command, "DIE") == 0) {
    eventPush(Ev::DemoDie);
  } else if (strcmp(command, "EVOLVE") == 0) {
    eventPush(Ev::DemoEvolve);
  }
}

void handleLine() {
  facts.pcConnected = true;
  lastRxMs = millis();

  if (strcmp(line, "EVT,PING") == 0) {
    return;
  }
  if (strcmp(line, "EVT,TYPING") == 0) {
    facts.lastTypingMs = millis();
    facts.typingSeen = true;
    return;
  }
  if (strncmp(line, "EVT,COMMIT,", 11) == 0) {
    handleCommit(line + 11);
    return;
  }
  if (strncmp(line, "EVT,TIME,", 9) == 0) {
    handleTime(line + 9);
    return;
  }
  if (strncmp(line, "DEMO,", 5) == 0) {
    handleDemo(line + 5);
  }
}

void readSerial() {
  uint8_t budget = READ_PER_UPDATE;
  while (budget > 0 && Serial.available() > 0) {
    budget--;
    char c = static_cast<char>(Serial.read());
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      if (!lineTooLong && lineLen > 0) {
        line[lineLen] = '\0';
        handleLine();
      }
      lineLen = 0;
      lineTooLong = false;
      continue;
    }
    if (lineLen < LINE_SIZE - 1) {
      line[lineLen] = c;
      lineLen++;
    } else {
      lineTooLong = true;
    }
  }
}

void checkTimeout() {
  if (facts.pcConnected && millis() - lastRxMs > Cfg::PC_TIMEOUT_MS) {
    facts.pcConnected = false;
  }
}

void sendState() {
  const PetView& v = Pet::view();
  Serial.print(F("STATE,"));
  Serial.print(static_cast<int>(v.stage));
  Serial.print(',');
  Serial.print(static_cast<int>(v.mood));
  Serial.print(',');
  Serial.print(static_cast<int>(v.fullness));
  Serial.print(',');
  Serial.print(static_cast<int>(v.rest));
  Serial.print(',');
  Serial.print(static_cast<int>(v.stamina));
  Serial.print(',');
  Serial.print(static_cast<unsigned int>(v.exp));
  Serial.print('\n');
}
}

void setup() {
  lineLen = 0;
  lineTooLong = false;
  facts.pcConnected = false;
  lastStateMs = millis();
}

void update() {
  readSerial();
  checkTimeout();
  if (millis() - lastStateMs >= STATE_EVERY_MS) {
    lastStateMs = millis();
    sendState();
  }
}

void log(const char* kind, long value) {
  Serial.print(F("LOG,"));
  Serial.print(GameClock::now());
  Serial.print(',');
  Serial.print(kind);
  Serial.print(',');
  Serial.print(value);
  Serial.print('\n');
}
}
