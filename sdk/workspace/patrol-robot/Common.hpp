#pragma once

#include <cstdint>
#include <ctype.h>
#include <functional>
#include <vector>
#include "ev3api.h"

template < typename Message>
using Event = std::function<void(Message)>;

extern FILE *bt;

using Position = int16_t;
using Distance = int16_t;

struct Coordinates
{
	int16_t x;
	int16_t y;
};

enum class Direction { Left, Right };

struct PositionMessage {
    Direction direction;
    Position position;
};

struct TowerMessage {
    enum class Command { LOCK, UNLOCK, FIRE };
    Command command;
    union  {
        Coordinates target;
        uint16_t shot_number;
    } params;
};

using TowerCommandEvent = Event<TowerMessage>;

struct FireMessage {
    uint16_t shot_number;
};

using FireEvent = Event<FireMessage>;
