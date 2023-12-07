#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_READERS 5
#define NUM_WRITERS 2

sem_t mutex, wrt, access_order;
int readcount = 0, waitreaders = 0;

void *reader(void *param) {
    do {
        sem_wait(&mutex);
        waitreaders++;  
        sem_post(&mutex);

        sem_wait(&access_order);
        sem_wait(&mutex);
        waitreaders--;
        if(readcount == 0)
            sem_wait(&wrt);
        readcount++;
        sem_post(&mutex);
        sem_post(&access_order);
        
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
        sem_wait(&access_order);
        if(waitreaders > 0) {
            sem_post(&access_order);
            sleep(1);
            continue;
        }
        sem_wait(&wrt);
        printf("Writer is writing\n");
        sleep(1);
        sem_post(&wrt);
        sem_post(&access_order);
    } while(1);
}

int main(){
    sem_init(&mutex, 0, 1);
    sem_init(&wrt, 0, 1);
    sem_init(&access_order, 0, 1);
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