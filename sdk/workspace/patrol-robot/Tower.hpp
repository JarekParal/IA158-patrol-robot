#ifndef TOWER_HPP
#define TOWER_HPP

#include "Position.hpp"

class Tower
{
	public:
		Tower ( PositionStore const & position_store );

		void task();

	private:
		PositionStore const & _position_store;
};

#endif // TOWER_HPP
