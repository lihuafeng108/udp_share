//UDP单播，能发能收
/*
    实现广播，离不开广播地址，同一个子网（局域网）的所有主机网卡都会接收所在网段广播地址的数据包。广播地址应用于局域网内的
所有主机。广播地址(Broadcast Address)是专门用于同时向网络中（通常指同一子网）所有工作站进行发送的一个地址。

    广播UDP与单播UDP的区别就是IP地址不同，广播使用广播地址255.255.255.255，将消息发送到在同一广播网络上的每个主机。值得强调
的是：本地广播信息是不会被路由器转发。当然这是十分容易理解的，因为如果路由器转发了广播信息，那么势必会引起网络瘫痪。

    其实广播顾名思义，就是想局域网内所有的人说话，但是广播还是要指明接收者的端口号的，因为不可能接受者的所有端口都来收听广播。

【UDP广播特点如下】：

1)可以向广播域内的所有主机发送数据；
2)不能够跨越不同的网络，被路由器所隔离开。

*/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define BROADCAST_ID ("192.168.6.255")
#define UDP_PORT (1188)

void *recv_handle(void *arg);

int main(int argc, int *argv[])
{
    int udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket_fd < 0)
    {
        printf("Create socket failed!\n");
        return -1;
    }

    int enable = 1;
    int ret = setsockopt(udp_socket_fd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));
    if(ret < 0)
    {
        printf("Set setsockopt error!\n");
        close(udp_socket_fd);
        return -1;
    }

    struct sockaddr_in local_addr = {0};
    local_addr.sin_family = AF_INET;         //使用IPV4协议
    local_addr.sin_port = htons(UDP_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY; //让系统自动检测本地网卡，自动绑定本地ip

    ret = bind(udp_socket_fd, (struct sockaddr*)&local_addr, sizeof(local_addr));
    if(ret < 0)
    {
        printf("Bind socket failed!\n");
        close(udp_socket_fd);
        return -1;
    }

    pthread_t recv_phread;
    pthread_create(&recv_phread, NULL, recv_handle, (void *)&udp_socket_fd);

    struct sockaddr_in dst_addr = {0};
    int dest_port = 0;  //之前改成short类型，会导致发不出
    char msg[1024] = {0};
    int always_true = 1;

    printf("You can recv and send messages.\n");

    while(always_true)
    {
        printf("Send msg bellow:\n");
        memset(msg, 0, sizeof(msg));
        scanf("%s", msg);  //输入的msg不要用空格，否则会放到下一次的输入

        dst_addr.sin_family = AF_INET;
        dst_addr.sin_port = htons(UDP_PORT);
        //dst_addr.sin_addr.s_addr = inet_addr(BROADCAST_ID);  //好像用这个广播地址也可以，最后一个字节255，但是192.168.255.255这个地址不可以
        dst_addr.sin_addr.s_addr = 0xFFFFFFFFUL;

        sendto(udp_socket_fd, msg, strlen(msg), 0, (struct sockaddr*)&dst_addr, sizeof(dst_addr));

        if((0 == strcmp(msg, "exit")) || (0 == strcmp(msg, "")) )
        {
            printf("Exit the appication\n");
            break;
        }

    }

    pthread_cancel(recv_phread);
    close(udp_socket_fd);
    return 0;

}

void *recv_handle(void *arg)
{
    int *socket_fd = (int *)arg;
    char msg[1024] = {0};
    struct sockaddr_in src_addr = {0};
    src_addr.sin_family = AF_INET;
    int len = sizeof(src_addr);
    int always_true = 1;

    while(always_true)
    {
        memset(msg, 0, sizeof(msg));
        int ret = recvfrom(*socket_fd, msg, sizeof(msg), 0, (struct sockaddr*)&src_addr, &len);
        if(-1 == ret)
        {
            printf("recvfrom error!\n");
            break;
        }

        printf("==>[%s:%d]", inet_ntoa(src_addr.sin_addr), ntohs(src_addr.sin_port));
        printf("msg:%s\n", msg);
        if((0 == strcmp(msg, "exit")) ||  (0== strcmp(msg, "")))
        {
            printf("Other let me exit the recv handle\n");
            break;
        }
    }
}
