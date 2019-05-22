#include "Actor.h"

#include <stdexcept>

using std::string;
using std::runtime_error;
using cocos2d::FileUtils;
using cocos2d::Vector;
using cocos2d::Animation;
using cocos2d::Sprite;
using cocos2d::SpriteFrame;
using cocos2d::SpriteBatchNode;
using cocos2d::SpriteFrameCache;

namespace vigilante {

Actor::Actor() : _body(), _bodySprite(), _bodySpritesheet() {}


b2Body* Actor::getBody() const {
  return _body;
}

Sprite* Actor::getBodySprite() const {
  return _bodySprite;
}

SpriteBatchNode* Actor::getBodySpritesheet() const {
  return _bodySpritesheet;
}


Animation* Actor::createAnimation(const string& textureResDir, string framesName,
                                  float interval, Animation* fallback) {
  SpriteFrameCache* frameCache = SpriteFrameCache::getInstance();

  // The texture resources under Resources/Texture/ has the following rules:
  //
  // Texture/character/player/player_attacking/0.png
  // |______________________| |____| |_______| |___|
  //      textureResDir          |   framesName
  //                      framesNamePrefix
  //
  // As you can see, each framesName (e.g., attacking) is preceded by a prefix,
  // this is to **prevent frames name collision** in cocos2d::SpriteFrameCache!
  string framesNamePrefix = Actor::extractTrailingDir(textureResDir);

  // Count how many frames (.png) are there in the corresponding directory.
  // Method: we will use FileUtils to test whether a file exists starting from 0.png, 1.png, ..., n.png
  string dir = textureResDir + "/" + framesNamePrefix + "_" + framesName;
  size_t frameCount = 0;
  while (FileUtils::getInstance()->isFileExist(dir + "/" + std::to_string(frameCount) + ".png")) {
    frameCount++;
  }

  // If there are no frames in the corresponding directory, fallback to IDLE_SHEATHED.
  if (frameCount == 0) {
    if (fallback) {
      return fallback;
    } else {
      throw runtime_error("Failed to create animations from " + dir + ", but fallback animation is not provided.");
    }
  }
  
  Vector<SpriteFrame*> frames;
  for (size_t i = 0; i < frameCount; i++) {
    const string& name = framesNamePrefix + "_" + framesName + "/" + std::to_string(i) + ".png";
    frames.pushBack(frameCache->getSpriteFrameByName(name));
  }
  Animation* animation = Animation::createWithSpriteFrames(frames, interval);
  animation->retain();
  return animation;
}

string Actor::extractTrailingDir(const string& textureResDir) {
  return textureResDir.substr(textureResDir.find_last_of('/') + 1);
}

} // namespace vigilante
