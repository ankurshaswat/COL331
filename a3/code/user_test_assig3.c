#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{

  int i = -1, pid = -1, type = -1, pipe_id = 0, pipes[6][2], container1, container2, container3;

  /* Setting up communication */

  for (i = 0; i < 6; i++)
    if (pipe(pipes[i]) < 0)
      exit();

  /* ---------------------- */

  /* Creating the containers . */
  container1 = create_container();
  printf(1, "Created Container ID = %d\n", container1);
  container2 = create_container();
  printf(1, "Created Container ID = %d\n", container2);
  container3 = create_container();
  printf(1, "Created Container ID = %d\n", container3);

  /* Three container managers ( user programs ) should be running now */

  /* Multiple process creation to test the scheduler */
  type = 1;
  for (i = 0; i < 3; i++)
  {
    pipe_id++;
    pid = fork();
    if (pid == 0)
    {
      join_container(container1); // called only by child created by preceeding fork call.
      break;
    }
  }

  /* Testing of resource isolation */
  if (pid != 0)
  {
    type = 2;
    pipe_id++;
    pid = fork();
    if (pid == 0)
    {
      i = 0;
      join_container(container2); // called only by child created by preceeding fork call.
    }
  }

  if (pid != 0)
  {
    type = 3;
    pipe_id++;
    pid = fork();
    if (pid == 0)
    {
      i = 0;
      join_container(container3); // called only by child created by preceeding fork call.
    }
    else
    {
      type = -1;
      pipe_id = 0;
    }
  }

  /* ---------------- BARRIER ---------------- */

  int *temp = (int *)malloc(sizeof(int));

  if (type == -1)
  {
    for (i = 1; i < 6; i++)
    {
      read(pipes[pipe_id][0], temp, sizeof(temp));
    }
    for (i = 1; i < 6; i++)
    {
      write(pipes[i][1], temp, sizeof(temp));
    }
  }
  else
  {
    write(pipes[0][1], temp, sizeof(temp));
    read(pipes[pipe_id][0], temp, sizeof(temp));
  }

  /* ---------------- PROCESS ISOLATION ---------------- */
  // called by atmost one process in every container .
  if (pid == 0 && (type != 1 || (type == 1 && i == 1)))
    ps();

  printf(1, "", type);

  // /* ---------------- SCHEDULER TEST ---------------- */

  // scheduler_log_on(); // This will enable logs from the container scheduler

  // /* Print statements of form ( without the quotes ): */
  // // " Container + <container_id > : Scheduling process + <pid >"

  // scheduler_log_off(); // Disable the scheduler log after scheduling all the child process atleast once.

  // /* ---------------- MEMORY ISOLATION TEST ---------------- */

  // memory_log_on(); // This will enable the logs from the memory allocator . It will print the mapping from GVA to HVA everytime a malloc request comes in.

  // // Executed by all the child processes across conatiners .
  // void *m = malloc(uint s);
  // // print the GVA and HVA , i.e. the mapping created in the container ’s
  // // page table . "GVA -> HVA"

  // memory_log_off(); // Disable the memory logs after printing mapping from all the containers atleast once.

  // /* ---------------- FILE SYSTEM TEST ---------------- */

  // /* Executed by all the child processes in all the containers */
  // ls();                    // This will print the host file system
  // create(" file_ " + pid); // pid can be same across containers , however ,in this case they will be different as they are created using fork and join a container later.

  // // BARRIER
  // // All the process should finish the create system call
  // ls(); // the container should see files created by processes running inside in it along with the original files from the host system.

  // /* Executed by only one child process in every container */
  // create(" my_file ");
  // open(" my_file ", 0x002);
  // write(" Modified by: " + pid);
  // close(" my_file ");
  // cat(" my_file "); // The contents of the file should be different for process running across containers.

  // /* Executed by all the child processes */
  // leave_container();

  // // ==========================================================
  // // Executed by the main process
  // destroy_container(1);
  // destroy_container(2);
  // destroy_container(3);
  exit();
}