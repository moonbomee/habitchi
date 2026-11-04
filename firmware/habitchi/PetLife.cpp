#include "PetLife.h"
#include "PetCore.h"
#include "PetConfig.h"
#include "Storage.h"
#include "PcLink.h"
#include "Sound.h"

namespace PetLife {
namespace {
constexpr uint8_t CAUSE_COLD = 1;
constexpr uint8_t CAUSE_REST = 2;
constexpr uint8_t CAUSE_HUNGER = 3;
constexpr uint8_t CAUSE_OVERWORK = 4;
constexpr uint16_t MINUTES_PER_DAY = 24 * 60;

unsigned long tombStartMs = 0;

uint16_t countZero(uint16_t stat, uint16_t minutes) {
  if (stat != 0) return 0;
  return minutes < 65535U ? minutes + 1 : minutes;
}

void checkNewSickness() {
  if (pet.restZeroMinutes >= PetCfg::ZERO_MINUTES_FOR_SICK) becomeSick(CAUSE_REST);
  else if (pet.fullnessZeroMinutes >= PetCfg::ZERO_MINUTES_FOR_SICK) becomeSick(CAUSE_HUNGER);
  else if (pet.staminaZeroMinutes >= PetCfg::ZERO_MINUTES_FOR_SICK) becomeSick(CAUSE_OVERWORK);
}

void checkRecovery() {
  if (pet.sickCause == CAUSE_REST && PetCore::percent(pet.rest) > PetCfg::SICK_RECOVER_ABOVE) cure();
  else if (pet.sickCause == CAUSE_HUNGER && PetCore::percent(pet.fullness) > PetCfg::SICK_RECOVER_ABOVE) cure();
  else if (pet.sickCause == CAUSE_OVERWORK && PetCore::percent(pet.stamina) > PetCfg::SICK_RECOVER_ABOVE) cure();
}

void checkGrowth() {
  if (pet.stage == Stage::Egg && pet.stageMinutes >= PetCfg::EGG_HATCH_MINUTES) {
    hatchIfEgg();
  } else if (pet.stage == Stage::Baby && pet.stageMinutes >= PetCfg::BABY_GROW_MINUTES) {
    setStage(Stage::Child);
    Sound::play(SoundId::Evolve);
  } else if (pet.stage == Stage::Child && pet.stageMinutes >= PetCfg::CHILD_GROW_MINUTES) {
    evolveAdult();
  }
}
}

void setup() {
  tombStartMs = millis();
}

void update() {
  if (pet.stage != Stage::Dead) return;
  if (millis() - tombStartMs < PetCfg::TOMB_MS) return;
  uint8_t nextGeneration = pet.generation < 255 ? pet.generation + 1 : pet.generation;
  PetCore::resetPet(nextGeneration, pet.lastLifespanDays);
  PetCore::notify(Notice::NewGeneration, nextGeneration);
  Storage::requestSave();
}

void minuteTick() {
  pet.ageMinutes++;
  pet.stageMinutes++;
  checkGrowth();

  pet.restZeroMinutes = countZero(pet.rest, pet.restZeroMinutes);
  pet.fullnessZeroMinutes = countZero(pet.fullness, pet.fullnessZeroMinutes);
  pet.staminaZeroMinutes = countZero(pet.stamina, pet.staminaZeroMinutes);

  if (pet.sickCause == 0) {
    checkNewSickness();
    return;
  }
  checkRecovery();
  if (pet.sickCause == 0) return;

  if (pet.sickCause == CAUSE_COLD) PetCore::addPerHour(pet.stamina, PetCfg::STAMINA_COLD_SICK_PER_HOUR);
  if (pet.sickMinutes < 65535U) pet.sickMinutes++;
  if (PetCfg::DEATH_ENABLED && pet.sickMinutes >= PetCfg::SICK_MINUTES_FOR_DEATH) die();
}

void setStage(Stage stage) {
  pet.stage = stage;
  pet.stageMinutes = 0;
  Storage::requestSave();
}

void hatchIfEgg() {
  if (pet.stage != Stage::Egg) return;
  setStage(Stage::Baby);
  Sound::play(SoundId::Hatch);
}

void evolveAdult() {
  uint16_t badCommits = pet.junkCommits + pet.lateCommits;
  uint16_t average = pet.sleepNights > 0 ? pet.sleepScoreSum / pet.sleepNights : 0;
  if (pet.commits >= PetCfg::OBESE_MIN_COMMITS && (uint32_t)badCommits * 2 > pet.commits) {
    setStage(Stage::Obese);
  } else if (average >= PetCfg::HEALTHY_SLEEP_SCORE && !pet.hadColdSickness) {
    setStage(Stage::Healthy);
  } else {
    setStage(Stage::Weak);
  }
  Sound::play(SoundId::Evolve);
}

void becomeSick(uint8_t cause) {
  if (pet.sickCause != 0) return;
  pet.sickCause = cause;
  pet.sickMinutes = 0;
  if (cause == CAUSE_COLD) pet.hadColdSickness = true;
  PcLink::log("SICK", cause);
  Storage::requestSave();
}

void cure() {
  pet.sickCause = 0;
  pet.sickMinutes = 0;
  Storage::requestSave();
}

void cureCold() {
  if (pet.sickCause != CAUSE_COLD) return;
  pet.coldNightsInRow = 0;
  cure();
}

void die() {
  uint32_t days = pet.ageMinutes / MINUTES_PER_DAY;
  pet.lastLifespanDays = days > 255 ? 255 : days;
  PcLink::log("DEATH", pet.generation);
  pet.stage = Stage::Dead;
  pet.sickCause = 0;
  pet.sickMinutes = 0;
  tombStartMs = millis();
  PetCore::clearNotices();
  PetCore::clearReactions();
  Sound::play(SoundId::Death);
  Storage::requestSave();
}

void afterNight() {
  if (pet.coldNightsInRow >= PetCfg::COLD_NIGHTS_FOR_SICK) becomeSick(CAUSE_COLD);
  else if (pet.coldNightsInRow == 0 && pet.sickCause == CAUSE_COLD) cure();
}

void demoStage(int16_t value) {
  if (value < 0 || value > (int16_t)Stage::Dead) return;
  Stage stage = (Stage)value;
  if (stage == Stage::Dead) {
    die();
    return;
  }
  setStage(stage);
  Sound::play(SoundId::Evolve);
}

void demoSick(int16_t cause) {
  if (cause < CAUSE_COLD || cause > CAUSE_OVERWORK) return;
  if (pet.stage == Stage::Dead) return;
  cure();
  if (cause == CAUSE_COLD) pet.coldNightsInRow = PetCfg::COLD_NIGHTS_FOR_SICK;
  if (cause == CAUSE_REST) pet.rest = 0;
  if (cause == CAUSE_HUNGER) pet.fullness = 0;
  if (cause == CAUSE_OVERWORK) pet.stamina = 0;
  becomeSick(cause);
}
}
