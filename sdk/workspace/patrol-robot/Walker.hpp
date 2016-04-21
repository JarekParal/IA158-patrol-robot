#ifndef WALKER_HPP
#define WALKER_HPP

#include <Port.h>
#include <Motor.h>
#include <ColorSensor.h>
#include "Position.hpp"

class Walker
{
	public:
		Walker ( ePortM motor_port,
				 ePortS color_port,
				 PositionStore & position_store );

		void init();
		void task();

	private:
		ev3api::Motor   _motor;
		ev3api::ColorSensor _color_sensor;
		PositionStore & _position_store;
};

#endif // WALKER_HPP
