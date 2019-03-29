#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

struct point_value {
  int row_id;
  float values[];
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

  char filename[] = "assig2a.inp", *line = NULL;
  FILE *fptr = fopen(filename, "r");
  size_t len = 0;
  int N, P, L;
  float E, T;

  getline(&line, &len, fptr);
  N = atoi(line);
  getline(&line, &len, fptr);
  E = atof(line);
  getline(&line, &len, fptr);
  T = atof(line);
  getline(&line, &len, fptr);
  P = atoi(line);
  getline(&line, &len, fptr);
  L = atoi(line);

  fclose(fptr);

  if (N == 1) {
    printf("%f\n", T);
    exit(1);
  } else if (N == 2) {
    printf("%f %f \n", T, T);
    printf("%f %f \n", T, 0.0);
    exit(1);
  }

  if (N - 2 < P) {
    P = N - 2;
  }

  int i, j, pids[P], receive_count, row_id,
      my_pid = getpid(), pipes[P][2], interrupt_pipes[P][2], count = 0, tid = 0,
      size = N - 2, pid = -1, range_start = 0, range_end = 0, pid_above = -1,
      pid_below = -1;
  int extra_items = size % P, num_items_to_process = size / P;

  float mean = 0.0;
  float u[N][N], w[N][N], diff;

  int MSGSIZE = sizeof(struct point_value *) + sizeof(float) * (N - 2);
  struct point_value *point_val = (struct point_value *)malloc(MSGSIZE);

  pids[0] = my_pid;

  for (i = 0; i < P; i++) {
    if (pipe(pipes[i]) < 0) {
      exit(1);
    }
    if (pipe(interrupt_pipes[i]) < 0) {
      exit(1);
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
    write(pipes[tid - 1][1], &my_pid, sizeof(int));
  }

  if (tid < P - 1) {
    read(pipes[tid][0], point_val, sizeof(int));
    pid_below = *((int *)point_val);
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

    // if (tid == 0) {
    //   printf("%d\n", count);
    // }

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
      write(pipes[0][1], &diff, sizeof(float));
      if (P > 1) {
        read(interrupt_pipes[tid][0], point_val, sizeof(float));
      }
      diff = *((float *)point_val);
    } else {
      receive_count = 1;
      while (receive_count < P) {
        read(pipes[tid][0], point_val, sizeof(float));
        diff = max(*((float *)point_val), diff);
        receive_count++;
      }
      for (i = 1; i < P; i++) {
        write(interrupt_pipes[i][1], &diff, sizeof(float));
      }
    }

    count++;

    if (diff <= E || count > L) {
      break;
    }

    receive_count = 0;

    // Send values to above and below

    if (pid_above != -1) {
      receive_count++;
      point_val->row_id = range_start;
      for (i = 1; i < N - 1; i++) {
        point_val->values[i - 1] = w[range_start][i];
      }
      write(pipes[tid - 1][1], point_val, MSGSIZE);
    }
    if (pid_below != -1) {
      receive_count++;
      point_val->row_id = range_end;
      for (i = 1; i < N - 1; i++) {
        point_val->values[i - 1] = w[range_end][i];
      }
      write(pipes[tid + 1][1], point_val, MSGSIZE);
    }

    // Wait Here For Required Updates if not terminated

    while (receive_count > 0) {
      read(pipes[tid][0], point_val, MSGSIZE);
      row_id = point_val->row_id;
      for (int i = 1; i < N - 1; i++) {
        u[row_id][i] = point_val->values[i - 1];
      }
      receive_count--;
    }

    // Synchronize Here
    if (tid == 0) {
      for (i = 1; i < P; i++) {
        read(interrupt_pipes[tid][0], point_val, 1);
      }
      for (i = 1; i < P; i++) {
        write(interrupt_pipes[i][1], point_val, 1);
      }
    } else {
      write(interrupt_pipes[0][1], point_val, 1);
      read(interrupt_pipes[tid][0], point_val, 1);
    }

    for (i = range_start; i <= range_end; i++)
      for (j = 1; j < N - 1; j++)
        u[i][j] = w[i][j];
  }

  if (pid == 0) {

    // Send back all new values to master thread
    for (i = range_start; i <= range_end; i++) {
      point_val->row_id = i;
      for (j = 1; j < N - 1; j++) {
        point_val->values[j - 1] = u[i][j];
      }
      write(pipes[0][1], point_val, MSGSIZE);
    }

    exit(1);
  }

  receive_count = N - 2 - (range_end - range_start + 1);

  while (receive_count > 0) {
    read(pipes[tid][0], point_val, MSGSIZE);
    row_id = point_val->row_id;
    for (int i = 1; i < N - 1; i++) {
      u[row_id][i] = point_val->values[i - 1];
    }
    receive_count--;
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++)
      printf("%f ", u[i][j]);
    printf("\n");
  }

  for (i = 0; i < P; i++) {
    close(pipes[i][0]);
    close(pipes[i][1]);
    close(interrupt_pipes[i][0]);
    close(interrupt_pipes[i][1]);
  }

  exit(1);
}
