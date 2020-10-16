//UDP�鲥
/*
1���ಥ�ĸ���
    ����������������֮��Ķ˶Զ�ͨ�ţ��㲥����һ�����������������������������ϵ�����ͨ�š������͹㲥���������ˣ�
Ҫô��һ����������ͨ�ţ�Ҫô�������������ϵ���������ͨ�š�ʵ������£�������Ҫ��һ���ض�����������ͨ�ţ���������
���������ϵ���������������Ƕಥ����;��
    IP �ಥ��Ҳ�ƶ�ַ�㲥���鲥����������һ������һ̨���̨�������ಥԴ�����͵�һ���ݰ�����̨������һ�εģ�ͬʱ�ģ�
�� TCP/IP ���缼�����ಥ��Ϊһ��Զ���ͨ�ţ����ݵ��շ�������ͬһ�����н��У��ǽ�ʡ����������Ч����֮һ��������Ӧ��
�У�����Ҫ��һ���ڵ���źŴ��͵�����ڵ�ʱ�������ǲ����ظ���Ե�ͨ�ŷ�ʽ�����ǲ��ù㲥��ʽ�����������˷��������ֻ��
�ಥ������õ�ѡ�񡣶ಥ��ʹһ�������ಥԴֻ�����ݰ����͸��ض��Ķಥ�飬��ֻ�м���öಥ����������ܽ��յ����ݰ���

2���ಥ�ĵ�ַ
    IP �ಥͨ�ű��������� IP �ಥ��ַ���� IPv4 ������һ�� D �� IP ��ַ����Χ�� 224.0.0.0 �� 239.255.255.255����������Ϊ
    �ֲ����Ӷಥ��ַ��Ԥ���ಥ��ַ�͹���Ȩ�޶ಥ��ַ���ࣺ
1���ֲ����Ӷಥ��ַ��Χ�� 224.0.0.0~224.0.0.255������Ϊ·��Э���������;�����ĵ�ַ��·��������ת�����ڴ˷�Χ��IP����
2��Ԥ���ಥ��ַΪ 224.0.1.0~238.255.255.255��������ȫ��Χ����Internet��������Э�飻
3������Ȩ�޶ಥ��ַΪ 239.0.0.0~239.255.255.255���ɹ���֯�ڲ�ʹ�ã�������˽�� IP ��ַ���������� Internet�������ƶಥ��Χ��

3���ಥ���ص�

���ŵ㡿��
�ಥ���������ض��ಥ��ַֻ����һ�����ݣ��������ڵ����пͻ��˶����յ����ݣ�
�뵥��һ�����ಥ�������ڹ�������Internet�Ͻ��д���ģ����㲥������ͬһ�������ϲ��ܽ��У�
���������ܴ����ܿͻ��˴�������ƣ�
�����ض��Ķಥ�鼴�ɽ��շ����öಥ������ݡ�

��ȱ�㡿��
�ಥ�뵥�����û�о�����ƣ������������ʱ�������ֲ������ǿ�����Ӧ�ò���ʵ�ִ��ֹ��ܣ�
�ಥ������֧�ִ���ȱ�ݣ���Ҫ·����������Э��ջ��֧��

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

    //�����鲥���鲥��ַ��Χ:224.0.0.0~239.255.255.255
    struct ip_mreq zu={0};
    zu.imr_multiaddr.s_addr = inet_addr(argv[1]);  //�����鲥��ַ
	zu.imr_interface.s_addr = inet_addr("0.0.0.0");
	int ret = setsockopt(udp_socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &zu, sizeof(zu));
	if(ret < 0)
	{
		perror("setsockopt fail\n");
		return -1;
	}
	else
	{
		printf("�Ѿ������飺[%s:%d], �ȴ���������Ϣ������\n", argv[1], port);
	}

    struct sockaddr_in local_addr = {0};
    local_addr.sin_family = AF_INET;         //ʹ��IPV4Э��
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = inet_addr("0.0.0.0"); //ע�⣺Linux�£������鲥�󣬰󶨵�ַֻ�ܰ�0.0.0.0��ַ�������ղ�������

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
    int dest_port = 0;  //֮ǰ�ĳ�short���ͣ��ᵼ�·�����
    char msg[1024] = {0};
    int always_true = 1;

    printf("You can recv and send messages.\n");

    while(always_true)
    {
        printf("Send format:ip port msg\n");
        memset(dest_ip, 0, sizeof(dest_port));
        memset(msg, 0, sizeof(msg));
        scanf("%s %d %s", dest_ip, &dest_port, msg);  //�����msg��Ҫ�ÿո񣬷����ŵ���һ�ε�����

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
