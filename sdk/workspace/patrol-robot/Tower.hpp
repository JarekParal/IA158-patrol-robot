#ifndef TOWER_HPP
#define TOWER_HPP

#include <Port.h>
#include <Motor.h>
#include "Common.hpp"

class Tower
{
	public:
		Tower(
            ePortM rotation_port,
            ePortM fire_port,
			ID mutex_id);

		void every_1ms();
		void walking_speed_changed(uint8_t new_speed);

		void received_position_message(PositionMessage msg);
        void received_command_message(TowerMessage msg);
		
		void calibrate(int16_t angle);

	private:

        void update_position(double angle);
		void update();

        ev3api::Motor _rotation_motor;
        ev3api::Motor _fire_motor;
		ID _mutex_id;

        int32_t _direction;
        bool _follow_target;
        Target _target;

        const int32_t speed = 200;
        const double gear_ratio = 1.95;
		int32_t walking_position; // in some small units
		int8_t  walking_speed;

};

#endif // TOWER_HPP
