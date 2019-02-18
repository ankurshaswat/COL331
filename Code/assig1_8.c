#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
	if(argc< 2){
		printf(1,"Need type and input filename\n");
		exit();
	}
	char *filename;
	filename=argv[2];
	int type = atoi(argv[1]);
	printf(1,"Type is %d and filename is %s\n",type, filename);

	int tot_sum = 0;	
	float variance = 0.0;

	int size=1000;
	short arr[size];
	char c;
	int fd = open(filename, 0);
	for(int i=0; i<size; i++){
		read(fd, &c, 1);
		arr[i]=c-'0';
		read(fd, &c, 1);
	}	
  	close(fd);
  	// this is to supress warning
  	printf(1,"first elem %d\n", arr[0]);
  
  	//----FILL THE CODE HERE for unicast sum and multicast variance

    int num_proc = 2;
    int master_thread = getpid();

    int tid=0;

    int pid;
    
    for(int i=1;i<num_proc;i++) {
        tid = i;
        pid = fork();
        
        if(pid==0) { // child process
            break;
        }
    }

    if(pid != 0) { // master thread has tid=0
        tid = 0;
    }
    int num_items_to_process;
    if(size%num_proc == 0) {
        num_items_to_process = size / num_proc;
    } else {
        num_items_to_process = (int) (1.0 + ((1.0 * size) / num_proc));
    }

    int local_sum = 0;
    for(int i=tid*num_items_to_process;i<(tid+1)*num_items_to_process && i<size;i++) {
        local_sum += arr[i];
    }

    if(pid == 0) {
        send(getpid(),master_thread,&local_sum);
        exit();
    }
        
    int count =1;
    while(count < num_proc) {
		int* msg;
        int res = recv(&msg);
        if(res>=0) {
            count++;
			local_sum += (*msg);
            // add msg to local sum
        }
    }

  	//------------------

  	if(type==0){ //unicast sum
		printf(1,"Sum of array for file %s is %d\n", filename,tot_sum);
	}
	else{ //mulicast variance
		printf(1,"Variance of array for file %s is %d\n", filename,(int)variance);
	}
	exit();
}
