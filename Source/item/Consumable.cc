// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "Consumable.h"

#include "util/JsonUtil.h"

using namespace std;
USING_NS_AX;

namespace vigilante {

Consumable::Consumable(const string& jsonFileName)
    : Item{jsonFileName},
      _consumableProfile{jsonFileName} {}

void Consumable::import(const string& jsonFileName) {
  Item::import(jsonFileName);
  _consumableProfile = Consumable::Profile{jsonFileName};
}

EventKeyboard::KeyCode Consumable::getHotkey() const {
  return _consumableProfile.hotkey;
}

void Consumable::setHotkey(EventKeyboard::KeyCode hotkey) {
  _consumableProfile.hotkey = hotkey;
}

Consumable::Profile::Profile(const string& jsonFileName) : hotkey() {
  rapidjson::Document json = json_util::parseJson(jsonFileName);

  duration = json["duration"].GetFloat();

  restoreHealth = json["restoreHealth"].GetInt();
  restoreMagicka = json["restoreMagicka"].GetInt();
  restoreStamina = json["restoreStamina"].GetInt();

  bonusPhysicalDamage = json["bonusPhysicalDamage"].GetInt();
  bonusMagicalDamage = json["bonusMagicalDamage"].GetInt();

  bonusStr = json["bonusStr"].GetInt();
  bonusDex = json["bonusDex"].GetInt();
  bonusInt = json["bonusInt"].GetInt();
  bonusLuk = json["bonusLuk"].GetInt();

  bonusMoveSpeed = json["bonusMoveSpeed"].GetInt();
  bonusJumpHeight = json["bonusJumpHeight"].GetInt();
}

}  // namespace vigilante
