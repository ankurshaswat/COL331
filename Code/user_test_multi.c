#include "types.h"
#include "stat.h"
#include "user.h"

volatile int x = 0;

void
interruptHandler(void* msg) 
{
    x = *((int*)msg);
    printf(1,"UPROG%d: Process in interruptHandler. Parameter Val =%d \n",getpid(),*((int*)msg));   
    return_to_kernel();
}

int
main(void)
{
    int master = getpid();
    registerI(interruptHandler);

    int pid = fork();
    if(pid==0) {
        int rec_pid[1] = {master};
        int msg = 1;
        send_multi(getpid(),rec_pid,&msg,1);
        msg = 2;
        send_multi(getpid(),rec_pid,&msg,1);
        printf(1,"Sent messages to parent from pid = %d\n",getpid());
        exit();
    }

    while(x!=2) {}

    exit();
}
