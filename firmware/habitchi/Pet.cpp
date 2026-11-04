#include "Pet.h"
#include "PetCore.h"
#include "PetConfig.h"
#include "PetSleep.h"
#include "PetCoding.h"
#include "PetCommit.h"
#include "PetExercise.h"
#include "PetLife.h"
#include "Storage.h"
#include "Events.h"
#include "GameClock.h"

namespace Pet {
namespace {
PetView current = {};
uint32_t lastGameTime = 0;
uint32_t secondsWaiting = 0;
bool wasDead = false;

bool dead() {
  return pet.stage == Stage::Dead;
}

void runMinuteTicks() {
  uint32_t now = GameClock::now();
  if (now > lastGameTime && now - lastGameTime <= PetCfg::MAX_CLOCK_STEP_SECONDS) {
    secondsWaiting += now - lastGameTime;
  }
  lastGameTime = now;
  while (secondsWaiting >= 60) {
    secondsWaiting -= 60;
    if (dead()) continue;
    PetSleep::minuteTick();
    PetCoding::minuteTick();
    PetCommit::minuteTick();
    PetLife::minuteTick();
  }
}

void handleDemo(const Event& e) {
  if (e.type == Ev::DemoStage) PetLife::demoStage(e.value);
  else if (e.type == Ev::DemoSick) PetLife::demoSick(e.value);
  else if (e.type == Ev::DemoDie) PetLife::die();
  else if (e.type == Ev::DemoEvolve && !dead()) PetLife::evolveAdult();
}

void handleEvent(const Event& e) {
  switch (e.type) {
    case Ev::CommitGood:
    case Ev::CommitJunk:
    case Ev::CommitBroken:
      PetCommit::onCommit(e.type);
      return;
    case Ev::DemoStage:
    case Ev::DemoSick:
    case Ev::DemoDie:
    case Ev::DemoEvolve:
      handleDemo(e);
      return;
    default:
      break;
  }
  if (dead()) return;
  switch (e.type) {
    case Ev::LoudNoise: PetSleep::onLoudNoise(); break;
    case Ev::PhoneBuzz: PetCoding::onPhoneBuzz(); break;
    case Ev::PhoneLifted: PetCoding::onPhoneLifted(); break;
    case Ev::PhonePlaced: PetCoding::onPhonePlaced(); break;
    case Ev::Drank: PetCoding::onDrank(e.value); break;
    case Ev::ExerciseButton: PetExercise::onButton(); break;
    case Ev::PhoneModeButton: PetCoding::onPhoneModeButton(); break;
    default: break;
  }
}

void resetModules() {
  PetSleep::reset();
  PetCoding::reset();
  PetExercise::reset();
}

Mood chooseMood() {
  if (dead()) return Mood::Dead;
  if (pet.sickCause != 0) return Mood::Sick;
  if (PetCore::reacting(Mood::Cold)) return Mood::Cold;
  if (PetCore::reacting(Mood::Stomachache)) return Mood::Stomachache;
  if (PetCore::reacting(Mood::Indigestion)) return Mood::Indigestion;
  if (PetSleep::sleeping()) return Mood::Sleeping;
  if (PetCoding::stretchDue()) return Mood::Stretch;
  if (PetCoding::thirsty()) return Mood::Thirsty;
  if (PetCore::reacting(Mood::SoreEyes)) return Mood::SoreEyes;
  if (PetCoding::allNighter()) return Mood::Sleepy;
  if (PetCore::percent(pet.fullness) < PetCfg::HUNGRY_BELOW) return Mood::Hungry;
  if (PetCoding::studying()) return Mood::Studying;
  if (PetExercise::active()) return Mood::Exercising;
  if (PetCore::reacting(Mood::Full)) return Mood::Full;
  if (PetCore::reacting(Mood::Happy)) return Mood::Happy;
  return Mood::Normal;
}

void buildView() {
  current.stage = pet.stage;
  current.character = pet.character;
  current.mood = chooseMood();
  current.notice = PetCore::currentNotice();
  current.noticeValue = PetCore::currentNoticeValue();
  current.fullness = PetCore::percent(pet.fullness);
  current.rest = PetCore::percent(pet.rest);
  current.stamina = PetCore::percent(pet.stamina);
  current.exp = pet.exp;
  current.generation = pet.generation;
  current.sleepScore = pet.lastSleepScore;
  current.phoneMode = PetCoding::phoneMode();
}
}

void setup() {
  if (!Storage::load()) {
    PetCore::resetPet(1, 0);
    Storage::save();
  }
  PetSleep::setup();
  PetLife::setup();
  lastGameTime = GameClock::now();
  wasDead = dead();
  buildView();
}

void update() {
  runMinuteTicks();

  Event e;
  while (eventPop(e)) handleEvent(e);

  if (dead() && !wasDead) resetModules();
  wasDead = dead();

  if (!dead()) {
    PetSleep::update();
    PetCoding::update();
    PetExercise::update();
  }
  PetLife::update();
  PetCore::updateNotices();
  buildView();
  Storage::update();
}

const PetView& view() { return current; }
}
