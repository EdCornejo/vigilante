#include "Item.h"

#include "json/document.h"

#include "Constants.h"
#include "CategoryBits.h"
#include "map/GameMapManager.h"
#include "util/box2d/b2BodyBuilder.h"
#include "util/JsonUtil.h"

using std::string;
using cocos2d::Sprite;
using vigilante::category_bits::kItem;
using vigilante::category_bits::kFeet;
using vigilante::category_bits::kWall;
using vigilante::category_bits::kGround;
using vigilante::category_bits::kPlatform;
using rapidjson::Document;

namespace vigilante {

const int Item::_kNumAnimations = 1;
const int Item::_kNumFixtures = 1;

Item::Item(const string& jsonFileName)
    : DynamicActor(_kNumAnimations, _kNumFixtures), _itemProfile(jsonFileName) {
  _bodySprite = Sprite::create(getIconPath());
  _bodySprite->getTexture()->setAliasTexParameters();
}


void Item::update(float delta) {
  // Sync the sprite with its b2body.
  b2Vec2 b2bodyPos = _body->GetPosition();
  _bodySprite->setPosition(b2bodyPos.x * kPpm, b2bodyPos.y * kPpm);
}

void Item::defineBody(b2BodyType bodyType, short categoryBits, short maskBits, float x, float y) {
  b2BodyBuilder bodyBuilder(GameMapManager::getInstance()->getWorld());

  _body = bodyBuilder.type(bodyType)
    .position(x, y, kPpm)
    .buildBody();

  bodyBuilder.newRectangleFixture(kIconSize / 2, kIconSize / 2, kPpm)
    .categoryBits(categoryBits)
    .maskBits(maskBits | kFeet) // Enable collision detection with feet fixtures
    .setSensor(true)
    .setUserData(this)
    .buildFixture();

  bodyBuilder.newRectangleFixture(kIconSize / 2, kIconSize / 2, kPpm)
    .categoryBits(categoryBits)
    .maskBits(maskBits)
    .setUserData(this)
    .buildFixture();
}

void Item::import(const string& jsonFileName) {
  _itemProfile = Item::Profile(jsonFileName);
}


void Item::showOnMap(float x, float y) {
  if (!_isShownOnMap) {
    short categoryBits = kItem;
    short maskBits = kGround | kPlatform | kWall;
    defineBody(b2BodyType::b2_dynamicBody, categoryBits, maskBits, x, y);  

    _bodySprite = Sprite::create(getIconPath());
    _bodySprite->getTexture()->setAliasTexParameters();

    GameMapManager::getInstance()->getLayer()->addChild(_bodySprite, 33);
    _isShownOnMap = true;
  }
}

void Item::removeFromMap() {
  if (_isShownOnMap) {
    _body->GetWorld()->DestroyBody(_body);

    GameMapManager::getInstance()->getLayer()->removeChild(_bodySprite);
    _bodySprite = nullptr;
    _isShownOnMap = false;
  }
}


Item::Profile& Item::getItemProfile() {
  return _itemProfile;
}

std::string Item::getIconPath() const {
  return _itemProfile.textureResDir + "/icon.png";
}


Item::Profile::Profile(const string& jsonFileName) {
  Document json = json_util::parseJson(jsonFileName);

  itemType = static_cast<Item::Type>(json["itemType"].GetInt());
  textureResDir = json["textureResDir"].GetString();
  name = json["name"].GetString();
  desc = json["desc"].GetString();
}

} // namespace vigilante
