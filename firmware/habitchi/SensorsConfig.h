#pragma once
#include <Arduino.h>

namespace SensorCfg {
constexpr unsigned long LIGHT_INTERVAL_MS = 100;
constexpr int LIGHT_SMOOTH_DIV = 4;
constexpr int LIGHT_DARK_BELOW = 200;
constexpr int LIGHT_BRIGHT_ABOVE = 300;

constexpr uint8_t SOUND_BURST_SAMPLES = 8;
constexpr unsigned long SOUND_WINDOW_MS = 50;
constexpr int SOUND_QUIET_P2P = 10;
constexpr int SOUND_MAX_P2P = 400;
constexpr uint8_t SOUND_RELEASE_PER_WINDOW = 1;
constexpr uint8_t SOUND_LOUD_LEVEL = 50;
constexpr unsigned long SOUND_EVENT_GAP_MS = 1500;
constexpr unsigned long SOUND_AFTER_BUSY_MS = 300;

constexpr unsigned long TEMP_INTERVAL_MS = 500;
constexpr uint8_t TEMP_SAMPLES = 16;
constexpr float TEMP_ADC_REF_MV = 5000.0f;
constexpr float TEMP_SMOOTHING = 0.5f;
constexpr float TEMP_VALID_MIN_C = 1.0f;
constexpr float TEMP_VALID_MAX_C = 60.0f;

constexpr unsigned long BLUE_INTERVAL_MS = 1000;
constexpr unsigned long BLUE_TIMEOUT_US = 900000UL;
constexpr unsigned long BLUE_GLOW_ON_US = 300000UL;
constexpr unsigned long BLUE_GLOW_OFF_US = 500000UL;

constexpr unsigned long FSR_INTERVAL_MS = 50;
constexpr int FSR_SMOOTH_DIV = 2;
constexpr bool FSR_HIGHER_MEANS_PRESSED = true;
constexpr int FSR_ON_ABOVE = 300;
constexpr int FSR_OFF_BELOW = 200;
constexpr unsigned long FSR_DEBOUNCE_MS = 300;

constexpr uint8_t PIEZO_BURST_SAMPLES = 8;
constexpr int PIEZO_SPIKE_RAW = 20;
constexpr unsigned long PIEZO_SPIKE_GAP_MS = 25;
constexpr unsigned long PIEZO_WINDOW_MS = 800;
constexpr uint8_t PIEZO_MIN_SPIKES = 6;
constexpr unsigned long PIEZO_BUZZ_GAP_MS = 4000;
constexpr unsigned long PIEZO_IGNORE_AFTER_FSR_MS = 1500;
constexpr unsigned long PIEZO_AFTER_BUSY_MS = 300;

constexpr unsigned long WATER_INTERVAL_MS = 500;
constexpr uint8_t WATER_SAMPLES = 10;
constexpr unsigned long WATER_TIMEOUT_MS = 10;
constexpr uint8_t WATER_STABLE_COUNT = 4;
constexpr long WATER_STABLE_SPREAD = 30;
constexpr long WATER_PRESENT_ABOVE = 150;
constexpr long WATER_ABSENT_BELOW = 100;
constexpr long WATER_EMPTY_RAW = 200;
constexpr long WATER_FULL_RAW = 1000;
constexpr long WATER_TOUCH_RAW = 3000;
constexpr uint8_t WATER_DRANK_MIN_DROP = 5;

constexpr unsigned long PULSE_SAMPLE_US = 2000;
constexpr int PULSE_MIN_AMPLITUDE = 20;
constexpr int PULSE_FALL_MARGIN = 5;
constexpr unsigned long PULSE_MIN_IBI_MS = 300;
constexpr unsigned long PULSE_MAX_IBI_MS = 2000;
constexpr uint8_t PULSE_AVG_BEATS = 5;
constexpr uint8_t PULSE_STABLE_PERCENT = 20;
}
