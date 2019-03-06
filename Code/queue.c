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
  if(q->head == 0) {
    q->head = n;
    q->tail = n;
  } else {
    q->tail->next = n;
    q->tail = n;
  }
}

struct msg*
remov(struct queue* q) {
  if(q->head==0) {
    return 0;
  } else {
    struct msg* m = q->head;
    if(q->head == q->tail) {
      q->head = 0;
      q->tail = 0;
    } else {
      q->head = q->head->next;
    }
    return m;
  }
}