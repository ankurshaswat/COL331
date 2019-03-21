#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

struct msg {
  char req_type;
  long time_stamp;
  int index;
};

struct requestQnode {
  int index;
  long time_stamp;
  struct requestQnode *next;
};

struct q {
  struct requestQnode *head, *tail;
};

struct requestQnode *createNode(int index, long time_stamp) {
  struct requestQnode *node =
      (struct requestQnode *)malloc(sizeof(struct requestQnode));
  node->index = index;
  node->time_stamp = time_stamp;
  node->next = NULL;
  return node;
}

struct q *createQ() {
  struct q *qu = (struct q *)malloc(sizeof(struct q));
  qu->head = qu->tail = NULL;
  return qu;
}

void insert(struct q *qu, int index, long time_stamp) {
  struct requestQnode *node = createNode(index, time_stamp);
  if (qu->tail == NULL) {
    qu->head = qu->tail = node;
    return;
  }
  qu->tail->next = node;
  qu->tail = node;
}

struct requestQnode *remove_item(struct q *qu) {
  if (qu->head == NULL) {
    return NULL;
  }
  struct requestQnode *top = qu->head;
  qu->head = qu->head->next;
  if (qu->head == NULL) {
    qu->tail = NULL;
  }
  return top;
}

int higherPriorityExists(struct q *qu, long time_stamp) {
  struct requestQnode *top = qu->head;
  while (top != NULL) {
    if (top->time_stamp < time_stamp) {
      return 1;
    }
    top = top->next;
  }
  return 0;
}

int empty(struct q *qu) {
  if (qu->head == NULL) {
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  char filename[] = "assig2b.inp";
  char ch;
  int P, P1, P2, P3, type;
  int i, tid, pid;
  struct msg msg_space;
  int locked = 0;
  int locked_by;
  // int fd = open(filename, 0);

  FILE *fptr;
  fptr = fopen(filename, "r");
  long locker_time_stamp;
  char *line = NULL;
  size_t len = 0;
  //   int read;

  getline(&line, &len, fptr);
  P = atoi(line);
  getline(&line, &len, fptr);
  P1 = atoi(line);
  getline(&line, &len, fptr);
  P2 = atoi(line);
  getline(&line, &len, fptr);
  P3 = atoi(line);

  int j;
  int num_rows = 1;
  int row_num, col_num;
  while (1) {
    if (P == num_rows * num_rows) {
      break;
    }
    num_rows++;
  }
  int pipes[num_rows][num_rows][2];
  int failed = 0;
  for (i = 0; i < num_rows; i++) {
    for (j = 0; j < num_rows; j++) {
      if (pipe(pipes[i][j]) < 0) {
        exit(1);
      }
    }
  }

  // generate processes with types
  for (i = 0; i < num_rows; i++) {
    for (j = 0; j < num_rows; j++) {

      if (i == 0 && j == 0) {
        continue;
      }

      tid = i * num_rows + j;
      //   printf("%d (%d,%d)\n",tid,i,j);
      pid = fork();

      if (pid != 0) {
        row_num = i;
        col_num = j;
        break;
      } else {
        row_num = 0;
        col_num = 0;
        tid = 0;
      }
    }
    if (pid != 0) {
      break;
    }
  }

  struct q *requestQ = createQ();
  struct q *relinquishQ = createQ();

  int locks_achieved = 0;

  if (tid + 1 >= 1 && tid + 1 < P1 + 1) {
    type = 1;
  } else if (tid + 1 >= P1 + 1 && tid + 1 < P1 + P2 + 1) {
    type = 2;
  } else {
    type = 3;
  }

  // printf("%d type %d  row %d col %d\n", tid, type,row_num,col_num);

  if (type == 2 || type == 3) {
    // Send Requests
    msg_space.req_type = 'R';
    msg_space.index = row_num * num_rows + col_num;
    msg_space.time_stamp = time(0);
    for (i = 0; i < num_rows; i++) {
      if (i != col_num) {
        write(pipes[row_num][i][1], &msg_space, sizeof(struct msg));
      }
      write(pipes[i][col_num][1], &msg_space, sizeof(struct msg));
    }
  }

  struct requestQnode *tempNode;

  while (1) {
    read(pipes[row_num][col_num][0], &msg_space, sizeof(struct msg));
    printf(" %c received from %d %d by %d %d\n", msg_space.req_type,
           msg_space.index / num_rows, msg_space.index % num_rows, row_num,
           col_num);
    switch (msg_space.req_type) {
    case 'R': /* Request */
      if (locked == 0) {
        locked = 1;
        locked_by = msg_space.index;
        locker_time_stamp = msg_space.time_stamp;

        msg_space.req_type = 'L';
        msg_space.index = row_num * num_rows + col_num;
        msg_space.time_stamp = time(0);

        write(pipes[locked_by / num_rows][locked_by % num_rows][1], &msg_space,
              sizeof(struct msg));
      } else {
        /* Add request to request queue */
        insert(requestQ, msg_space.index, msg_space.time_stamp);

        if (!(locker_time_stamp < msg_space.time_stamp ||
              higherPriorityExists(requestQ, msg_space.time_stamp))) {
          /* Send inquire to current locker */
          msg_space.req_type = 'I';
          msg_space.index = row_num * num_rows + col_num;
          msg_space.time_stamp = time(0);

          write(pipes[locked_by / num_rows][locked_by % num_rows][1],
                &msg_space, sizeof(struct msg));
        }
      }
      break;
    case 'L': /* Locked */
      /* Increase achieved locks count */
      locks_achieved++;
      break;
    case 'F': /* Failed */
      /* Remember failure */
      failed = 1;
      /* Send relinquish to all items in inquire q */
      do {
        tempNode = remove_item(relinquishQ);

        msg_space.req_type = 'Q';
        msg_space.index = row_num * num_rows + col_num;
        msg_space.time_stamp = time(0);

        locks_achieved--;

        /* Send relinquish to temp */
        write(pipes[tempNode->index / num_rows][tempNode->index % num_rows][1],
              &msg_space, sizeof(struct msg));
      } while (tempNode != NULL);
      break;
    case 'I': /* Inquire */
      if (failed == 1) {
        /* If failed previously send relinquish back*/
        int receiver_addr = msg_space.index;

        locks_achieved--;

        msg_space.req_type = 'Q';
        msg_space.index = row_num * num_rows + col_num;
        msg_space.time_stamp = time(0);

        write(pipes[receiver_addr / num_rows][receiver_addr % num_rows][1],
              &msg_space, sizeof(struct msg));
      } else {
        /* Else store relinquish in q to defer msg */
        insert(relinquishQ, msg_space.index, msg_space.time_stamp);
      }
      break;
    case 'Q': /* Relinquish */
    {
      int relinquisherAdd = msg_space.index;
      long relinquisher_time_stamp = msg_space.time_stamp;
      /* Get q item and send locked to it */
      tempNode = remove_item(requestQ);

      msg_space.req_type = 'L';
      msg_space.index = row_num * num_rows + col_num;
      msg_space.time_stamp = time(0);

      write(pipes[tempNode->index / num_rows][tempNode->index % num_rows][1],
            &msg_space, sizeof(struct msg));
      /* Add requester to wait q */
      insert(requestQ, relinquisherAdd, relinquisher_time_stamp);
      break;
    }
    case 'r': /* Released */
      if (empty(requestQ)) {
        locked = 0;
      } else {
        /* If request q non empty send locked to first item */
        tempNode = remove_item(requestQ);

        msg_space.req_type = 'L';
        msg_space.index = row_num * num_rows + col_num;
        msg_space.time_stamp = time(0);

        write(pipes[tempNode->index / num_rows][tempNode->index % num_rows][1],
              &msg_space, sizeof(struct msg));
      }
      break;

    default:
      printf("Check types %c", msg_space.req_type);
      break;
    }

    if (locks_achieved == 2 * num_rows - 1) {
      printf("%d acquired the lock at time %li\n", getpid(), time(0));
      if (type == 2) {
        sleep(2);
      }
      printf("%d released the lock at time %li\n", getpid(), time(0));

      /* Send released to all vertical and horizontal neighbors */
      msg_space.req_type = 'r';
      msg_space.index = row_num * num_rows + col_num;
      msg_space.time_stamp = time(0);
      for (i = 0; i < num_rows; i++) {
        if (i != col_num) {
          write(pipes[row_num][i][1], &msg_space, sizeof(struct msg));
        }
        write(pipes[i][col_num][1], &msg_space, sizeof(struct msg));
      }
      // break;
      locks_achieved++;
    }
  }
      printf("geeg");

  if (tid == 0) {
    int stat;
    for (int i = 0; i < P - 1; i++) {
      wait(&stat);
      printf("geeg");
    }
  }
  exit(1);
}