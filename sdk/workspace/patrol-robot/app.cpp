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
#include <Clock.h>

#include "Position.hpp"
#include "Walker.hpp"
#include "Scanner.hpp"
#include "Tower.hpp"

void main_task(intptr_t unused) {

}

static void wait_for_press()
{
	while(!ev3_button_is_pressed(ENTER_BUTTON));
	while(ev3_button_is_pressed(ENTER_BUTTON));
	ev3_speaker_play_tone(NOTE_C5, 300);
}

PositionStore position_store;
Walker walker ( position_store );
Scanner scanner ( position_store );
Tower tower ( position_store );

void walker_task(intptr_t exinf)
{
	ev3api::Motor main_motor(ePortM::PORT_A);
	ev3api::Clock c;
	while(true) {
		main_motor.setPWM(100);
		c.sleep(1000);
		main_motor.setPWM(-100);
		c.sleep(1000);
	}

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
