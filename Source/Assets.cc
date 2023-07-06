// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "Assets.h"

extern "C" {
#include <unistd.h>
}

#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>

#include <axmol.h>

#include "util/Logger.h"

using namespace std;
USING_NS_AX;

namespace vigilante::assets {

void loadSpritesheets(const fs::path& spritesheetsListFileName) {
  char buf[256] = {0};
  getcwd(buf, 256);
  std::cout << buf << std::endl;

  ifstream fin(spritesheetsListFileName);
  if (!fin.is_open()) {
    throw runtime_error("Failed to load spritesheets from " + spritesheetsListFileName.native());
  }

  VGLOG(LOG_INFO, "Loading textures...");
  SpriteFrameCache* frameCache = SpriteFrameCache::getInstance();
  string line;
  while (std::getline(fin, line)) {
    if (!line.empty()) {
      frameCache->addSpriteFramesWithFile(line);
    }
  }
}

}  // namespace vigilante::assets
