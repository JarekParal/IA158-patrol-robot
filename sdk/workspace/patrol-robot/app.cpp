/**
 * Hello EV3
 *
 * This is a program used to test the whole platform.
 */

#include "ev3api.h"
#include "app.h"
#include <unistd.h>
#include <ctype.h>
#include <Port.h>
#include <Motor.h>
#include <Clock.h>

#include "Common.hpp"
#include "Walker.hpp"
#include "Scanner.hpp"
#include "Tower.hpp"
#include "Control.hpp"
#include "SmoothMotor.hpp"

FILE* bt;

class PatrolRobot {
	public:
	    PatrolRobot();

		void every_1s();
		void every_1ms();

	public:

	SmoothMotor walking_motor;
    Walker walker;
    Scanner scanner;
    Tower tower;
	Control control;

};

PatrolRobot::PatrolRobot() :
	walking_motor ( ePortM::PORT_A, WALKING_MOTOR_MTX ),
	walker        ( walking_motor, ePortS::PORT_1 ),
    scanner       ( ePortS::PORT_2 ),
    tower         ( ePortM::PORT_B, ePortM::PORT_C, TOWER_MTX ),
	control       ( CONTROL_MTX )
{
	walking_motor.on_speed_change = [this](uint8_t speed) {
		tower.walking_speed_changed(speed);
	};

	walker.on_position_change = [this] (PositionMessage m) {
		tower   .received_position_message ( m );
		scanner .received_position_message ( m );
	};

	scanner.on_target = [this](Target t){
		control.here_is_a_target ( t );
	};
}

void PatrolRobot::every_1s()
{
	control.every_1s();
}

void PatrolRobot::every_1ms()
{
	tower.every_1ms();
	walking_motor.every_1ms();
}

PatrolRobot* robot;

void main_task(intptr_t unused) {
    bt = fdopen(/*SIO_BT_FILENO*/ 5, "a+");
    assert(bt != NULL);

    robot = new PatrolRobot;

	TowerMessage ev;
	ev.command = TowerMessage::Command::LOCK;
	ev.params.target = {5, 10};
	robot->tower.received_command_message(ev);

    act_tsk(SCANNER_TASK);

	ev3_sta_cyc ( EVERY_1S  );
	ev3_sta_cyc ( EVERY_1MS  );


    act_tsk(WALKER_TASK);

	robot->control.loop();
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

void every_1ms()
{
	robot->every_1ms();
}

void every_1s()
{
	robot->every_1s();
}

