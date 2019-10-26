#include "server.h"

void errorMsg(char *str)
{
    printf("error: %s\n", str);
}
int main(int argc, char **argv)
{
    //1. creat socket
    int tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket_fd < 0)
    {
        errorMsg("tcp socket creat error...");
        exit(1);
    }

    // 2. bind port and ip
    struct sockaddr_in addr_set;
    addr_set.sin_family      = AF_INET;
    addr_set.sin_port        = htons(SERVER_PORT);
    addr_set.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    if (bind(tcp_socket_fd, (struct sockaddr *)&addr_set, sizeof(addr_set)) < 0)
    {
        perror("bind error:");
        exit(1);
    }
    //3. 把主动套接字变为被动套接字
    if (listen(tcp_socket_fd, 5) < 0)
    {
        perror("listen");
        exit(1);
    }
    printf("Server waitting for connecting....\n");
    //4. 阻赛等待客户端连接， 一个线程对应一路连接
    pthread_t thread_tid;
    struct clien_info *temp;

    while(1)
    {
        int tcp_socket_newfd = accept(tcp_socket_fd, NULL, NULL);
        if (tcp_socket_newfd < 0)
        {
            perror("accept error:");
            continue;
        }
        // FIX ME ...!!!

        else
        {
            printf("connect established\n");
        }
        // FIX me  pthread_create();
    }

    return 0;
}
