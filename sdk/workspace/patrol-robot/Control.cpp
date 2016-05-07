#include "Common.hpp"
#include "Control.hpp"

void control_loop()
{
	fprintf ( bt, "Robot started\n" );
	fprintf ( bt, "Commands: next, shoot [seconds]\n");

	while(true)
		tslp_tsk(50);
}
