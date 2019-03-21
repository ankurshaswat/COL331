#include "types.h"
#include "stat.h"
#include "user.h"

int x = 1;

void
interruptHandler(void* msg) 
{
    x = *((int*)msg);
    printf(1,"UPROG%d: Process in interruptHandler. Parameter Val = %d %d \n",getpid(),&msg,*((int*)msg));   
    return_to_kernel();
}

int
main(void)
{
    int master = getpid();
    printf(1,"UPROG%d: Program Started. Interrupt handler address =%d. Calling Register\n",getpid(),interruptHandler);
    registerI(interruptHandler);
    printf(1,"UPROG%d: Returned from register system call\n",getpid());

    int pid = fork();

    if(pid==0) {
        int a[1];
        a[0] = master;
        printf(1,"UPROG%d: Sending multicast message\n",getpid());
        int mean = 103;
        send_multi(getpid(),a,&mean,1);
        exit();
    }

    while(x==1){
        printf(1,"UPROG%d: In While Loop. x=%d\n",getpid(),x);
    }

    printf(1,"UPROG%d: Out of while loop. x=%d\n",getpid(),x);

    exit();
}
