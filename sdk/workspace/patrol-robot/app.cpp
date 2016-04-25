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

void main_task(intptr_t unused) {

}

PositionEvent position_event; // must be constructed before scanner and tower!
Walker walker ( ePortM::PORT_A, ePortS::PORT_1, position_event );
Scanner scanner ( position_event );
Tower tower ( position_event );

void walker_task(intptr_t exinf)
{
	while (true)
		walker.task();
}

void scanner_task(intptr_t exinf)
{
	while (true)
		scanner.task();
}

void tower_task(intptr_t exinf)
{
	while (true)
		tower.task();
}
