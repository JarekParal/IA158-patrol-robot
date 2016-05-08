#ifndef SMOOTH_MOTOR_HPP_
#define SMOOTH_MOTOR_HPP_

#include <Motor.h>
#include "Common.hpp"

class SmoothMotor
{
	public:
		SmoothMotor ( ePortM motor_port, ID mutex_id );
		using Milliseconds_t = uint16_t;
		using Speed_t = int8_t;
		using SpeedDiff_t = int16_t;

		/* @pre time_to_change >= 1 */
		void set_speed ( Speed_t speed, Milliseconds_t time_to_change );
		void init();


		void every_1ms();

		Event<Speed_t> on_speed_change;

	private:
		void update_speed();
		static bool valid(Speed_t speed);

		ev3api::Motor _motor;
		ID            _mutex_id;

		// < protected by _mutex_id >
		Speed_t _last_stable_speed;
		Speed_t _desired_speed;
		Milliseconds_t _time_to_change;
		Milliseconds_t _current_time;
		Speed_t _current_speed;
		// </protected by _mutex_id >
};



#endif // SMOOTH_MOTOR_HPP_
