#include "net_transfer.h"

void send_file(char *filename, int socket)
{
    FILE *file = fopen (filename, "r");
    if (file == NULL)
    {
        perror("[-] Failed to open file");
        exit(EXIT_FAILURE);
    }

    struct stat st;
    stat(filename, &st);
    size_t file_size = st.st_size;
    printf("[+] File size: %ld\n", file_size); // DEBUG

    send(socket, &file_size, sizeof(file_size), 0); // 发送文件大小

    char file_buffer[file_size];
    if(fread(file_buffer, 1, file_size, file) != file_size)
    {
        perror("[-] Failed to read file");
        exit(EXIT_FAILURE);
    }

    size_t ret = 0;
    while ( ret < file_size )
    {
        ssize_t b = send(socket, file_buffer + ret, file_size - ret, 0);
        if(b == 0)
        {
            printf("[-] Connection closed.\n");
        }
        if(b < 0)
        {
            perror("[-] Failed to send file.");
            exit(EXIT_FAILURE);
        }
        ret += b;
    }
    fclose(file);
}

void recv_file(char *filename, int socket)
{
    if(filename == NULL)
    {
        send(socket, NEED_FILE_NAME, sizeof(NEED_FILE_NAME), 0);
    }
    else
    {
        if(access(filename, F_OK) == -1)
        {
            FILE *file = fopen(filename, "w");
            if(file == NULL)
            {
                perror("[-] Failed to open file");
                // exit(EXIT_FAILURE);
            }

            size_t file_size;
            recv(socket, &file_size, sizeof(file_size), 0); // 接收文件大小
            printf("[+] File size: %ld\n", file_size); // DEBUG
            char file_buffer[file_size];

            size_t ret = 0;
            while ( ret < file_size )
            {
                ssize_t b = recv(socket, file_buffer + ret, file_size - ret, 0);
                if(b == 0)
                {
                    printf("[-] Connection closed.\n");
                }
                if(b < 0)
                {
                    perror("[-] Failed to receive file.");
                    // exit(EXIT_FAILURE);
                }
                ret += b;
            }
            fwrite(file_buffer, 1, file_size, file);
            fclose(file);
        }
        else
        {
            send(socket, FILE_EXIST, sizeof(FILE_EXIST), 0);
            char file_buffer[1024];
            while (true)
            {
                ssize_t bytes_rcvd = recv(socket, file_buffer, sizeof(file_buffer), 0);
                if (bytes_rcvd <= 0)
                {
                    break;
                }
            }
        }
    }
}

