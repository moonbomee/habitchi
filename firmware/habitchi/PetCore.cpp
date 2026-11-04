#include "PetCore.h"
#include "PetConfig.h"
#include "Facts.h"
#include "GameClock.h"

PetSave pet = {};

namespace PetCore {
namespace {
constexpr uint8_t REACTION_COUNT = 6;
const Mood REACTIONS[REACTION_COUNT] = {
  Mood::Cold, Mood::Stomachache, Mood::Indigestion, Mood::SoreEyes, Mood::Full, Mood::Happy
};
unsigned long reactionStart[REACTION_COUNT];
uint8_t reactionOn = 0;

constexpr uint8_t NOTICE_QUEUE = 4;
Notice waitingNotice[NOTICE_QUEUE];
int16_t waitingValue[NOTICE_QUEUE];
uint8_t waitingCount = 0;
Notice shownNotice = Notice::None;
int16_t shownValue = 0;
unsigned long shownStartMs = 0;

constexpr uint16_t STAT_MAX = 100 * STAT_SCALE;
constexpr uint16_t MINUTES_PER_DAY = 24 * 60;

int8_t reactionIndex(Mood mood) {
  for (uint8_t i = 0; i < REACTION_COUNT; i++) {
    if (REACTIONS[i] == mood) return i;
  }
  return -1;
}

void setStat(uint16_t& stat, long value) {
  if (value < 0) value = 0;
  if (value > STAT_MAX) value = STAT_MAX;
  stat = value;
}
}

void resetPet(uint8_t generation, uint8_t lastLifespanDays) {
  pet = {};
  pet.stage = Stage::Egg;
  pet.generation = generation;
  pet.lastLifespanDays = lastLifespanDays;
  pet.fullness = PetCfg::START_FULLNESS * STAT_SCALE;
  pet.rest = PetCfg::START_REST * STAT_SCALE;
  pet.stamina = PetCfg::START_STAMINA * STAT_SCALE;
}

uint8_t percent(uint16_t stat) {
  return (stat + STAT_SCALE - 1) / STAT_SCALE;
}

void addPoints(uint16_t& stat, int16_t points) {
  setStat(stat, (long)stat + (long)points * STAT_SCALE);
}

void addPerHour(uint16_t& stat, int16_t pointsPerHour) {
  setStat(stat, (long)stat + (long)pointsPerHour * STAT_SCALE / 60L);
}

void gainExp(uint8_t base, uint8_t halvings) {
  uint8_t amount = base >> halvings;
  if (amount == 0) amount = 1;
  if (pet.exp > 65535U - amount) pet.exp = 65535U;
  else pet.exp += amount;
}

bool typingActive() {
  return facts.typingSeen && millis() - facts.lastTypingMs < Cfg::TYPING_ACTIVE_MS;
}

uint16_t minuteOfDay() {
  return GameClock::hour() * 60U + GameClock::minute();
}

bool between(uint16_t minute, uint16_t start, uint16_t end) {
  if (start <= end) return minute >= start && minute < end;
  return minute >= start || minute < end;
}

bool inSleepHours() {
  return between(minuteOfDay(), PetCfg::BEDTIME_MINUTE, PetCfg::WAKE_MINUTE);
}

bool inSleepWindow() {
  uint16_t start = (PetCfg::BEDTIME_MINUTE + MINUTES_PER_DAY - PetCfg::LIGHTS_OFF_EARLY_MINUTES) % MINUTES_PER_DAY;
  return between(minuteOfDay(), start, PetCfg::WAKE_MINUTE);
}

long gameMinutesFromMs(unsigned long ms) {
  if (GameClock::demoMode()) return (long)(ms / 1000UL) * Cfg::DEMO_SPEED / 60L;
  return ms / 60000UL;
}

void react(Mood mood) {
  int8_t i = reactionIndex(mood);
  if (i < 0) return;
  reactionStart[i] = millis();
  reactionOn |= (1 << i);
}

bool reacting(Mood mood) {
  int8_t i = reactionIndex(mood);
  if (i < 0 || !(reactionOn & (1 << i))) return false;
  if (millis() - reactionStart[i] < PetCfg::REACTION_MS) return true;
  reactionOn &= ~(1 << i);
  return false;
}

void clearReactions() {
  reactionOn = 0;
}

void notify(Notice notice, int16_t value) {
  if (waitingCount == NOTICE_QUEUE) return;
  waitingNotice[waitingCount] = notice;
  waitingValue[waitingCount] = value;
  waitingCount++;
}

void updateNotices() {
  if (shownNotice != Notice::None && millis() - shownStartMs < PetCfg::NOTICE_MS) return;
  shownNotice = Notice::None;
  shownValue = 0;
  if (waitingCount == 0) return;
  shownNotice = waitingNotice[0];
  shownValue = waitingValue[0];
  shownStartMs = millis();
  for (uint8_t i = 1; i < waitingCount; i++) {
    waitingNotice[i - 1] = waitingNotice[i];
    waitingValue[i - 1] = waitingValue[i];
  }
  waitingCount--;
}

Notice currentNotice() {
  return shownNotice;
}

int16_t currentNoticeValue() {
  return shownValue;
}

void clearNotices() {
  waitingCount = 0;
  shownNotice = Notice::None;
  shownValue = 0;
}
}
