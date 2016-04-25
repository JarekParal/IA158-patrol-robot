#include "Tower.hpp"

Tower::Tower(PositionEvent & position_event)
{
	position_event.insert([this](PositionMessage msg){
		received_position_message(msg);
	});
}

void Tower::task()
{

}

void Tower::received_position_message(PositionMessage msg)
{

}
