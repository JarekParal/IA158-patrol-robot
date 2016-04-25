#ifndef POSITION_HPP
#define POSITION_HPP

#include <atomic>
#include "Event.hpp"

using Position = int32_t;
using PositionStore = std::atomic<Position>;

enum class Direction
{
	Left,
	Right
};

struct PositionMessage
{
	Direction direction;
	Position position;
};

using PositionEvent = Event<PositionMessage>;

#endif // POSITION_HPP
