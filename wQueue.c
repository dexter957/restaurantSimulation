#include "wQueue.h"
#include <stdio.h>
#include <stdlib.h>


/*
The customer processes wishing to enter the restaurant, may need to wait in a queue to be served by the doorman.
What we want to do is store their pids and suspend them until they can be served.
We store the pids in a queue, that has been implemented as an ADT.
*/


struct waitingCustomerNode
{
	int customerPid;
	int persons;
	//int semId;
	waitingCustomerNodePointer next;
};

struct wQueue
{
	/*Pointer to the list that implements the queue*/
	waitingCustomerNodePointer front;
	waitingCustomerNodePointer rear;
	int queueLength;
};



int empty(wQueuePointer theQueue)
{/*Checks if the queue is empty*/
	if(theQueue->queueLength==0)
	{
		//printf("Empty\n");
		return TRUE;
	}
	else
	{
		//printf("Not empty\n");
		return FALSE;
	}
}

wQueuePointer wQueueInit()
{/*Initialises the queue*/
	printf("In queueInit\n");
	wQueuePointer theQueue=malloc(sizeof(wQueue));
	if(theQueue==NULL)
	{
		printf("Got NULL\n");
		return NULL;
	}
	else
	{
		printf("Not NULL\n");
		theQueue->queueLength=0;
		theQueue->front=NULL;
		theQueue->rear=NULL;
		printf("Returning %p\n",theQueue);
		return theQueue;
	}
}

int wQueueLength(wQueuePointer theQueue)
{/*Returns the length of the queue*/
//printf("IN QUEUE LENGTH\n");
//printf("THERE ARE %d CUSTOMERS IN THE QUEUE\n",theQueue->queueLength );
	return theQueue->queueLength;
}

int wQueueInsert(wQueuePointer theQueue, int pid, int persons)
{/*Inserts a pid in the queue*/
	if(empty(theQueue)==TRUE)
	{/*First element to be added*/
		theQueue->front=malloc(sizeof(struct waitingCustomerNode));
		//printf("Malloced for first node\n");
		if(theQueue->front==NULL)
		{
			printf("Cannot allocate memory for waiting queue.\n");
			return FALSE;
		}
		else
		{
			//printf("Going to initialise the first node\n");
			theQueue->front->customerPid=pid;
			theQueue->front->persons=persons;
			//theQueue->front->semId=semId;
			theQueue->front->next=NULL;
			theQueue->queueLength=1;
			//theQueue->front=start;
			theQueue->rear=theQueue->front;
			//printf("DONE\n");
			return TRUE;
		}
	}
	else
	{
		/*Not the first element to be added to the queue*/
		//printf("Going to malloc to build a node\n");
		theQueue->rear->next=malloc(sizeof(struct waitingCustomerNode));
		if(theQueue->rear->next==NULL)
		{
			printf("Cannot allocate memory for waiting queue.\n");
			return FALSE;
		}
		else
		{
			//printf("Going to initalize the newly malloced node\n");
			theQueue->rear=theQueue->rear->next;
			theQueue->rear->customerPid=pid;
			theQueue->rear->persons=persons;
			//theQueue->rear->semId=semId;
			theQueue->rear->next=NULL;
			(theQueue->queueLength)=(theQueue->queueLength)+1;
			//printf("Initalized and returning\n");
			return TRUE;
		}
	}
}


int wQueueExtract(wQueuePointer theQueue)
{/*Extracts the first pid, out of the queue*/
	int pid=theQueue->front->customerPid;
	waitingCustomerNodePointer toExtract=theQueue->front;
	theQueue->front=theQueue->front->next;
	(theQueue->queueLength)=(theQueue->queueLength)-1;
	free(toExtract);
	return pid;
}

void wQueueDestroy(wQueuePointer theQueue)
{/*Deletes the queue*/
	waitingCustomerNodePointer assistant=theQueue->front;
	waitingCustomerNodePointer toDelete;
	while(assistant!=NULL)
	{
		toDelete=assistant;
		assistant=assistant->next;
		free(toDelete);
	}
}

int wQueueGetPid_ith(wQueuePointer theQueue, int i)
{
	if(i>theQueue->queueLength)
	{
		printf("Number i larger than queue\n");
		return -1;
	}
	else
	{
		waitingCustomerNodePointer assistant=theQueue->front;
		int j;
		for(j=0;j<i;++j)
		{
			assistant=assistant->next;
		}
		return assistant->customerPid;
	}
}

int wQueueGetPersons_ith(wQueuePointer theQueue, int i)
{
	if(i>theQueue->queueLength)
	{
		printf("Number i larger than queue\n");
		return -1;
	}
	else
	{
		waitingCustomerNodePointer assistant=theQueue->front;
		int j;
		for(j=0;j<i;++j)
		{
			assistant=assistant->next;
		}
		return assistant->persons;
	}
}

void wQueueExtract_ith(wQueuePointer theQueue, int i)
{
	if(i>theQueue->queueLength)
	{
		printf("Number i larger than queue\n");
		return;
	}
	else
	{
		waitingCustomerNodePointer assistant=theQueue->front;
		waitingCustomerNodePointer toExtract;
		waitingCustomerNodePointer previous=assistant;
		int j;
		for(j=0;j<i;++j)
		{
		//	printf("Looping to find the proper node\n");
		//	printf("We are currently at:%d\n",assistant->customerPid);
			previous=assistant;
			assistant=assistant->next;
		}
		//printf("Going for the rest\n");
		toExtract=assistant;
		previous->next=toExtract->next;
		//printf("Going to check if we need to fix first and last\n");
		if(toExtract->next==NULL)
		{
		//	printf("Fix rear\n");
			theQueue->rear=previous;
		}
		//printf("Checked rear, going for the front\n");
		if(toExtract==theQueue->front)
		{
		//	printf("Fix front\n");
			theQueue->front=toExtract->next;
		}
		(theQueue->queueLength)=(theQueue->queueLength)-1;
		free(toExtract);
	}
}



void wQueuePrint(wQueuePointer theQueue)
{
	printf("Came here to print\n");
	printf("Length=%d\n",theQueue->queueLength );
	printf("Front=%p, front pid=%d\n",theQueue->front,theQueue->front->customerPid );
	printf("Rear=%p, rear pid=%d\n",theQueue->rear,theQueue->rear->customerPid );
	waitingCustomerNodePointer assistant=theQueue->front;
	while(assistant!=NULL)
	{
		printf("pid:%d\n",assistant->customerPid );
		printf("persons:%d\n",assistant->persons );
		//printf("semId:%d\n",assistant->semId );
		assistant=assistant->next;
	}
	printf("Now printing the front node %d\n",theQueue->front->customerPid );
	printf("And now the last %d\n",theQueue->rear->customerPid );
}
























