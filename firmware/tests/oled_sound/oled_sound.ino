#include <U8g2lib.h>
#include "Sound.h"
#include "Sprites.h"

const uint8_t OLED_ADDRESS = 0x3C;
const unsigned long STEP_MS = 5000;
const uint8_t SOUND_COUNT = 10;
const uint8_t SPRITE_COUNT = 7;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

const uint8_t* const SPRITES[SPRITE_COUNT] = {
  SPRITE_EGG, SPRITE_BABY, SPRITE_CHILD, SPRITE_HEALTHY, SPRITE_WEAK, SPRITE_OBESE, SPRITE_TOMB
};

const char* const SOUND_NAMES[SOUND_COUNT] = {
  "Click", "Alarm", "Warning", "Eat", "Happy", "Sneeze", "Pain", "Hatch", "Evolve", "Death"
};

uint8_t step = 0;
unsigned long stepStartMs = 0;

void drawStep() {
  oled.clearBuffer();
  oled.setFont(u8g2_font_6x10_tr);
  oled.drawStr(0, 8, SOUND_NAMES[step]);
  oled.drawXBMP(48, 12, SPRITE_W, SPRITE_H, SPRITES[step % SPRITE_COUNT]);
  oled.setFont(u8g2_font_unifont_t_korean1);
  oled.drawUTF8(28, 62, "소리 시험");
  oled.sendBuffer();
}

void setup() {
  oled.setI2CAddress(OLED_ADDRESS * 2);
  oled.begin();
  oled.setBitmapMode(1);
  Sound::setup();
  drawStep();
  Sound::play((SoundId)step);
  stepStartMs = millis();
}

void loop() {
  Sound::update();
  if (millis() - stepStartMs < STEP_MS) return;
  stepStartMs = millis();
  step = (step + 1) % SOUND_COUNT;
  drawStep();
  Sound::play((SoundId)step);
}
