#ifndef SCANNER_HPP
#define SCANNER_HPP

#include "Position.hpp"

class Scanner
{
	public:
		Scanner ( PositionEvent & position_event );
		void task();

	private:
		void received_position_message(PositionMessage msg);
};

#endif // SCANNER_HPP
