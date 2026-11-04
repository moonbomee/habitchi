#include "Storage.h"
#include <EEPROM.h>
#include "PetCore.h"
#include "PetConfig.h"

namespace Storage {
namespace {
struct SaveBlock {
  uint16_t magic;
  uint8_t version;
  PetSave data;
};

bool saveWanted = false;
unsigned long lastSaveMs = 0;
}

bool load() {
  SaveBlock block;
  EEPROM.get(PetCfg::SAVE_ADDRESS, block);
  if (block.magic != PetCfg::SAVE_MAGIC) return false;
  if (block.version != PetCfg::SAVE_VERSION) return false;
  if (block.data.stage > Stage::Dead) return false;
  if (block.data.generation == 0) return false;
  pet = block.data;
  return true;
}

void save() {
  SaveBlock block;
  block.magic = PetCfg::SAVE_MAGIC;
  block.version = PetCfg::SAVE_VERSION;
  block.data = pet;
  EEPROM.put(PetCfg::SAVE_ADDRESS, block);
  saveWanted = false;
  lastSaveMs = millis();
}

void requestSave() {
  saveWanted = true;
}

void update() {
  if (saveWanted || millis() - lastSaveMs >= pick(PetCfg::SAVE_EVERY)) {
    save();
  }
}
}
