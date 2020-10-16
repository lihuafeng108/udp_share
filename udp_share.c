//UDP�������ܷ�����
/*
    ʵ�ֹ㲥���벻���㲥��ַ��ͬһ��������������������������������������������ι㲥��ַ�����ݰ����㲥��ַӦ���ھ������ڵ�
�����������㲥��ַ(Broadcast Address)��ר������ͬʱ�������У�ͨ��ָͬһ���������й���վ���з��͵�һ����ַ��

    �㲥UDP�뵥��UDP���������IP��ַ��ͬ���㲥ʹ�ù㲥��ַ255.255.255.255������Ϣ���͵���ͬһ�㲥�����ϵ�ÿ��������ֵ��ǿ��
���ǣ����ع㲥��Ϣ�ǲ��ᱻ·����ת������Ȼ����ʮ���������ģ���Ϊ���·����ת���˹㲥��Ϣ����ô�Ʊػ���������̱����

    ��ʵ�㲥����˼�壬����������������е���˵�������ǹ㲥����Ҫָ�������ߵĶ˿ںŵģ���Ϊ�����ܽ����ߵ����ж˿ڶ��������㲥��

��UDP�㲥�ص����¡���

1)������㲥���ڵ����������������ݣ�
2)���ܹ���Խ��ͬ�����磬��·���������뿪��

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
    local_addr.sin_family = AF_INET;         //ʹ��IPV4Э��
    local_addr.sin_port = htons(UDP_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY; //��ϵͳ�Զ���Ȿ���������Զ��󶨱���ip

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
    int dest_port = 0;  //֮ǰ�ĳ�short���ͣ��ᵼ�·�����
    char msg[1024] = {0};
    int always_true = 1;

    printf("You can recv and send messages.\n");

    while(always_true)
    {
        printf("Send msg bellow:\n");
        memset(msg, 0, sizeof(msg));
        scanf("%s", msg);  //�����msg��Ҫ�ÿո񣬷����ŵ���һ�ε�����

        dst_addr.sin_family = AF_INET;
        dst_addr.sin_port = htons(UDP_PORT);
        //dst_addr.sin_addr.s_addr = inet_addr(BROADCAST_ID);  //����������㲥��ַҲ���ԣ����һ���ֽ�255������192.168.255.255�����ַ������
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
