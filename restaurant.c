#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define LINE 200
#define TRUE 1
#define FALSE 0
#define INT_SIZE 50
#define ACCESS_RIGHTS 0666
#define FIFO_ACCESS_RIGHTS 0666
#define R_RAND_MAX 1000
#define MIN 1

/*These definitions below are for the configuration file*/
#define ini_W "WAITERS"
#define ini_B "BAR"
#define ini_C "CUSTOMERS"
#define ini_T2 "TABLES2"
#define ini_T4 "TABLES4"
#define ini_T6 "TABLES6"
#define ini_T8 "TABLES8"

/*This is the name of the mutex semaphore for the shared memory segment*/
#define MUTEX "/mutex_named_semaphore"
/*This semaphore is for fifo read-write*/
#define FIFOSEMAPHORE "/semaphore_for_res_door_fifo"

/*Paths and names for children*/
#define CUSTOMERPATH "FULL_PATH_TO_PROJECT_FOLDER/customer"
#define CUSTOMEREXEC "customer"
#define WAITERPATH "FULL_PATH_TO_PROJECT_FOLDER/waiter"
#define WAITEREXEC "waiter"
#define DOORMANPATH "FULL_PATH_TO_PROJECT_FOLDER/doorman"
#define DOORMANEXEC "doorman"

/*This is partly the path for the fifo*/
#define FIFOPATH "FULL_PATH_TO_PROJECT_FOLDER"

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

int confgProcess(char* fileName, int* waiters, int* customersM, int* bar, int* tabls2, int* tabls4, int* tabls6, int* tabls8);
void toString(int theInteger,char** stringed);
int maxChargeForWaiter();
void myFifoName(int batch, char** fifoName);

int main(int argc, char** argv)
{
	/*First check if you have enough input arguments*/
	if(argc<3)
	{
		printf("Not enough input arguments.\nExiting . . .");
		exit(1);
	}
	/*Now gather your input arguments*/
	printf("Now going to get our input arguments\n");
	char* custmsFlag="-n";
	int customers;
	char* confgFileFlag="-l";
	int confgFile;
	int i;
	for(i=0;i<argc;++i)
	{
		if(strcmp(argv[i],custmsFlag)==0)
		{
			customers=i+1;
		}
		else if(strcmp(argv[i],confgFileFlag)==0)
		{
			confgFile=i+1;
		}
	}
	/*Now process the configuration file*/
	int waiters, maxNumOfCustms, barSeats, tabl2, tabl4, tabl6, tabl8;
	printf("Now going to process the configuration file\n");
	if(confgProcess(argv[confgFile],&waiters,&maxNumOfCustms,&barSeats,&tabl2,&tabl4,&tabl6,&tabl8)==FALSE)
	{
		printf("Error in input. Force exit.\n");
		exit(1);
	}
	else
	{/*File processing went fine, we got initial data*/
		/*Restaurant should create the shared memory segments*/
		printf("The configuration file was processed successfully\n");
		int allTables=tabl2+tabl4+tabl6+tabl8;
		int size=sizeof(struct Bar)+allTables*sizeof(struct Table);
		int shmemId=shmget(IPC_PRIVATE,size,ACCESS_RIGHTS);
		printf("shmemId=%d\n",shmemId );
		void* shmatRet;
		void* shmemStart; /*This helps us remember the start of the shared memory segment. This variable's value should not change.*/
		if(shmemId<0)
		{/*Error*/
			printf("Cannot allocate memory.Force exit.\n");
			exit(1);
		}
		else
		{/*Shared memory segment allocated, go attach it*/
			printf("Shared memory segment allocated\n");
			shmatRet=shmat(shmemId,(void*)0,0);
			if((int)shmatRet==-1)
			{
				printf("Memory failure. Force exit.\n");
				exit(1);
			}
			else
			{/*Shared memory segment attached fine, now initialize it*/
				shmemStart=shmatRet;
				printf("Shared memory segment attached %p\n",shmatRet);
				struct Bar theBar;
				theBar.occupiedSeats=0;
				theBar.freeSeats=barSeats;

				struct Table aTable;
				aTable.occupied=FALSE;
				aTable.waiterPid=-1;

				//int offset=0;
				//int whereYouLeftMe;
				printf("Now going for initialization\n");
				aTable.personsMax=2;
				int tabNum=0;
				for(i=0;i<tabl2;++i)
				{
					aTable.tableSerial=tabNum;
					//if(i!=0)
					//{
					//	shmatRet=shmatRet+sizeof(struct Table);
					//}
					memmove(shmatRet,(void *)&aTable,sizeof(struct Table));
					shmatRet=shmatRet+sizeof(struct Table);
					//++offset;
					++tabNum;
				}
				printf("Done with two tables\n");
				//whereYouLeftMe=i;
				printf("Going for four tables\n");
				aTable.personsMax=4;
				for(i=0;i<tabl4;++i)
				{
					aTable.tableSerial=tabNum;
				//	shmatRet=shmatRet+sizeof(struct Table);
					memmove(shmatRet,(void *)&aTable,sizeof(struct Table));
					shmatRet=shmatRet+sizeof(struct Table);
					//++offset;
					++tabNum;
				}
				printf("Done with four tables.\nGoing for six tables\n");
				aTable.personsMax=6;
				for(i=0;i<tabl6;++i)
				{
					aTable.tableSerial=tabNum;
					//shmatRet=shmatRet+sizeof(struct Table);
					memmove(shmatRet,(void *)&aTable,sizeof(struct Table));
					shmatRet=shmatRet+sizeof(struct Table);
					//++offset;
					++tabNum;
				}
				printf("Done with six tables.\nGoing for eight tables\n");
				aTable.personsMax=8;
				for(i=0;i<tabl8;++i)
				{
					aTable.tableSerial=tabNum;
					//shmatRet=shmatRet+sizeof(struct Table);
					memmove(shmatRet,(void *)&aTable,sizeof(struct Table));
					shmatRet=shmatRet+sizeof(struct Table);
					//++offset;
					++tabNum;
				}
				/*And now the bar*/
				//shmatRet=shmatRet+sizeof(struct Table);
				printf("Done with tables\nGoing for the bar\n");
				memmove(shmatRet,(void *)&theBar,sizeof(struct Bar));

				/*DEBUGGING AREA*/
				/*Retrieve the info you've written*/
				//shmatRet=shmemStart;/*Go back to the start*/
				//struct Table anotherTable;
				//printf("Now go check what you have stored in the shared memory segment\n");
				//for(i=0;i<allTables;++i)
				//{
				//	memmove((void *) &anotherTable,shmatRet,sizeof(struct Table));
				//	shmatRet=shmatRet+sizeof(struct Table);
				//	printf("Table #%d, occupied:%d and personsMax:%d\n",anotherTable.tableSerial,anotherTable.occupied,anotherTable.personsMax );
				//}
				//struct Bar anotherBar;
				//memmove((void *) &anotherBar, shmatRet,sizeof(struct Bar));
				//printf("Bar seats occupied:%d and free:%d\n",anotherBar.occupiedSeats,anotherBar.freeSeats );
				/*END OF DEBUGGING AREA*/

				/*
				Before forking children, restaurant should create the semaphore for the shared memory segment.
				The semaphore name is defined, so there is no need to be passed through command line arguments
				*/
				/*We will use a named semaphore called mutex, whose initial value should be 1*/
				sem_t* mutex=sem_open(MUTEX,O_CREAT|O_EXCL, S_IRUSR|S_IWUSR,1);
				if(mutex==SEM_FAILED)
				{
					printf("Cannot create mutex named semaphore\nForce exit\n");
					exit(1);
				}
				printf("Created semaphore %s with initial value ",MUTEX );
				sem_t* fifoSem=sem_open(FIFOSEMAPHORE,O_CREAT|O_EXCL,S_IRUSR|S_IWUSR,0);
				if(fifoSem==SEM_FAILED)
				{
					printf("Cannot create fifo semaphore\nForce exit\n");
					exit(1);
				}
				int semVal;
				sem_getvalue(mutex,&semVal);
				printf("%d\n",semVal );
				printf("Going to fork children\n");
				int custmsBatch=0;
				int numOfCustomers=5;
				int numOfWaiters=2;
				int customers[numOfCustomers]; /*Pids for customers*/
				int waiters[numOfWaiters]; /*Pids for waiters*/
				int doorman; /*Pid for doorman*/
				char* shmemIdStr; /*Shared memory id to string, to be passed as input argument*/
				toString(shmemId,&shmemIdStr);
				char* allTablesSTr;
				char* tables2STr;
				char* tables4STr;
				char* tables6STr;
				char* tables8STr;
				char* maxCharge;
				toString(allTables,&allTablesSTr);
				toString(tabl2,&tables2STr);
				toString(tabl4,&tables4STr);
				toString(tabl6,&tables6STr);
				toString(tabl8,&tables8STr);
				doorman=fork();
				if(doorman==0)
				{/*Doorman code*/
					execlp(DOORMANPATH,DOORMANEXEC,"-s",shmemIdStr,"-t2",tables2STr,"-t4",tables4STr,"-t6",tables6STr,"-t8",tables8STr,(char*)NULL);
				}
				printf("(%d)Forked doorman with pid %d\n",getpid(),doorman );
				free(tables2STr);
				free(tables4STr);
				free(tables6STr);
				free(tables8STr);
				for(i=0;i<numOfWaiters;++i)
				{/*Waiter forking*/
					toString(maxChargeForWaiter(),&maxCharge);/*Moneyamount*/
					int pid=fork();
					if(pid==0)
					{
						execlp(WAITERPATH,WAITEREXEC,"-s",shmemIdStr,"-t",allTablesSTr,"-m",maxCharge,(char*)NULL);
					}
					else
					{
						printf("(%d)Forked waiter with pid %d\n", getpid(),pid);
						waiters[i]=pid;
					}
					free(maxCharge);
				}
				/*Now going to fork customers*/
				for(i=0;i<numOfCustomers;++i)
				{/*Customer forking*/
					int pid=fork();
					if(pid==0)
					{/*Customer code*/
						execlp(CUSTOMERPATH,CUSTOMEREXEC,"-s",shmemIdStr,"-t",allTablesSTr,(char*)NULL);
					}
					else
					{
						printf("(%d)Forked customer with pid %d\n",getpid(), pid);
						customers[i]=pid;
					}
				}
				/*Now make the fifo to inform the doorman about the new batch of customers*/
				printf("(%d)Generated customers, going to pass the info to the doorman\n",getpid() );
				char* myFifoToDoorman;
				myFifoName(custmsBatch,&myFifoToDoorman);
				printf("(%d)The name of my fifo with doorman is %s\n",getpid(),myFifoToDoorman );
				int f=mkfifo(myFifoToDoorman,FIFO_ACCESS_RIGHTS);
				if(f==-1)
				{
					printf("(%d)Cannot create fifo\n",getpid() );
				}
				else
				{
					printf("(%d)Going to open my fifo with the doorman\n",getpid() );
					int fd=open(myFifoToDoorman,O_WRONLY);
					if(fd==-1)
					{
						printf("(%d)Cannot open fifo %s\n",getpid(),myFifoToDoorman );
					}
					else
					{
						printf("(%d)Fifo %s opened, going to write\n",getpid() ,myFifoToDoorman);
						write(fd,&numOfCustomers,sizeof(int));
						for(i=0;i<numOfCustomers;++i)
						{
							write(fd,&(customers[i]),sizeof(int));	
							printf("(%d)Wrote %d\n",getpid(),customers[i] );
						}
						printf("(%d)Going to V the semaphore for the doorman to read\n",getpid() );
						sem_post(fifoSem);
						close(fd);
						unlink(myFifoToDoorman);
					}
				}
				free(myFifoToDoorman);
				++custmsBatch; /*Go for the next batch of customers*/
				/*End of customer forking code*/
				/*And now wait for the customers to finish*/
				printf("(%d)Parent made the fifo,gave the info, and now going to wait for everyone\n",getpid());
				int status;
				int counter=0;
				int finished;
				while(counter<numOfCustomers)
				{
					finished=wait(&status);
					for(i=0;i<numOfCustomers;++i)
					{
						if(finished==customers[i])
						{
							printf("(%d)Customer %d finished\n",getpid(),finished );
							++counter;
							break;
						}
					}
					printf("(%d)Looping again,counter=%d\n",getpid(),counter );
				}
				/*Kill waiter*/
				for(i=0;i<numOfWaiters;++i)
				{
					kill(waiters[i],SIGKILL);
				}
				printf("(%d)Going to kill the doorman\n",getpid() );
				kill(doorman,SIGKILL);
				printf("Killed waiter(s).\nGoing to detach and leave.\n");
			/*Generate your waiters and customers*/
				//printf("Going to detach and leave\n");
			/*Now go clean up*/
				free(shmemIdStr);
				/*Destroy the mutex semaphore*/
				sem_unlink(MUTEX);/*Unlink*/
				sem_close(mutex);/*Close the semaphore*/
				sem_unlink(FIFOSEMAPHORE);
				sem_close(fifoSem);
				printf("Semaphore %s destroyed\n",MUTEX );
				/*Detach the shared memory segment*/
				int error=shmctl(shmemId,IPC_RMID,0);
				if(error==-1)
				{
					printf("Detachment error\n");
				}
				else
				{
					printf("Detachment OK\n");
				}
			}
		}
	}
	printf("Exiting . . .\n");
}



int confgProcess(char* fileName, int* waiters, int* customersM, int* bar, int* tabls2, int* tabls4, int* tabls6, int* tabls8)
{/*This function processes the configuration file*/
	FILE *fp;
	fp=fopen(fileName,"r");
	if(fp==NULL)
	{
		printf("Cannot open file %s\n",fileName );
		return FALSE;
	}
	else
	{
		char buffer[LINE];
		char instruction[20];
		char semicolon[3];
		int number;
		while(fgets(buffer,LINE,fp)!=NULL)
		{/*Read until EOF*/
			if(sscanf(buffer,"%s %s %d",instruction,semicolon,&number)==3)
			{/*Read one line successfully*/
				if(strcmp(instruction,ini_W)==0)
				{/*Get the number of waiters*/
					(*waiters)=number;
				}
				else if(strcmp(instruction,ini_C)==0)
				{/*Get the number of customers*/
					(*customersM)=number;
				}
				else if(strcmp(instruction,ini_B)==0)
				{/*Get the number of seats in the bar area*/
					(*bar)=number;
				}
				else if(strcmp(instruction,ini_T2)==0)
				{/*Get the number of tables for two*/
					(*tabls2)=number;
				}
				else if(strcmp(instruction,ini_T4)==0)
				{/*Get the number of tables for four*/
					(*tabls4)=number;
				}
				else if(strcmp(instruction,ini_T6)==0)
				{/*Get the number of tables for six*/
					(*tabls6)=number;
				}
				else if(strcmp(instruction,ini_T8)==0)
				{/*Get the number of tables for eight*/
					(*tabls8)=number;
				}
				else
				{/*Maybe an error, do nothing*/
					printf("Bad data in this line of file %s\n",fileName );
				}
			}
			else
			{/*Maybe an error*/
				printf("Bad data in this line of file %s\n",fileName);
			}
		}
		fclose(fp);
		return TRUE;
	}
}

void toString(int theInteger, char** stringed)
{/*This function turns an integer to a string*/
	(*stringed)=malloc(INT_SIZE*sizeof(char));
	if(stringed==NULL)
	{
		printf("Cannot turn integer to string\n");
		return;
	}
	sprintf((*stringed),"%d",theInteger);
	return;
}

int maxChargeForWaiter()
{
	time_t t;
	srand((unsigned) time(&t));
	return ((int)(rand()%(R_RAND_MAX+1-MIN)+MIN));
}

void myFifoName(int batch, char** fifoName)
{
	char* batchToStr;
	toString(batch,&batchToStr);
	//printf("THE POINTER IS %p\n",(*fifoName) );
	(*fifoName)=malloc((strlen(FIFOPATH)+strlen(batchToStr)+strlen("_batch.fifo")+1)*sizeof(char));
	strcpy((*fifoName),FIFOPATH);
	strcat((*fifoName),batchToStr);
	strcat((*fifoName),"_batch.fifo");
	free(batchToStr);
}
