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
#include "wQueue.h"

#define INT_SIZE 50

/*This is the name of the mutex semaphore for the shared memory segment*/
#define MUTEX "/mutex_named_semaphore"
/*This semaphore is for fifo read-write*/
#define FIFOSEMAPHORE "/semaphore_for_res_door_fifo"

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

void toString(int theInteger,char** stringed);
void masterFifoName(int custmsBatch,char** fifoName);
void custmsFifoNames(int kids, char*** kidsFifosNames,int* kidsPids);
void semaphoreName(int pid, char** semName);

int main(int argc, char const *argv[])
{
	//sleep(1000);
	if(argc<2)
	{
		printf("Not enough input arguments.\nForce exit\n");
		exit(1);
	}
	/*First gather input arguments*/
	char* shmemFlag="-s";
	int shmemId;
	char* numOfTables2Flag="-t2";
	int numOfTables2;
	char* numOfTables4Flag="-t4";
	int numOfTables4;
	char* numOfTables6Flag="-t6";
	int numOfTables6;
	char* numOfTables8Flag="-t8";
	int numOfTables8;
	int i;
	for(i=0;i<argc;++i)
	{
		if(strcmp(argv[i],shmemFlag)==0)
		{
			//shmemId=atoi(argv[j+1]);
			sscanf(argv[i+1],"%d",&shmemId);
			printf("(%d)shmemId=%d\n",getpid(),shmemId );
			continue;
		}
		else if(strcmp(argv[i],numOfTables2Flag)==0)
		{
			numOfTables2=atoi(argv[i+1]);
			continue;
		}
		else if(strcmp(argv[i],numOfTables4Flag)==0)
		{
			numOfTables4=atoi(argv[i+1]);
			continue;
		}
		else if(strcmp(argv[i],numOfTables6Flag)==0)
		{
			numOfTables6=atoi(argv[i+1]);
			continue;
		}
		else if(strcmp(argv[i],numOfTables8Flag)==0)
		{
			numOfTables8=atoi(argv[i+1]);
			continue;
		}
	}
	printf("(%d)Got my input arguments, going to attach my shared memory segment\n",getpid());
	/*Now attach the shared memory segment*/
	void* shmatRet;
	shmatRet=shmat(shmemId,(void*)0, 0);
	if((int)shmatRet==-1)
	{
		printf("(%d)Attachment error.\nForce exit.\n",getpid());
		exit(1);
	}
	else
	{/*Shared memory segment attached*/
		void* shmemStart=shmatRet;/*Where the shared memory segment starts. This value should not change*/
		/*And now open the named semaphore*/
		int allTables=numOfTables2+numOfTables4+numOfTables6+numOfTables8;
		sem_t* mutex=sem_open(MUTEX,0);
		printf("(%d)Opened name semaphore %s \n",getpid(),MUTEX );
		sem_t* fifoSem=sem_open(FIFOSEMAPHORE,0);
		printf("(%d)Opened name semaphore %s \n",getpid(),FIFOSEMAPHORE );
		/*Going to set up a waitng queue*/
		printf("(%d)Doorman\n",getpid());
		wQueuePointer processQueue;
		processQueue=wQueueInit();
		printf("Initialised the processQueue %p\n",processQueue);
		int custmsBatch=0;
		int masterFifofd;/*The file descriptor of the fifo between the master and the doorman*/
		/*Doorman code*/
		/*Doorman should put the waiting customers in the restaurant*/
		int times=0;/*DEBUGGING VARIABLE*/
		while(1)
		{
			//if(times==3)
			//{
			//	break;
			//}
			//printf("(%d)Doorman loop\n",getpid() );
			if(empty(processQueue)==TRUE)
			{/*There are no waiting customers*/
				//printf("(%d)There are no waiting customers;going to check for a batch\n",getpid() );;
			}
			else
			{/*Code fot the doorman to search the right table for the customers in FCFS fashion*/
				printf("(%d)Going to serve my waiting customers\n",getpid() );
				int custmsInQueue=wQueueLength(processQueue);
				printf("(%d)There are %d customers waiting\n",getpid(),custmsInQueue );
				int* toBeExtracted=malloc(custmsInQueue*sizeof(int));
				int wPid,wPersons,j,offset;
				struct Table aTable;
				int extracted=FALSE;
				for(i=0;i<custmsInQueue;++i)
				{/*For every customer waiting in the queue*/
					printf("(%d)In loop for customer %d\n",getpid(),i );
					wPid=wQueueGetPid_ith(processQueue, i);
					wPersons=wQueueGetPersons_ith(processQueue, i);
					printf("(%d)Got customer %d with pid=%d and num of persons=%d\n",getpid(),i,wPid,wPersons );
					/*Now search the critical section for a table fitting the customer with pid and persons*/
					offset=0;
					/*Find the table area you are*/
					if(wPersons>=6)
					{/*6<=persons<=8*/
						offset=offset+numOfTables2+numOfTables4;
					}
					else 
					{
						if(wPersons>=4)
						{
							offset=offset+numOfTables2;
						}
					}
					//printf("(%d)Offset=%d\n",getpid(),offset );
					shmatRet=shmatRet+offset*sizeof(struct Table);
					sem_wait(mutex);/*P(mutex)*/
					//printf("(%d)IN CS\n",getpid() );
					for(j=offset;j<allTables;++j)
					{/*Find a free table where there are enough seats*/
						memmove((void*)&aTable,shmatRet,sizeof(struct Table));
					/*DEBUGGING AREA*/
					//printf("\t(%d)occupied:%d\n",getpid(),aTable.occupied );
					//printf("\t(%d)tableSerial:%d\n",getpid(),aTable.tableSerial );
					//printf("\t(%d)personsintable:%d\n",getpid(),aTable.personsMax );
					/*END OF DEBUGGING AREA*/
						if((aTable.occupied==FALSE)&&(wPersons<=aTable.personsMax))
						{/*Found the table*/
							aTable.customerPid=wPid;
							aTable.occupied=TRUE;
							memmove(shmatRet,(void*)&aTable,sizeof(struct Table));
							extracted=TRUE;
							//printf("(%d)Found table#%d for customer %d\nGoing to check",getpid(),aTable.tableSerial,wPid );
							//memmove((void*)&aTable,shmatRet,sizeof(struct Table));
							//printf("\t(%d)#%d\n",getpid(),aTable.tableSerial );
							//printf("\t(%d)occupied:%d\n",getpid(),aTable.occupied );
							//printf("\t(%d)persons:%d\n",getpid(),aTable.personsMax );
							break;
						}
						shmatRet=shmatRet+sizeof(struct Table);
					}
					sem_post(mutex);/*V(mutex)*/
					if(extracted==TRUE)
					{
						printf("(%d)Going to extract the customer from the queue and put them in the table\n",getpid() );
						/*Get the pid of the extracted customer*/
						int extPid=wQueueGetPid_ith(processQueue, i);
						//wQueueExtract_ith(processQueue,i);/*Extract the i_th customer from the queue*/
						/*Raise their semaphore*/
						printf("(%d)Extracted customer %d\nGoing to wake them up ;)\n",getpid(),extPid );
						char* semName;
						semaphoreName(extPid, &semName);
						printf("(%d)The name of the child's semaphore is %s\n",getpid(),semName );
						sem_t* extCustmSem=sem_open(semName,0);
						sem_post(extCustmSem);
						printf("(%d)Woke up the child\n",getpid() );
						free(semName);
						extracted=FALSE;
						toBeExtracted[i]=TRUE;
						printf("(%d)Going to extract the customer %d from the queue\n",getpid(),i );
						//wQueueExtract_ith(processQueue,i);/*Extract the i_th customer from the queue*/
						//printf("(%d)Extracted customer, going to keep looping\n",getpid() );
					}
					else
					{
						toBeExtracted[i]=FALSE;
					}
					shmatRet=shmemStart;/*Go back to the start of the shared memory segment*/
				}
				//printf("(%d)Doorman going to exit %d\n",getpid() );
				//exit(1);
				printf("(%d)Doorman going to extract the served customers from the queue\n",getpid() );
				for(i=0;i<custmsInQueue;++i)
				{
					if(toBeExtracted[i]==TRUE)
					{
						wQueueExtract_ith(processQueue,i);/*Extract the i_th customer from the queue*/
					}
				}
				free(toBeExtracted);
				printf("(%d)Served customers extracted\n",getpid() );
			}
			/*Doorman must check if parent process has generated a new batch of customers*/
			/*Doorman puts the newly generated customers in the queue*/
			/*Doorman opens a fifo to read from the master the newly created customers' ids*/
			char* fifoName;
			masterFifoName(custmsBatch, &fifoName);
			//printf("(%d)The master's fifo name is %s\n",getpid(),fifoName );
			/*
			Every time master created new customers, a new fifo is created between the master and the doorma, where master writes the pids of the customers
			That fifo is named after the number of the customers' generation; ie if is the 3rd time master generates customers, that fifo's name should be
			FIFOPATH/3.fifo
			*/
			/*Now doorman tries to open the fifo*/
			masterFifofd=open(fifoName,O_RDONLY,O_NONBLOCK);
			/*DEBUGGING AREA*/
			//while(masterFifofd<0)
			//{
			//	printf("(%d)Master fifo not ready\n",getpid() );
			//	masterFifofd=open(fifoName,O_RDONLY,O_NONBLOCK);
			//}
			/*END OF DEBUGGING AREA*/
			/*If the fifo exists, that means that the parent has created the new batch of processes*/
			if(masterFifofd==-1)
			{/*Master hasn't created tis fifo yet*/
				//printf("Fifo does not exist\n");/*Move on with your code*/
			}
			else
			{/*The fifo exists, so we need to read it*/
				printf("(%d)Going to read master fifo\n",getpid() );
				int pidsToRead;
				sem_wait(fifoSem);
				/*First you will read how many pids are written there*/
				read(masterFifofd,&pidsToRead,sizeof(int));
				printf("(%d)I should read %d pids\n",getpid(),pidsToRead );
				/*And now read the pids*/
				int *pidsBuffer=malloc(pidsToRead*sizeof(int));
				int actRead=read(masterFifofd,pidsBuffer,pidsToRead*sizeof(int));/*The file descriptor goes along with cursor*/
				/*Close the master's fifo*/
				close(masterFifofd);
				printf("(%d)Read master fifo,%d pids, going for the children\n",getpid(),actRead );
				/*
				Each customer has a fifo of their own, named FIFOPATH/customerPid.fifo
				In that fifo, the customer has written the persons they consist of, as well as the id of the semaphore it uses for the waiting
				*/
				char** kidsFifosNames;
				custmsFifoNames(pidsToRead, &kidsFifosNames,pidsBuffer);
				int *kidFds=malloc(pidsToRead*sizeof(int));/*The file descriptor for each of those fifos*/
				/*Initialize them*/
				for(i=0;i<pidsToRead;++i)
				{
					kidFds[i]=-10;
					/*DEBUGGING*/
					printf("(%d)Read pid:%d\n",getpid(),pidsBuffer[i] );
					/*END OF DEBUGGING*/
				}
				printf("(%d)Going to open all kids fifos\n",getpid() );
				/*Now doorman needs to open all of them*/
				int allOpen;
				do
				{
					allOpen=TRUE;
					//printf("(%d)Looping to open kids fifos\n",getpid());
					for(i=0;i<pidsToRead;++i)
					{
						//printf("(%d)The file descriptor for i=%d is %d\n",getpid(),i,kidFds[i] );
						if(kidFds[i]>0)
						{
							;/*This one is open*/
						}
						else
						{
							kidFds[i]=open(kidsFifosNames[i],O_RDONLY,O_NONBLOCK);
							if(kidFds[i]<0)
							{/*Fifo does not exist yet to be opened*/
								allOpen*=FALSE;
								//printf("(%d)Did not open fifo %s, fds=%d\n",getpid(),kidsFifosNames[i],kidFds[i] );
							}
							//printf("(%d)Opened fifo %s\n",getpid(),kidsFifosNames[i] );
						}
					}
				} while (allOpen==FALSE);
				/*Now that all of them are open, go read them*/
				printf("(%d)Opened all kids fifos, going to read them\n",getpid() );
				struct waitingCustomer aCustomer;
				for(i=0;i<pidsToRead;++i)
				{
					read(kidFds[i],&aCustomer,sizeof(struct waitingCustomer));/*Read the info from the customer's fifo*/
					close(kidFds[i]);/*Close it*/
					/*And put those processes and the info in the queue of waiting processes-customers*/
					wQueueInsert(processQueue, aCustomer.customerPid, aCustomer.persons);
				}
				++custmsBatch;/*Move on to the next batch*/
				free(pidsBuffer);
				/*V ALL THE SEMAPHORES*/
				printf("(%d)Going to print all the new processes\n",getpid());
				//wQueuePrint(processQueue);
				printf("(%d)Printed them, going to loop again\n",getpid());
			}
			++times;
		}
		/*Doorman serves the customers in an FCFS fashion*/
		//printf("(%d)Now I will extrat all of them and then I'll exit\n",getpid());
		//int customersWaiting=wQueueLength(processQueue);
		//printf("(%d)There are %d processes in the queue\n",getpid(),customersWaiting );
		//for(i=0;i<customersWaiting;++i)
		//{
		//	int extPid=wQueueExtract(processQueue);
		//	printf("(%d)Extracted customer %d\nGoing to wake them up ;)\n",getpid(),extPid );
		//	char* semName;
		//	semaphoreName(extPid, &semName);
		//	printf("(%d)The name of the child's semaphore is %s\n",getpid(),semName );
		//	sem_t* extCustmSem=sem_open(semName,0);
		//	sem_post(extCustmSem);
		//	printf("(%d)Woke up the child\n",getpid() );
		//	free(semName);
		//}
		//printf("(%d)Woke all kids\n",getpid() );
		wQueueDestroy(processQueue);
	}
	printf("(%d)Doorman exiting . . .\n",getpid());
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


void masterFifoName(int custmsBatch,char** fifoName)
{
	char* fifoSerial;
	toString(custmsBatch,&fifoSerial);
	(*fifoName)=malloc((strlen(FIFOPATH)+strlen(fifoSerial)+strlen("_batch")+strlen(".fifo")+1)*sizeof(char));
	strcpy((*fifoName),FIFOPATH);
	strcat((*fifoName),fifoSerial);
	strcat((*fifoName),"_batch");
	strcat((*fifoName),".fifo"); /*The master's fifo name is FIFOPATH/customersBatch.fifo*/
	free(fifoSerial);
}

void custmsFifoNames(int kids, char*** kidsFifosNames,int* kidsPids)
{
	int j;
	(*kidsFifosNames)=malloc(kids*sizeof(char*));
	for(j=0;j<kids;++j)
	{
		char* kidPidStr;
		toString(kidsPids[j],&kidPidStr);
		(*kidsFifosNames)[j]=malloc(((strlen(FIFOPATH)+strlen(kidPidStr)+strlen(".fifo")+1))*sizeof(char));
		strcpy((*kidsFifosNames)[j],FIFOPATH);
		strcat((*kidsFifosNames)[j],kidPidStr);
		strcat((*kidsFifosNames)[j],".fifo");
		//printf("CREATED FIFO NAME %s\n",(*kidsFifosNames)[j]);
		free(kidPidStr);
	}
}

void semaphoreName(int pid, char** semName)
{/*This function returns the name of the waiting semaphore used by the process with pid pid*/
	char* pidToStr;
	toString(pid,&pidToStr);
	(*semName)=malloc((strlen(pidToStr)+strlen("/")+strlen("_process_semaphore")+1)*sizeof(char));
	strcpy((*semName),"/");
	strcat((*semName),pidToStr);
	strcat((*semName),"_process_semaphore");
	free(pidToStr);
}