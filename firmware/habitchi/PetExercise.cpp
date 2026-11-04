#include "PetExercise.h"
#include "PetCore.h"
#include "PetConfig.h"
#include "Facts.h"
#include "PcLink.h"
#include "Sound.h"

namespace PetExercise {
namespace {
enum class Step : uint8_t { Idle, MeasureBefore, Moving, MeasureAfter };

Step step = Step::Idle;
unsigned long stepStartMs = 0;
int16_t bpmBefore = 0;

void goTo(Step next) {
  step = next;
  stepStartMs = millis();
}

void finish(int16_t bpmAfter) {
  int16_t rise = bpmAfter - bpmBefore;
  if (rise >= PetCfg::EXERCISE_GOOD_RISE) {
    PetCore::addPoints(pet.stamina, PetCfg::EXERCISE_STAMINA_GAIN);
    PetCore::react(Mood::Happy);
    Sound::play(SoundId::Happy);
  }
  PetCore::notify(Notice::ExerciseResult, PetCfg::EXERCISE_SHOW_RISE ? rise : bpmAfter);
  goTo(Step::Idle);
}
}

void reset() {
  step = Step::Idle;
}

void update() {
  unsigned long waited = millis() - stepStartMs;
  if (step == Step::MeasureBefore || step == Step::MeasureAfter) {
    if (facts.heartValid) {
      int16_t bpm = facts.heartBpm;
      PcLink::log("HEART", bpm);
      if (step == Step::MeasureBefore) {
        bpmBefore = bpm;
        goTo(Step::Moving);
      } else {
        finish(bpm);
      }
    } else if (waited >= PetCfg::HEART_WAIT_MS) {
      PetCore::notify(Notice::MeasureAgain, 0);
      goTo(Step::Idle);
    }
  } else if (step == Step::Moving && waited >= pick(PetCfg::EXERCISE_MAX)) {
    goTo(Step::Idle);
  }
}

void onButton() {
  if (step == Step::Idle) goTo(Step::MeasureBefore);
  else if (step == Step::Moving) goTo(Step::MeasureAfter);
}

bool active() {
  return step != Step::Idle;
}
}
