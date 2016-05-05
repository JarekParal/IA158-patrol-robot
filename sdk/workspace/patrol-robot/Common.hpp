#pragma once

#include <cstdint>
#include "ev3api.h"
#include <ctype.h>

extern FILE *bt;

struct Target {
    int16_t position;
    int16_t distance;
};

using TargetEvent = Event<Target>;

struct TowerMessage {
    enum class Command { LOCK, UNLOCK };
    Command command;
    Target target;
};

using TowerEvent = Event<TowerMessage>;

struct FireMessage {
    uint16_t shot_number;
};

using FireEvent = Event<FireMessage>;