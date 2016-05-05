#include "Tower.hpp"
#include <Clock.h>
#include <cmath>

Tower::Tower(ePortM rotation_port,
             ePortM fire_port,
             PositionEvent& position_event,
             TowerCommandEvent& command_event)
    :
    _rotation_motor(rotation_port, false),
    _fire_motor(fire_port, true, MEDIUM_MOTOR),
    _direction(0),
    _follow_target(false)
{
	position_event.insert([this](PositionMessage msg) {
		received_position_message(msg);
	});

    command_event.insert([this](TowerMessage msg) {
        received_command_message(msg);
    });

    _rotation_motor.setBrake(false);
    _rotation_motor.stop();
}

void Tower::task()
{

}

void Tower::received_position_message(PositionMessage msg)
{
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

