#include "net_transfer.h"

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
        getchar();

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
    }
    else if(strcmp(reg_or_log, "l") == 0)
    {
        // 登录
        char username[50];
        char password[50];
        printf("[+] Username: ");
        scanf("%s", username);
        printf("[+] Password: ");
        scanf("%s", password);
        getchar();

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
        char input_buffer[BUFFER];
        scanf("%[^\n]", input_buffer); // 读取输入的命令
        getchar(); // 读取换行符
        char *command = strdup(input_buffer);
        char *token = strtok(input_buffer, " ");
        if(strcmp(token, "help") == 0)
        {
            printf("[+] Commands:\n");
            printf("[+] help: show this help message.\n");
            printf("[+] upload: upload file to server.\n");
            printf("[+] download: download file from server.\n");
            printf("[+] logout: exit the program and logout.\n");
        }
        else if (strcmp(token, "upload") == 0)
        {
            // 文件上传
            char filename[50];
            printf("[+] Filename: ");
            scanf("%s", filename);
            getchar();

            sprintf(buffer, "upload %s", filename);
            send(sock, buffer, strlen(buffer), 0);

            send_file(filename, sock); // 发送文件

            char response[BUFFER];
            recv(sock, response, sizeof(response), 0);
            if(strcmp(response, SUCCESS) == 0)
            {
                printf("[+] Upload successful.\n");
            }
            else if (strcmp(response, FILE_EXIST) == 0)
            {
                printf("[-] File already exists.\n");
            }
        }
        else if (strcmp(token, "download") == 0)
        {
            // 文件下载
            char filename[50];
            printf("[+] Filename: ");
            scanf("%s", filename);
            getchar();

            sprintf(buffer, "download %s", filename);
            send(sock, buffer, strlen(buffer), 0);

            recv_file(filename, sock); // 接收文件
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
            send(sock, command, strlen(command), 0);
            char response[1024] = {0};
            ssize_t len = recv(sock, response, sizeof(response) - 1, 0);
            if (len < 0)
            {
                perror("[-] Failed to receive message.");
                exit(EXIT_FAILURE);
            }
            if(response == NULL)
            {
                printf("[+] Command executed.\n");
                continue;
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

    handle_connection((void *) &sock);

    return 0;
}