#include "Item.h"

#include "map/GameMapManager.h"
#include "util/box2d/b2BodyBuilder.h"
#include "util/CategoryBits.h"
#include "util/Constants.h"

using std::string;
using cocos2d::Sprite;
using vigilante::category_bits::kItem;
using vigilante::category_bits::kFeet;
using vigilante::category_bits::kWall;
using vigilante::category_bits::kGround;
using vigilante::category_bits::kPlatform;


namespace vigilante {

const float Item::_kIconWidth = 16.0f;
const float Item::_kIconHeight = 16.0f;

Item::~Item() {
  _b2body->GetWorld()->DestroyBody(_b2body);
}

Item::Item(const Item::Type itemType,
           const string& name,
           const string& desc,
           const string& imgPath,
           float x,
           float y)
    : _itemType(itemType),
      _name(name),
      _desc(desc),
      _b2body(),
      _sprite(Sprite::create(imgPath)) {
  // Define b2body and fixture.
  short categoryBits = kItem;
  short maskBits = kGround | kPlatform | kWall;
  defineBody(b2BodyType::b2_dynamicBody, categoryBits, maskBits, x, y);  
  // Disable sprite antialiasing
  _sprite->getTexture()->setAliasTexParameters();
}

void Item::update(float delta) {
  // Sync the sprite with its b2body.
  b2Vec2 b2bodyPos = _b2body->GetPosition();
  _sprite->setPosition(b2bodyPos.x * kPpm, b2bodyPos.y * kPpm);
}

void Item::defineBody(b2BodyType bodyType,
                      short categoryBits,
                      short maskBits,
                      float x,
                      float y) {
  b2BodyBuilder bodyBuilder(GameMapManager::getInstance()->getWorld());

  _b2body = bodyBuilder.type(bodyType)
    .position(x, y, kPpm)
    .buildBody();

  bodyBuilder.newRectangleFixture(_kIconWidth / 2, _kIconHeight / 2, kPpm)
    .categoryBits(categoryBits)
    .maskBits(maskBits | kFeet) // Enable collision detection with feet fixtures
    .setSensor(true)
    .setUserData(this)
    .buildFixture();

  bodyBuilder.newRectangleFixture(_kIconWidth / 2, _kIconHeight / 2, kPpm)
    .categoryBits(categoryBits)
    .maskBits(maskBits)
    .setUserData(this)
    .buildFixture();
}


const Item::Type Item::getItemType() const {
  return _itemType;
}

const string& Item::getName() const {
  return _name;
}

const string& Item::getDesc() const {
  return _desc;
}

b2Body* Item::getB2Body() const {
  return _b2body;
}

Sprite* Item::getSprite() const {
  return _sprite;
}

} // namespace vigilante