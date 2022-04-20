#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>


#include "io.c"
#define NumThread 2
#define MaxInQueue 20
#define MSPERSEC 1000
#define	NSPERMS	1000000


pthread_mutex_t mutBuf;
sem_t semEmpty;
sem_t semFull;
sem_t semMaxHum;
sem_t Lock;

struct QNode {
    int key;
    struct QNode* next;
};

// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct Queue {
    struct QNode *front, *rear;
};

// A utility function to create a new linked list node.
struct QNode* newNode(int k)
{
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->key = k;
    temp->next = NULL;
    return temp;
}
struct Queue* createQueue()
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

// The function to add a key k to q
void enQueue(struct Queue* q, int k)
{
    // Create a new LL node
    struct QNode* temp = newNode(k);

    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}
// Function to remove a key from given queue q
void deQueue(struct Queue* q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL){

        printf("EMPTYYYYYYY");
        return;
}
    // Store previous front and move front one node ahead
    struct QNode* temp = q->front;

    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
}






struct QueTrack{


    int RequestQueue[2];
    int ProducedCount[2];
    int CostConsumedCount[2];
    int FastConsumedCount[2];
    int CostConsumeSleep;
    int FastConsumeSleep;
    int HumanProdSleep;
    int RobotProdSleep;
    int MaxRequests;

//throw everything at this
};
typedef struct{
    struct QueTrack* Que;
    struct Queue* q;
    int sleepCost;
    int sleepFast;
    int sleepHuman;
    int sleepRobot;


}SharedData;


int ReqQueue[2]={0,0};
int Produced[2]={0,0};//Array to keep track of produced human requests which always increments
int CostConsumed[2]={0,0};
int FastConsumed[2]={0,0};
//int Consumed[2][2]= {{0, 0},{0, 0}};
int county=0;

int times[];
char FIFO[12];
int current;















void *produce(void *data) {

    SharedData *Data_ptr = (SharedData *) data;

    struct timespec tim, tim2;
    int time=(Data_ptr->Que->HumanProdSleep);
    tim.tv_sec  = time/1000;
    tim.tv_nsec = 0;




    //Sleeptime.tv_nsec =(DelayMS % MSPERSEC) * NSPERMS;

    //nanosleep(&Sleeptime,NULL);



    while ((Data_ptr->Que->ProducedCount[0])+(Data_ptr->Que->ProducedCount[1])<(Data_ptr->Que->MaxRequests)){ //REPLACEMENT LOOP

    //blocks if we have no more human slots available (4)
    //sem_wait(&Lock);
        sem_wait(&semMaxHum);
        sem_wait(&semEmpty);
        nanosleep(&tim, &tim2);
        //sleep(time/1000);

        pthread_mutex_lock(&mutBuf);//Checking if statement must be in critical section so we must lock first

        if((Data_ptr->Que->ProducedCount[0])+(Data_ptr->Que->ProducedCount[1])<(Data_ptr->Que->MaxRequests)){//REPLACEMENT LOOP

            (Data_ptr->Que->RequestQueue[0])++;//REPLACEMENT

            (Data_ptr->Que->ProducedCount[0])++;//REPLACEMENT//Array to keep track of produced human requests which always increments
        io_add_type(HumanDriver, Data_ptr->Que->RequestQueue, Data_ptr->Que->ProducedCount);
            enQueue(Data_ptr->q,0);
            //printf("%d\n",Data_ptr->q->front->key);
        }

        pthread_mutex_unlock(&mutBuf);

        sem_post(&semFull);
        //sem_post(&Lock);

        //printf("%d\n",sizeof(Data_ptr->q));
}
    pthread_exit(NULL);
}

void *produce2(void *data) {
    SharedData *Data_ptr = (SharedData *) data;

    struct timespec Sleeptime;
    struct timespec tim, tim2;
    int time=(Data_ptr->Que->RobotProdSleep);
    tim.tv_sec  = time/1000;
    tim.tv_nsec = 0;


    //enQueue(Data_ptr->q,6);


    while ((Data_ptr->Que->ProducedCount[0])+(Data_ptr->Que->ProducedCount[1])<(Data_ptr->Que->MaxRequests)){ //REPLACEMENT LOOP

        //sem_wait(&Lock);
        sem_wait(&semEmpty);
        nanosleep(&tim, &tim2);
        //sleep(time/1000);
        pthread_mutex_lock(&mutBuf);



        if((Data_ptr->Que->ProducedCount[0])+(Data_ptr->Que->ProducedCount[1])<(Data_ptr->Que->MaxRequests)){//REPLACEMENT LOOP

            (Data_ptr->Que->RequestQueue[1])++;//REPLACEMENT

            (Data_ptr->Que->ProducedCount[1])++;//REPLACEMENT//Array to keep track of produced human requests which always increments

        io_add_type(RoboDriver, Data_ptr->Que->RequestQueue, Data_ptr->Que->ProducedCount);
            enQueue(Data_ptr->q,1);
            //printf("%d\n",Data_ptr->q->front->key);
    }


        pthread_mutex_unlock(&mutBuf);

        sem_post(&semFull);
        //sem_post(&Lock);

       // printf("%d\n",sizeof(Data_ptr->q));
    }

    pthread_exit(NULL);
}


void *consume(void *data){
    SharedData *Data_ptr = (SharedData *) data;

    struct timespec Sleeptime;
    int time = (Data_ptr->Que->CostConsumeSleep);
    struct timespec tim, tim2;
    tim.tv_sec  = time/1000;
    tim.tv_nsec = 0;

    int DelayMS=time;
    int ProdID;

    Sleeptime.tv_nsec =(DelayMS % MSPERSEC) * NSPERMS;



    while((Data_ptr->Que->CostConsumedCount[0])+(Data_ptr->Que->CostConsumedCount[1])+(Data_ptr->Que->FastConsumedCount[0])+(Data_ptr->Que->FastConsumedCount[1])<(Data_ptr->Que->MaxRequests)){

        //sem_wait(&Lock);

        sem_wait(&semFull);

        pthread_mutex_lock(&mutBuf);
        if((Data_ptr->Que->CostConsumedCount[0])+(Data_ptr->Que->CostConsumedCount[1])+(Data_ptr->Que->FastConsumedCount[0])+(Data_ptr->Que->FastConsumedCount[1])<(Data_ptr->Que->MaxRequests)){

            if (Data_ptr->q->front->key==0){

                deQueue(Data_ptr->q);


     //if((Data_ptr->Que->RequestQueue[0])>0){}  //Replacement IF

                (Data_ptr->Que->RequestQueue[0])--; //REPLACEMENT DECREMENT

                (Data_ptr->Que->CostConsumedCount[0])++;//Replacement INCREMENT

                io_remove_type(CostAlgoDispatch, HumanDriver, Data_ptr->Que->RequestQueue, Data_ptr->Que->CostConsumedCount);
                pthread_mutex_unlock(&mutBuf);
                nanosleep(&tim, &tim2);
                sem_post(&semMaxHum);
                sem_post(&semEmpty);
                //sleep(time/1000);

        }

            else {//Replacement IF if((Data_ptr->Que->RequestQueue[1])>0)
            //sleep(time/1000);
                deQueue(Data_ptr->q);
                (Data_ptr->Que->RequestQueue[1])--; //REPLACEMENT DECREMENT

                (Data_ptr->Que->CostConsumedCount[1])++;//Replacement INCREMENT

                io_remove_type(CostAlgoDispatch, RoboDriver, Data_ptr->Que->RequestQueue, Data_ptr->Que->CostConsumedCount);
                pthread_mutex_unlock(&mutBuf);
                nanosleep(&tim, &tim2);
                //sleep(time/1000);
                sem_post(&semEmpty);

        }
}



        //sem_post(&Lock);
         //Figure out deadlock issue
    }

    pthread_exit(NULL);
}
void *consume2(void *data){
    SharedData *Data_ptr = (SharedData *) data;

    struct timespec tim, tim2;
    int time=(Data_ptr->Que->FastConsumeSleep);
    tim.tv_sec  = time/1000;
    tim.tv_nsec = 0;

    struct timespec Sleeptime;
    int DelayMS=0;
    int ProdID;

    //Sleeptime.tv_nsec =(DelayMS % MSPERSEC) * NSPERMS;



    while((Data_ptr->Que->CostConsumedCount[0])+(Data_ptr->Que->CostConsumedCount[1])+(Data_ptr->Que->FastConsumedCount[0])+(Data_ptr->Que->FastConsumedCount[1])<(Data_ptr->Que->MaxRequests)){


        //sem_wait(&Lock);


        sem_wait(&semFull);

        pthread_mutex_lock(&mutBuf);

        if((Data_ptr->Que->CostConsumedCount[0])+(Data_ptr->Que->CostConsumedCount[1])+(Data_ptr->Que->FastConsumedCount[0])+(Data_ptr->Que->FastConsumedCount[1])<(Data_ptr->Que->MaxRequests)){
            if (Data_ptr->q->front->key==0){
                deQueue(Data_ptr->q);

                // if((Data_ptr->Que->RequestQueue[0])>0){  Replacement IF

                (Data_ptr->Que->RequestQueue[0])--; //REPLACEMENT DECREMENT

                (Data_ptr->Que->FastConsumedCount[0])++;//Replacement INCREMENT
                io_remove_type(FastAlgoDispatch, HumanDriver, Data_ptr->Que->RequestQueue, Data_ptr->Que->FastConsumedCount);

                pthread_mutex_unlock(&mutBuf);
                //sleep(time/1000);
                nanosleep(&tim, &tim2);
                sem_post(&semMaxHum);
                sem_post(&semEmpty);

        }

            else {//Replacement IF if((Data_ptr->Que->RequestQueue[1])>0)
                deQueue(Data_ptr->q);
           // sleep(time/1000);
                (Data_ptr->Que->RequestQueue[1])--;
                (Data_ptr->Que->FastConsumedCount[1])++;//Replacement INCREMENT
                io_remove_type(FastAlgoDispatch, RoboDriver, Data_ptr->Que->RequestQueue, Data_ptr->Que->FastConsumedCount);
                pthread_mutex_unlock(&mutBuf);
                nanosleep(&tim, &tim2);
                //sleep(time/1000);
                sem_post(&semEmpty);

            }
        }


        //sem_post(&Lock);

    }
pthread_exit(NULL);
}

void *CreateProducerThreads(void *data){





    pthread_t thread[2];

    int i;
    int k;
    for( i=0; i<2;i++){
        if(i==0){
            //printf("Gets to here");
            if(pthread_create(&thread[i],NULL, &produce,data)!=0){
                perror("pthread_create failed");
            }

        }

        else {
            if(pthread_create(&thread[i],NULL, &produce2,data)!=0){
                perror("pthread_create failed");
            }

        }

    }
    for(k=0;k<NumThread;k++){


        if(pthread_join(thread[k],NULL)!=NULL){
            perror("pthread_join failed");
        }

    }



    pthread_exit(NULL);
}

void *CreateConsumerThreads(void *data){



    pthread_t thread[2];
    int i;
    int k;
    for( i=0; i<2;i++){
        if(i==0){
            if(pthread_create(&thread[i],NULL, &consume,data)!=0){
                perror("pthread_create failed");
            }

        }

        else {
            if(pthread_create(&thread[i],NULL, &consume2,data)!=0){
                perror("pthread_create failed");
            }

        }

    }
    for( k=0;k<NumThread;k++){


        if(pthread_join(thread[k],NULL)!=NULL){
            perror("pthread_join failed");
        }

    }
    pthread_exit(NULL);
}

void SetToZero(struct QueTrack* queData){
    *(&queData->CostConsumedCount[0])=0;
    *(&queData->CostConsumedCount[1])=0;
    *(&queData->FastConsumedCount[0])=0;
    *(&queData->FastConsumedCount[1])=0;
    *(&queData->ProducedCount[0])=0;
    *(&queData->ProducedCount[1])=0;
    *(&queData->RequestQueue[0])=0;
    *(&queData->RequestQueue[1])=0;
    *(&queData->CostConsumeSleep)= 0;
    *(&queData->FastConsumeSleep)=0;
    *(&queData->HumanProdSleep)=0;
    *(&queData->RobotProdSleep)=0;
    *(&queData->MaxRequests)=120;

}


int main(int argc, char *argv[]) {

    SharedData QueData;

    struct QueTrack tracker;

    SetToZero(&tracker);

    QueData.Que=&tracker;
    QueData.q =createQueue();

    int r=3, c=4, len=0;
    int *ptr, **arr;
    int count = 0,i,j;
    int option;

    /*
    -n N Total number of requests of requests (production limit). Default is 120 if not
specified.

-c N   Specifies the number of milliseconds N that the cost-saving dispatcher
(consumer) requires dispatching a request and should be invoked each time
the cost-saving dispatcher removes a request from queue regardless of the
request type. You would simulate this time to consume a request by putting
the consumer thread to sleep for N milliseconds. Other consumer and
producer threads (fast-matching dispatcher, producing human driver
request, and producing autonomous driver request) are handled similarly.

-f N  Similar argument for the fast-matching dispatcher.

-h N  Specifies the number of milliseconds required to produce a ride request for
a human driver.

-a N  Specifies the number of milliseconds required to produce a ride request for
an autonomous car.

Important: If an argument is not given for any one of the threads, that thread should
incur no delay, i.e., the default for -c, -f, -h, -a above should be 0.
*/
    while((option = getopt(argc, argv, "n:c:f:h:a:")) != -1){

        switch(option){ //Work on Cases and test

            case 'n':
                tracker.MaxRequests=atoi(optarg);
                break;
            case 'c':
                tracker.CostConsumeSleep=atoi(optarg);
                break;
            case 'f':
                tracker.FastConsumeSleep=atoi(optarg);
                break;
            case 'h':
                tracker.HumanProdSleep=atoi(optarg);
                break;

            case 'a':
                tracker.RobotProdSleep=atoi(optarg);
                break;


            default:

                printf("hi");
                break;

        }


    }



    int k;
    sem_init(&semEmpty,0,12);
    sem_init(&semFull,0,0);
    sem_init(&semMaxHum,0,4);
    sem_init(&Lock,0,1);

    pthread_t thread[NumThread];
    pthread_mutex_init(&mutBuf, NULL);


    //pthread_t thread2;
    for(k=0;k<NumThread;k++){

        if(k==0){
            if(pthread_create(&thread[k],NULL, &CreateProducerThreads,&QueData)!=0){
                perror("pthread_create failed");
            }

        }
        else{
            if(pthread_create(&thread[k],NULL, &CreateConsumerThreads,&QueData)!=0){
                perror("pthread_create failed");

            }

        }

    }

    for(k=0;k<NumThread;k++){


        if(pthread_join(thread[k],NULL)!=NULL){
            perror("pthread_join failed");
        }

    }



    int Consumed[2][2]= {{0, 0},{0, 0}};
    Consumed[0][0]=tracker.CostConsumedCount[0];
    Consumed[0][1]=tracker.CostConsumedCount[1];
    Consumed[1][0]=tracker.FastConsumedCount[0];
    Consumed[1][1]=tracker.FastConsumedCount[1];
   //printf("%d  %d %d %d\n",Consumed[0][0],Consumed[0][1],Consumed[1][0],Consumed[1][1]);


    len = sizeof(int *) * r + sizeof(int) * c * r;
    arr = (int **)malloc(len);
    ptr = (int *)(arr + r);

    for(i = 0; i < r; i++)
        arr[i] = (ptr + c * i);

    for (i = 0; i < 2; i++)
        for (j = 0; j < 2; j++)
            arr[i][j] = Consumed[i][j];


        //printf("%d",QueData.q->front->key);
        //printf("%d",QueData.q->rear->key);
    //printf("Max reqs: %d\n  Fast cosume sleep time: %d\n Cost consume sleep time: %d\n Human Prod sleep time: %d\n Robot prod sleep time: %d\n",tracker.MaxRequests,tracker.FastConsumeSleep,tracker.CostConsumeSleep,tracker.RobotProdSleep,tracker.HumanProdSleep);
    io_production_report(tracker.ProducedCount, arr);
    sem_destroy(&semEmpty);
    sem_destroy(&semFull);
    sem_destroy(&semMaxHum);
    pthread_mutex_destroy(&mutBuf);
    pthread_exit(NULL);
    printf("Hello, World!\n");
    return 0;
}
