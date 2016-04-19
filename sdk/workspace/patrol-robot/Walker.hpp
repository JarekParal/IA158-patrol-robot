#ifndef WALKER_HPP
#define WALKER_HPP

#include "Position.hpp"

class Walker
{
	public:
		Walker ( PositionStore & position_store );

	private:
		PositionStore & _position_store;
};

#endif // WALKER_HPP
