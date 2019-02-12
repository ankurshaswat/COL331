#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
	// Uncommend these after sys_toggle and sys_add is implemented
	toggle(); // This toggles the system trace on or off
	printf(1, "sum of 2 and 3 is: %d\n",add(2,3));

	exit();
}
