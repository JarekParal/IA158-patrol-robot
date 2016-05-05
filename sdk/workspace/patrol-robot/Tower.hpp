#ifndef TOWER_HPP
#define TOWER_HPP

#include "Common.hpp"
#include "Position.hpp"

class Tower
{
	public:
		Tower (PositionEvent& position_event, TowerCommandEvent& commands);
		void task();

	private:
		void received_position_message(PositionMessage msg);
};

#endif // TOWER_HPP
