// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_INPUT_MANAGER_H_
#define VIGILANTE_INPUT_MANAGER_H_

#include <functional>
#include <set>
#include <stack>

#include <axmol.h>

#include "input/Keybindable.h"

#define IS_KEY_PRESSED(keyCode) \
  InputManager::the().isKeyPressed(keyCode)

#define IS_KEY_JUST_PRESSED(keyCode) \
  InputManager::the().isKeyJustPressed(keyCode)

namespace vigilante {

class InputManager final {
 public:
  using OnKeyPressedEvLstnr = std::function<void (ax::EventKeyboard::KeyCode, ax::Event*)>;

  static InputManager& the();

  void activate(ax::Scene* scene);
  void deactivate();

  inline bool isActivated() const {
    return _scene != nullptr;
  }

  inline bool isKeyPressed(ax::EventKeyboard::KeyCode keyCode) const {
    return _pressedKeys.find(keyCode) != _pressedKeys.end();
  }

  inline bool isKeyJustPressed(ax::EventKeyboard::KeyCode keyCode) {
    return _pressedKeys.erase(keyCode) > 0;
  }

  inline bool isCapsLocked() const {
    return _isCapsLocked;
  }

  inline bool isShiftPressed() const {
    return isKeyPressed(ax::EventKeyboard::KeyCode::KEY_SHIFT);
  }

  inline bool hasSpecialOnKeyPressed() const {
    return static_cast<bool>(_specialOnKeyPressed);
  }

  inline void setSpecialOnKeyPressed(const OnKeyPressedEvLstnr& onKeyPressed) {
    _specialOnKeyPressed = onKeyPressed;
  }

  inline void clearSpecialOnKeyPressed() {
    _specialOnKeyPressed = nullptr;
  }

 private:
  InputManager() = default;

  ax::Scene* _scene{};
  ax::EventListenerKeyboard* _keyboardEvLstnr{};

  bool _isCapsLocked{};

  // Pressed Keys are stored in this set.
  // Relevant method: isKeyPressed(), isKeyJustPressed()
  std::set<ax::EventKeyboard::KeyCode> _pressedKeys;

  OnKeyPressedEvLstnr _specialOnKeyPressed{};
};

}  // namespace vigilante

#endif  // VIGILANTE_INPUT_MANAGER_H_
