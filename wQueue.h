
#define TRUE 1
#define FALSE 0



typedef struct waitingCustomerNode *waitingCustomerNodePointer;
typedef struct wQueue wQueue;
typedef wQueue *wQueuePointer;



int empty(wQueuePointer theQueue);
wQueuePointer wQueueInit();
int wQueueLength(wQueuePointer theQueue);
int wQueueInsert(wQueuePointer theQueue, int pid,int persons);
int wQueueExtract(wQueuePointer theQueue);
void wQueueDestroy(wQueuePointer theQueue);
void wQueuePrint(wQueuePointer theQueue);
int wQueueGetPid_ith(wQueuePointer theQueue, int i);
int wQueueGetPersons_ith(wQueuePointer theQueue, int i);
void wQueueExtract_ith(wQueuePointer theQueue, int i);