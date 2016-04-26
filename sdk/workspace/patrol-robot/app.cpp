/**
 * Hello EV3
 *
 * This is a program used to test the whole platform.
 */

#include "ev3api.h"
#include "app.h"
#include <unistd.h>
#include <Port.h>
#include <Motor.h>

#include "Position.hpp"
#include "Walker.hpp"
#include "Scanner.hpp"
#include "Tower.hpp"

extern "C"
{
	void *__dso_handle = NULL;
}

struct PatrolRobot {
    PositionEvent position_event; // must be constructed before scanner and tower!
    Walker walker;
    Scanner scanner;
    Tower tower;

    PatrolRobot() :
        walker(ePortM::PORT_A, ePortS::PORT_1, position_event),
        scanner(position_event),
        tower(position_event)
    {}
};

PatrolRobot* robot;

void main_task(intptr_t unused) {
    PatrolRobot _robot;
    robot = &_robot;

    act_tsk(WALKER_TASK);
}

void walker_task(intptr_t exinf)
{
    ev3_speaker_set_volume(100);
    robot->walker.init();
    ev3_speaker_play_tone(2000, 800);
	while (true)
		robot->walker.task();
}

void scanner_task(intptr_t exinf)
{
	/*while (true)
		scanner.task();*/
}

void tower_task(intptr_t exinf)
{
	/*while (true)
		tower.task();*/
}
