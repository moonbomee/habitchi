#include "Sound.h"
#include "Pins.h"

namespace Sound {
namespace {

const uint16_t REST = 0;
const uint16_t NOTE_C4 = 262;
const uint16_t NOTE_E4 = 330;
const uint16_t NOTE_F4 = 349;
const uint16_t NOTE_G4 = 392;
const uint16_t NOTE_A4 = 440;
const uint16_t NOTE_C5 = 523;
const uint16_t NOTE_D5 = 587;
const uint16_t NOTE_DS5 = 622;
const uint16_t NOTE_E5 = 659;
const uint16_t NOTE_F5 = 698;
const uint16_t NOTE_G5 = 784;
const uint16_t NOTE_A5 = 880;
const uint16_t NOTE_B5 = 988;
const uint16_t NOTE_C6 = 1047;
const uint16_t NOTE_E6 = 1319;
const uint16_t NOTE_G6 = 1568;
const uint16_t NOTE_C7 = 2093;

const unsigned long NOTE_GAP_MS = 15;
const unsigned long BUSY_TAIL_MS = 200;
const uint8_t ALARM_REPEATS = 6;

struct Note {
  uint16_t freq;
  uint16_t ms;
};

const Note CLICK[] PROGMEM = {
  {NOTE_E6, 25}
};

const Note ALARM[] PROGMEM = {
  {NOTE_C6, 120}, {REST, 60}, {NOTE_C6, 120}, {REST, 60},
  {NOTE_G5, 120}, {REST, 60}, {NOTE_C6, 200}, {REST, 300}
};

const Note WARNING[] PROGMEM = {
  {NOTE_A5, 150}, {REST, 80}, {NOTE_A5, 150}, {REST, 80}, {NOTE_E5, 300}
};

const Note EAT[] PROGMEM = {
  {NOTE_G5, 60}, {REST, 50}, {NOTE_G5, 60}, {REST, 50}, {NOTE_C6, 120}
};

const Note HAPPY[] PROGMEM = {
  {NOTE_C5, 100}, {NOTE_E5, 100}, {NOTE_G5, 100}, {NOTE_C6, 220}
};

const Note SNEEZE[] PROGMEM = {
  {NOTE_E5, 100}, {NOTE_G5, 100}, {NOTE_B5, 140}, {REST, 120},
  {NOTE_C7, 40}, {NOTE_A5, 40}, {NOTE_F5, 60}
};

const Note PAIN[] PROGMEM = {
  {NOTE_E5, 140}, {NOTE_DS5, 140}, {NOTE_E5, 140}, {NOTE_DS5, 140}, {NOTE_C5, 350}
};

const Note HATCH[] PROGMEM = {
  {NOTE_C5, 80}, {NOTE_D5, 80}, {NOTE_E5, 80}, {NOTE_F5, 80},
  {NOTE_G5, 80}, {REST, 60}, {NOTE_C6, 250}
};

const Note EVOLVE[] PROGMEM = {
  {NOTE_C5, 100}, {NOTE_E5, 100}, {NOTE_G5, 100},
  {NOTE_C6, 100}, {NOTE_E6, 100}, {NOTE_G6, 350}
};

const Note DEATH[] PROGMEM = {
  {NOTE_G4, 280}, {NOTE_F4, 280}, {NOTE_E4, 280}, {NOTE_A4, 120}, {NOTE_C4, 700}
};

const Note* melody = nullptr;
uint8_t melodyLength = 0;
uint8_t noteIndex = 0;
uint8_t repeatsLeft = 0;
unsigned long noteStartMs = 0;
unsigned long noteLengthMs = 0;
unsigned long stoppedMs = 0;
bool playing = false;

void startNote() {
  uint16_t freq = pgm_read_word(&melody[noteIndex].freq);
  noteLengthMs = pgm_read_word(&melody[noteIndex].ms);
  noteStartMs = millis();
  if (freq == REST) {
    noTone(Pins::SPEAKER);
  } else if (noteLengthMs > NOTE_GAP_MS) {
    tone(Pins::SPEAKER, freq, noteLengthMs - NOTE_GAP_MS);
  } else {
    tone(Pins::SPEAKER, freq, noteLengthMs);
  }
}

void stopSound() {
  noTone(Pins::SPEAKER);
  playing = false;
  stoppedMs = millis();
}

}

void setup() {
  pinMode(Pins::SPEAKER, OUTPUT);
  noTone(Pins::SPEAKER);
}

void play(SoundId id) {
  repeatsLeft = 1;
  switch (id) {
    case SoundId::Click: melody = CLICK; melodyLength = sizeof(CLICK) / sizeof(Note); break;
    case SoundId::Alarm: melody = ALARM; melodyLength = sizeof(ALARM) / sizeof(Note); repeatsLeft = ALARM_REPEATS; break;
    case SoundId::Warning: melody = WARNING; melodyLength = sizeof(WARNING) / sizeof(Note); break;
    case SoundId::Eat: melody = EAT; melodyLength = sizeof(EAT) / sizeof(Note); break;
    case SoundId::Happy: melody = HAPPY; melodyLength = sizeof(HAPPY) / sizeof(Note); break;
    case SoundId::Sneeze: melody = SNEEZE; melodyLength = sizeof(SNEEZE) / sizeof(Note); break;
    case SoundId::Pain: melody = PAIN; melodyLength = sizeof(PAIN) / sizeof(Note); break;
    case SoundId::Hatch: melody = HATCH; melodyLength = sizeof(HATCH) / sizeof(Note); break;
    case SoundId::Evolve: melody = EVOLVE; melodyLength = sizeof(EVOLVE) / sizeof(Note); break;
    case SoundId::Death: melody = DEATH; melodyLength = sizeof(DEATH) / sizeof(Note); break;
    default: return;
  }
  noteIndex = 0;
  playing = true;
  startNote();
}

void stop() {
  if (playing) stopSound();
}

void update() {
  if (!playing) return;
  if (millis() - noteStartMs < noteLengthMs) return;

  noteIndex++;
  if (noteIndex >= melodyLength) {
    repeatsLeft--;
    if (repeatsLeft == 0) {
      stopSound();
      return;
    }
    noteIndex = 0;
  }
  startNote();
}

bool busy() {
  if (playing) return true;
  return millis() - stoppedMs < BUSY_TAIL_MS;
}

}
