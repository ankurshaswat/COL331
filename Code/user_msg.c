#include "types.h"
#include "user.h"
#include "date.h"
int
main(int argc, char *argv[])
{
// If you follow the naming convetion, system call
// name will be sys_toggle and you
// call it by calling
send(getpid(),1,"hello");

int rec_id;
char *msg;
rec(getpid(),&rec_id,&msg);
printf(1,"%d %s\n",rec_id,msg);
exit();
}