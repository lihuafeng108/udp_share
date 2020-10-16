//UDP组播
/*
1、多播的概念
    单播用于两个主机之间的端对端通信，广播用于一个主机对整个局域网上所有主机上的数据通信。单播和广播是两个极端，
要么对一个主机进行通信，要么对整个局域网上的主机进行通信。实际情况下，经常需要对一组特定的主机进行通信，而不是整
个局域网上的所有主机，这就是多播的用途。
    IP 多播（也称多址广播或组播）技术，是一种允许一台或多台主机（多播源）发送单一数据包到多台主机（一次的，同时的）
的 TCP/IP 网络技术。多播作为一点对多点的通信，数据的收发仅仅在同一分组中进行，是节省网络带宽的有效方法之一。在网络应用
中，当需要将一个节点的信号传送到多个节点时，无论是采用重复点对点通信方式，还是采用广播方式，都会严重浪费网络带宽，只有
多播才是最好的选择。多播能使一个或多个多播源只把数据包发送给特定的多播组，而只有加入该多播组的主机才能接收到数据包。

2、多播的地址
    IP 多播通信必须依赖于 IP 多播地址，在 IPv4 中它是一个 D 类 IP 地址，范围从 224.0.0.0 到 239.255.255.255，并被划分为
    局部链接多播地址、预留多播地址和管理权限多播地址三类：
1）局部链接多播地址范围在 224.0.0.0~224.0.0.255，这是为路由协议和其它用途保留的地址，路由器并不转发属于此范围的IP包；
2）预留多播地址为 224.0.1.0~238.255.255.255，可用于全球范围（如Internet）或网络协议；
3）管理权限多播地址为 239.0.0.0~239.255.255.255，可供组织内部使用，类似于私有 IP 地址，不能用于 Internet，可限制多播范围。

3、多播的特点

【优点】：
多播服务端针对特定多播地址只发送一次数据，但是组内的所有客户端都能收到数据；
与单播一样，多播是允许在广域网即Internet上进行传输的，而广播仅仅在同一局域网上才能进行；
服务器的总带宽不受客户端带宽的限制；
加入特定的多播组即可接收发往该多播组的数据。

【缺点】：
多播与单播相比没有纠错机制，当发生错误的时候难以弥补，但是可以在应用层来实现此种功能；
多播的网络支持存在缺陷，需要路由器及网络协议栈的支持

*/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

void *recv_handle(void *arg);

int main(int argc, int *argv[])
{
    if(argc != 3)
    {
        printf("Please input the ip and port!\n");
        return -1;
    }

    int port = atoi(argv[2]);
    if((port<1025) || (port>65535))
    {
        printf("The port is illegal! Please let it in[1025, 65535]\n");
        return -1;
    }

    int udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket_fd < 0)
    {
        printf("Create socket failed!\n");
        return -1;
    }

    //加入组播，组播地址范围:224.0.0.0~239.255.255.255
    struct ip_mreq zu={0};
    zu.imr_multiaddr.s_addr = inet_addr(argv[1]);  //设置组播地址
	zu.imr_interface.s_addr = inet_addr("0.0.0.0");
	int ret = setsockopt(udp_socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &zu, sizeof(zu));
	if(ret < 0)
	{
		perror("setsockopt fail\n");
		return -1;
	}
	else
	{
		printf("已经加入组：[%s:%d], 等待接收组消息！！！\n", argv[1], port);
	}

    struct sockaddr_in local_addr = {0};
    local_addr.sin_family = AF_INET;         //使用IPV4协议
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = inet_addr("0.0.0.0"); //注意：Linux下，加入组播后，绑定地址只能绑定0.0.0.0地址否则会接收不到数据

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
    char dest_ip[32]={0};
    int dest_port = 0;  //之前改成short类型，会导致发不出
    char msg[1024] = {0};
    int always_true = 1;

    printf("You can recv and send messages.\n");

    while(always_true)
    {
        printf("Send format:ip port msg\n");
        memset(dest_ip, 0, sizeof(dest_port));
        memset(msg, 0, sizeof(msg));
        scanf("%s %d %s", dest_ip, &dest_port, msg);  //输入的msg不要用空格，否则会放到下一次的输入

        if((dest_port<1025) || (dest_port>65535))
        {
            printf("dest port error!\n");
            close(udp_socket_fd);
            pthread_cancel(recv_phread);
            return -1;
        }

        dst_addr.sin_family = AF_INET;
        dst_addr.sin_port = htons(dest_port);
        dst_addr.sin_addr.s_addr = inet_addr(dest_ip);

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
