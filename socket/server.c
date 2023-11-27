#include "net_transfer.h"

#define PORT 8080
#define USER 0
#define ADMIN 1

int MAX_CLIENTS = 5;

bool handle_request(int socket, bool is_admin, char *username)
{
    char user_dir[50];
    sprintf(user_dir, "./%s", username);

    // 接收客户端请求
    char buffer[1024] = {0};
    read(socket, buffer, 1024);
    char *command = strdup(buffer);

    char *action = strtok(buffer, " ");

    if (strcmp(action, "logout") == 0)
    {
        // 用户登出
        char *message = "[+] Logout successful.\n";
        send(socket, message, strlen(message), 0);
        // 切换到原目录
        if(chdir("~/OSP/OS_projects/socket") == -1)
        {
            perror("[-] Failed to change directory"); 
        }
        return false;
    }
    else if (strcmp(action, "upload") == 0)
    {
        // 文件上传
        char *filename = strtok(NULL, " ");
        recv_file(filename, socket);
    }
    else if (strcmp(action, "download") == 0)
    {
        // 文件下载
        char *filename = strtok(NULL, " ");
        send_file(filename, socket);
    }
    else
    {
        // 执行命令
        char tmpfile[] = "command.XXXXXX";
        int fd = mkstemp(tmpfile);
        if (fd == -1)
        {
            perror("[-] Failed to create temporary file");
            return;
        }
        close(fd);

        char cmd[1024];
        sprintf(cmd, "%s > %s", command, tmpfile);
        int ret = system(cmd);
        if (ret != 0)
        {
            perror("[-] Failed to execute command");
            remove(tmpfile);
            return;
        }

        FILE *file = fopen(tmpfile, "r");
        if (file == NULL)
        {
            perror("[-] Failed to open file");
        }
        else
        {
            char buffer[1024];
            while (true)
            {
                ssize_t bytes_read = fread(buffer, sizeof(char), sizeof(buffer), file);
                if (bytes_read <= 0)
                {
                    break;
                }

                send(socket, buffer, bytes_read, 0);
            }
            fclose(file);
        }

        remove(tmpfile);
    }

    return true;
}

void *handle_client(void *arg)
{
    bool is_valid = false;
    bool is_admin = false;
    int new_socket = *(int *)arg;

    char buffer[1024] = {0};
    read(new_socket, buffer, 1024);
    printf("[+] Request received from client.\n");

    char *action = strtok(buffer, " ");
    char *username = strtok(NULL, " ");
    char *password = strtok(NULL, " ");

    if (action == NULL || username == NULL || password == NULL)
    {
        char *message = "[-] Invalid request.\n";
        send(new_socket, message, strlen(message), 0);
    }
    else if (strcmp(action, "register") == 0)
    {
        // 在此添加用户注册逻辑
        FILE *file = fopen("users.txt", "r");
        if (file == NULL)
        {
            perror("[-] Failed to open file");
            // exit(EXIT_FAILURE);
        }

        char stored_username[50];
        bool is_duplicate = false;
        while (fscanf(file, "%s", stored_username) != EOF)
        {
            if (strcmp(stored_username, username) == 0)
            {
                is_duplicate = true;
                break;
            }
        }
        fclose(file);

        if (is_duplicate)
        {
            char *message = "[-] Username already exists.\n";
            send(new_socket, message, strlen(message), 0);
        }
        else
        {
            file = fopen("users.txt", "a");
            if (file == NULL)
            {
                perror("[-] Failed to open file");
                // exit(EXIT_FAILURE);
            }
            int role = USER;
            fprintf(file, "%s %s %d\n", username, password, role);
            fclose(file);
            printf("[+] Registering new user: %s\n", username);
            char *message = "[+] Registration successful.\n";
            send(new_socket, message, strlen(message), 0);
            is_valid = true;

            char user_dir[50];
            sprintf(user_dir, "./%s", username);

            // 检查用户是否有足够的权限
            if (getuid() != 0)
            {
                perror("[-] You need to be root to chroot");
                exit(EXIT_FAILURE);
            }

            // 创建用户目录
            if (mkdir(user_dir, 0755) == -1 && errno != EEXIST)
            {
                perror("[-] Failed to create user directory");
                exit(EXIT_FAILURE);
            }

            // 限制用户访问权限
            if (chroot(user_dir) == -1)
            {
                perror("[-] Failed to chroot");
                exit(EXIT_FAILURE);
            }

            if (chdir("/") == -1)
            {
                perror("[-] Failed to chdir");
                exit(EXIT_FAILURE);
            }
        }
    }
    else if (strcmp(action, "login") == 0)
    {
        // 在此添加用户登录逻辑
        char command[1024];
        sprintf(command, "./check_user.sh %s %s", username, password);
        FILE *fp = popen(command, "r");
        if (fp == NULL)
        {
            perror("[+] Failed to run command");
            // exit(EXIT_FAILURE);
        }

        char output[1024];
        fgets(output, sizeof(output), fp);
        pclose(fp);

        if (strcmp(output, "valid 0\n") == 0)
        {
            printf("[+] Logging in user: %s\n", username);
            char *message = "[+] Login successful.\n";
            send(new_socket, message, strlen(message), 0);
            is_valid = true;
        }
        else if (strcmp(output, "valid 1\n") == 0)
        {
            printf("[+] Logging in admin: %s\n", username);
            char *message = "[+] Login successful.\n";
            send(new_socket, message, strlen(message), 0);
            is_valid = true;
            is_admin = true;
        }
        else
        {
            char *message = "[-] Invalid username or password.\n";
            send(new_socket, message, strlen(message), 0);
        }

        // 切换到用户目录
        char user_dir[50];
        sprintf(user_dir, "./%s", username);

        if(chdir(user_dir) == -1)
        {
            perror("[-] Failed to change directory"); 
        }
    }
    else
    {
        char *message = "[-] Invalid request.\n";
        send(new_socket, message, strlen(message), 0);
    }

    if (is_valid)
    {
        bool continue_loop = true;
        while (continue_loop)
        {
            continue_loop = handle_request(new_socket, is_admin, username);
        }
    }

    close(new_socket);

    // free(arg);

    printf("[-] Client %d disconnected.\n", new_socket);

    return NULL;
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // 创建 socket 文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("[-] Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 绑定 socket 到端口 8080
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("[-] Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        perror("[-] Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("[+] Server listening on port %d\n", PORT);

    while (true)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("[-] Accept failed");
            // exit(EXIT_FAILURE);
        }

        printf("[+] New connection, socket fd is %d, IP is: %s, port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        // 创建套接字描述符的副本
        int *new_sock = malloc(sizeof *new_sock);
        if (new_sock == NULL)
        {
            perror("[-] Memory allocation failed");
            // exit(EXIT_FAILURE);
        }
        *new_sock = new_socket;
        MAX_CLIENTS -= 1;

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, &new_socket);
    }

    return 0;
}