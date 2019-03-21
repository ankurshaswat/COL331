#include "types.h"
#include "stat.h"
#include "user.h"

#define N 11
#define E 0.00001
#define T 100.0
#define P 6
#define L 20000

struct point_value
{
    float value;
    int index;
};


volatile float diff = 0.0;

volatile int paused = 0;

void unblocker(void *msg) {
    // printf(1,"Got here\n");
    diff = *((float*)msg);
    return_to_kernel();
    printf(1,"Shouldn't be here\n");
}

void increase_pause(void * msg) {
    // printf(1,"Got here2\n");
    paused++;
    return_to_kernel();
    printf(1,"Shouldn't be here2\n");
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
    int vals_with_me ,i,j ,pids[P],receive_count,index;
    int tid=0 ,pid_below = -1 ,pid_above = -1 ,pid=-1 ,count=0 ,size = N-2  ,range_start = 0 ,range_end = 0 ,my_pid ;
    int num_items_to_process = size/P, extra_items = size%P;

	float mean ,u[N][N] ,w[N][N] ,local_diff ,value;

	void *msg_space = malloc(MSGSIZE);

    struct point_value point_val;


	mean = 0.0;
    my_pid = getpid();
    pids[0] = my_pid;
	for (i = 0; i < N; i++){
		u[i][0] = u[i][N-1] = u[0][i] = T;
		u[N-1][i] = 0.0;
		mean += u[i][0] + u[i][N-1] + u[0][i] + u[N-1][i];
	}
	mean /= (4.0 * N);

	for (i = 1; i <= N-2; i++ )
		for ( j= 1; j < N-1; j++) u[i][j] = mean;
    
    registerI(increase_pause);

    for(i=1;i<P;i++) {
        tid = i;
        pid = fork();
        if(pid == 0) {
            /* Children */
            registerI(unblocker);
            break;
        }
        // if(getpid() == 3) {
        //     printf(1,"inside\n");
        // }
        /* Master Thread */
        pids[i] = pid;
        tid = 0;
    }
    sleep(10);
    if(tid == 0) {
        /* Master Thread */
        while(paused < P-1){
        }
        paused = 0;
        local_diff = diff;
        send_multi(my_pid,pids,&local_diff,P);
    } else {
        /* Children */
        local_diff = diff;
        diff = -1;
        send_multi(my_pid,pids,&local_diff,1);
        while(diff<0.0) {
        }
    }

    // printf(1,"pid %d\n",pid);
    
    if(pid != 0) { 
        /* Master thread */
        tid = 0;
        if(P>1) {
            pid_below = pids[1];
        }
    } else { 
        /* Children */
        my_pid = getpid();
        pids[tid] = my_pid;
        pid_above = pids[tid-1];
        if(tid-1 != 0) {
            send(my_pid,pids[tid-1],&my_pid);
        }
    }

    if(tid < P - 1 && tid != 0) {
        recv(msg_space);
        pid_below = *((int*)msg_space);
    }

    if(tid+1 <= extra_items) {
        range_start = tid*(num_items_to_process+1);
        range_end = min(range_start+num_items_to_process+1,size);
    } else {
        range_start = extra_items*(num_items_to_process+1) + (tid-extra_items)*num_items_to_process;
        range_end = min(range_start+num_items_to_process,size);
    }

    range_start++;

    // printf(1,"Here\n");

    if(tid == 0) {
        /* Master Thread */
        while(paused < P-1){
        }
        paused = 0;
        local_diff = diff;
        send_multi(my_pid,pids,&local_diff,P);
    } else {
        /* Children */
        local_diff = diff;
        diff = -1;
        send_multi(my_pid,pids,&local_diff,1);
        while(diff<0.0) {
        }
    }
    // printf(1,"Her1e\n");

    // exit();

    for(;;){
        if(tid==0) {
            printf(1,"%d\n",count);
        }

		diff = 0.0;
		for(i =range_start ; i <= range_end; i++){
			for(j =1 ; j < N-1; j++){
				w[i][j] = ( u[i-1][j] + u[i+1][j]+ u[i][j-1] + u[i][j+1])/4.0;
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
            receive_count = 1;
            while(receive_count<P) {
                recv(msg_space);
                local_diff = *((float*)msg_space);
                diff = max(local_diff,diff);
                receive_count++;
            }
            local_diff = diff;
            send_multi(my_pid,pids,&local_diff,P);
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
            u[index/N][index%N] = value;
            receive_count--;
        }

        // Synchronize Here

        if(tid == 0) {
            while(paused < P-1){
            }
            paused = 0;
            local_diff = diff;
            send_multi(my_pid,pids,&local_diff,P);
        } else {
            local_diff = diff;
            diff = -1;
            send_multi(my_pid,pids,&local_diff,1);
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

    receive_count = 1;
    while(receive_count < P) {
        wait();
        receive_count++;
    }

    vals_with_me = (range_end-range_start+1)*(N-2);
    receive_count = (N-2)*(N-2) - vals_with_me;

    while(receive_count >0) {
        recv(msg_space);
        point_val = *((struct point_value*)msg_space);
        index = point_val.index ;
        value = point_val.value;
        u[index/N][index%N] = value;
        receive_count--;
    }


	for(i =0; i <N; i++){
		for(j = 0; j<N; j++)
			printf(1,"%d ",((int)u[i][j]));
		printf(1,"\n");
	}
	exit();

}