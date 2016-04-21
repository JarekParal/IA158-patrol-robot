#include <Clock.h>
#include "Walker.hpp"

Walker::Walker(ePortM motor_port, ePortS color_port, PositionStore & position_store)
:
	_motor          ( motor_port     ),
	_color_sensor   ( color_port     ),
	_position_store ( position_store )
{

}

void Walker::init()
{

}

void Walker::task()
{
	ev3api::Clock c;

	_motor.setPWM(100);
	c.sleep(1000);
	_motor.setPWM(-100);
	c.sleep(1000);
}

