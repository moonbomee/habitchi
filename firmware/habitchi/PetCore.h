#pragma once
#include <Arduino.h>
#include "PetView.h"

struct PetSave {
  Stage stage;
  uint8_t generation;
  uint8_t lastLifespanDays;
  uint16_t fullness;
  uint16_t rest;
  uint16_t stamina;
  uint16_t exp;
  uint32_t ageMinutes;
  uint32_t stageMinutes;
  uint8_t sickCause;
  uint16_t sickMinutes;
  uint16_t restZeroMinutes;
  uint16_t fullnessZeroMinutes;
  uint16_t staminaZeroMinutes;
  uint16_t commits;
  uint16_t junkCommits;
  uint16_t lateCommits;
  uint16_t sleepScoreSum;
  uint16_t sleepNights;
  uint8_t lastSleepScore;
  uint8_t coldNightsInRow;
  bool hadColdSickness;
  bool nightStarted;
  uint32_t nightStartTime;
  uint16_t nightFirstOffMinute;
  uint16_t nightSleptMinutes;
  uint8_t nightNoises;
  uint8_t nightSnacks;
  bool nightCold;
  bool nightGlow;
  bool phoneMode;
  Character character;
};

extern PetSave pet;

namespace PetCore {
constexpr uint16_t STAT_SCALE = 60;

void resetPet(uint8_t generation, uint8_t lastLifespanDays);

uint8_t percent(uint16_t stat);
void addPoints(uint16_t& stat, int16_t points);
void addPerHour(uint16_t& stat, int16_t pointsPerHour);
void gainExp(uint8_t base, uint8_t halvings);

bool typingActive();
uint16_t minuteOfDay();
bool between(uint16_t minute, uint16_t start, uint16_t end);
bool inSleepHours();
bool inSleepWindow();
long gameMinutesFromMs(unsigned long ms);

void react(Mood mood);
bool reacting(Mood mood);
void clearReactions();

void notify(Notice notice, int16_t value);
void updateNotices();
Notice currentNotice();
int16_t currentNoticeValue();
void clearNotices();
}
