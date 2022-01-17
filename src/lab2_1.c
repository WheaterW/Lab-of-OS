#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/times.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define mutex 0
#define ticketSum 20

union semun;
int create_Sem(int key, int size);
void destroy_Sem(int semid);
int get_Sem(int key, int size);
void set_N(int semid, int index, int n);
void P(int semid, int index);
void V(int semid, int index);
int get_time_ms(void);

int semid;
int ticketID;

void *pthread1(){
    int sold = 0;
    while(1){
        P(semid, mutex);
        if(ticketID >= ticketSum){
            printf("pthread1 sold %d tickets!\n", sold);
            V(semid, mutex);
            break;
        }
        sold++;
        ticketID++;
        printf("pthread1 sold ticket %d!\n", ticketID);
        V(semid, mutex);
    }
}

void *pthread2(){
    int sold = 0;
    while(1){
        P(semid, mutex);
        if(ticketID >= ticketSum){
            printf("pthread2 sold %d tickets!\n", sold);
            V(semid, mutex);
            break;
        }
        sold++;
        ticketID++;
        printf("pthread2 sold ticket %d!\n", ticketID);
        V(semid, mutex);
    }
}

void *pthread3(){
    int sold = 0;
    while(1){
        P(semid, mutex);
        if(ticketID >= ticketSum){
            printf("pthread3 sold %d tickets!\n", sold);
            V(semid, mutex);
            break;
        }
        sold++;
        ticketID++;
        printf("pthread3 sold ticket %d!\n", ticketID);
        V(semid, mutex);
    }
}

int main(){
    if((semid = create_Sem(IPC_PRIVATE, 1) < 0)){ return -1; };  //Create two semaphores
    set_N(semid, mutex, 1);     //set init value

    //    printf("Hello0\n");
    pthread_t pthd1, pthd2, pthd3;
    if(pthread_create(&pthd1, NULL, pthread1, NULL) != 0 ||
    pthread_create(&pthd2, NULL, pthread2, NULL) != 0 ||
    pthread_create(&pthd3, NULL, pthread3, NULL) != 0){
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
    id = semget(key, size, IPC_CREAT|0666);//创建size个信号量
    if(id < 0) {//判断是否创建成功
        printf("create sem %d,%d error\n", key, size);//创建失败，打印错误
    }
    return id;
}

void destroy_Sem(int semid)
{
    int res = semctl(semid,0,IPC_RMID,0);//从系统中删除信号量
    if (res < 0) {//判断是否删除成功
        printf("destroy sem %d error\n", semid);//信号量删除失败，输出信息
    }
    return;
}

int get_Sem(int key, int size)
{
    int id;
    id = semget(key, size, 0666);//获取已经创建的信号量
    if(id < 0) {
        printf("get sem %d,%d error\n", key, size);
    }
    return id;
}

void set_N(int semid, int index, int n)
{
    union semun semopts;
    semopts.val = n;//设定信号量的初值为n
    semctl(semid,index,SETVAL,semopts);//初始化信号量，信号量编号为index
    return;
}

void P(int semid, int index)
{
    struct sembuf sem;//信号量操作数组
    sem.sem_num = index;//信号量编号
    sem.sem_op = -1;//信号量操作，-1为P操作
    sem.sem_flg = 0;//操作标记：0或IPC_NOWAIT等
    semop(semid,&sem,1);//1:表示执行命令的个数
    return;
}

void V(int semid, int index)
{
    struct sembuf sem;//信号量操作数组
    sem.sem_num = index;//信号量编号
    sem.sem_op =  1;//信号量操作，1为V操作
    sem.sem_flg = 0; //操作标记
    semop(semid,&sem,1);//1:表示执行命令的个数
    return;
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

