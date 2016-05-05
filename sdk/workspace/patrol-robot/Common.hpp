#pragma once

#include <cstdint>
#include <ctype.h>
#include "ev3api.h"
#include "Event.hpp"

extern FILE *bt;

struct Target {
    int16_t position;
    int16_t distance;
};

using TargetEvent = Event<Target>;

struct TowerMessage {
    enum class Command { LOCK, UNLOCK, FIRE };
    Command command;
    union  {
        Target target;
        uint16_t shot_number;
    } params;
};

using TowerCommandEvent = Event<TowerMessage>;

struct FireMessage {
    uint16_t shot_number;
};

using FireEvent = Event<FireMessage>;