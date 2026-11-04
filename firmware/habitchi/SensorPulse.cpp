#include "SensorPulse.h"
#include "Pins.h"
#include "Facts.h"
#include "SensorsConfig.h"

namespace SensorPulse {
namespace {
unsigned long lastSampleUs = 0;
int threshold = 512;
int peak = 512;
int trough = 512;
bool inBeat = false;
bool hasBeat = false;
unsigned long lastBeatMs = 0;
unsigned long pendingInterval = 0;

unsigned long intervals[SensorCfg::PULSE_AVG_BEATS];
uint8_t intervalCount = 0;
uint8_t nextSlot = 0;
bool hasGoodBeat = false;
unsigned long lastGoodBeatMs = 0;

void clearIntervals() {
  intervalCount = 0;
  nextSlot = 0;
}

void restartDetector(int signal, unsigned long now) {
  threshold = signal;
  peak = signal;
  trough = signal;
  inBeat = false;
  hasBeat = false;
  lastBeatMs = now;
  clearIntervals();
}

void addInterval(unsigned long interval, unsigned long now) {
  intervals[nextSlot] = interval;
  nextSlot = (nextSlot + 1) % SensorCfg::PULSE_AVG_BEATS;
  if (intervalCount < SensorCfg::PULSE_AVG_BEATS) intervalCount++;
  hasGoodBeat = true;
  lastGoodBeatMs = now;
}

void onRise(unsigned long now) {
  inBeat = true;
  pendingInterval = hasBeat ? now - lastBeatMs : 0;
  hasBeat = true;
  lastBeatMs = now;
}

void onFall(unsigned long now) {
  inBeat = false;
  int amplitude = peak - trough;
  threshold = trough + amplitude / 2;
  peak = threshold;
  trough = threshold;
  if (amplitude < SensorCfg::PULSE_MIN_AMPLITUDE) {
    hasBeat = false;
    clearIntervals();
    return;
  }
  if (pendingInterval > 0) addInterval(pendingInterval, now);
}

void processSample(int signal, unsigned long now) {
  unsigned long sinceBeat = now - lastBeatMs;
  if (sinceBeat > SensorCfg::PULSE_MAX_IBI_MS) {
    restartDetector(signal, now);
    return;
  }
  if (signal < threshold && signal < trough) trough = signal;
  if (signal > threshold && signal > peak) peak = signal;
  if (!inBeat && signal > threshold && sinceBeat > SensorCfg::PULSE_MIN_IBI_MS) onRise(now);
  if (inBeat && signal < threshold - SensorCfg::PULSE_FALL_MARGIN) onFall(now);
}

unsigned long averageInterval() {
  unsigned long sum = 0;
  for (uint8_t i = 0; i < intervalCount; i++) sum += intervals[i];
  return sum / intervalCount;
}

bool intervalsAreSteady(unsigned long average) {
  if (intervalCount < SensorCfg::PULSE_AVG_BEATS) return false;
  unsigned long allowed = average * SensorCfg::PULSE_STABLE_PERCENT / 100;
  for (uint8_t i = 0; i < intervalCount; i++) {
    unsigned long diff = intervals[i] > average ? intervals[i] - average : average - intervals[i];
    if (diff > allowed) return false;
  }
  return true;
}

void publish(unsigned long now) {
  facts.fingerOn = hasGoodBeat && now - lastGoodBeatMs < SensorCfg::PULSE_MAX_IBI_MS;
  if (!facts.fingerOn || intervalCount == 0) {
    facts.heartBpm = 0;
    facts.heartValid = false;
    return;
  }
  unsigned long average = averageInterval();
  facts.heartBpm = 60000UL / average;
  facts.heartValid = intervalsAreSteady(average);
}
}

void setup() {
  facts.fingerOn = false;
  facts.heartBpm = 0;
  facts.heartValid = false;
  lastSampleUs = micros();
  lastBeatMs = millis();
}

void update() {
  unsigned long nowUs = micros();
  if (nowUs - lastSampleUs < SensorCfg::PULSE_SAMPLE_US) return;
  lastSampleUs = nowUs;
  unsigned long now = millis();
  processSample(analogRead(Pins::PULSE), now);
  publish(now);
}
}
