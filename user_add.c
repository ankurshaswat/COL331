#include "types.h"
#include "user.h"
#include "date.h"
int
main(int argc, char *argv[])
{
// If you follow the naming convetion, system call
// name will be sys_toggle and you
// call it by calling
int n1 = atoi(argv[1]);
int n2 = atoi(argv[2]);
// printf("%s\n", argv[0]);
int n3 = add(n1,n2);

int stdout = 1;
printf(stdout, "%d \n",n3);


exit();
}