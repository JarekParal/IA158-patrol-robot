/**:
 * Hello EV3
 *
 * This is a program used to test the whole platform.
 */

#include <unistd.h>
#include <cstdlib>
#include <ctype.h>
#include <utility> // std::move

#include "ev3api.h"

#include "app.h"
#include <Port.h>
#include <Motor.h>
#include <Clock.h>
#include <TouchSensor.h>
#include <IrSensor.h>

// Project
#include "Common.hpp"
#include "Walker.hpp"
#include "Scanner.hpp"
#include "Tower.hpp"
#include "Control.hpp"
#include "SmoothMotor.hpp"

FILE* bt;


extern "C"
{
	void * __dso_handle = nullptr;
}

using DistanceSensor = ev3api::SonarSensor;
//using DistanceSensor = ev3api::IrSensor;

class PatrolRobot {
public:
    PatrolRobot();

    void every_1s();
    void every_1ms();

public:
    SmoothMotor walking_motor;
    Walker walker;
    Scanner<ev3api::SonarSensor> scanner;
    Tower tower;
    Control control;
};

PatrolRobot::PatrolRobot()
    : walking_motor(ePortM::PORT_A, WALKING_MOTOR_MTX)
    , walker(walking_motor, ePortS::PORT_1)
    , scanner(ePortS::PORT_2)
    , tower(ePortM::PORT_B, ePortM::PORT_C, TOWER_MTX)
    , control(CONTROL_MTX, tower, scanner) {
    walking_motor.on_speed_change = [this](uint8_t speed) {
        tower.walking_speed_changed(speed);
    };

    walker.on_position_change = [this](PositionMessage m) {
        tower.received_position_message(m);
        scanner.received_position_message(m);
    };

    scanner.on_target = [this](DepthObject o) {
        control.here_is_a_target(o);
    };
}

void PatrolRobot::every_1s() {
    control.every_1s();
}

void PatrolRobot::every_1ms() {
    tower.every_1ms();
    walking_motor.every_1ms();
}

PatrolRobot* robot;

void main_task(intptr_t unused) {
    //    bt = fdopen(/*SIO_BT_FILENO*/ 5, "a+");
    bt = ev3_serial_open_file(EV3_SERIAL_BT);
    assert(bt != NULL);

    robot = new PatrolRobot;

    act_tsk(SCANNER_TASK);

    ev3_sta_cyc(EVERY_1S);
    ev3_sta_cyc(EVERY_1MS);

    act_tsk(WALKER_TASK);

    robot->control.loop();
}

void kill_btn_task(intptr_t exinf) {
    ev3api::TouchSensor btn = ev3api::TouchSensor(ePortS::PORT_3);
    while (true) {
        if (btn.isPressed()) {
            turn_off();
        }
        tslp_tsk(50);
    }
}

void turn_off() {
    if (robot != nullptr) {
        ev3_motor_stop(EV3_PORT_A, true);
        ev3_motor_stop(EV3_PORT_B, true);
        ev3_motor_stop(EV3_PORT_C, true);
        ev3_motor_stop(EV3_PORT_D, true);
    }
}

void walker_task(intptr_t exinf) {
    ev3_speaker_set_volume(100);
    robot->walker.init();
    ev3_speaker_play_tone(1000, 100);
    ev3api::Clock c;
    while (true) {
        robot->walker.task();
        tslp_tsk(2);
    }
}

void scanner_task(intptr_t exinf) {
    while (true) {
        robot->scanner.task();
        tslp_tsk(20);
    }
}

void every_1ms() {
    robot->every_1ms();
}

void every_1s() {
    robot->every_1s();
}
