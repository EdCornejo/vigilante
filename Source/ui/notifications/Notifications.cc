// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "Notifications.h"

#define STARTING_X 10.0f
#define STARTING_Y 25.0f
#define MAX_LABEL_COUNT 10
#define LABEL_LIFETIME 5.0f
#define LABEL_ALIGNMENT TimedLabelService::TimedLabel::kLeft

namespace vigilante {

Notifications::Notifications()
    : TimedLabelService(STARTING_X, STARTING_Y, MAX_LABEL_COUNT, LABEL_LIFETIME, LABEL_ALIGNMENT) {}

}  // namespace vigilante
