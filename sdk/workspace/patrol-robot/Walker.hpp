#ifndef WALKER_HPP
#define WALKER_HPP

#include <Port.h>
#include <Motor.h>
#include <ColorSensor.h>
#include "Position.hpp"
#include "SmoothMotor.hpp"

class Walker
{
	public:
		Walker ( SmoothMotor & motor,
				 ePortS color_port,
				 PositionEvent const & position_event
				 );

		void init();
		void task();

	private:
		enum class PositionColor
		{
			Black,
			White,
			Blue
		};

		PositionColor next_color(rgb_raw_t const & from) const;
		void update_position(PositionColor c);

		void step();
		void change_direction();
		void update_led();
		PositionColor candidate_color(PositionColor c);

		SmoothMotor     & _motor;
		ev3api::ColorSensor _color_sensor;
		PositionEvent const & _position_event;


		PositionColor _current_color;
		PositionColor _candidate_color;
		uint8_t       _candidate_remaining;
		Position      _current_position;
		Direction     _current_direction;

		const int8_t abs_speed = 15;
};

#endif // WALKER_HPP
