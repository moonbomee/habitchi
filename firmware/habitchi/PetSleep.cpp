#include "PetSleep.h"
#include "PetCore.h"
#include "PetConfig.h"
#include "PetLife.h"
#include "Storage.h"
#include "Facts.h"
#include "GameClock.h"
#include "PcLink.h"
#include "Sound.h"

namespace PetSleep {
namespace {
constexpr uint16_t MINUTES_PER_DAY = 24 * 60;

bool asleep = false;
uint32_t sleepStartTime = 0;
bool coldTiming = false;
unsigned long coldSinceMs = 0;
bool coldNow = false;
bool alarmOn = false;
unsigned long alarmStartMs = 0;
uint16_t lastMinute = 0;
uint16_t tempLogMinutes = 0;
bool tempLogDue = false;

uint16_t sessionMinutes() {
  uint32_t now = GameClock::now();
  if (now <= sleepStartTime) return 0;
  uint32_t minutes = (now - sleepStartTime) / 60UL;
  if (minutes > MINUTES_PER_DAY) minutes = MINUTES_PER_DAY;
  return minutes;
}

uint8_t addCount(uint8_t count) {
  return count < 255 ? count + 1 : count;
}

void fallAsleep() {
  asleep = true;
  sleepStartTime = GameClock::now();
  PcLink::log("SLEEP_START", 0);
  if (!pet.nightStarted) {
    pet.nightStarted = true;
    pet.nightStartTime = sleepStartTime;
    pet.nightFirstOffMinute = PetCore::minuteOfDay();
    pet.nightSleptMinutes = 0;
    pet.nightNoises = 0;
    pet.nightCold = false;
    pet.nightGlow = false;
  }
  Storage::requestSave();
}

void wakeUp() {
  uint16_t minutes = sessionMinutes();
  asleep = false;
  coldTiming = false;
  coldNow = false;
  PcLink::log("SLEEP_END", minutes);
  if (pet.nightStarted) pet.nightSleptMinutes += minutes;
  Storage::requestSave();
}

void checkSleepChange() {
  bool keysQuiet = !facts.typingSeen || millis() - facts.lastTypingMs >= pick(PetCfg::FALL_ASLEEP_NO_TYPING);
  if (!asleep && facts.roomDark && PetCore::inSleepWindow() && keysQuiet) {
    fallAsleep();
  } else if (asleep && (!facts.roomDark || PetCore::typingActive())) {
    wakeUp();
  }
}

void checkCold() {
  if (facts.tempValid && facts.roomTempC < PetCfg::COLD_BELOW_C) {
    if (!coldTiming) {
      coldTiming = true;
      coldSinceMs = millis();
    }
    if (!coldNow && millis() - coldSinceMs >= pick(PetCfg::COLD_AFTER)) {
      coldNow = true;
      pet.nightCold = true;
      PetCore::react(Mood::Cold);
      Sound::play(SoundId::Sneeze);
    }
  } else if (!facts.tempValid || facts.roomTempC >= PetCfg::COLD_RECOVER_C) {
    coldTiming = false;
    coldNow = false;
  }
}

void checkWarmCure() {
  if (pet.nightCold || !pet.nightStarted) return;
  if (pet.nightSleptMinutes + sessionMinutes() >= PetCfg::WARM_SLEEP_CURES_COLD_MINUTES) PetLife::cureCold();
}

void checkGlow() {
  if (facts.screenGlow && facts.roomDark && !pet.nightGlow) {
    pet.nightGlow = true;
    PetCore::addPoints(pet.rest, PetCfg::REST_GLOW_PENALTY);
  }
}

uint8_t limitPenalty(uint16_t value, uint8_t most) {
  return value > most ? most : value;
}

uint8_t sleepScore(uint16_t slept) {
  int16_t score = 100;
  if (PetCore::between(pet.nightFirstOffMinute, PetCfg::BEDTIME_MINUTE, PetCfg::WAKE_MINUTE)) {
    uint16_t late = (pet.nightFirstOffMinute + MINUTES_PER_DAY - PetCfg::BEDTIME_MINUTE) % MINUTES_PER_DAY;
    score -= limitPenalty(late / PetCfg::SCORE_LATE_STEP_MINUTES, PetCfg::SCORE_LATE_MAX);
  }
  if (slept < PetCfg::SLEEP_TARGET_MINUTES) {
    uint16_t missing = PetCfg::SLEEP_TARGET_MINUTES - slept;
    score -= limitPenalty(missing / PetCfg::SCORE_SHORT_STEP_MINUTES, PetCfg::SCORE_SHORT_MAX);
  }
  score -= limitPenalty((uint16_t)pet.nightNoises * PetCfg::SCORE_PER_NOISE, PetCfg::SCORE_NOISE_MAX);
  if (pet.nightCold) score -= PetCfg::SCORE_COLD;
  if (pet.nightGlow) score -= PetCfg::SCORE_GLOW;
  score -= limitPenalty((uint16_t)pet.nightSnacks * PetCfg::SCORE_PER_SNACK, PetCfg::SCORE_SNACK_MAX);
  if (score < 0) score = 0;
  return score;
}

void recordScore(uint8_t score) {
  PcLink::log("SLEEP_SCORE", score);
  pet.lastSleepScore = score;
  if (pet.sleepScoreSum <= 65535U - score) {
    pet.sleepScoreSum += score;
    pet.sleepNights++;
  }
  PetCore::notify(Notice::GoodMorning, score);
}

void finishMorning() {
  uint16_t slept = pet.nightSleptMinutes;
  if (asleep) slept += sessionMinutes();
  recordScore(sleepScore(slept));

  if (pet.nightCold) pet.coldNightsInRow = addCount(pet.coldNightsInRow);
  else pet.coldNightsInRow = 0;

  if (pet.nightCold) PetCore::notify(Notice::ColdNight, 0);
  if (pet.nightNoises >= PetCfg::NOISY_NIGHT_NOISES) PetCore::notify(Notice::NoisyNight, 0);
  if (pet.nightGlow) PetCore::notify(Notice::ScreenBeforeSleep, 0);

  pet.nightStarted = false;
  pet.nightSnacks = 0;
  PetLife::afterNight();
  Storage::requestSave();
}

void finishSleeplessNight() {
  recordScore(0);
  pet.nightSnacks = 0;
  Storage::requestSave();
}

void wakeTimeReached() {
  if (!pet.nightStarted) {
    finishSleeplessNight();
    return;
  }
  if (facts.roomDark) {
    alarmOn = true;
    alarmStartMs = millis();
    Sound::play(SoundId::Alarm);
  } else {
    finishMorning();
  }
}

void checkWakeTime() {
  uint16_t nowMinute = PetCore::minuteOfDay();
  uint16_t step = (nowMinute + MINUTES_PER_DAY - lastMinute) % MINUTES_PER_DAY;
  uint16_t toWake = (PetCfg::WAKE_MINUTE + MINUTES_PER_DAY - lastMinute) % MINUTES_PER_DAY;
  bool smallStep = step <= PetCfg::MAX_CLOCK_STEP_SECONDS / 60UL;
  lastMinute = nowMinute;
  if (step > 0 && smallStep && toWake > 0 && toWake <= step && !alarmOn) {
    wakeTimeReached();
  }
}

void updateAlarm() {
  if (!alarmOn) return;
  if (!facts.roomDark || millis() - alarmStartMs >= PetCfg::ALARM_MAX_MS) {
    alarmOn = false;
    Sound::stop();
    finishMorning();
  } else if (!Sound::busy()) {
    Sound::play(SoundId::Alarm);
  }
}
}

void setup() {
  lastMinute = PetCore::minuteOfDay();
  uint32_t now = GameClock::now();
  if (pet.nightStarted && (now < pet.nightStartTime || now - pet.nightStartTime > PetCfg::NIGHT_MAX_AGE_SECONDS)) {
    pet.nightStarted = false;
    pet.nightSnacks = 0;
  }
}

void reset() {
  asleep = false;
  coldTiming = false;
  coldNow = false;
  alarmOn = false;
}

void update() {
  checkSleepChange();
  if (asleep) {
    checkCold();
    checkGlow();
    checkWarmCure();
  }
  checkWakeTime();
  updateAlarm();
  if (tempLogDue) {
    tempLogDue = false;
    if (facts.tempValid) PcLink::log("TEMP", lround(facts.roomTempC * 10.0f));
  }
}

void minuteTick() {
  if (asleep) {
    int16_t gain = PetCfg::REST_SLEEP_PER_HOUR - (int16_t)pet.nightNoises * PetCfg::REST_NOISE_CUT_PER_HOUR;
    if (coldNow) gain -= PetCfg::REST_COLD_CUT_PER_HOUR;
    if (gain < PetCfg::REST_SLEEP_MIN_PER_HOUR) gain = PetCfg::REST_SLEEP_MIN_PER_HOUR;
    PetCore::addPerHour(pet.rest, gain);
    PetCore::addPerHour(pet.stamina, PetCfg::STAMINA_SLEEP_PER_HOUR);
  } else {
    PetCore::addPerHour(pet.rest, PetCfg::REST_AWAKE_PER_HOUR);
    if (PetCore::inSleepHours()) PetCore::addPerHour(pet.rest, PetCfg::REST_AWAKE_LATE_PER_HOUR);
  }

  tempLogMinutes++;
  if (tempLogMinutes >= PetCfg::TEMP_LOG_EVERY_MINUTES) {
    tempLogMinutes = 0;
    tempLogDue = true;
  }
}

void onLoudNoise() {
  if (!asleep) return;
  pet.nightNoises = addCount(pet.nightNoises);
  PcLink::log("NOISE", facts.noiseLevel);
}

void onLateSnack() {
  pet.nightSnacks = addCount(pet.nightSnacks);
}

bool sleeping() {
  return asleep;
}
}
