#include "Display.h"
#include <U8g2lib.h>
#include "GameClock.h"
#include "Facts.h"
#include "Pet.h"
#include "Sprites.h"
#include "SpritesWonsaki.h"
#include "SpritesSaero.h"
#include "SpritesDoguri.h"
#include "DisplayGlyphs.h"
#include "DisplayTexts.h"
#include "PetConfig.h"

namespace Display {
namespace {

const uint8_t OLED_ADDRESS = 0x3C;
const unsigned long CHECK_MS = 100;
const unsigned long FRAME_MS = 500;
const unsigned long STATS_PAGE_MS = 10000;

const uint8_t SCREEN_W = 128;
const uint8_t TILE_COLUMNS = 16;
const uint8_t TILE_ROWS = 8;

const uint8_t PET_X = 48;
const uint8_t PET_Y = 13;
const uint8_t EFFECT_X = 84;
const uint8_t EFFECT_Y = 14;
const uint8_t BOTTOM_TEXT_TOP = 49;
const unsigned long NIGHT_SNACK_INTRO_MS = 1500;

const uint8_t PAGE_PET = 1;
const uint8_t PAGE_STATS = 2;

enum class Motion : uint8_t { Bob, Still, Shiver, Jump };

struct Look {
  const uint8_t* face;
  const uint8_t* effect;
  Motion motion;
};

struct Shown {
  uint8_t page;
  uint8_t frame;
  uint8_t hour;
  uint8_t minute;
  bool demo;
  bool pc;
  bool nightSnackIntro;
  PetView pet;
};

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

uint8_t page = PAGE_PET;
unsigned long pageStartMs = 0;
unsigned long lastCheckMs = 0;
uint8_t nextRowToSend = TILE_ROWS;
bool forceRedraw = true;
Shown shown;
bool showingNightSnack = false;
unsigned long nightSnackStartMs = 0;
char line[48];

void setText(const char* text) {
  strncpy_P(line, text, sizeof(line) - 1);
  line[sizeof(line) - 1] = 0;
}

void addText(const char* text) {
  strncat_P(line, text, sizeof(line) - 1 - strlen(line));
}

void addNumber(long number) {
  char digits[12];
  ltoa(number, digits, 10);
  strncat(line, digits, sizeof(line) - 1 - strlen(line));
}

uint16_t nextCode(const char*& p) {
  uint8_t first = *p++;
  if (first < 0x80) return first;
  if ((first & 0xE0) == 0xC0) {
    uint16_t code = (first & 0x1F) << 6;
    if (*p) code |= *p++ & 0x3F;
    return code;
  }
  uint16_t code = (uint16_t)(first & 0x0F) << 12;
  if (*p) code |= (uint16_t)(*p++ & 0x3F) << 6;
  if (*p) code |= *p++ & 0x3F;
  return code;
}

int8_t extraGlyphIndex(uint16_t code) {
  for (uint8_t i = 0; i < EXTRA_GLYPH_COUNT; i++) {
    if (pgm_read_word(&EXTRA_GLYPH_CODES[i]) == code) return i;
  }
  return -1;
}

uint8_t textWidth(const char* text) {
  uint8_t width = 0;
  while (*text) {
    width += nextCode(text) < 0x80 ? 8 : 16;
  }
  return width;
}

void drawText(int x, int top, const char* text) {
  oled.setFont(u8g2_font_unifont_t_korean1);
  int baseline = top + 13;
  while (*text) {
    uint16_t code = nextCode(text);
    int8_t extra = extraGlyphIndex(code);
    if (extra >= 0) {
      oled.drawXBMP(x, baseline - 14, 16, 16, EXTRA_GLYPHS[extra]);
      x += 16;
    } else {
      x += oled.drawGlyph(x, baseline, code);
    }
  }
}

void drawTextCentered(int top, const char* text) {
  int width = textWidth(text);
  int x = (SCREEN_W - width) / 2;
  if (x < 0) x = 0;
  drawText(x, top, text);
}

void drawTextRight(int right, int top, const char* text) {
  drawText(right - textWidth(text), top, text);
}

struct BodySprite {
  const uint8_t* bitmap;
  uint8_t faceY;
};

const BodySprite CHARACTER_BODIES[][5] PROGMEM = {
  {
    {SPRITE_WONSAKI_BABY, 14},
    {SPRITE_WONSAKI_CHILD, 13},
    {SPRITE_WONSAKI_HEALTHY, 13},
    {SPRITE_WONSAKI_WEAK, 11},
    {SPRITE_WONSAKI_OBESE, 13}
  },
  {
    {SPRITE_SAERO_BABY, 17},
    {SPRITE_SAERO_CHILD, 14},
    {SPRITE_SAERO_HEALTHY, 15},
    {SPRITE_SAERO_WEAK, 15},
    {SPRITE_SAERO_OBESE, 18}
  },
  {
    {SPRITE_DOGURI_BABY, DOGURI_FACE_Y_BABY},
    {SPRITE_DOGURI_CHILD, DOGURI_FACE_Y_CHILD},
    {SPRITE_DOGURI_HEALTHY, DOGURI_FACE_Y_HEALTHY},
    {SPRITE_DOGURI_WEAK, DOGURI_FACE_Y_WEAK},
    {SPRITE_DOGURI_OBESE, DOGURI_FACE_Y_OBESE}
  }
};

BodySprite bodyFor(const PetView& pet) {
  if (pet.stage < Stage::Baby || pet.stage > Stage::Obese ||
      pet.character < Character::Wonsaki || pet.character > Character::Doguri) {
    return {SPRITE_EGG, 0};
  }
  uint8_t character = static_cast<uint8_t>(pet.character) - static_cast<uint8_t>(Character::Wonsaki);
  uint8_t stage = static_cast<uint8_t>(pet.stage) - static_cast<uint8_t>(Stage::Baby);
  BodySprite body;
  memcpy_P(&body, &CHARACTER_BODIES[character][stage], sizeof(body));
  return body;
}

const char* characterName(Character character) {
  switch (character) {
    case Character::Wonsaki: return TXT_WONSAKI;
    case Character::Saero: return TXT_SAERO;
    case Character::Doguri: return TXT_DOGURI;
    default: return TXT_STAGE_EGG;
  }
}

bool hasFace(Stage stage) {
  return stage != Stage::Egg && stage != Stage::Dead;
}

Look lookFor(Mood mood) {
  switch (mood) {
    case Mood::Normal: return {FACE_NORMAL, nullptr, Motion::Bob};
    case Mood::Happy: return {FACE_HAPPY, FX_HEART, Motion::Jump};
    case Mood::Hungry: return {FACE_OPEN, FX_FOOD, Motion::Bob};
    case Mood::Sleepy: return {FACE_TIRED, nullptr, Motion::Still};
    case Mood::Sleeping: return {FACE_CLOSED, FX_ZZZ, Motion::Still};
    case Mood::SoreEyes: return {FACE_SORE, nullptr, Motion::Bob};
    case Mood::Stretch: return {FACE_NORMAL, FX_EXCL, Motion::Jump};
    case Mood::Stomachache: return {FACE_PAIN, FX_SWEAT, Motion::Shiver};
    case Mood::Indigestion: return {FACE_PAIN, FX_SWEAT, Motion::Still};
    case Mood::Full: return {FACE_HAPPY, nullptr, Motion::Still};
    case Mood::Thirsty: return {FACE_OPEN, FX_DROP, Motion::Bob};
    case Mood::Cold: return {FACE_SAD, FX_SNOW, Motion::Shiver};
    case Mood::Sick: return {FACE_SICK, FX_SNEEZE, Motion::Still};
    case Mood::Studying: return {FACE_NORMAL, FX_BOOK, Motion::Still};
    case Mood::Exercising: return {FACE_HAPPY, FX_SWEAT, Motion::Jump};
    case Mood::Dead: return {FACE_SICK, nullptr, Motion::Still};
  }
  return {FACE_NORMAL, nullptr, Motion::Bob};
}

const char* moodText(Mood mood) {
  switch (mood) {
    case Mood::Normal: return TXT_MOOD_NORMAL;
    case Mood::Happy: return TXT_MOOD_HAPPY;
    case Mood::Hungry: return TXT_MOOD_HUNGRY;
    case Mood::Sleepy: return TXT_MOOD_SLEEPY;
    case Mood::Sleeping: return TXT_MOOD_SLEEPING;
    case Mood::SoreEyes: return TXT_MOOD_SORE_EYES;
    case Mood::Stretch: return TXT_MOOD_STRETCH;
    case Mood::Stomachache: return TXT_MOOD_STOMACHACHE;
    case Mood::Indigestion: return TXT_MOOD_INDIGESTION;
    case Mood::Full: return TXT_MOOD_FULL;
    case Mood::Thirsty: return TXT_MOOD_THIRSTY;
    case Mood::Cold: return TXT_MOOD_COLD;
    case Mood::Sick: return TXT_MOOD_SICK;
    case Mood::Studying: return TXT_MOOD_STUDYING;
    case Mood::Exercising: return TXT_MOOD_EXERCISING;
    case Mood::Dead: return TXT_MOOD_DEAD;
  }
  return TXT_MOOD_NORMAL;
}

const char* stageText(Stage stage) {
  switch (stage) {
    case Stage::Egg: return TXT_STAGE_EGG;
    case Stage::Baby: return TXT_STAGE_BABY;
    case Stage::Child: return TXT_STAGE_CHILD;
    case Stage::Healthy: return TXT_STAGE_HEALTHY;
    case Stage::Weak: return TXT_STAGE_WEAK;
    case Stage::Obese: return TXT_STAGE_OBESE;
    case Stage::Dead: return TXT_STAGE_DEAD;
  }
  return TXT_STAGE_EGG;
}

void buildBottomText(const PetView& pet) {
  switch (pet.notice) {
    case Notice::GoodMorning:
      setText(TXT_GOOD_MORNING);
      addNumber(pet.noticeValue);
      addText(TXT_SCORE_UNIT);
      return;
    case Notice::ColdNight: setText(TXT_COLD_NIGHT); return;
    case Notice::NoisyNight: setText(TXT_NOISY_NIGHT); return;
    case Notice::ScreenBeforeSleep: setText(TXT_SCREEN_BEFORE_SLEEP); return;
    case Notice::StretchNow: setText(TXT_STRETCH_NOW); return;
    case Notice::DrinkWater: setText(TXT_DRINK_WATER); return;
    case Notice::PhoneDown: setText(TXT_PHONE_DOWN); return;
    case Notice::ExerciseResult:
      setText(TXT_HEART_RATE);
      if (PetCfg::EXERCISE_SHOW_RISE && pet.noticeValue >= 0) addText(TXT_PLUS);
      addNumber(pet.noticeValue);
      return;
    case Notice::MeasureAgain: setText(TXT_MEASURE_AGAIN); return;
    case Notice::NewGeneration:
      line[0] = 0;
      addNumber(pet.generation);
      addText(TXT_NEW_GENERATION);
      return;
    case Notice::Hatched:
      setText(characterName(static_cast<Character>(pet.noticeValue)));
      addText(TXT_HATCHED);
      return;
    case Notice::None: break;
  }
  if (pet.stage == Stage::Dead) setText(TXT_MOOD_DEAD);
  else setText(moodText(pet.mood));
}

void drawTopBar(const Shown& s) {
  oled.setFont(u8g2_font_6x10_tr);
  char clock[6];
  clock[0] = '0' + s.hour / 10;
  clock[1] = '0' + s.hour % 10;
  clock[2] = ':';
  clock[3] = '0' + s.minute / 10;
  clock[4] = '0' + s.minute % 10;
  clock[5] = 0;
  oled.drawStr(0, 8, clock);

  if (s.demo) {
    oled.drawBox(36, 0, 21, 9);
    oled.setDrawColor(0);
    oled.setFont(u8g2_font_4x6_tr);
    setText(TXT_DEMO_BADGE);
    oled.drawStr(39, 7, line);
    oled.setDrawColor(1);
  }

  if (s.pet.phoneMode) {
    oled.drawXBMP(SCREEN_W - ICON_PC_W - ICON_PHONE_W - 3, 0, ICON_PHONE_W, ICON_H, ICON_PHONE);
  }
  if (s.pc) {
    oled.drawXBMP(SCREEN_W - ICON_PC_W, 0, ICON_PC_W, ICON_H, ICON_PC);
  }
}

void drawNightSnack() {
  oled.drawDisc(EFFECT_X + 5, EFFECT_Y + 5, 5);
  oled.setDrawColor(0);
  oled.drawDisc(EFFECT_X + 8, EFFECT_Y + 3, 5);
  oled.setDrawColor(1);
  oled.drawXBMP(EFFECT_X, EFFECT_Y + 14, FX_W, FX_H, FX_FOOD);
}

void drawPet(const Shown& s) {
  const PetView& pet = s.pet;
  Look look = lookFor(pet.mood);
  if (s.nightSnackIntro) look = {FACE_OPEN, nullptr, Motion::Still};
  if (pet.stage == Stage::Dead) {
    oled.drawXBMP(PET_X, PET_Y, SPRITE_W, SPRITE_H, SPRITE_TOMB);
    return;
  }

  int x = PET_X;
  int y = PET_Y;
  if (pet.stage == Stage::Egg) {
    if (look.motion != Motion::Still && s.frame == 1) x += 1;
  } else if (look.motion == Motion::Bob) {
    y += s.frame;
  } else if (look.motion == Motion::Jump) {
    y -= s.frame * 3;
  } else if (look.motion == Motion::Shiver) {
    x += s.frame == 1 ? 1 : -1;
  }

  BodySprite body = bodyFor(pet);
  oled.drawXBMP(x, y, SPRITE_W, SPRITE_H, body.bitmap);

  if (hasFace(pet.stage)) {
    oled.setDrawColor(2);
    oled.drawXBMP(x + FACE_X, y + body.faceY, FACE_W, FACE_H, look.face);
    oled.setDrawColor(1);
  }

  if (s.nightSnackIntro) drawNightSnack();

  if (look.effect != nullptr) {
    oled.drawXBMP(EFFECT_X, EFFECT_Y - s.frame * 2, FX_W, FX_H, look.effect);
  }
}

void drawPetPage(const Shown& s) {
  drawTopBar(s);
  drawPet(s);
  if (s.pet.notice == Notice::Hatched) buildBottomText(s.pet);
  else if (s.nightSnackIntro) setText(TXT_NIGHT_SNACK);
  else if (s.pet.mood == Mood::Indigestion) setText(TXT_MOOD_INDIGESTION);
  else buildBottomText(s.pet);
  drawTextCentered(BOTTOM_TEXT_TOP, line);
}

uint8_t limitPercent(uint8_t value) {
  return value > 100 ? 100 : value;
}

void drawStat(int x, int width, uint8_t value, const char* label) {
  oled.drawFrame(x, 34, width, 10);
  oled.drawBox(x + 1, 35, (width - 2) * limitPercent(value) / 100, 8);
  setText(label);
  drawText(x + (width - textWidth(line)) / 2, BOTTOM_TEXT_TOP, line);
}

void drawStatsPage(const Shown& s) {
  const PetView& pet = s.pet;

  setText(stageText(pet.stage));
  drawText(0, 0, line);

  line[0] = 0;
  addNumber(pet.generation);
  addText(TXT_GENERATION_UNIT);
  drawTextRight(SCREEN_W, 0, line);

  if (pet.character != Character::Unhatched) {
    setText(characterName(pet.character));
    drawText(0, 16, line);
  }
  setText(TXT_EXP_SHORT);
  addNumber(pet.exp);
  drawTextRight(SCREEN_W, 16, line);

  drawStat(0, 48, pet.fullness, TXT_FULLNESS);
  drawStat(56, 32, pet.rest, TXT_REST);
  drawStat(96, 32, pet.stamina, TXT_STAMINA);
}

Shown readState() {
  Shown s;
  memset(&s, 0, sizeof(s));
  unsigned long now = millis();
  s.page = page;
  s.frame = page == PAGE_PET ? (now / FRAME_MS) % 2 : 0;
  s.hour = GameClock::hour();
  s.minute = GameClock::minute();
  s.demo = GameClock::demoMode();
  s.pc = facts.pcConnected;
  s.pet = Pet::view();
  bool nightSnack = s.pet.mood == Mood::Indigestion && hasFace(s.pet.stage);
  if (nightSnack && !showingNightSnack) nightSnackStartMs = now;
  showingNightSnack = nightSnack;
  s.nightSnackIntro = page == PAGE_PET && nightSnack &&
                      now - nightSnackStartMs < NIGHT_SNACK_INTRO_MS;
  return s;
}

void drawScreen(const Shown& s) {
  oled.clearBuffer();
  if (s.page == PAGE_STATS) drawStatsPage(s);
  else drawPetPage(s);
}

}

void setup() {
  oled.setI2CAddress(OLED_ADDRESS * 2);
  oled.begin();
  oled.setFontMode(1);
  oled.setBitmapMode(1);
  oled.setFontPosBaseline();
  page = PAGE_PET;
  forceRedraw = true;
}

void update() {
  if (nextRowToSend < TILE_ROWS) {
    oled.updateDisplayArea(0, nextRowToSend, TILE_COLUMNS, 1);
    nextRowToSend++;
    return;
  }

  unsigned long now = millis();
  if (now - lastCheckMs < CHECK_MS) return;
  lastCheckMs = now;

  if (page == PAGE_STATS && now - pageStartMs >= STATS_PAGE_MS) {
    page = PAGE_PET;
  }

  Shown s = readState();
  if (!forceRedraw && memcmp(&s, &shown, sizeof(s)) == 0) return;
  shown = s;
  forceRedraw = false;

  drawScreen(s);
  nextRowToSend = 0;
}

void nextPage() {
  page = page == PAGE_PET ? PAGE_STATS : PAGE_PET;
  pageStartMs = millis();
  forceRedraw = true;
}

}
