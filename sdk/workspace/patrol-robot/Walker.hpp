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
				 PositionStore & position_store,
				 PositionEvent const & position_event
				 );

		void init();
		void task();

	private:
		enum class PositionColor
		{
			Black,
			White,
			Red
		};

		PositionColor next_color(rgb_raw_t const & from) const;
		void update_position(PositionColor c);

		void step();
		void change_direction();

		ev3api::Motor   _motor;
		ev3api::ColorSensor _color_sensor;
		PositionStore & _position_store;
		PositionEvent const & _position_event;


		PositionColor _current_color;
		Position      _current_position;
		Direction     _current_direction;
};

#endif // WALKER_HPP
