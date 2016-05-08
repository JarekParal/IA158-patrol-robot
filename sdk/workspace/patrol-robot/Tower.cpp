#include "Tower.hpp"
#include <Clock.h>
#include <cmath>

Tower::Tower(ePortM rotation_port,
             ePortM fire_port,
             PositionEvent& position_event,
             TowerCommandEvent& command_event,
			 ID mutex_id)
    :
    _rotation_motor(rotation_port, false),
    _fire_motor(fire_port, true, MEDIUM_MOTOR),
    _direction(0),
    _follow_target(false)
{
	_mutex_id = mutex_id;

	position_event.insert([this](PositionMessage msg) {
		received_position_message(msg);
	});

    command_event.insert([this](TowerMessage msg) {
        received_command_message(msg);
    });

    _rotation_motor.setBrake(false);
    _rotation_motor.stop();

	walking_speed = 0;
	walking_position = 0;
}

void Tower::every_1ms()
{
	loc_mtx ( _mutex_id );
	update();
	unl_mtx ( _mutex_id );
}

void Tower::update()
{
	walking_position += walking_speed;
}

void Tower::walking_speed_changed(uint8_t new_speed)
{
	loc_mtx ( _mutex_id );
	walking_speed = new_speed;
	unl_mtx ( _mutex_id );
}

void Tower::received_position_message(PositionMessage msg)
{
	loc_mtx ( _mutex_id );
	/*
	 * Abychom mohli odhadnout, jaka je vzdalenost mezi dvema ctvereckami
	 * v jednotkach [tick * rychlost]
	 */
	fprintf ( bt, "walking position: %d\n", walking_position );
	walking_position = 0;
	unl_mtx ( _mutex_id );

    if (!_follow_target)
        return;

    // Compute angle
    double diff = msg.position - _target.position;
    double dist = _target.distance;

    double angle = tan(diff / dist) * 180.0 / M_PI;
    update_position(angle);
}

void Tower::received_command_message(TowerMessage msg)
{
    switch(msg.command) {
    case TowerMessage::Command::LOCK:
        _target = msg.params.target;
        _follow_target = true;
        break;
    case TowerMessage::Command::UNLOCK:
        _follow_target = false;
        break;
    case TowerMessage::Command::FIRE:
        //ToDo
        break;
    }
}

void Tower::update_position(double angle) {
    double desired_angle = angle * gear_ratio;
    double current_pos = _rotation_motor.getCount();
    double diff = current_pos - desired_angle;

    _rotation_motor.stop();
    _rotation_motor.rotate(diff, speed, false);
}

