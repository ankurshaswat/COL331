#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
	// Uncomment this when sys_toggle and sys_ps is implemented
	toggle();	// toggle the system trace on or off
	ps();
	exit();
}
