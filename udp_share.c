//UDP组播

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
