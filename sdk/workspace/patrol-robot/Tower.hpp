#ifndef TOWER_HPP
#define TOWER_HPP

#include "Position.hpp"

class Tower
{
	public:
		Tower ( PositionEvent & position_event );

		void task();

	private:
		void received_position_message(PositionMessage msg);
};

#endif // TOWER_HPP
