#include "types.h"
#include "stat.h"
#include "user.h"

#define NULL (void *)0

struct msg {
  char req_type;
  int time_stamp;
  int index;
};

struct requestQnode {
  int index;
  int time_stamp;
  struct requestQnode *next;
};

struct q {
  struct requestQnode *head, *tail;
};

struct requestQnode *createNode(int index, int time_stamp) {
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

void insert(struct q *qu, int index, int time_stamp) {
  struct requestQnode *node = createNode(index, time_stamp);
  if (qu->tail == NULL) {
    qu->head = qu->tail = node;
    return;
  }
  qu->tail->next = node;
  qu->tail = node;
}

void insert_in_priority(struct q *qu, int index, int time_stamp) {
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

int higherPriorityExists(struct q *qu, int time_stamp, int index) {
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

  char *filename = "assig2b.inp", line;
  int fptr = open(filename, 0), P = 0, P1 = 0, P2 = 0;

  read(fptr, &line, 1);
  while (line != '\n') {
    P = P * 10 + atoi(&line);
    read(fptr, &line, 1);
  }
  read(fptr, &line, 1);
  while (line != '\n') {
    P1 = P1 * 10 + atoi(&line);
    read(fptr, &line, 1);
  }
  read(fptr, &line, 1);
  while (line != '\n') {
    P2 = P2 * 10 + atoi(&line);
    read(fptr, &line, 1);
  }
  close(fptr);

  int type, i, j, my_index, tid, pid, receiver_addr, row_num, col_num,
      locked_by, locked = 0, num_rows = 1, locks_achieved = 0, failed = 0,
                 count_pipe[2];

  struct msg msg_space;

  struct q *requestQ = createQ(), *relinquishQ = createQ();

  struct requestQnode *tempNode;

  long locker_time_stamp, relinquisher_time_stamp;

  while (1) {
    if (P == num_rows * num_rows) {
      break;
    }
    num_rows++;
  }

  int pipes[num_rows][num_rows][2], pids[num_rows][num_rows],
      locks_achieved_pos[num_rows][num_rows];

  for (i = 0; i < num_rows; i++) {
    for (j = 0; j < num_rows; j++) {
      locks_achieved_pos[i][j] = 0;
      if (pipe(pipes[i][j]) < 0) {
        exit();
      }
    }
  }

  if (pipe(count_pipe) < 0) {
    exit();
  }

  /* Generate processes with types */
  for (i = 0; i < num_rows; i++) {
    for (j = 0; j < num_rows; j++) {

      tid = i * num_rows + j;

      // printf("%d (%d,%d)\n", tid, i, j);

      pid = fork();

      if (pid == 0) {
        row_num = i;
        col_num = j;
        break;
      } else {
        pids[i][j] = pid;
      }
    }
    if (pid == 0) {
      break;
    }
  }

  if (pid != 0) {
    int counted = 0;
    while (counted < P - P1) {
      read(count_pipe[0], &msg_space, sizeof(struct msg));
      // printf("Counted %d\n",counted);
      counted++;
    }
    // printf("Exiting\n");

    close(count_pipe[0]);
    close(count_pipe[1]);

    for (int i = 0; i < num_rows; i++) {
      for (int j = 0; j < num_rows; j++) {
        kill(pids[i][j]);
        close(pipes[i][j][0]);
        close(pipes[i][j][1]);
      }
    }
    exit();
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
    msg_space.time_stamp = uptime();
    for (i = 0; i < num_rows; i++) {
      if (i != col_num) {
        write(pipes[row_num][i][1], &msg_space, sizeof(struct msg));
      }
      write(pipes[i][col_num][1], &msg_space, sizeof(struct msg));
    }
  }

  while (1) {
    read(pipes[row_num][col_num][0], &msg_space, sizeof(struct msg));

    switch (msg_space.req_type) {
    case 'R': /* Request */
      if (locked == 0) {
        locked = 1;
        locked_by = msg_space.index;
        locker_time_stamp = msg_space.time_stamp;

        msg_space.req_type = 'L';
        msg_space.index = my_index;

        write(pipes[locked_by / num_rows][locked_by % num_rows][1], &msg_space,
              sizeof(struct msg));

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

          write(pipes[locked_by / num_rows][locked_by % num_rows][1],
                &msg_space, sizeof(struct msg));
        } else {

          receiver_addr = msg_space.index;

          msg_space.req_type = 'F';
          msg_space.index = my_index;

          write(pipes[receiver_addr / num_rows][receiver_addr % num_rows][1],
                &msg_space, sizeof(struct msg));
        }
      }
      break;
    case 'L': /* Locked */
      /* Increase achieved locks count */
      locks_achieved++;
      locks_achieved_pos[msg_space.index / num_rows]
                        [msg_space.index % num_rows] = 1;
      break;
    case 'F': /* Failed */
      /* Remember failure */
      failed = 1;
      /* Send relinquish to all items in inquire q */
      tempNode = remove_item(relinquishQ);

      while (tempNode != NULL) {

        msg_space.req_type = 'Q';
        msg_space.index = my_index;

        if (locks_achieved_pos[tempNode->index / num_rows]
                              [tempNode->index % num_rows] == 1) {
          locks_achieved_pos[tempNode->index / num_rows]
                            [tempNode->index % num_rows] = 0;
          locks_achieved--;
        }

        /* Send relinquish to temp */
        write(pipes[tempNode->index / num_rows][tempNode->index % num_rows][1],
              &msg_space, sizeof(struct msg));
        // fprintf(fp, "%d %d sent %c to %d %d\n", row_num, col_num,
        //         msg_space.req_type, tempNode->index / num_rows,
        //         tempNode->index % num_rows);
        tempNode = remove_item(relinquishQ);
      }
      break;
    case 'I': /* Inquire */
    {

      if (locks_achieved_pos[msg_space.index / num_rows]
                            [msg_space.index % num_rows] == 0) {
        break;
      }
      if (failed == 1) {
        /* If failed previously send relinquish back*/
        receiver_addr = msg_space.index;

        if (locks_achieved_pos[msg_space.index / num_rows]
                              [msg_space.index % num_rows] == 1) {
          locks_achieved_pos[msg_space.index / num_rows]
                            [msg_space.index % num_rows] = 0;
          locks_achieved--;
        } else {
          continue;
        }

        msg_space.req_type = 'Q';
        msg_space.index = my_index;

        write(pipes[receiver_addr / num_rows][receiver_addr % num_rows][1],
              &msg_space, sizeof(struct msg));
      } else {
        /* Else store relinquish in q to defer msg */
        insert(relinquishQ, msg_space.index, msg_space.time_stamp);
      }
      break;
    }
    case 'Q': /* Relinquish */
    {
      receiver_addr = msg_space.index;
      relinquisher_time_stamp = msg_space.time_stamp;
      /* Get q item and send locked to it */
      tempNode = remove_item(requestQ);

      msg_space.req_type = 'L';
      msg_space.index = my_index;

      locked = 1;
      locked_by = tempNode->index;
      locker_time_stamp = tempNode->time_stamp;

      write(pipes[locked_by / num_rows][locked_by % num_rows][1], &msg_space,
            sizeof(struct msg));
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

        locked = 1;
        locked_by = tempNode->index;
        locker_time_stamp = tempNode->time_stamp;

        write(pipes[locked_by / num_rows][locked_by % num_rows][1], &msg_space,
              sizeof(struct msg));
      }
      break;

    default:
      printf(1, "Check types %c", msg_space.req_type);
      break;
    }

    if (locks_achieved == 2 * num_rows - 1) {
      printf(1, "%d acquired the lock at time %d\n", getpid(), uptime());
      if (type == 2) {
        sleep(200);
      }
      printf(1, "%d released the lock at time %d\n", getpid(), uptime());

      write(count_pipe[1], &msg_space, sizeof(struct msg));

      /* Send released to all vertical and horizontal neighbors */
      msg_space.req_type = 'r';
      msg_space.index = my_index;
      for (i = 0; i < num_rows; i++) {
        if (i != col_num) {
          write(pipes[row_num][i][1], &msg_space, sizeof(struct msg));
        }
        write(pipes[i][col_num][1], &msg_space, sizeof(struct msg));
      }
      locks_achieved++;
    }
  }
}