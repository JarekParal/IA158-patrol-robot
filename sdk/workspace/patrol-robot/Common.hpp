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

struct ScannedTarget {
	ScannedTarget() = default;
	ScannedTarget ( ScannedTarget const & orig ) = delete;
	ScannedTarget ( ScannedTarget && old ) = default;
	ScannedTarget & operator= ( ScannedTarget const & origin ) = delete;
	ScannedTarget & operator= ( ScannedTarget && old ) = default;

	Position _from;
	Position _to;
	std::vector<Distance> distances;

	Position from() const { return _from; }
	Position to() const { return _from + distances.size(); }
};


struct Target {
    Position position;
    Distance distance;
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
