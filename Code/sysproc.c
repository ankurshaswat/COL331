#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int sys_ps(void)
{
  print_processes();
  return 0;
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_registerI(void) {
  void (*interruptHandler)(void*);
  if(argptr(0,(void*)&interruptHandler,sizeof(interruptHandler))<0) {
    return -1;
  }
  int id = myproc()->pid;
  registerI(id,(uint)interruptHandler);
  return  0;
}

int
sys_send_multi(void) {
  int sender_pid;
  int *rec_pids;
  void* msg;
  int length;

  if(argint(0,&sender_pid) <0) {
    return -1;
  }

  if(argint(3,&length) <0) {
    return -1;
  }

  if(argptr(1,(void*)&rec_pids, length*sizeof(int*)) <0) {
    return -1;
  }

  if(argptr(2,(void*)&msg, sizeof(void*)) <0) {
    return -1;
  }

  for(int i=0;i<length;i++) {
    cprintf("pid of child %d\n",rec_pids[i]);
  }

  for(int i=0;i<length;i++) {
    int rec_pid = rec_pids[i];
    callInterrupt(rec_pid,msg);
    cprintf("SYS_send_multi: Interrupt called for %d\n",rec_pid);
  }

  return 0;
}

int
sys_return_to_kernel(void) {
  cprintf("SYS_return_to_kernel: PID %d returned to kernel\n",myproc()->pid);
  return_to_kernel(myproc()->pid);
  return 0;
}