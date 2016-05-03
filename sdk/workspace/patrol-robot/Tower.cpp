#include "Tower.hpp"

void foo(PositionMessage event) {
    fprintf(bt, "%lu\n", event.position);
}

Tower::Tower(PositionEvent & position_event)
{
    position_event.insert(foo);
	/*position_event.insert([this](PositionMessage msg){
		received_position_message(msg);
	});*/
}

void Tower::task()
{

}

void Tower::received_position_message(PositionMessage msg)
{

}
