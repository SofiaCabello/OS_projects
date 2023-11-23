#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 8080
#define USER 0
#define ADMIN 1
#define REGISTER "register"
#define LOGIN "login"

void *handle_connection(void *socket)
{
    int sock = *(int *)socket;
    char buffer[1024] = {0};

    printf("[+] Client running on thread %ld.\n", pthread_self());
    char reg_or_log[2];
    printf("[+] Choose to register or login(r/l): ");
    scanf("%s", reg_or_log);
    
    if(strcmp(reg_or_log, "r") == 0){
        // 注册
        char username[50];
        char password[50];
        printf("[+] Username: ");
        scanf("%s", username);
        printf("[+] Password: ");
        scanf("%s", password);

        sprintf(buffer, "%s %s %s", REGISTER, username, password);
        send(sock, buffer, strlen(buffer), 0);

        char response[1024] = {0};
        ssize_t len = recv(sock, response, sizeof(response) - 1, 0);
        if (len < 0)
        {
            perror("[-] Failed to receive message.");
            exit(EXIT_FAILURE);
        }
        response[len] = '\0';
        printf("%s", response);
    }else if(strcmp(reg_or_log, "l") == 0){
        // 登录
        char username[50];
        char password[50];
        printf("[+] Username: ");
        scanf("%s", username);
        printf("[+] Password: ");
        scanf("%s", password);

        sprintf(buffer, "%s %s %s", LOGIN, username, password);
        send(sock, buffer, strlen(buffer), 0);

        char response[1024] = {0};
        ssize_t len = recv(sock, response, sizeof(response) - 1, 0);
        if (len < 0)
        {
            perror("[-] Failed to receive message.");
            exit(EXIT_FAILURE);
        }
        response[len] = '\0';
        printf("%s", response);
    }
    else{
        printf("[-] Invalid input.\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] For help, type 'help'.\n");
    while(true){
        printf("Client> ");
        scanf("%s", buffer);
        char *command = buffer;
        char *token = strtok(buffer, " ");
        if(strcmp(token, "help") == 0)
        {
            printf("[+] Commands:\n");
            printf("[+] help: show this help message.\n");
            printf("[+] upload: upload file to server.\n");
            printf("[+] download <filename>: download file from server.\n");
            printf("[+] logout: exit the program and logout.\n");
        }
        else if (strcmp(token, "upload") == 0)
        {
            // 文件上传
            char filename[50];
            printf("[+] Filename: ");
            scanf("%s", filename);

            sprintf(buffer, "upload %s", filename);
            send(sock, buffer, strlen(buffer), 0);

            char response[1024] = {0};
            ssize_t len = recv(sock, response, sizeof(response) - 1, 0);
            if (len < 0)
            {
                perror("[-] Failed to receive message.");
                exit(EXIT_FAILURE);
            }
            response[len] = '\0';
            printf("%s", response);

            if(strcmp(response, "[+] File exists.\n") == 0){
                continue;
            }

            FILE *file = fopen(filename, "r");
            if (file == NULL)
            {
                perror("[-] Failed to open file");
                exit(EXIT_FAILURE);
            }

            char buffer[1024];
            while (true)
            {
                ssize_t bytes_rcvd = fread(buffer, sizeof(char), sizeof(buffer), file);
                if (bytes_rcvd <= 0)
                {
                    break;
                }

                send(sock, buffer, bytes_rcvd, 0);
            }
            fclose(file);
        }
        else if (strcmp(token, "logout") == 0)
        {
            send(sock, "logout", strlen("logout"), 0);
            char response[1024] = {0};
            ssize_t len = recv(sock, response, sizeof(response) - 1, 0);
            if (len < 0)
            {
                perror("[-] Failed to receive message.");
                exit(EXIT_FAILURE);
            }
            response[len] = '\0';
            printf("%s", response);
            break;
        }
        else
        {
            send(sock, command, strlen(buffer), 0);
            char response[1024] = {0};
            ssize_t len = recv(sock, response, sizeof(response) - 1, 0);
            if (len < 0)
            {
                perror("[-] Failed to receive message.");
                exit(EXIT_FAILURE);
            }
            response[len] = '\0';
            printf("%s", response);
        }
    }
}


int main(int argc, char const *argv[])
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("[-] Socket creation error");
        exit(EXIT_FAILURE);        
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        perror("[-] Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("[-] Connection Failed");
        exit(EXIT_FAILURE);
    }

    pthread_t thread;
    pthread_create(&thread, NULL, handle_connection, (void *)&sock);
    pthread_join(thread, NULL);

    return 0;
}