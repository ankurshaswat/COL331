#include "types.h"
#include "stat.h"
#include "user.h"

#define N 11
#define E 0.00001
#define T 100.0
#define P 2
#define L 20000

struct point_value
{
    float value;
    int index;
};


volatile float diff = 0.0;

volatile int paused = 0;

void increase_pause(void * msg) {
    paused++;
    return_to_kernel();
}

void unblocker(void *msg) {
    diff = *((float*)msg);
    return_to_kernel();
}

float fabsm(float a){
	if(a<0)
	    return -1*a;
    return a;
}

int min(int a,int b) {
    if(a<b) {
        return a;
    }
    return b;
}

float max(float a,float b) {
    if(a<b) {
        return b;
    }
    return a;
}

int main(int argc, char *argv[])
{
	int i,j;
	float mean;
	float u[N][N];
	float w[N][N];

	int count=0;
	mean = 0.0;
	for (i = 0; i < N; i++){
		u[i][0] = u[i][N-1] = u[0][i] = T;
		u[N-1][i] = 0.0;
		mean += u[i][0] + u[i][N-1] + u[0][i] + u[N-1][i];
	}
	mean /= (4.0 * N);

    int pids[P];
    pids[0] = getpid();

    int tid=0;
    int pid=-1;
    int pid_above = -1;
    int pid_below = -1;
    int my_pid = pids[0];

    for(int i=1;i<P;i++) {
        tid = i;
        pid = fork();

        if(pid == 0) {
            break;
        } else {
            pids[i] = pid;
        }
    }

    
    if(pid != 0) {
        tid = 0;
        registerI(increase_pause);
    } else {
        registerI(unblocker);
    }

    if(tid>0) {
        my_pid = getpid();
        pids[tid] = my_pid;
        pid_above = pids[tid-1];
        send(my_pid,pids[tid-1],&my_pid);
    }

	void *msg_space = malloc(MSGSIZE);

    if(tid < P - 1) {
        recv(msg_space);
        pid_below = *((int*)msg_space);
    }

    int size = N-2;

    int num_items_to_process = size/P; 
    int extra_items = size%P;

    int range_start = 0;
    int range_end = 0;

    if(tid+1 <= extra_items) {
        range_start = tid*(num_items_to_process+1);
        range_end = min(range_start+num_items_to_process+1,size);
    } else {
        range_start = extra_items*(num_items_to_process+1) + (tid-extra_items)*num_items_to_process;
        range_end = min(range_start+num_items_to_process,size);
    }

    range_start++;

	for (i = range_start; i <= range_end; i++ )
		for ( j= 1; j < N-1; j++) u[i][j] = mean;

    struct point_value point_val;

    // int loop_num = 0;
    float local_diff;
    float received_diff;
    float new_diff;
	int loc_count;
    int receive_count;
    int index;
    float value;
    float old_diff;

    for(;;){
        if(tid==0) {
            printf(1,"%d Loop num %d\n",my_pid,count);
        }

		diff = 0.0;
		for(i =range_start ; i <= range_end; i++){
			for(j =1 ; j < N-1; j++){
				w[i][j] = ( u[i-1][j] + u[i+1][j]+
					    u[i][j-1] + u[i][j+1])/4.0;
				if( fabsm(w[i][j] - u[i][j]) > diff )
					diff = fabsm(w[i][j]- u[i][j]);	
			}
		}
 
        // Send diff to master thread
        if(pid==0) {
            local_diff = diff;
            diff = -1;
            send(my_pid,pids[0],&local_diff);
            while(diff<0.0) {
            }
        } else {
            loc_count = 1;
            while(loc_count<P) {
                recv(msg_space);
                received_diff = *((float*)msg_space);
                diff = max(received_diff,diff);
                loc_count++;
            }
            new_diff = diff;
            send_multi(my_pid,pids,&new_diff,P);
        }

	    count++;
	       
		if(diff<= E || count > L){ 
			break;
		}
        receive_count = 0; 
        // Send values to above and below
        if(pid_above != -1) {
            receive_count += N-2;
            index = range_start*N + 1;
            for(i=1;i<N-1;i++) {
                point_val.index = index;
                point_val.value = w[range_start][i];
                *((struct point_value*)msg_space) = point_val;
                send(my_pid,pid_above,msg_space);

                index++;
            }
        }
        if(pid_below != -1) {
            receive_count += N-2;
            index = range_end*N + 1;
            for(i=1;i<N-1;i++) {
                point_val.value = w[range_end][i];
                point_val.index = index;
                *((struct point_value*)msg_space) = point_val;
                send(my_pid,pid_below,msg_space);

                index++;
            }
        }

        // Wait Here For Required Updates if not terminated
        while(receive_count>0) {
            recv(msg_space);
            point_val = *((struct point_value*)msg_space);
            index = point_val.index ;
            value = point_val.value;
            // if(my_pid==4) {
            //     printf(1,"%d %d\n",index/N,index%N);
            // }
            u[index/N][index%N] = value;
            receive_count--;
        }

        // Synchronize Here

        if(tid == 0) {
            while(paused < P-1){
            }
            paused = 0;
            new_diff = diff;
            send_multi(my_pid,pids,&new_diff,P);
        } else {
            old_diff = diff;
            diff = -1;
            send_multi(my_pid,pids,&old_diff,1);
            while(diff<0.0) {
            }
        }

		for (i =range_start; i<= range_end; i++)	
			for (j =1; j< N-1; j++) u[i][j] = w[i][j];
	}

    if(pid == 0) {
        // Send back all new values to master thread
        for (i =range_start; i<= range_end; i++) {
			for (j =1; j< N-1; j++) {
                point_val.value = u[i][j];
                point_val.index = i*N+j;
                *((struct point_value*)msg_space) = point_val;
                send(my_pid,pids[0],msg_space);
            }
        }
        exit();
    }

    int num_joined = 1;
    while(num_joined < P) {
        wait();
        num_joined++;
    }

    int vals_with_me = (range_end-range_start+1)*(N-2);

    int num_vals_to_receive = (N-2)*(N-2) - vals_with_me;

    while(num_vals_to_receive >0) {
        recv(msg_space);
        point_val = *((struct point_value*)msg_space);
        index = point_val.index ;
        value = point_val.value;
        u[index/N][index%N] = value;
        num_vals_to_receive--;
    }


	for(i =0; i <N; i++){
		for(j = 0; j<N; j++)
			printf(1,"%d ",((int)u[i][j]));
		printf(1,"\n");
	}
	exit();

}