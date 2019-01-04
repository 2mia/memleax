#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/mman.h>

static void *thread_func(void *ptr) {
    sleep(1);
    printf("\tcalling 2nd malloc\n");
    ptr = malloc(1024);
    sleep(3);
    printf("[x] exit from thread");
    exit(0);
    return NULL;
}

static void handler(int signum) {
    printf("\treceived signal %d\n", signum);

    switch(signum){
        case SIGUSR1:{
            printf("\tcalling malloc\n");
            char *ptr = malloc(1024);
            
            printf("\tcalling memalign\n");
            char *ptr2;
            posix_memalign(&ptr2, 16, 768);
            
            printf("\tcalling mmap\n");
            char *ptr3 = mmap(NULL, 512, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
            char *ptr4 = mmap(ptr3, 1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0);

            printf("calling frees\n");
            free(ptr);
            free(ptr2);
            munmap(ptr4, 1024);

            pthread_t thread;
            pthread_create(&thread, NULL, thread_func, NULL);
            break;
        }
        case SIGUSR2:
            break;
    }
}

int main(int argc, char **argv) {
    printf("=> test 01\n");

    struct sigaction sa = {0,};

    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    //sa.sa_flags = SA_SIGINFO | SA_RESTART; 

    if (sigaction(SIGUSR1, &sa, NULL) == -1){
        printf("error setting handler\n");
        exit(1);
    }
    if (sigaction(SIGUSR2, &sa, NULL) == -1){
        printf("error setting handler\n");
        exit(1);
    }

    while(1)
        sleep(10);
}
