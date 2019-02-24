#include "types.h"
#include "stat.h"
#include "user.h"
     
float mean = -1;

void signal_handler(void*msg) {
    mean = *((float*)msg);
}

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

    registerI(signal_handler);
    
    int num_proc = 2;
    int thread_ids[num_proc-1];
    int master_thread = getpid();
    int tid=0;
    int pid;
    
    for(int i=1;i<num_proc;i++) {
        tid = i;
        pid = fork();
        
        if(pid==0) { 
            // child process
            registerI(signal_handler);
            break;
        } else {
            thread_ids[i-1] = pid;
            tid = 0;
        }
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
        if(type==0) {
            exit();
        }
    } else {
        int count =1;
        while(count < num_proc) {
		    int *msg = (int *)malloc(MSGSIZE);

            int res = recv(msg);
            
            if(res>=0) {
                count++;
                local_sum += *(msg);
            }
        }

        tot_sum = local_sum;

        if(type!=0) {
            mean = (tot_sum*1.0) / size;
            send_multi(master_thread,thread_ids,&mean,num_proc-1);
        }
    }
    
    if(type!=0) {
        while(1) {
            if(mean>=0){
                break;
            }
        }
        float local_variance = 0;
        for(int i=tid*num_items_to_process;i<(tid+1)*num_items_to_process && i<size;i++) {
            local_variance += (arr[i]-mean)*(arr[i]-mean);
        }
        if(pid==0) {
            send(getpid(),master_thread,&local_variance);
            exit();
        }

        int count =1;
        while(count < num_proc) {
            float msg;
            int res = recv(&msg);

            if(res>=0) {
                count++;
                local_variance += (msg);
            }
        }
        variance = local_variance/size;
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
