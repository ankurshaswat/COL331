#include "types.h"
#include "stat.h"
#include "user.h"

#define NULL (void *)0

struct point_value {
  float value;
  int index;
};

float fabsm(float a) {
  if (a < 0)
    return -1 * a;
  return a;
}

int min(int a, int b) {
  if (a < b) {
    return a;
  }
  return b;
}

float max(float a, float b) {
  if (a < b) {
    return b;
  }
  return a;
}

int main(int argc, char *argv[]) {

  char filename[] = "assig2a.inp", line;
  int fptr = open(filename, 0), N = 0, P = 0, L = 0, buf;
  float E = 0, T = 0, decimal = 0.1;

  read(fptr, &line, 1);
  while (line != '\n') {
    N = N * 10 + atoi(&line);
    read(fptr, &line, 1);
  }

  read(fptr, &line, 1);
  while (line != '.') {
    if (atoi(&line) != 0) {
      E = E * 10 + atoi(&line);
    } else {
      E *= 10;
    }
    read(fptr, &line, 1);
  }
  read(fptr, &line, 1);
  while (line != '\n') {
    if (atoi(&line) != 0) {
      E += decimal * atoi(&line);
    }
    decimal /= 10;
    read(fptr, &line, 1);
  }

  decimal = 0.1;
  read(fptr, &line, 1);

  while (line != '.') {
    T = T * 10 + atoi(&line);
    read(fptr, &line, 1);
  }
  while (line != '\n') {
    T += decimal * atoi(&line);
    decimal /= 10;
    read(fptr, &line, 1);
  }

  read(fptr, &line, 1);
  while (line != '\n') {
    P = P * 10 + atoi(&line);
    read(fptr, &line, 1);
  }

  buf = read(fptr, &line, 1);
  while (line != '\n' && buf > 0) {
    L = L * 10 + atoi(&line);
    buf = read(fptr, &line, 1);
  }

  close(fptr);

  int i, j, vals_with_me, pids[P], index, receive_count,
      my_pid = getpid(), pipes[P][2], interrupt_pipes[P][2];
  int count = 0, tid = 0, size = N - 2, pid = -1, range_start = 0,
      range_end = 0, pid_above = -1, pid_below = -1;
  int extra_items = size % P, num_items_to_process = size / P;

  float mean = 0.0;
  float u[N][N], w[N][N], value, diff;

  void *msg_space = malloc(sizeof(struct point_value));

  struct point_value point_val;

  pids[0] = my_pid;

  for (i = 0; i < P; i++) {
    if (pipe(pipes[i]) < 0) {
      exit();
    }
    if (pipe(interrupt_pipes[i]) < 0) {
      exit();
    }
  }

  for (i = 0; i < N; i++) {
    u[i][0] = u[i][N - 1] = u[0][i] = T;
    u[N - 1][i] = 0.0;
    mean += u[i][0] + u[i][N - 1] + u[0][i] + u[N - 1][i];
  }
  mean /= (4.0 * N);

  for (i = 1; i < P; i++) {
    tid = i;
    pid = fork();

    if (pid == 0) {
      break;
    } else {
      pids[i] = pid;
    }
  }

  if (pid != 0) {
    tid = 0;
  }

  if (tid > 0) {
    my_pid = getpid();
    pids[tid] = my_pid;
    pid_above = pids[tid - 1];
    write(pipes[tid - 1][1], &my_pid, sizeof(struct point_value));
  }

  if (tid < P - 1) {
    read(pipes[tid][0], msg_space, sizeof(struct point_value));
    pid_below = *((int *)msg_space);
  }

  if (tid + 1 <= extra_items) {
    range_start = tid * (num_items_to_process + 1);
    range_end = min(range_start + num_items_to_process + 1, size);
  } else {
    range_start = extra_items * (num_items_to_process + 1) +
                  (tid - extra_items) * num_items_to_process;
    range_end = min(range_start + num_items_to_process, size);
  }

  range_start++;

  for (i = 1; i <= N - 2; i++)
    for (j = 1; j < N - 1; j++)
      u[i][j] = mean;

  for (;;) {
    if (tid == 0) {
      printf(1, "%d\n", count);
    }

    diff = 0.0;
    for (i = range_start; i <= range_end; i++) {
      for (j = 1; j < N - 1; j++) {
        w[i][j] = (u[i - 1][j] + u[i + 1][j] + u[i][j - 1] + u[i][j + 1]) / 4.0;
        if (fabsm(w[i][j] - u[i][j]) > diff)
          diff = fabsm(w[i][j] - u[i][j]);
      }
    }

    // Send diff to master thread
    if (pid == 0) {
      write(pipes[0][1], &diff, sizeof(struct point_value));
      if (P > 1) {
        read(interrupt_pipes[tid][0], msg_space, sizeof(struct point_value));
      }
      diff = *((float *)msg_space);
    } else {
      receive_count = 1;
      while (receive_count < P) {
        read(pipes[tid][0], msg_space, sizeof(struct point_value));
        diff = max(*((float *)msg_space), diff);
        receive_count++;
      }
      for (i = 1; i < P; i++) {
        write(interrupt_pipes[i][1], &diff, sizeof(struct point_value));
      }
    }

    count++;

    if (diff <= E || count > L) {
      break;
    }

    receive_count = 0;

    // Send values to above and below

    if (pid_above != -1) {
      receive_count += N - 2;
      index = range_start * N + 1;
      for (i = 1; i < N - 1; i++) {
        point_val.index = index;
        point_val.value = w[range_start][i];
        *((struct point_value *)msg_space) = point_val;
        write(pipes[tid - 1][1], msg_space, sizeof(struct point_value));
        index++;
      }
    }
    if (pid_below != -1) {
      receive_count += N - 2;
      index = range_end * N + 1;
      for (i = 1; i < N - 1; i++) {
        point_val.value = w[range_end][i];
        point_val.index = index;

        *((struct point_value *)msg_space) = point_val;
        write(pipes[tid + 1][1], msg_space, sizeof(struct point_value));
        index++;
      }
    }

    // Wait Here For Required Updates if not terminated

    while (receive_count > 0) {
      read(pipes[tid][0], msg_space, sizeof(struct point_value));
      point_val = *((struct point_value *)msg_space);
      index = point_val.index;
      value = point_val.value;
      u[index / N][index % N] = value;
      receive_count--;
    }

    // Synchronize Here

    if (tid == 0) {
      for (i = 1; i < P; i++) {
        write(interrupt_pipes[i][1], msg_space, sizeof(struct point_value));
      }
      for (i = 1; i < P; i++) {
        read(interrupt_pipes[tid][0], msg_space, sizeof(struct point_value));
      }
    } else {
      write(interrupt_pipes[0][1], msg_space, sizeof(struct point_value));
      read(interrupt_pipes[tid][0], msg_space, sizeof(struct point_value));
    }

    for (i = range_start; i <= range_end; i++)
      for (j = 1; j < N - 1; j++)
        u[i][j] = w[i][j];
  }

  if (pid == 0) {

    // Send back all new values to master thread

    for (i = range_start; i <= range_end; i++) {
      for (j = 1; j < N - 1; j++) {
        point_val.value = u[i][j];
        point_val.index = i * N + j;
        *((struct point_value *)msg_space) = point_val;
        write(pipes[0][1], msg_space, sizeof(struct point_value));
      }
    }

    exit();
  }

  receive_count = 1;
  while (receive_count < P) {
    wait();
    receive_count++;
  }

  vals_with_me = (range_end - range_start + 1) * (N - 2);
  receive_count = (N - 2) * (N - 2) - vals_with_me;

  while (receive_count > 0) {
    read(pipes[tid][0], msg_space, sizeof(struct point_value));
    point_val = *((struct point_value *)msg_space);
    index = point_val.index;
    value = point_val.value;
    u[index / N][index % N] = value;
    receive_count--;
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++)
      printf(1, "%d ", (int)u[i][j]);
    printf(1, "\n");
  }

  for (i = 0; i < P; i++) {
    close(pipes[i][0]);
    close(pipes[i][1]);
    close(interrupt_pipes[i][0]);
    close(interrupt_pipes[i][1]);
  }

  exit();
}
