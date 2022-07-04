#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define TRUE 1
#define FALSE 0
#define MIN 1


/*This is the name of the mutex semaphore for the shared memory segment*/
#define MUTEX "/mutex_named_semaphore"

struct Table
{
	int occupied;
	int customerPid;
	int waiterPid;
	int personsMax;
	int tableSerial;
};

struct Bar
{
	int occupiedSeats;
	int freeSeats;
};

int chargeCustomers(int maxCharge);

int main(int argc, char** argv)
{
	//if(argc<2)
	//{
	//	printf("Not enough input arguments\nExiting . . .");
	//	exit(1);
	//}
	printf("I am waiter %d\n",getpid() );
	//sleep(10);
	/*Now get your input arguments*/
	char* shmemFlag="-s";
	int shmemId;
	char* moneyamountFlag="-m";
	int moneyamountPos;
	char* numOfTablesFlag="-t";
	int allTables;
	int j;
	for(j=0;j<argc;++j)
	{
		if(strcmp(argv[j],shmemFlag)==0)
		{
			//shmemId=atoi(argv[j+1]);
			sscanf(argv[j+1],"%d",&shmemId);
			printf("(%d)shmemId=%d\n",getpid(),shmemId );
			continue;
		}
		else if(strcmp(argv[j],moneyamountFlag)==0)
		{
			moneyamountPos=j+1;
			continue;
		}
		else if(strcmp(argv[j],numOfTablesFlag)==0)
		{
			allTables=atoi(argv[j+1]);
			continue;
		}
	}
	printf("(%d)Got my input arguments, going to attach my shared memory segment\n",getpid());
	/*Now waiter process must attach the shared memory segment*/
	void* shmatRet;
	shmatRet=shmat(shmemId,(void*)0, 0);
	if((int)shmatRet==-1)
	{
		printf("(%d)Attachment error.\nForce exit.\n",getpid());
		exit(1);
	}
	else
	{/*The shared memory segment was attached successfully*/
		printf("(%d)Shared memory segment attached %p\n",getpid(),shmatRet);
		void* shmemStart=shmatRet;/*Where the shared memory segment starts. This value should not change*/
		/*And now open the named semaphore*/
		sem_t* mutex=sem_open(MUTEX,0);
		printf("(%d)Opened name semaphore %s \n",getpid(),MUTEX );
		/*Waiter code*/
		while(1)
		{/*Waiter will be killed by the parent*/
			/*First check the shared memory segment for customers waiting to be served*/
			//printf("(%d)Waiter Looping\n",getpid() );
			struct Table aTable;
			int tookOrder=FALSE;
			/*Enter the critical section*/
		//	printf("(%d)Right before CS\n",getpid() );
			sem_wait(mutex);/*P(mutex)*/
			//int tookOrder=FALSE;
		//	printf("(%d)IN CS\n",getpid() );
			for(j=0;j<allTables;++j)
			{
				//printf("(%d)j=%d\n",getpid(),j);
				memmove((void*)&aTable,shmatRet,sizeof(struct Table));
				//printf("(%d)Table I am going to check\n",getpid() );
				//printf("\tTable#%d\n",aTable.tableSerial );
				//printf("\tOccupied:%d\n",aTable.occupied );
				//printf("\tCustomer:%d\n",aTable.customerPid );
				//printf("\tWaiter:%d\n",aTable.waiterPid );
				if((tookOrder==FALSE)&&(aTable.occupied==TRUE)&&(aTable.waiterPid==-1))
				{/*The table is occupied, but there is no waiter arranged to it*/
					printf("(%d)Going to take order from table#%d\n",getpid(),aTable.tableSerial );
					aTable.waiterPid=getpid();/*Arrange the table to yourself*/
					memmove(shmatRet,(void*)&aTable,sizeof(struct Table));
					tookOrder=TRUE;/*Each waiter gets one table at a time*/
				}
				else if((aTable.occupied==TRUE)&&(aTable.customerPid==-1)&&(aTable.waiterPid==getpid()))
				{/*If they want to pay and YOU are their waiter*/
					printf("(%d)Going to charge table#%d\n",getpid(),aTable.tableSerial );
					int charge=chargeCustomers(atoi(argv[moneyamountPos]));
					printf("(%d)Charged them\n",getpid() );
					aTable.occupied=FALSE;
					aTable.waiterPid=-1;
					memmove(shmatRet,(void*)&aTable,sizeof(struct Table));/*Empty the table*/
					printf("(%d)Cleaned table#%d\n",getpid(),aTable.tableSerial );

				}
				shmatRet=shmatRet+sizeof(struct Table);/*Move on to the next table*/
			}
			/*You are done with the critical section*/
		//	printf("(%d)Getting out of CS\n",getpid() );
			sem_post(mutex);/*V(mutex)*/
			/*Since a waiter's code is all about the CS, let's make sure that every other process will have time to execute*/
			printf("(%d)Waiter going to sleep for a while\n",getpid() );
			shmatRet=shmemStart;
			sleep(2);
		}
	}
}



int chargeCustomers(int maxCharge)
{
	//printf("CHARGING 1\n");
	time_t t;
	srand((unsigned) time(&t));
	//int charge=
	//printf("CHARGING 2\n");
	return ((int)(rand()%(maxCharge+1-MIN)+MIN));
}