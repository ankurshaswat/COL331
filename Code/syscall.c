#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"
#include "queue.h"
#include "spinlock.h"

// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// Fetch the int at addr from the current process.
int
fetchint(uint addr, int *ip)
{
  struct proc *curproc = myproc();

  if(addr >= curproc->sz || addr+4 > curproc->sz)
    return -1;
  *ip = *(int*)(addr);
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int
fetchstr(uint addr, char **pp)
{
  char *s, *ep;
  struct proc *curproc = myproc();

  if(addr >= curproc->sz)
    return -1;
  *pp = (char*)addr;
  ep = (char*)curproc->sz;
  for(s = *pp; s < ep; s++){
    if(*s == 0)
      return s - *pp;
  }
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int
argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();
 
  if(argint(n, &i) < 0)
    return -1;
  if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int
argstr(int n, char **pp)
{
  int addr;
  if(argint(n, &addr) < 0)
    return -1;
  return fetchstr(addr, pp);
}

extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_print_count(void);
extern int sys_toggle(void);
extern int sys_add(void);
extern int sys_ps(void);
extern int sys_send(void);
extern int sys_recv(void);
extern int sys_registerI(void);
extern int sys_send_multi(void);
extern int sys_return_to_kernel(void);

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_print_count] sys_print_count,
[SYS_toggle]  sys_toggle,
[SYS_add]  sys_add,
[SYS_ps]  sys_ps,
[SYS_send]  sys_send,
[SYS_recv]  sys_recv,
[SYS_registerI]  sys_registerI,
[SYS_send_multi]  sys_send_multi,
[SYS_return_to_kernel]  sys_return_to_kernel,
};

char *sys_call_names[] = {
[SYS_fork]    "sys_fork",
[SYS_exit]    "sys_exit",
[SYS_wait]    "sys_wait",
[SYS_pipe]    "sys_pipe",
[SYS_read]    "sys_read",
[SYS_kill]    "sys_kill",
[SYS_exec]    "sys_exec",
[SYS_fstat]   "sys_fstat",
[SYS_chdir]   "sys_chdir",
[SYS_dup]     "sys_dup",
[SYS_getpid]  "sys_getpid",
[SYS_sbrk]    "sys_sbrk",
[SYS_sleep]   "sys_sleep",
[SYS_uptime]  "sys_uptime",
[SYS_open]    "sys_open",
[SYS_write]   "sys_write",
[SYS_mknod]   "sys_mknod",
[SYS_unlink]  "sys_unlink",
[SYS_link]    "sys_link",
[SYS_mkdir]   "sys_mkdir",
[SYS_close]   "sys_close",
[SYS_print_count] "sys_print_count",
[SYS_toggle]  "sys_toggle",
[SYS_add]   "sys_add",
[SYS_ps]   "sys_ps",
[SYS_send]   "sys_send",
[SYS_recv]   "sys_recv",
[SYS_registerI]  "sys_registerI",
[SYS_send_multi]  "sys_send_multi",
[SYS_return_to_kernel]  "sys_return_to_kernel",
};

int alphabetical_mapping[] = {
SYS_add,
SYS_chdir,
SYS_close,
SYS_dup,
SYS_exec,
SYS_exit,
SYS_fork,
SYS_fstat,
SYS_getpid,
SYS_kill,
SYS_link,
SYS_mkdir,
SYS_mknod,
SYS_open,
SYS_pipe,
SYS_print_count,
SYS_ps,
SYS_read,
SYS_recv,
SYS_registerI,
SYS_return_to_kernel,
SYS_sbrk,
SYS_send,
SYS_send_multi,
SYS_sleep,
SYS_toggle,
SYS_unlink,
SYS_uptime,
SYS_wait,
SYS_write
};

int count[NELEM(syscalls)];

enum State { TRACE_ON=1,TRACE_OFF=0 };

int display_sys_calls = TRACE_OFF;

void
syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    if(display_sys_calls==TRACE_ON) {
      count[num]++;
    }
    curproc->tf->eax = syscalls[num]();
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }
}

int sys_add(void) 
{
  int n1;
  int n2;

  if(argint(0,&n1) < 0) {
    return -1;
  }
  if(argint(1,&n2) < 0) {
    return -1;
  }
  return n1+n2;
}

int
sys_toggle(void) 
{
  if(display_sys_calls == TRACE_OFF) {
    display_sys_calls = TRACE_ON;
    for(int i=0;i<NELEM(count);i++) {
      count[i]=0;
    }
  } else {
    display_sys_calls = TRACE_OFF;
  }
  return 0;
}

int
sys_print_count(void)
{
  for(int i=0;i<NELEM(alphabetical_mapping);i++) {
    int sys_call_number = alphabetical_mapping[i];
    if(count[sys_call_number]!=0) {
    cprintf("%s %d\n",sys_call_names[sys_call_number],count[sys_call_number]);
    }
  }
  return 0;
}


/*
 * 
 * Extra Code
 * 
*/


struct msg msgBuffer[256];
int bufferAllocated[256] = {0};
struct spinlock msgQLocks1[NPROC];
struct spinlock bufferLock;

struct queue msgQ[NPROC];
int initQ = 0;  

int sys_send(void) {
  if(initQ == 0) {
    for(int i=0;i<NPROC;i++) {
      initlock(&(msgQLocks1[i]),"msgQLocks1");
      initlock(&bufferLock,"bufferLock");
    }
    for(int i=0;i<NPROC;i++) {
      init(&msgQ[i]);
    }
    initQ=1;
  }

  int sender_pid;
  int rec_pid;
  void* msg;

  if(argint(0,&sender_pid) <0) {
    return -1;
  }

  if(argint(1,&rec_pid) <0) {
    return -1;
  }

  // if(rec_pid > 30) {
  //   cprintf("gonna fail %d %d\n",rec_pid,myproc()->pid);
  // }

  if(argptr(2,(void*)&msg, sizeof(void*)) <0) {
    return -1;
  }

  struct msg* new_msg;
  acquire(&bufferLock);
  for(int i=0;i<NELEM(msgBuffer);i++) {
    if(bufferAllocated[i] == 0) {
      bufferAllocated[i] = 1;
      release(&bufferLock);
      new_msg = &msgBuffer[i];
      
      new_msg->bufferPosition = i;
      new_msg->sender_pid = sender_pid;
      memmove(new_msg->msg,msg,MSGSIZE);
      new_msg->next = 0;

      acquire(&msgQLocks1[rec_pid]);
      insert(&msgQ[rec_pid],new_msg);
      unblock(rec_pid);
      release(&msgQLocks1[rec_pid]);
      return 0;
    }
  }

  panic("No Free Buffer\n");

}

int sys_recv(void) {
  if(initQ == 0) {
    for(int i=0;i<NPROC;i++) {
      initlock(&(msgQLocks1[i]),"msgQLocks1");
      initlock(&bufferLock,"bufferLock");
    }
    for(int i=0;i<NPROC;i++) {
      init(&(msgQ[i]));
    }
    initQ=1;
  }

  int myid;
  void *msg;

  myid = myproc()->pid;
  
  if(argptr(0,(void*)&msg,sizeof(void*))<0) {
    return -1;
  }

  acquire(&(msgQLocks1[myid]));
  struct msg* msg_obj = remov(&msgQ[myid]);
  
  if(msg_obj == 0) {
    block(&(msgQLocks1[myid]));
    msg_obj = remov(&msgQ[myid]);
  }

  release(&(msgQLocks1[myid]));

  memmove(msg,msg_obj->msg,MSGSIZE);

  if(msg_obj == 0) {
    panic("Msg Obj 0 from q\n");
  }

  bufferAllocated[msg_obj->bufferPosition] = 0;
  return 0;
}