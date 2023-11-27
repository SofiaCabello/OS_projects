#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <errno.h>

#define PORT 8080
#define USER 0
#define ADMIN 1
#define BUFFER 1024

#define SUCCESS "100"
#define FAIL_OPEN_FILE "101"
#define FAIL_READ_FILE "102"
#define FAIL_SEND_FILE "103"
#define FAIL_RECV_FILE "104"
#define FILE_NOT_EXIST "105"
#define FILE_EXIST "106"
#define NEED_FILE_NAME "201"

void send_file(char *filename, int socket);
void recv_file(char *filename, int socket);
void send_response(int socket, int response);
void recv_response(int socket, int *response);
int *code(int CODE);

#endif