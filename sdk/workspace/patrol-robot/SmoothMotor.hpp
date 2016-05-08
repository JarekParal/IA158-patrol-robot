#ifndef SMOOTH_MOTOR_HPP_
#define SMOOTH_MOTOR_HPP_

#include <Motor.h>
#include <atomic>

class SmoothMotor
{
	public:
		SmoothMotor ( ePortM motor_port, ID mutex_id );
		using Milliseconds_t = uint16_t;
		using Speed_t = int8_t;

		/* @pre time_to_change >= 1 */
		void set_speed ( Speed_t speed, Milliseconds_t time_to_change );
		void init();


		void every_1ms();

	private:
		void update_speed();

		Speed_t _current_speed;
		Speed_t _last_stable_speed;
		Speed_t _desired_speed;
		Milliseconds_t _time_to_change;
		Milliseconds_t _current_time;

		ev3api::Motor _motor;
		ID            _mutex_id;
};



#endif // SMOOTH_MOTOR_HPP_
