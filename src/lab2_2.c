#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/times.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define empty 0 //key of sem
#define full_e 1
#define full_o 2

#define ticketSum 20

union semun;
int create_Sem(int key, int size);
void destroy_Sem(int semid);
int get_Sem(int key, int size);
void set_N(int semid, int index, int n);
int P(int semid, int index);
int V(int semid, int index);
int get_time_ms(void);

int semid;
int a;

/* calculate pthread */
void *calculate(){
    int i = 1;
    while(1){
        P(semid, empty);
        if(i > 100){
            // printf("Calculator out!\n");
            break;
        }
        a += i;
        // printf("Calculate: %d, i: %d\n", a, i);
        i++;
        if(a % 2 == 0){
            V(semid, full_e);
        }
        else {
            V(semid, full_o);
        }
    }
}
/* printer1---even */
void *print1(){
    while(1){
        P(semid, full_e);
        printf("Printer1: Current sum(a) is %d\n", a);
        if(a >= 5050){
            V(semid, empty);
            // printf("p1 out\n");
            break;
        }
        V(semid, empty);
    }
}
/* printer2----odd */
void *print2(){
    while(1){
        P(semid, full_o);
        printf("Printer2: Current sum(a) is %d\n", a);
        if(a >= 4851){
            V(semid, empty);
            // printf("p2 out\n");
            break;
        }
        V(semid, empty);
    }
}

int main(){
    if((semid = create_Sem(IPC_PRIVATE, 3)) < 0){ return -1; };  //Create two semaphores
    set_N(semid, empty, 1);     //set init value
    set_N(semid, full_e, 0);
    set_N(semid, full_o, 0);

    pthread_t pthd1, pthd2, pthd3;
    if(pthread_create(&pthd1, NULL, calculate, NULL) != 0 ||
    pthread_create(&pthd2, NULL, print1, NULL) != 0 ||
    pthread_create(&pthd3, NULL, print2, NULL) != 0){
        printf("pthread create failed\n");
        return -1;
    }

    pthread_join(pthd1, NULL);
    pthread_join(pthd2, NULL);
    pthread_join(pthd3, NULL);

    return 0;
}


union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO */
};

int create_Sem(int key, int size)
{
    int id;
    id = semget((key_t)key, size, 0666 | IPC_CREAT);
    if(id < 0) {//????????????????????????
        printf("create sem %d,%d error\n", key, size);//???????????????????????????
    }
    return id;
}

void destroy_Sem(int semid)
{
    int res = semctl(semid,0,IPC_RMID,0);//???????????????????????????
    if (res < 0) {//????????????????????????
        printf("destroy sem %d error\n", semid);//????????????????????????????????????
    }
    return;
}

int get_Sem(int key, int size)
{
    int id;
    id = semget(key, size, 0666);//??????????????????????????????
    if(id < 0) {
        printf("get sem %d,%d error\n", key, size);
    }
    return id;
}

void set_N(int semid, int index, int n)
{
    union semun semopts;
    semopts.val = n;//???????????????????????????n
    semctl(semid, index, SETVAL, semopts);//???????????????????????????????????????index
    return;
}

int P(int semid, int index)
{
    struct sembuf sem;//?????????????????????
    sem.sem_num = index;//???????????????
    sem.sem_op = -1;//??????????????????-1???P??????
    sem.sem_flg = 0;//???????????????0???IPC_NOWAIT???
    if(semop(semid,&sem,1) == -1){//1:???????????????????????????
        printf("P failed! Index:%d.\n", index);
        return -1;
    }
    return 0;
}

int V(int semid, int index)
{
    struct sembuf sem;//?????????????????????
    sem.sem_num = index;//???????????????
    sem.sem_op =  1;//??????????????????1???V??????
    sem.sem_flg = 0; //????????????
    if(semop(semid,&sem,1) == -1){//1:???????????????????????????
        printf("V failed! Index:%d.\n", index);
        return -1;
    }
    return 0;
}

int get_time_ms(void)
{
    static struct timeval start_t;
    struct timeval now_t;
    if((start_t.tv_sec == 0)&&(start_t.tv_usec == 0)) {
        gettimeofday(&start_t, NULL);
    }
    gettimeofday(&now_t, NULL);
    return (now_t.tv_sec-start_t.tv_sec)*1000+(now_t.tv_usec-start_t.tv_usec)/1000;
}

