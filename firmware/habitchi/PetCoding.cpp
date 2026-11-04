#include "PetCoding.h"
#include "PetCore.h"
#include "PetConfig.h"
#include "Facts.h"
#include "PcLink.h"
#include "Sound.h"
#include "Storage.h"

namespace PetCoding {
namespace {
bool inSession = false;
unsigned long sessionStartMs = 0;
bool stretchOn = false;

bool bottleSeen = false;
bool thirstyOn = false;
unsigned long thirstClockMs = 0;

bool allNighterOn = false;
unsigned long allNighterMs = 0;

bool buzzOn = false;
unsigned long buzzMs = 0;
bool liftedOn = false;
unsigned long liftedMs = 0;
bool distractedOn = false;
unsigned long distractedMs = 0;

void becomeDistracted() {
  distractedOn = true;
  distractedMs = millis();
  PetCore::notify(Notice::PhoneDown, 0);
}

void updateSession(bool typing) {
  unsigned long now = millis();
  if (typing && !inSession) {
    inSession = true;
    sessionStartMs = now;
    stretchOn = false;
    thirstyOn = false;
    thirstClockMs = now;
  }
  if (!inSession) return;

  if (now - facts.lastTypingMs >= pick(Cfg::BREAK_AFTER)) {
    inSession = false;
    stretchOn = false;
    thirstyOn = false;
    return;
  }
  if (!stretchOn && now - sessionStartMs >= pick(Cfg::STRETCH_AFTER)) {
    stretchOn = true;
    PetCore::notify(Notice::StretchNow, 0);
    Sound::play(SoundId::Warning);
    PcLink::log("STRETCH", PetCore::gameMinutesFromMs(now - sessionStartMs));
  }
  if (bottleSeen && !thirstyOn && now - thirstClockMs >= pick(PetCfg::THIRSTY_AFTER)) {
    thirstyOn = true;
    PetCore::notify(Notice::DrinkWater, 0);
  }
}

void updateTimers() {
  unsigned long now = millis();
  if (allNighterOn && now - allNighterMs >= pick(PetCfg::ALL_NIGHTER_HOLD)) allNighterOn = false;
  if (distractedOn && now - distractedMs >= pick(PetCfg::DISTRACTED_FOR)) distractedOn = false;
  if (buzzOn && now - buzzMs >= pick(Cfg::PHONE_GRACE)) buzzOn = false;
  if (liftedOn && now - liftedMs >= pick(Cfg::PHONE_GRACE)) {
    liftedOn = false;
    becomeDistracted();
  }
}
}

void reset() {
  inSession = false;
  stretchOn = false;
  thirstyOn = false;
  allNighterOn = false;
  pet.phoneMode = false;
  buzzOn = false;
  liftedOn = false;
  distractedOn = false;
}

void update() {
  bool typing = PetCore::typingActive();
  if (facts.bottlePresent) bottleSeen = true;
  updateSession(typing);
  if (typing && facts.roomDark && !PetCore::inSleepHours()) PetCore::react(Mood::SoreEyes);
  if (typing && PetCore::inSleepHours()) markAllNighter();
  updateTimers();
}

void minuteTick() {
  if (allNighterOn) {
    PetCore::addPerHour(pet.rest, PetCfg::REST_ALL_NIGHTER_PER_HOUR);
    PetCore::addPerHour(pet.stamina, PetCfg::STAMINA_ALL_NIGHTER_PER_HOUR);
  }
  if (inSession && stretchOn) {
    PetCore::addPerHour(pet.stamina, PetCfg::STAMINA_OVERWORK_PER_HOUR);
  }
}

void markAllNighter() {
  allNighterOn = true;
  allNighterMs = millis();
}

void onPhoneModeButton() {
  if (pet.phoneMode) {
    pet.phoneMode = false;
    buzzOn = false;
    liftedOn = false;
    Storage::requestSave();
  } else if (facts.phonePresent) {
    pet.phoneMode = true;
    PcLink::log("PHONE", 1);
    Storage::requestSave();
  } else {
    PetCore::notify(Notice::PhoneDown, 0);
  }
}

void onPhoneBuzz() {
  if (!pet.phoneMode) return;
  buzzOn = true;
  buzzMs = millis();
}

void onPhoneLifted() {
  if (!pet.phoneMode) return;
  PcLink::log("PHONE", 0);
  if (buzzOn) {
    buzzOn = false;
    liftedOn = true;
    liftedMs = millis();
  } else {
    becomeDistracted();
  }
}

void onPhonePlaced() {
  if (!pet.phoneMode) return;
  PcLink::log("PHONE", 1);
  liftedOn = false;
}

void onDrank(int16_t dropPercent) {
  PcLink::log("WATER", dropPercent);
  bottleSeen = true;
  thirstyOn = false;
  thirstClockMs = millis();
}

bool stretchDue() {
  return inSession && stretchOn;
}

bool thirsty() {
  return inSession && thirstyOn;
}

bool allNighter() {
  return allNighterOn;
}

bool distracted() {
  return distractedOn;
}

bool studying() {
  return pet.phoneMode && facts.phonePresent;
}

bool phoneMode() {
  return pet.phoneMode;
}
}
