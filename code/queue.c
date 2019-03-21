#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "queue.h"

void
init(struct queue* q) {
  q->head = 0;
  q->tail = 0;
}

void
insert(struct queue* q,struct msg* n) {
  n->next = 0;
  
  if(q->tail == 0) {
    q->head = q->tail = n;
    return;
  }

  q->tail->next = n;
  q->tail = n;

}

struct msg*
remov(struct queue* q) {

  if(q->head==0) {
    return 0;
  }

  struct msg* m = q->head;
  q->head = q->head->next;

  if(q->head == 0) {
    q->tail = 0;
  }

  return m;  
}