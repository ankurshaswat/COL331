#include "types.h"
#include "stat.h"
#include "user.h"

int main(void)
{
	toggle(); 
	printf(1,"%s\n","IPC Test case");
	char *msg = (char *)malloc(20);
	int myid;
	int from;	
	
	int cid = fork();
	if(cid==0){
		// This is child
		int stat=-1;
		while(stat==-1){
			stat = recv(&myid ,&from, msg);
		}
		printf(1,"CHILD: Stat is:  %d  and myid: %d\n",stat,myid );
		printf(1,"CHILD: Message from: %d, msg is: %s \n",from, msg );

		msg = "Message from child";
		send(myid,from,msg);	
		exit();
	}else{
		// This is parent
		msg = "Message from parent";
		send(getpid(),cid,msg);	

		int stat=-1;
		while(stat==-1){
			stat = recv(&myid ,&from, msg);
		}
		printf(1,"PARENT: Stat is:  %d  and myid: %d\n",stat,myid );
		printf(1,"PARENT: Message from:  %d , msg is: %s \n",from, msg );
	}
	
	exit();
}
