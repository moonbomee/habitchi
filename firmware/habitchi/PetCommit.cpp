#include "PetCommit.h"
#include "PetCore.h"
#include "PetConfig.h"
#include "PetLife.h"
#include "PetSleep.h"
#include "PetCoding.h"
#include "Storage.h"
#include "Facts.h"
#include "PcLink.h"
#include "Sound.h"

namespace PetCommit {
namespace {
bool ateBefore = false;
unsigned long lastMealMs = 0;
uint16_t pcOffMinutes = 0;

uint16_t addCount(uint16_t count) {
  return count < 65535U ? count + 1 : count;
}
}

void onCommit(Ev type) {
  uint8_t kind = 0;
  uint8_t food = PetCfg::FOOD_GOOD;
  uint8_t exp = PetCfg::EXP_GOOD;
  if (type == Ev::CommitJunk) {
    kind = 1;
    food = PetCfg::FOOD_JUNK;
    exp = PetCfg::EXP_JUNK;
  } else if (type == Ev::CommitBroken) {
    kind = 2;
    food = PetCfg::FOOD_BROKEN;
    exp = PetCfg::EXP_BROKEN;
  }
  PcLink::log("COMMIT", kind);
  if (pet.stage == Stage::Dead) return;

  PetLife::hatchIfEgg();

  bool late = PetCore::inSleepHours();
  bool tooSoon = ateBefore && millis() - lastMealMs < pick(PetCfg::OVEREAT_GAP);
  bool overeat = tooSoon || PetCore::percent(pet.fullness) > PetCfg::OVEREAT_ABOVE;
  ateBefore = true;
  lastMealMs = millis();

  uint8_t halvings = 0;
  if (late) halvings++;
  if (PetCoding::allNighter()) halvings++;
  if (PetCoding::distracted()) halvings++;
  if (overeat) {
    food /= 2;
    halvings++;
  }

  PetCore::addPoints(pet.fullness, food);
  PetCore::gainExp(exp, halvings);

  pet.commits = addCount(pet.commits);
  if (type == Ev::CommitJunk) pet.junkCommits = addCount(pet.junkCommits);

  if (type == Ev::CommitBroken) {
    PetCore::react(Mood::Stomachache);
    Sound::play(SoundId::Pain);
  } else {
    Sound::play(SoundId::Eat);
  }

  if (late) {
    pet.lateCommits = addCount(pet.lateCommits);
    PetCore::addPoints(pet.rest, PetCfg::REST_SNACK_PENALTY);
    PetSleep::onLateSnack();
    PetCoding::markAllNighter();
    PetCore::react(Mood::Indigestion);
  } else if (overeat) {
    PetCore::react(Mood::Full);
  } else if (type == Ev::CommitGood) {
    PetCore::react(Mood::Happy);
  }
  Storage::requestSave();
}

void minuteTick() {
  if (facts.pcConnected) pcOffMinutes = 0;
  else if (pcOffMinutes < 65535U) pcOffMinutes++;
  if (pcOffMinutes < PetCfg::PC_OFF_PAUSE_MINUTES) {
    PetCore::addPerHour(pet.fullness, PetCfg::FULLNESS_PER_HOUR);
  }
}
}
