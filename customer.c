#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define TRUE 1
#define FALSE 0
#define FIFO_ACCESS_RIGHTS 0666
#define INT_SIZE 50
#define MAX_NUM_OF_PEOPLE 8
#define MIN_NUM_OF_PEOPLE 1

/*This is the name of the mutex semaphore for the shared memory segment*/
#define MUTEX "/mutex_named_semaphore"

/*This is partly the path for the fifo*/
#define FIFOPATH "FULL_PATH_TO_PROJECT_FOLDER"

struct waitingCustomer
{
	int customerPid;
	int persons;
};

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

void toString(int theInteger, char** stringed);
void semaphoreName(int pid, char** semName);
void myFifoName(int pid,char** fifoName);
void cleanUpSemaphore(char* semName,sem_t* semaphore);
void waitYourOrder(int time);
void eatANDchat(int time);
int numberOfPeople();

int main(int argc, char** argv)
{
	if(argc<2)
	{
		printf("Not enough input arguments.\nExiting . . .");
		exit(1);
	}
	/*Get input arguments*/
	printf("I am a customer %d \n",getpid());
	char* shmemFlag="-s";
	int shmemId;
	char* numOfTablesFlag="-t";
	int allTables;
	int k;
	for(k=0;k<argc;++k)
	{
		if(strcmp(argv[k],shmemFlag)==0)
		{
			//shmemId=atoi(argv[k+1]);
			sscanf(argv[k+1],"%d",&shmemId);
			printf("(%d)shmemId=%d\n",getpid(),shmemId );
			continue;
		}
		if(strcmp(argv[k],numOfTablesFlag)==0)
		{
			allTables=atoi(argv[k+1]);
			continue;
		}
	}
	printf("(%d)Got my input arguments going to attach my shared memory segment\n",getpid() );
	/*Customer waiting code*/
	/*This is the code only for the waiting customer, that means a part only, for debbuging purposes*/
	/*
	As soon as it is generated, the customer must wait in a queue to be served in the restaurant.
	To do that, it creates a semaphore, with initial value equal to zero, and then it decrements it.
	To give the doorman info about himself, it writes everything that should be known to a fifo that should be read by the doorman.
	*/
	/*First ask for a semaphore to wait on*/
	printf("(%d)Customer\n",getpid() );
	char* semName;
	semaphoreName(getpid(), &semName);
	printf("(%d)Going to create named semaphore %s with initial value equal to 0\n",getpid(),semName );
	sem_t* waitingSemaphore=sem_open(semName,O_CREAT|O_EXCL, S_IRUSR|S_IWUSR,0);
	if(waitingSemaphore==SEM_FAILED)
	{
		printf("Cannot create mutex named semaphore\nForce exit\n");
		exit(1);
	}
	else
	{
		printf("(%d)Created my waiting semaphore %s\n",getpid(),semName );
		printf("(%d)Now going to create my fifo\n",getpid() );
		int weAreCompany=numberOfPeople();/*How many ppl are in the group*/
		//int weAreCompany=4;
		char* myFifo;
		myFifoName(getpid(),&myFifo);
		int f=mkfifo(myFifo,FIFO_ACCESS_RIGHTS);
		if(f==-1)
		{
			printf("(%d)Cannot create fifo %s\nForce exit\n",getpid(),myFifo);
			cleanUpSemaphore(semName,waitingSemaphore);
			free(semName);
			exit(1);
		}
		else
		{
			printf("(%d)Successfully created fifo %s\n",getpid(),myFifo );
			printf("(%d)Now going to make my waiting self\n",getpid() );
			struct waitingCustomer myself;
			myself.customerPid=getpid();
			myself.persons=weAreCompany;
			int fd;/*File descriptor for my fifo*/
			printf("(%d)Created myself;going to try open the fifo\n",getpid() );
			fd=open(myFifo,O_WRONLY);
			if(fd<0)
			{
				printf("Cannot open fifo %sForce exit\n",myFifo );
				unlink(myFifo);
				cleanUpSemaphore(semName,waitingSemaphore);
				free(semName);
				exit(1);
			}
			else
			{
				write(fd,&myself,sizeof(struct waitingCustomer));
				close(fd);
				unlink(myFifo);
			}
			/*Now that you've given the doorman the info about yourself, go wait on the semaphore*/
			printf("(%d)Customer going to wait\n",getpid() );
			sem_wait(waitingSemaphore);
			//sleep(100);
			printf("(%d)Customer woke up\n",getpid() );
			cleanUpSemaphore(semName,waitingSemaphore);
			printf("(%d)Released my named semaphore\n",getpid() );
		}
	}
	printf("(%d)Customer going to eat\n",getpid() );
	/*End of customer waiting code*/

	/*Customer in restaurant code*/

	/*First attach the shared memory segment*/
	void* shmatRet;
	shmatRet=shmat(shmemId,(void*)0, 0);
	if((int)shmatRet==-1)
	{
		printf("(%d)Attachment error.\nForce exit.",getpid());
		exit(1);
	}
	else
	{
		/*The shared memory segment was attached successfully*/
		printf("(%d)Shared memory segment attached %p\n",getpid(),shmatRet);
		void* shmemStart=shmatRet;/*Where the shared memory segment starts. This value should not change*/
		/*And now open the named semaphore*/
		sem_t* mutex=sem_open(MUTEX,0);
		printf("(%d)Opened name semaphore %s \n",getpid(),MUTEX );
		/*Customer code*/
		int myTableSerial=-1;
		int myWaiterPid=-1;
		struct Table myTable;
		printf("(%d)Going to learn what my table is\n",getpid() );
		sem_wait(mutex);/*P(mutex)*/
		for(k=0;k<allTables;++k)
		{
			memmove((void*)&myTable,shmatRet,sizeof(struct Table));
			if((myTable.occupied==TRUE)&&(myTable.customerPid==getpid()))
			{
				printf("(%d)Found my table:%d\n",getpid(),myTable.tableSerial );
				printf("(%d)Waiter's pid:%d\n",getpid(),myTable.waiterPid );
				printf("(%d)Persons max:%d\n",getpid(),myTable.personsMax);
				printf("(%d)Occupied:%d\n",getpid(),myTable.occupied );
				break;
			}
			shmatRet=shmatRet+sizeof(struct Table);
		}
		sem_post(mutex);/*V(mutex)*/
		//while(myTableSerial==-1)
		//{/*Get yourself a table*/
			/*Enter the critical section*/
			//printf("(%d)Getting myself a table\n",getpid() );
			//sem_wait(mutex);/*P(mutex)*/
			//printf("(%d)IN CS\n",getpid() );
			//for(k=0;k<allTables;++k)
			//{
			//	printf("(%d) k=%d\n",getpid(),k );
			//	memmove((void*)&myTable,shmatRet,sizeof(struct Table));
			//	printf("(%d)Going to check the table I got\n",getpid() );
			//	if(myTable.occupied==FALSE)
			//	{
			//		printf("(%d)Found a free table\n",getpid() );
					/*Occupy the first free table you find*/
			//		myTableSerial=myTable.tableSerial;
			//		myTable.occupied=TRUE;
			//		myTable.customerPid=getpid();
			//		memmove(shmatRet,(void*)&myTable,sizeof(struct Table));
					/*DEBUGGING AREA*/
					//memmove((void*)&myTable,shmatRet,sizeof(struct Table));
					//printf("(%d)Printing my table:\n",getpid() );
					//printf("\tTable#%d\n",myTable.tableSerial );
					//printf("\tOccupied:%d\n",myTable.occupied );
					//printf("\tCustomer:%d\n",myTable.customerPid );
					//printf("\tWaiter:%d\n",myTable.waiterPid );
					/*END OF DEBUGGING AREA*/
			//		break;
			//	}
			//	shmatRet=shmatRet+sizeof(struct Table);
			//	printf("(%d)Will keep searching for a table\n",getpid() );
			//}
			/*You are done with the critical section*/
			//printf("(%d)Going to exit the CS\n",getpid() );
			//sem_post(mutex);/*V(mutex)*/
			//printf("(%d)oUT OF CS\n",getpid() );
		//}
		//printf("(%d)Got myself a table with serial number #%d\n",getpid(),myTable.tableSerial );
		/*Now wait for a waiter to come to you*/
		printf("(%d)Now going to wait for a waiter\n",getpid() );
		while(myTable.waiterPid==-1)
		{
			printf("(%d)Waiting for waiter\n",getpid() );
			waitYourOrder(2);/*Wait for 2 ms*/
			printf("(%d)Woke up!\n",getpid() );
			/*Now check if you have a waiter arranged*/
			sem_wait(mutex);
			memmove((void*)&myTable,shmatRet,sizeof(struct Table));
			sem_post(mutex);
			printf("(%d)Checked my table and got:%d\n",getpid(),myTable.tableSerial );
			printf("(%d)waiterPid:%d\n",getpid(),myTable.waiterPid );
		}
		printf("(%d)Founf myself a waiter %d\n",getpid(),myTable.waiterPid );
		//shmatRet=shmemStart; /*Go back to the start*/
		/*Customer got table and waiter, time to eat and chat*/
		printf("(%d)Going to eat and chat\n",getpid() );
		eatANDchat(4);
		printf("(%d)Ate and chatted, going to pay\n",getpid() );
		/*Customer ate, time to pay and leave*/
		/*Enter the critical section once again*/
		myTable.customerPid=-1;

		sem_wait(mutex);
		/*Memory is where we left it;that is our table*/
		memmove(shmatRet,(void*)&myTable,sizeof(struct Table));
		sem_post(mutex);
		/*Leave the restaurant*/
		/*First do cleanup*/
		printf("(%d)Paid and now going to leave\n",getpid());
		int error=shmdt(shmemStart);/*Detach the shared memory segment*/
		if(error<0)
		{
			printf("(%d)Error detaching shared memory segment\n",getpid() );
		}
		printf("(%d)Customer exiting . . .\n",getpid() );
		exit(0);
		/*End of customer in restaurant code*/
	}
}


void waitYourOrder(int time)
{
	sleep(time);
}

void eatANDchat(int time)
{
	sleep(time);
}

/*DO NOT FORGET TO FREE IT*/
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

void semaphoreName(int pid, char** semName)
{/*This function creates a name for the semaphore to be used by this process*/
	char* pidToStr;
	toString(pid,&pidToStr);
	(*semName)=malloc((strlen(pidToStr)+strlen("/")+strlen("_process_semaphore")+1)*sizeof(char));
	strcpy((*semName),"/");
	strcat((*semName),pidToStr);
	strcat((*semName),"_process_semaphore");
	free(pidToStr);
}

void myFifoName(int pid,char** fifoName)
{
	char* pidToStr;
	toString(pid,&pidToStr);
	(*fifoName)=malloc((strlen(FIFOPATH),strlen(pidToStr)+strlen(".fifo")+1)*sizeof(char));
	strcpy((*fifoName),FIFOPATH);
	strcat((*fifoName),pidToStr);
	strcat((*fifoName),".fifo");
	free(pidToStr);
}

void cleanUpSemaphore(char* semName,sem_t* semaphore)
{/*Releases a named semaphore as resource*/
	sem_unlink(semName);/*Unlink*/
	sem_close(semaphore);/*Close the semaphore*/
}

int numberOfPeople()
{
	//int maxNumOfPeople=8;/*A customer must consist of up to 8 customers*/
	time_t t;
	srand((unsigned) time(&t));
	return ((int)(rand()%(MAX_NUM_OF_PEOPLE+1-MIN_NUM_OF_PEOPLE)+MIN_NUM_OF_PEOPLE));
}



