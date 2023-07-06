// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "Skill.h"

#include <axmol.h>

#include "skill/BatForm.h"
#include "skill/BackDash.h"
#include "skill/ForwardSlash.h"
#include "skill/MagicalMissile.h"
#include "util/JsonUtil.h"
#include "util/Logger.h"

using namespace std;

namespace vigilante {

unique_ptr<Skill> Skill::create(const string& jsonFileName, Character* user) {
  if (jsonFileName.find("bat_form") != jsonFileName.npos) {
    return std::make_unique<BatForm>(jsonFileName, user);
  } else if (jsonFileName.find("back_dash") != jsonFileName.npos) {
    return std::make_unique<BackDash>(jsonFileName, user);
  } else if (jsonFileName.find("forward_slash") != jsonFileName.npos) {
    return std::make_unique<ForwardSlash>(jsonFileName, user);
  } else if (jsonFileName.find("ice_spike") != jsonFileName.npos) {
    return std::make_unique<MagicalMissile>(jsonFileName, user);
  }

  VGLOG(LOG_ERR, "Unable to determine skill: %s", jsonFileName.c_str());
  return nullptr;
}

Skill::Profile::Profile(const string& jsonFileName) : jsonFileName(jsonFileName), hotkey() {
  rapidjson::Document json = json_util::parseJson(jsonFileName);

  skillType = static_cast<Skill::Type>(json["skillType"].GetInt());
  characterFramesName = json["characterFramesName"].GetString();
  framesDuration = json["framesDuration"].GetFloat();
  frameInterval = json["frameInterval"].GetFloat();

  textureResDir = json["textureResDir"].GetString();
  name = json["name"].GetString();
  desc = json["desc"].GetString();

  requiredLevel = json["requiredLevel"].GetInt();
  cooldown = json["cooldown"].GetFloat();

  physicalDamage = json["physicalDamage"].GetInt();
  magicalDamage = json["magicalDamage"].GetInt();

  deltaHealth = json["deltaHealth"].GetInt();
  deltaMagicka = json["deltaMagicka"].GetInt();
  deltaStamina = json["deltaStamina"].GetInt();

  sfxActivate = json["sfxActivate"].GetString();
  sfxHit = json["sfxHit"].GetString();
}

}  // namespace vigilante
