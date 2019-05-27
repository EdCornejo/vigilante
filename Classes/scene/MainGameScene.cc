#include "MainGameScene.h"

#include <string>

#include "SimpleAudioEngine.h"

#include "Constants.h"
#include "GameAssetManager.h"
#include "character/Player.h"
#include "input/GameInputManager.h"
#include "skill/Skill.h"
#include "skill/ForwardSlash.h"
#include "skill/MagicalMissile.h"
#include "map/GameMap.h"
#include "util/box2d/b2DebugRenderer.h"
#include "util/CameraUtil.h"
#include "util/CallbackUtil.h"
#include "util/RandUtil.h"

using std::string;
using std::unique_ptr;
using std::shared_ptr;
using cocos2d::Vec3;
using cocos2d::Camera;
using cocos2d::CameraFlag;
using cocos2d::Director;
using cocos2d::Layer;
using cocos2d::FadeIn;
using cocos2d::FadeOut;
using cocos2d::CallFunc;
using cocos2d::Sequence;
using cocos2d::EventKeyboard;
using cocos2d::ui::ImageView;

namespace vigilante {

MainGameScene::~MainGameScene() {}

bool MainGameScene::init() {
  if (!Scene::init()) {
    return false;
  }

  // Camera note:
  // DEFAULT (orthographic): used to render tilemaps/game objects
  // USER1 (orthographic): used to render HUD
  // Initialize the default camera from "perspective" to "orthographic",
  // and use it as the game world camera.
  auto winSize = Director::getInstance()->getWinSize();
  cocos2d::log("winSize: w=%f h=%f", winSize.width, winSize.height);
  _gameCamera = getDefaultCamera();
  _gameCamera->initOrthographic(winSize.width, winSize.height, 1, 1000);
  _gameCamera->setPosition(0, 0);

  // Initialize HUD camera.
  _hudCamera = Camera::createOrthographic(winSize.width, winSize.height, 1, 1000);
  _hudCamera->setDepth(2);
  _hudCamera->setCameraFlag(CameraFlag::USER1);
  const Vec3& eyePosOld = _gameCamera->getPosition3D();
  Vec3 eyePos = {eyePosOld.x, eyePosOld.y, eyePosOld.z};
  _hudCamera->setPosition3D(eyePos);
  _hudCamera->lookAt(eyePos);
  _hudCamera->setPosition(0, 0);
  addChild(_hudCamera);
  
  // Initialize shade.
  _shade = ImageView::create(vigilante::asset_manager::kShade); // 1px * 1px
  _shade->setScaleX(winSize.width);
  _shade->setScaleY(winSize.height);
  _shade->setAnchorPoint({0, 0});
  _shade->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  _shade->runAction(FadeOut::create(.3f));
  addChild(_shade, 100);

  // Initialize HUD.
  _hud = unique_ptr<Hud>(Hud::getInstance());
  _hud->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  _hud->getLayer()->setPosition(75, winSize.height - 40);
  addChild(_hud->getLayer(), 90);

  // Initialize notification manager.
  _notifications = unique_ptr<NotificationManager>(NotificationManager::getInstance());
  _notifications->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  addChild(_notifications->getLayer(), 91);
  _notifications->show("Notification Manager initialized!");
  _notifications->show("Welcome to Vigilante 0.0.1 alpha");

  // Initialize floating damage manager.
  _floatingDamages = unique_ptr<FloatingDamageManager>(FloatingDamageManager::getInstance());
  addChild(_floatingDamages->getLayer(), 89);

  // Initialize Vigilante's CallbackUtil.
  vigilante::callback_util::init(this);

  // Initialize Vigilante's RandUtil.
  vigilante::rand_util::init();
  
  // Initialize GameMapManager.
  // b2World is created when GameMapManager's ctor is called.
  _gameMapManager = unique_ptr<GameMapManager>(GameMapManager::getInstance());
  _gameMapManager->loadGameMap("Map/starting_point.tmx");
  addChild(static_cast<Layer*>(_gameMapManager->getLayer()));

  _gameMapManager->getPlayer()->addItem(new Equipment("Resources/Database/equipment/iron_broadsword.json"));
  _gameMapManager->getPlayer()->addItem(new Equipment("Resources/Database/equipment/royal_cape.json"));

  // Initialize GameInputManager.
  // GameInputManager keep tracks of which keys are pressed.
  GameInputManager::getInstance()->activate(this);

  // Create b2DebugRenderer.
  _b2dr = b2DebugRenderer::create(getWorld());
  addChild(_b2dr);

  _hud->setPlayer(_gameMapManager->getPlayer());

  // Initialize Pause Menu.
  _pauseMenu = unique_ptr<PauseMenu>(new PauseMenu(_gameMapManager->getPlayer()));
  _pauseMenu->getLayer()->setCameraMask(static_cast<uint16_t>(CameraFlag::USER1));
  _pauseMenu->getLayer()->setVisible(false);
  addChild(_pauseMenu->getLayer(), 95);

  // Tick the box2d world.
  schedule(schedule_selector(MainGameScene::update));
  return true;
}

void MainGameScene::update(float delta) {
  handleInput();

  if (_pauseMenu->getLayer()->isVisible()) {
    return;
  }

  getWorld()->Step(1 / kFps, kVelocityIterations, kPositionIterations);

  _gameMapManager->update(delta);
  _floatingDamages->update(delta);
  _notifications->update(delta);

  vigilante::camera_util::lerpToTarget(_gameCamera, _gameMapManager->getPlayer()->getBody()->GetPosition());
  vigilante::camera_util::boundCamera(_gameCamera, _gameMapManager->getGameMap()->getTmxTiledMap());
  vigilante::camera_util::updateShake(_gameCamera, delta);
}

void MainGameScene::handleInput() {
  auto inputMgr = GameInputManager::getInstance();
  auto player = _gameMapManager->getPlayer();

  // Toggle b2dr (b2DebugRenderer)
  if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_0)) {
    bool isVisible = _b2dr->isVisible();
    _b2dr->setVisible(!isVisible);
    _notifications->show("[b2dr] is " + std::to_string(!isVisible));
  }


  if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_ESCAPE)) {
    bool isVisible = _pauseMenu->getLayer()->isVisible();
    _pauseMenu->getLayer()->setVisible(!isVisible);
    _pauseMenu->update();
    return;
  }
  if (_pauseMenu->getLayer()->isVisible()) {
    _pauseMenu->handleInput();
    return;
  }


  if (player->isSetToKill() || player->isAttacking() || player->isUsingSkill() || player->isSheathingWeapon() || player->isUnsheathingWeapon()) {
    return;
  }

  if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_UP_ARROW)) {
    if (player->getPortal()) {
      _shade->runAction(Sequence::create(
        FadeIn::create(.3f),
        CallFunc::create([=]() {
          GameMap::Portal* portal = player->getPortal();
          std::string targetTmxMapFileName = portal->targetTmxMapFileName; // copy this shit here, don't use reference
          int targetPortalId = portal->targetPortalId;
          _gameMapManager->loadGameMap(targetTmxMapFileName);
          auto pos = _gameMapManager->getGameMap()->getPortals().at(targetPortalId)->body->GetPosition();
          player->setPosition(pos.x, pos.y);
          _gameCamera->setPosition(pos.x * kPpm - 600 / 2, pos.y * kPpm - 300 / 2);
        }),
        FadeOut::create(.5f),
        nullptr
      ));
    }
    return;
  }

  if (inputMgr->isKeyPressed(EventKeyboard::KeyCode::KEY_DOWN_ARROW)) {
    player->crouch();
    if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_LEFT_ALT)) {
      player->jumpDown();
    }
  }

  if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_LEFT_CTRL)) {
    if (!player->isWeaponSheathed()) {
      player->attack();
    } else {
      NotificationManager::getInstance()->show("You haven't equipped a weapon yet.");
    }
  }

  if (inputMgr->isKeyPressed(EventKeyboard::KeyCode::KEY_LEFT_ARROW)) {
    player->moveLeft();
  } else if (inputMgr->isKeyPressed(EventKeyboard::KeyCode::KEY_RIGHT_ARROW)) {
    player->moveRight();
  }

  if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_R)) {
    if (player->getEquipmentSlots()[Equipment::Type::WEAPON]
        && player->isWeaponSheathed() && !player->isUnsheathingWeapon()) {
      player->unsheathWeapon();
    } else if (!player->isWeaponSheathed() && !player->isSheathingWeapon()) {
      player->sheathWeapon();
    }
  }

  if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_X)) {
    Skill* skill = new MagicalMissile("Resources/Database/skill/ice_spike.json", player);
    player->useSkill(skill);
  } else if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_C)) {
    player->useSkill(new ForwardSlash("Resources/Database/skill/forward_slash.json", player));
  }

  if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_Z)) {
    if (!player->getInRangeItems().empty()) {
      Item* item = *player->getInRangeItems().begin();
      player->pickupItem(item);
      _notifications->show("Acquired item: " + item->getItemProfile().name + ".");
    }
  }

  if (inputMgr->isKeyJustPressed(EventKeyboard::KeyCode::KEY_LEFT_ALT)) {
    player->jump();
  }

  if (player->isCrouching() && !inputMgr->isKeyPressed(EventKeyboard::KeyCode::KEY_DOWN_ARROW)) {
    player->getUp();
  }
}


b2World* MainGameScene::getWorld() const {
  return _gameMapManager->getWorld();
}

} // namespace vigilante
