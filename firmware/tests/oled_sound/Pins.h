#pragma once
#include <Arduino.h>

namespace Pins {
constexpr uint8_t SOUND = A0;
constexpr uint8_t LM35 = A1;
constexpr uint8_t LIGHT = A2;
constexpr uint8_t SPEAKER = A3;
constexpr uint8_t FSR = A4;
constexpr uint8_t PIEZO_SENSE = A5;
constexpr uint8_t PULSE = A6;
constexpr uint8_t BUTTON_A = 24;
constexpr uint8_t BUTTON_B = 25;
constexpr uint8_t BUTTON_C = 26;
constexpr uint8_t BLUE_LED_ANODE = 30;
constexpr uint8_t BLUE_LED_CATHODE = 31;
constexpr uint8_t WATER_SEND = 32;
constexpr uint8_t WATER_RECEIVE = 33;
}
