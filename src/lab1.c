#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> //pid_t
#include <signal.h>

#define READ_END 0
#define WRITE_END 1

pid_t p1, p2;
int pipefd[2];
int msg_num = 0;

void MY_SIGUSR(int sig){
    if(sig == SIGUSR1){
        printf("Child Process 1 received %d messages!\n", msg_num);
        printf("Child Process 1 is Killed by Parent!\n");
    }
    else{
        printf("Child Process 2 received %d messages!\n", msg_num);
        printf("Child Process 2 is Killed by Parent!\n");
    }
    exit(1);
}

void MY_SIGINT(){
    printf("Parent: SIGINT received!\n");
    kill(p1, SIGUSR1);
    kill(p2, SIGUSR2);
    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
    printf("Parent Process sent %d messages!\n", msg_num);
    printf("Parent Process is Killed!\n");
    exit(1);
}

int parent(){
    char send[30];
    //printf("I'm parent!\n");
    for(int i = 0; i < 10; i++){
        sprintf(send, "I send you %d times", i+1);
        write(pipefd[WRITE_END], send, sizeof(send));
        msg_num++;
        sleep(1);
    }
    kill(p1, SIGUSR1);
    kill(p2, SIGUSR2);
    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
    printf("Parent Process sent %d messages!\n", msg_num);
    printf("Parent Process is Killed!\n");
}

int child1(){
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1, MY_SIGUSR);
    char rcv[30];
    //printf("I'm Child1\n");
    while(1){
        printf("Child Process 1: ");
        read(pipefd[READ_END], rcv, sizeof(rcv));
        msg_num++;
        printf("%s\n", rcv);
        //write(pipefd[WRITE_END], rcv, sizeof(rcv));
    }
    return 0;
}

int child2(){
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR2, MY_SIGUSR);
    char rcv[30];
    //printf("I'm Child2\n");
    while(1){
        printf("Child Process 2: ");
        read(pipefd[READ_END], rcv, sizeof(rcv));
        msg_num++;
        printf("%s\n", rcv);
    }
    return 0;
}

int main(){
    signal(SIGINT, MY_SIGINT);
    if(pipe(pipefd) == -1){	//create a pipe
        printf("Pipe create error!\n");
        return -1;
    }

    p1 = fork();	//son1
    if(p1 < 0){
        printf("Fork error!\n");
        return -1;
    }
    if(p1 > 0) p2 = fork();	//if parent, create son2
    if(p2 < 0){
        printf("Fork error!\n");
        return -1;
    }
    if(p1 == 0) child1();
    else if(p2 == 0) child2();
    else parent();
    close(pipefd[READ_END]);
    close(pipefd[WRITE_END]);
    return 0;

}