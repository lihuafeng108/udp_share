#RSU交叉编译器:arm-linux-gnueabihf-gcc
#笔记本跑的使用gcc编译器

all:
	arm-linux-gnueabihf-gcc udp_share.c -o udp_share -pthread

send:
	chmod 755 udp_share
	zftp_g2n udp_share /home/root ${ip}
	
clean:
	rm -rf udp_share