// Copyright (c) 2023 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "PlayerController.h"

#include "character/Player.h"
#include "combat/CombatMotion.h"
#include "combat/ComboSystem.h"
#include "input/InputManager.h"
#include "scene/SceneManager.h"
#include "util/Logger.h"

using namespace std;
USING_NS_AX;

namespace vigilante {

namespace {

constexpr auto kMoveLeftKey = EventKeyboard::KeyCode::KEY_LEFT_ARROW;
constexpr auto kMoveRightKey = EventKeyboard::KeyCode::KEY_RIGHT_ARROW;
constexpr auto kCrouchKey = EventKeyboard::KeyCode::KEY_DOWN_ARROW;
constexpr auto kJumpKey = EventKeyboard::KeyCode::KEY_LEFT_ALT;
constexpr auto kSheatheUnsheatheWeaponKey = EventKeyboard::KeyCode::KEY_R;
constexpr auto kDodgeKey = EventKeyboard::KeyCode::KEY_X;
constexpr auto kAttackKey = EventKeyboard::KeyCode::KEY_LEFT_CTRL;
constexpr auto kInteractKey = EventKeyboard::KeyCode::KEY_E;
constexpr auto kPickupItemKey = EventKeyboard::KeyCode::KEY_Z;
constexpr auto kUsePortalKey = EventKeyboard::KeyCode::KEY_F;

}  // namespace

void PlayerController::handleInput() {
  if (shouldBlockInput()) {
    return;
  }

  if (_player.getInRangeInteractables().size() && IS_KEY_JUST_PRESSED(kInteractKey)) {
    _player.interact(*(_player.getInRangeInteractables().begin()));
    return;
  }

  if (_player.getPortal() && IS_KEY_JUST_PRESSED(kUsePortalKey)) {
    _player.interact(_player.getPortal());
    return;
  }

  if (_player.getInRangeItems().size() && IS_KEY_JUST_PRESSED(kPickupItemKey)) {
    _player.pickupItem(*(_player.getInRangeItems().begin()));
  }

  if (IS_KEY_PRESSED(kCrouchKey)) {
    _player.crouch();
  } else if (_player.isCrouching()) {
    _player.getUp();
  }

  if (IS_KEY_PRESSED(kMoveLeftKey)) {
    _player.moveLeft();
  } else if (IS_KEY_PRESSED(kMoveRightKey)) {
    _player.moveRight();
  }

  if (IS_KEY_JUST_PRESSED(kDodgeKey)) {
    if (IS_KEY_PRESSED(kMoveLeftKey) || IS_KEY_PRESSED(kMoveRightKey)) {
      _player.dodgeForward();
    } else {
      _player.dodgeBackward();
    }
  }

  if (IS_KEY_JUST_PRESSED(kJumpKey)) {
    _player.isCrouching() ? _player.jumpDown() : _player.jump();
  }

  if (IS_KEY_JUST_PRESSED(kAttackKey)) {
    handleAttackInput();
    return;
  }

  /*
  if (IS_KEY_JUST_PRESSED(kSheatheUnsheatheWeaponKey)) {
    handleSheatheUnsheatheWeaponInput();
  }
  */

  handleHotkeyInput();
}

bool PlayerController::shouldBlockInput() const {
  return _player.isSetToKill() ||
         _player.isAttacking() ||
         _player.isUsingSkill() ||
         _player.isSheathingWeapon() ||
         _player.isUnsheathingWeapon();
}

void PlayerController::handleSheatheUnsheatheWeaponInput() {
  if (_player.getEquipmentSlots()[Equipment::Type::WEAPON] &&
      _player.isWeaponSheathed() &&
      !_player.isUnsheathingWeapon()) {
    _player.unsheathWeapon();
  } else if (!_player.isWeaponSheathed() && !_player.isSheathingWeapon()) {
    _player.sheathWeapon();
  }
}

void PlayerController::handleAttackInput() {
  if (_player.isWeaponSheathed()) {
    return;
  }

  const optional<Character::State> attackState = _player.getCombatSystem().determineNextAttackState();
  if (!attackState.has_value()) {
    _player.attack();
    return;
  }
  
  if (!handleCombatMotion(_player, *attackState)) {
    VGLOG(LOG_ERR, "Failed to handle combat motion, character: [%s], attackState: [%d]",
          _player.getCharacterProfile().jsonFileName.c_str(), *attackState);
  }
}

void PlayerController::handleHotkeyInput() {
  auto hotkeyMgr = SceneManager::the().getCurrentScene<GameScene>()->getHotkeyManager();
  for (auto keyCode : HotkeyManager::kBindableKeys) {
    Keybindable* action = hotkeyMgr->getHotkeyAction(keyCode);
    if (IS_KEY_JUST_PRESSED(keyCode) && action) {
      if (dynamic_cast<Skill*>(action)) {
        _player.activateSkill(dynamic_cast<Skill*>(action));
      } else if (dynamic_cast<Consumable*>(action)) {
        _player.useItem(dynamic_cast<Consumable*>(action));
      }
    }
  }
}

}  // namespace vigilante
