#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/times.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "sem.h"

#define FULL 0
#define EMPTY 1
#define BLOCK_SIZE 4096
#define BLOCK_NUM 100
char srcpath[50];
char dstpath[50];

pid_t p1;
int semid, shmid;
int pos = 0;

/* parent is writer */
int writer(char *S){
    FILE * src = fopen(srcpath, "rb");
    int time0 = get_time_ms();
    while(1){
        P(semid, EMPTY);
        unsigned short blocksize = fread(S + pos * BLOCK_SIZE + 2, sizeof(char), BLOCK_SIZE - 2, src);
        *(unsigned short *)(S + pos * BLOCK_SIZE) = blocksize;  // the first two bytes of each block is blocksize
        //        printf("W: blocknum %d, blocksize %d\n", pos, blocksize);
        pos = (pos + 1) % BLOCK_NUM;  // rotating buf
        V(semid, FULL);
        if(blocksize != BLOCK_SIZE - 2) break;
    }
    printf("Waiting reader finishes\n");
    waitpid(p1, NULL, 0);   //wait until reader finishes
    printf("Time consume: %dms!\n", get_time_ms() - time0);
    destroy_Sem(semid);
    return 0;
}
/* son is reader */
int reader(char *S){
    FILE * dst = fopen(dstpath, "wb");
    while(1){
        P(semid, FULL);
        unsigned short blocksize = *(unsigned short *)(S + pos * BLOCK_SIZE);
        fwrite(S + pos * BLOCK_SIZE + 2, sizeof(char), blocksize, dst);
        //        printf("R: blocknum %d, blocksize %d\n", pos, blocksize);
        pos = (pos + 1) % BLOCK_NUM;  // rotating buf
        if(blocksize != BLOCK_SIZE - 2) break;
        V(semid, EMPTY);
    }
    fclose(dst);
    printf("Copy finished!\n");
    return 0;
}

int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Arguments error!\n");
        return -1;
    }
    strcpy(srcpath, argv[1]);
    strcpy(dstpath, argv[2]);

    shmid = shmget(IPC_PRIVATE, BLOCK_NUM * BLOCK_SIZE, IPC_CREAT | 0666);
    unsigned char *S = (char *)shmat(shmid, NULL, SHM_R | SHM_W);
    memset(S, 0, BLOCK_NUM * BLOCK_SIZE);
    if((semid = create_Sem(IPC_PRIVATE, 2)) < 0){ return -1; };  //Create two semaphores
    set_N(semid, FULL, 0);     //set init value
    set_N(semid, EMPTY, BLOCK_NUM);

    p1 = fork();	//son1
    if(p1 < 0){
        printf("Fork error!\n");
        return -1;
    }
    if(p1 == 0) reader(S);
    else writer(S);
    return 0;

}


