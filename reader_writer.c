#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_READERS 5
#define NUM_WRITERS 5

sem_t mutex, wrt, wrt_first;
int readcount = 0, waitreaders = 0;

void *reader(void *param) {
    do {
        sem_wait(&wrt_first);
        waitreaders++;
        sem_wait(&mutex);
        waitreaders--;
        readcount++;
        if(readcount == 0)
            sem_wait(&wrt);
        sem_post(&mutex);
        sem_post(&wrt_first);
        
        printf("Reader is reading\n");
        sleep(1);
        
        sem_wait(&mutex);
        readcount--;
        if(readcount == 0)
            sem_post(&wrt);
        sem_post(&mutex);
    } while(1);
}

void *writer(void *param) {
    do {
        sem_wait(&wrt_first);
        if(waitreaders > 0) {
            sem_post(&wrt_first);
            sleep(1);
            continue;
        }
        sem_wait(&wrt);
        printf("Writer is writing\n");
        sleep(1);
        sem_post(&wrt);
        sem_post(&wrt_first);
    } while(1);
}

int main(){
    sem_init(&mutex, 0, 1);
    sem_init(&wrt, 0, 1);
    sem_init(&wrt_first, 0, 1);
    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];
    for(int i = 0; i < NUM_READERS; i++)
        pthread_create(&readers[i], NULL, reader, NULL);
    for(int i = 0; i < NUM_WRITERS; i++)
        pthread_create(&writers[i], NULL, writer, NULL);
    for(int i = 0; i < NUM_READERS; i++)
        pthread_join(readers[i], NULL);
    for(int i = 0; i < NUM_WRITERS; i++)    
        pthread_join(writers[i], NULL);
    return 0;
}