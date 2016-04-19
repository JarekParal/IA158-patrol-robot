#ifndef SCANNER_HPP
#define SCANNER_HPP

#include "Position.hpp"

class Scanner
{
	public:
		Scanner ( PositionStore const & position_store );
		void task();

	private:
		PositionStore const & _position_store;
};

#endif // SCANNER_HPP
