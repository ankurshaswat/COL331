#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {

  int container_num = create_container();
  printf(1,"%d started container %d\n",getpid(),container_num);
  container_num = create_container();
  printf(1,"%d started container %d\n",getpid(),container_num);
  while(1) {}
  exit();
  //   /* Creating the containers . */
  //   int container1 = create_container();
  //   int container2 = create_container();
  //   int container3 = create_container();

  //   /* Three container managers ( user programs ) should be running now */

  //   // ==========================================================

  //   /* Multiple process creation to test the scheduler */
  //   fork();
  //   join_container(1); // called only by child created by preceeding fork
  //   call. fork(); join_container(1); // called only by child created by
  //   preceeding fork call. fork(); join_container(1); // called only by child
  //   created by preceeding fork call.

  //   /* Testing of resource isolation */
  //   fork();
  //   join_container(2); // called only by child created by preceeding fork
  //   call. fork(); join_container(3); // called only by child created by
  //   preceeding fork call.

  //   /* ---------- PROCESS ISOLATION ------------------*/
  //   // called by atmost one process in every container .
  //   ps();

  //   /* ---------------- SCHEDULER TEST - - - - - - - - - - - - - - - - - - -
  //   - - -
  //    * - - */

  //   scheduler_log_on(); // This will enable logs from the container scheduler

  //   /* Print statements of form ( without the quotes ): */
  //   // " Container + <container_id > : Scheduling process + <pid "
  //   // ">"
  //   scheduler_log_off(); // Disable the scheduler log after scheduling all
  //   the
  //                        // child process atleast once.

  //   /* ---------------- MEMORY ISOLATION TEST - - - - - - - - - - - - - - - -
  //      - - - - - - - - */
  //   memory_log_on(); // This will enable the logs from the memory allocator .
  //                    // It will print the mapping from GVA to HVA everytime a
  //                    // malloc request comes in.
  //   // Executed by all the child processes across conatiners .
  //   void *m = malloc(uint s);
  //   // print the GVA and HVA , i.e. the mapping created in the container ’s
  //   page
  //   // table . "GVA -> HVA"
  //   memory_log_off(); // Disable the memory logs after printing
  //                     // mapping from all the containers atleast once.

  //   /* ---------------- FILE SYSTEM TEST - - - - - - - - - - - - - - - - - -
  //   -
  //      - - - - - */

  //   /* Executed by all the child processes in all the containers */
  //   ls();                    // This will print the host file system
  //   create(" file_ " + pid); // pid can be same across containers , however ,
  //                            // in this case they will be different as they
  //                            are
  //                            // created using fork and join a container later
  //                            .

  //   // BARRIER
  //   // All the process should finish the create system call
  //   ls(); // the container should see files created by processes running
  //   inside
  //         // in it along with the original files from the host system.

  //   /* Executed by only one child process in every container */
  //   create(" my_file ");
  //   open(" my_file ",0x002);
  //   write(" Modified by: " + pid);
  //   close(" my_file ");
  //   cat(" my_file "); // The contents of the file should be different for
  //   process
  //                     // running across containers.

  //   /* Executed by all the child processes */
  //   leave_container();

  //   // ==========================================================
  //   // Executed by the main process
  //   destroy_container(1);
  //   destroy_container(2);
  //   destroy_container(3);
  //   exit();
}