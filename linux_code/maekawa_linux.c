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

void insert_in_priority(struct q *qu, int index, long time_stamp) {
  struct requestQnode *node = createNode(index, time_stamp),
                      *prev_node = qu->head, *temp_node = qu->head;
  if (qu->tail == NULL) {
    qu->head = qu->tail = node;
    return;
  }

  if (prev_node->time_stamp > time_stamp ||
      (prev_node->time_stamp == time_stamp && prev_node->index > index)) {
    node->next = qu->head;
    qu->head = node;
    return;
  }

  while (1) {
    prev_node = temp_node;
    temp_node = temp_node->next;
    if (temp_node == NULL) {
      qu->tail->next = node;
      qu->tail = node;
      return;
    } else if (temp_node->time_stamp > time_stamp ||
               (temp_node->time_stamp == time_stamp &&
                temp_node->index > index)) {

      prev_node->next = node;
      node->next = temp_node;

      return;
    }
  }
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

int higherPriorityExists(struct q *qu, long time_stamp, int index) {
  struct requestQnode *top = qu->head;
  if (top == NULL) {
    return 0;
  } else {
    if (top->time_stamp < time_stamp ||
        (top->time_stamp == time_stamp && top->index < index)) {
      return 1;
    }
    return 0;
  }
}

int empty(struct q *qu) {
  if (qu->head == NULL) {
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[]) {

  char filename[] = "assig2b.inp", *line = NULL;

  int P, P1, P2, type, i, j, my_index, tid, pid, receiver_addr, row_num,
      col_num, locked_by, locked = 0, num_rows = 1, locks_achieved = 0,
                          failed = 0;

  struct msg msg_space;

  struct q *requestQ = createQ(), *relinquishQ = createQ();

  struct requestQnode *tempNode;

  FILE *fptr;

  fptr = fopen(filename, "r");

  long locker_time_stamp, relinquisher_time_stamp;

  size_t len = 0;

  getline(&line, &len, fptr);
  P = atoi(line);
  getline(&line, &len, fptr);
  P1 = atoi(line);
  getline(&line, &len, fptr);
  P2 = atoi(line);

  while (1) {
    if (P == num_rows * num_rows) {
      break;
    }
    num_rows++;
  }

  int pipes[num_rows][num_rows][2];

  for (i = 0; i < num_rows; i++) {
    for (j = 0; j < num_rows; j++) {
      if (pipe(pipes[i][j]) < 0) {
        exit(1);
      }
    }
  }

  /* Generate processes with types */
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

  my_index = row_num * num_rows + col_num;

  if (tid + 1 >= 1 && tid + 1 < P1 + 1) {
    type = 1;
  } else if (tid + 1 >= P1 + 1 && tid + 1 < P1 + P2 + 1) {
    type = 2;
  } else {
    type = 3;
  }

  // printf("%d type %d  row %d col %d\n", tid, type,row_num,col_num);

  if (type == 2 || type == 3) {
    /* Send Requests */
    msg_space.req_type = 'R';
    msg_space.index = my_index;
    msg_space.time_stamp = time(0);
    for (i = 0; i < num_rows; i++) {
      if (i != col_num) {
        write(pipes[row_num][i][1], &msg_space, sizeof(struct msg));
      }
      write(pipes[i][col_num][1], &msg_space, sizeof(struct msg));
    }
  }

  char str[10];
  sprintf(str, "%d_%d.txt", row_num, col_num);

  FILE *fp = fopen(str, "w");

  while (1) {
    read(pipes[row_num][col_num][0], &msg_space, sizeof(struct msg));
    fprintf(fp, "%d %d received %c from %d %d\n", row_num, col_num,
            msg_space.req_type, msg_space.index / num_rows,
            msg_space.index % num_rows);

    switch (msg_space.req_type) {
    case 'R': /* Request */
      if (locked == 0) {
        locked = 1;
        locked_by = msg_space.index;
        locker_time_stamp = msg_space.time_stamp;

        msg_space.req_type = 'L';
        msg_space.index = my_index;
        msg_space.time_stamp = time(0);

        write(pipes[locked_by / num_rows][locked_by % num_rows][1], &msg_space,
              sizeof(struct msg));

        fprintf(fp, "%d %d sent %c to %d %d\n", row_num, col_num,
                msg_space.req_type, locked_by / num_rows, locked_by % num_rows);
      } else {
        /* Add request to request queue */
        insert_in_priority(requestQ, msg_space.index, msg_space.time_stamp);

        if (!(locker_time_stamp < msg_space.time_stamp ||
              higherPriorityExists(requestQ, msg_space.time_stamp,
                                   msg_space.index) ||
              (locker_time_stamp == msg_space.time_stamp &&
               locked_by < msg_space.index))) {
          /* Send inquire to current locker */
          msg_space.req_type = 'I';
          msg_space.index = my_index;
          msg_space.time_stamp = time(0);

          write(pipes[locked_by / num_rows][locked_by % num_rows][1],
                &msg_space, sizeof(struct msg));
          fprintf(fp, "%d %d sent %c to %d %d\n", row_num, col_num,
                  msg_space.req_type, locked_by / num_rows,
                  locked_by % num_rows);
        } else {

          receiver_addr = msg_space.index;

          msg_space.req_type = 'F';
          msg_space.index = my_index;
          msg_space.time_stamp = time(0);

          write(pipes[receiver_addr / num_rows][receiver_addr % num_rows][1],
                &msg_space, sizeof(struct msg));
          fprintf(fp, "%d %d sent %c to %d %d\n", row_num, col_num,
                  msg_space.req_type, receiver_addr / num_rows,
                  receiver_addr % num_rows);
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
      tempNode = remove_item(relinquishQ);

      while (tempNode != NULL) {

        msg_space.req_type = 'Q';
        msg_space.index = my_index;
        msg_space.time_stamp = time(0);

        locks_achieved--;

        /* Send relinquish to temp */
        write(pipes[tempNode->index / num_rows][tempNode->index % num_rows][1],
              &msg_space, sizeof(struct msg));
        fprintf(fp, "%d %d sent %c to %d %d\n", row_num, col_num,
                msg_space.req_type, tempNode->index / num_rows,
                tempNode->index % num_rows);
        tempNode = remove_item(relinquishQ);
      }
      break;
    case 'I': /* Inquire */
      if (failed == 1) {
        /* If failed previously send relinquish back*/
        receiver_addr = msg_space.index;

        locks_achieved--;

        msg_space.req_type = 'Q';
        msg_space.index = my_index;
        msg_space.time_stamp = time(0);

        write(pipes[receiver_addr / num_rows][receiver_addr % num_rows][1],
              &msg_space, sizeof(struct msg));
        fprintf(fp, "%d %d sent %c to %d %d\n", row_num, col_num,
                msg_space.req_type, receiver_addr / num_rows,
                receiver_addr % num_rows);
      } else {
        /* Else store relinquish in q to defer msg */
        insert(relinquishQ, msg_space.index, msg_space.time_stamp);
      }
      break;
    case 'Q': /* Relinquish */
    {
      receiver_addr = msg_space.index;
      relinquisher_time_stamp = msg_space.time_stamp;
      /* Get q item and send locked to it */
      tempNode = remove_item(requestQ);

      msg_space.req_type = 'L';
      msg_space.index = my_index;
      msg_space.time_stamp = time(0);

      locked = 1;
      locked_by = tempNode->index;
      locker_time_stamp = tempNode->time_stamp;

      write(pipes[locked_by / num_rows][locked_by % num_rows][1], &msg_space,
            sizeof(struct msg));
      fprintf(fp, "%d %d sent %c to %d %d\n", row_num, col_num,
              msg_space.req_type, locked_by / num_rows, locked_by % num_rows);
      /* Add requester to wait q */
      insert_in_priority(requestQ, receiver_addr, relinquisher_time_stamp);
      break;
    }
    case 'r': /* Released */
      if (empty(requestQ)) {
        locked = 0;
      } else {
        /* If request q non empty send locked to first item */
        tempNode = remove_item(requestQ);

        msg_space.req_type = 'L';
        msg_space.index = my_index;
        msg_space.time_stamp = time(0);

        locked = 1;
        locked_by = tempNode->index;
        locker_time_stamp = tempNode->time_stamp;

        write(pipes[locked_by / num_rows][locked_by % num_rows][1], &msg_space,
              sizeof(struct msg));
        fprintf(fp, "%d %d sent %c to %d %d\n", row_num, col_num,
                msg_space.req_type, locked_by / num_rows, locked_by % num_rows);
      }
      break;

    default:
      printf("Check types %c", msg_space.req_type);
      break;
    }

    if (locks_achieved == 2 * num_rows - 1) {
      printf("%d acquired the lock at time %li\n", getpid(), time(0));
      fprintf(fp, "%d acquired the lock at time %li\n", getpid(), time(0));
      if (type == 2) {
        sleep(2);
      }
      printf("%d released the lock at time %li\n", getpid(), time(0));
      fprintf(fp, "%d released the lock at time %li\n", getpid(), time(0));

      /* Send released to all vertical and horizontal neighbors */
      msg_space.req_type = 'r';
      msg_space.index = my_index;
      msg_space.time_stamp = time(0);
      for (i = 0; i < num_rows; i++) {
        if (i != col_num) {
          write(pipes[row_num][i][1], &msg_space, sizeof(struct msg));
          fprintf(fp, "%d %d sent %c to %d %d\n", row_num, col_num,
                  msg_space.req_type, row_num, i);
        }
        write(pipes[i][col_num][1], &msg_space, sizeof(struct msg));
        fprintf(fp, "%d %d sent %c to %d %d\n", row_num, col_num,
                msg_space.req_type, i, col_num);
      }
      // break;
      locks_achieved++;
    }
    fflush(fp);
  }

  exit(1);
}