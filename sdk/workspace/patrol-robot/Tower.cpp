#include "Tower.hpp"
#include <Clock.h>
#include <cmath>

Tower::Tower(ePortM rotation_port, ePortM fire_port, ID mutex_id)
    : _rotation_motor(rotation_port, false)
    , _fire_motor(fire_port, true, MEDIUM_MOTOR)
    , _direction(0)
    , _follow_target(false) {
    _mutex_id = mutex_id;

    _rotation_motor.setBrake(false);
    _rotation_motor.stop();

    _rotation_motor.setCount(0);
    _fire_motor.setCount(0);

    walking_speed = 0;
    walking_position = 0;
}

void Tower::every_1ms() {
    loc_mtx(_mutex_id);
    update();
    unl_mtx(_mutex_id);
}

void Tower::update() {
    walking_position += walking_speed;
}

void Tower::walking_speed_changed(uint8_t new_speed) {
    loc_mtx(_mutex_id);
    walking_speed = new_speed;
    unl_mtx(_mutex_id);
}

void Tower::received_position_message(PositionMessage msg) {
    loc_mtx(_mutex_id);
    /*
     * Abychom mohli odhadnout, jaka je vzdalenost mezi dvema ctvereckami
     * v jednotkach [tick * rychlost]
     */
    // fprintf ( bt, "walking position: %d\n", walking_position );
    walking_position = 0;
    unl_mtx(_mutex_id);

    if (!_follow_target)
        return;

    // Compute angle
    double diff = (msg.position - _target.x) * walking_step;
    double dist = _target.y;

    double angle = atan(diff / dist) * 180.0 / M_PI;
    update_position(angle);
}

void Tower::lock_at(Coordinates target) {
    loc_mtx(_mutex_id);
    _target = target;
    _follow_target = true;
    unl_mtx(_mutex_id);
}

void Tower::unlock() {
    _follow_target = false;
}

void Tower::shoot(uint8_t shot_number) {
    uint32_t angle = shot_number * degrees_per_shot;
    uint32_t speed = degrees_per_shot * shots_per_sec;
    _fire_motor.rotate(angle, speed, false);
}

void Tower::update_position(double angle) {
    double desired_angle = angle * gear_ratio;
    double current_pos = _rotation_motor.getCount();
    double diff = current_pos - desired_angle;

    _rotation_motor.stop();
    _rotation_motor.rotate(diff, speed, false);
}

void Tower::calibrate(int16_t angle) {
    if (angle > 180)
        return;

    if (angle < -180)
        return;

    _rotation_motor.rotate(angle * gear_ratio, speed, true);
    _rotation_motor.reset();
}
