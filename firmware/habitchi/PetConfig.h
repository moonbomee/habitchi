#pragma once
#include <Arduino.h>
#include "Timing.h"

namespace PetCfg {
constexpr bool DEATH_ENABLED = true;

constexpr uint16_t BEDTIME_MINUTE = 23 * 60;
constexpr uint16_t WAKE_MINUTE = 7 * 60;
constexpr uint16_t LIGHTS_OFF_EARLY_MINUTES = 60;

constexpr uint8_t START_FULLNESS = 80;
constexpr uint8_t START_REST = 80;
constexpr uint8_t START_STAMINA = 80;

constexpr uint32_t MAX_CLOCK_STEP_SECONDS = 3UL * 60UL * 60UL;

constexpr int16_t REST_SLEEP_PER_HOUR = 12;
constexpr int16_t REST_NOISE_CUT_PER_HOUR = 2;
constexpr int16_t REST_COLD_CUT_PER_HOUR = 6;
constexpr int16_t REST_SLEEP_MIN_PER_HOUR = 2;
constexpr int16_t REST_AWAKE_PER_HOUR = -3;
constexpr int16_t REST_AWAKE_LATE_PER_HOUR = -4;
constexpr int16_t REST_ALL_NIGHTER_PER_HOUR = -6;
constexpr int16_t REST_GLOW_PENALTY = -5;
constexpr int16_t REST_SNACK_PENALTY = -10;

constexpr int16_t STAMINA_SLEEP_PER_HOUR = 6;
constexpr int16_t STAMINA_ALL_NIGHTER_PER_HOUR = -6;
constexpr int16_t STAMINA_OVERWORK_PER_HOUR = -20;
constexpr int16_t STAMINA_COLD_SICK_PER_HOUR = -3;

constexpr int16_t FULLNESS_PER_HOUR = -4;
constexpr uint16_t PC_OFF_PAUSE_MINUTES = 60;

constexpr float COLD_BELOW_C = 18.0f;
constexpr float COLD_RECOVER_C = 18.5f;
constexpr Timing COLD_AFTER{10UL * 60UL * 1000UL, 3UL * 1000UL};

constexpr Timing FALL_ASLEEP_NO_TYPING{5UL * 60UL * 1000UL, 5UL * 1000UL};
constexpr unsigned long ALARM_MAX_MS = 2UL * 60UL * 1000UL;
constexpr uint16_t TEMP_LOG_EVERY_MINUTES = 30;

constexpr uint16_t SLEEP_TARGET_MINUTES = 7 * 60;
constexpr uint8_t SCORE_SHORT_STEP_MINUTES = 12;
constexpr uint8_t SCORE_SHORT_MAX = 40;
constexpr uint8_t SCORE_LATE_STEP_MINUTES = 6;
constexpr uint8_t SCORE_LATE_MAX = 25;
constexpr uint8_t NOISY_NIGHT_NOISES = 1;
constexpr uint8_t SCORE_PER_NOISE = 5;
constexpr uint8_t SCORE_NOISE_MAX = 20;
constexpr uint8_t SCORE_COLD = 15;
constexpr uint8_t SCORE_GLOW = 10;
constexpr uint8_t SCORE_PER_SNACK = 10;
constexpr uint8_t SCORE_SNACK_MAX = 20;

constexpr uint8_t FOOD_GOOD = 25;
constexpr uint8_t FOOD_JUNK = 20;
constexpr uint8_t FOOD_BROKEN = 15;
constexpr uint8_t EXP_GOOD = 10;
constexpr uint8_t EXP_JUNK = 3;
constexpr uint8_t EXP_BROKEN = 2;
constexpr uint8_t OVEREAT_ABOVE = 90;
constexpr Timing OVEREAT_GAP{10UL * 60UL * 1000UL, 5UL * 1000UL};

constexpr Timing ALL_NIGHTER_HOLD{10UL * 60UL * 1000UL, 10UL * 1000UL};
constexpr Timing DISTRACTED_FOR{10UL * 60UL * 1000UL, 10UL * 1000UL};
constexpr Timing THIRSTY_AFTER{60UL * 60UL * 1000UL, 20UL * 1000UL};

constexpr unsigned long HEART_WAIT_MS = 15UL * 1000UL;
constexpr Timing EXERCISE_MAX{60UL * 60UL * 1000UL, 3UL * 60UL * 1000UL};
constexpr int16_t EXERCISE_GOOD_RISE = 15;
constexpr int16_t EXERCISE_STAMINA_GAIN = 30;
constexpr bool EXERCISE_SHOW_RISE = true;

constexpr uint8_t HUNGRY_BELOW = 20;
constexpr uint16_t EGG_HATCH_MINUTES = 60;
constexpr uint16_t BABY_GROW_MINUTES = 24 * 60;
constexpr uint16_t CHILD_GROW_MINUTES = 3 * 24 * 60;
constexpr uint8_t OBESE_MIN_COMMITS = 3;
constexpr uint8_t HEALTHY_SLEEP_SCORE = 70;

constexpr uint8_t COLD_NIGHTS_FOR_SICK = 2;
constexpr uint16_t WARM_SLEEP_CURES_COLD_MINUTES = 3 * 60;
constexpr uint16_t ZERO_MINUTES_FOR_SICK = 6 * 60;
constexpr uint8_t SICK_RECOVER_ABOVE = 30;
constexpr uint16_t SICK_MINUTES_FOR_DEATH = 24 * 60;
constexpr unsigned long TOMB_MS = 6000;

constexpr unsigned long REACTION_MS = 4000;
constexpr unsigned long NOTICE_MS = 4000;

constexpr int SAVE_ADDRESS = 0;
constexpr uint16_t SAVE_MAGIC = 0x4842;
constexpr uint8_t SAVE_VERSION = 2;
constexpr Timing SAVE_EVERY{10UL * 60UL * 1000UL, 60UL * 1000UL};
constexpr uint32_t NIGHT_MAX_AGE_SECONDS = 16UL * 60UL * 60UL;
}
