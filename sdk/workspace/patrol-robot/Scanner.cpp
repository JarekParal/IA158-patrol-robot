#include "Scanner.hpp"

Scanner::Scanner ( PositionEvent & position_event )
{
	position_event.insert([this](PositionMessage msg){
		received_position_message(msg);
	});
}

void Scanner::task()
{

}

void Scanner::received_position_message(PositionMessage msg)
{

}

