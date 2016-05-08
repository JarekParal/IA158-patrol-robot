#pragma once

#include <cstdint>
#include <ctype.h>
#include <functional>
#include "ev3api.h"

template < typename Message>
using Event = std::function<void(Message)>;

extern FILE *bt;

using Position = int16_t;
using Distance = int16_t;

enum class Direction { Left, Right };

struct PositionMessage {
    Direction direction;
    Position position;
};

struct Target {
    Position position;
    Distance distance;
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
