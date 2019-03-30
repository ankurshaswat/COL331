#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    printf(1,"Container Manager for container %d started\n",*(int*)argv[1]);
    // printf(1,"Container Manager started\n");
    while(1) {}
    exit();
}