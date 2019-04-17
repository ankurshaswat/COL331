#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "container.h"
#include "spinlock.h"

#define NCONTAINERS 5

struct
{
    struct spinlock lock;
    struct container container[NCONTAINERS];
} ctable;

int next_container_id = 0;

void container_init(void)
{
    initlock(&ctable.lock, "container_table");
}

int create_container(void)
{
    struct container *c;

    acquire(&ctable.lock);

    for (c = ctable.container; c < &ctable.container[NCONTAINERS]; c++)
    {
        if (c->state == UNUSED_CONTAINER)
        {
            goto found;
        }
    }

    release(&ctable.lock);
    return -1;

found:
    c->state = IN_USE;
    c->container_id = next_container_id;
    next_container_id++;

    release(&ctable.lock);

    return c->container_id;

    //   int selected_container = -1;

    //   for(int i=0;i<NELEM(containers);i++) {
    //     if(containers[i] == 0) {
    //       containers[i] = 1;
    //       selected_container = i;
    //       break;
    //     }
    //   }

    //   if(selected_container<0) {
    //     return -1;
    //   }

    //   // char c = '0' + selected_container;

    //   char *argv[] = { "container_manager", (char*)&selected_container ,0 };
    //   fork_modified2("container_manager",argv);

    //   // cprintf("Fork started by %d\n",myproc()->pid);

    //   // free container available marker if failed

    //   return selected_container;

    //   // if(fork_modified(selected_container)<0) {
    //   //   return -1;
    //   // }
    //   // // char x = '0' +selected_container;
    //   // char *argv[] = { "container_manager", (char*)selected_container , 0 };
    //   // exec("container_manager",argv);
    //   // return 0;
}

int leave_container(void)
{
    return -1;
}

int join_container(int n)
{
    return -1;
}

int destroy_container(int n)
{
    return -1;
}