// 这段代码是一个基于 netmap 库的网络数据包处理示例。它演示了如何使用 netmap 接口来捕获和处理网络数据包，包括 ARP 和 IP (UDP, TCP) 数据包。以下是代码中各部分的解释：

#include <stdio.h>

#include <sys/poll.h>
#include <arpa/inet.h>

#define NETMAP_WITH_LIBS

#include <net/netmap_user.h>

#pragma pack(1)


#define ETH_ADDR_LENGTH		6

#define PROTO_IP			0x0800
#define PROTO_ARP			0x0806
#define PROTO_UDP			17
#define PROTO_TCP			6
#define PROTO_ICMP			1

// ethhdr：以太网帧头结构体，包括目的地址、源地址和协议类型。
struct ethhdr {
	unsigned char h_dst[ETH_ADDR_LENGTH];
	unsigned char h_src[ETH_ADDR_LENGTH];
	unsigned short h_proto;
	
}; // 14

// iphdr：IP协议头结构体，包含了版本、头长度、服务类型、总长度、标识、片偏移、TTL(Time To Live)、协议类型、校验和、源IP地址和目的IP地址。

struct iphdr {

	unsigned char hdrlen:4,
				  version:4; // 0x45
				  

	unsigned char tos;

	unsigned short totlen;
	
	unsigned short id;

	unsigned short flag_offset; //

	unsigned char ttl; //time to live
	// 0x1234// htons

	unsigned char type;

	unsigned short check;

	unsigned int sip;
	unsigned int dip;

}; // 20


// udphdr：UDP头结构体，包括源端口、目的端口、长度和校验和。

struct udphdr {

	unsigned short sport;
	unsigned short dport;

	unsigned short length;
	unsigned short check;

}; // 8

// udppkt：组合了以太网头、IP头和UDP头的UDP数据包结构体。

struct udppkt {

	struct ethhdr eh; // 14
	struct iphdr ip;  // 20 
	struct udphdr udp; // 8

	unsigned char data[0];

}; // sizeof(struct udppkt) == 

// arphdr：ARP协议头结构体，包含硬件类型、协议类型、硬件地址长度、协议地址长度、操作码、发送方MAC地址、发送方IP地址、目标MAC地址和目标IP地址。

struct arphdr {

	unsigned short h_type;
	unsigned short h_proto;

	unsigned char h_addrlen;
	unsigned char h_protolen;

	unsigned short oper;

	unsigned char smac[ETH_ADDR_LENGTH];
	unsigned int sip;

	unsigned char dmac[ETH_ADDR_LENGTH];
	unsigned int dip;
};
// arppkt：包含以太网头和ARP头的ARP数据包结构体。

struct arppkt {

	struct ethhdr eh;
	struct arphdr arp;

};

// tcphdr：TCP头结构体，包括源端口、目的端口、序列号、确认号、头部长度、标志、窗口大小、校验和和紧急指针。

struct tcphdr {

	unsigned short sport;
	unsigned short dport;

	unsigned int seqnum;
	unsigned int acknum;

	unsigned char hdrlen_resv; //

	unsigned char flag; 

	unsigned short window; // 1460

	unsigned short checksum;
	unsigned short urgent_pointer;

	unsigned int options[0];
				  

};

// tcppkt：组合了以太网头、IP头和TCP头的TCP数据包结构体。

struct tcppkt {

	struct ethhdr eh; // 14
	struct iphdr ip;  // 20 
	struct tcphdr tcp; // 8

	unsigned char data[0];

};

// 定义了一个枚举类型 _tcp_status 来标识 TCP 连接的不同状态。
typedef enum _tcp_status {

	TCP_STATUS_CLOSED,
	TCP_STATUS_LISTEN,
	TCP_STATUS_SYN_REVD,
	TCP_STATUS_SYN_SENT,
	TCP_STATUS_ESTABLISHED,
	TCP_STATUS_FIN_WAIT_1,
	TCP_STATUS_FIN_WAIT_2,
	TCP_STATUS_CLOSING,
	TCP_STATUS_TIME_WAIT,

	TCP_STATUS_CLOSE_WAIT,
	TCP_STATUS_LAST_ACK,

};

// 定义了一系列宏来表示 TCP 头中的标志位。
#define TCP_CWR_FLAG		0x80
#define TCP_ECE_FLAG		0x40
#define TCP_URG_FLAG		0x20
#define TCP_ACK_FLAG		0x10
#define TCP_PSH_FLAG		0x08
#define TCP_RST_FLAG		0x04
#define TCP_SYN_FLAG		0x02
#define TCP_FIN_FLAG		0x01

// arp table
//定义了一个结构体 ntcb 用来维护 TCP 连接的状态和信息。
struct ntcb {

	unsigned int sip;
	unsigned int dip;
	unsigned short sport;
	unsigned short dport;

	unsigned char smac[ETH_ADDR_LENGTH];
	unsigned char dmac[ETH_ADDR_LENGTH];


	unsigned char status;
	

};


// 一个将字符串格式的MAC地址转换为字节格式的函数。
int str2mac(char *mac, char *str) {

	char *p = str;
	unsigned char value = 0x0;
	int i = 0;

	while (p != '\0') {
		
		if (*p == ':') {
			mac[i++] = value;
			value = 0x0;
		} else {
			
			unsigned char temp = *p;
			if (temp <= '9' && temp >= '0') {
				temp -= '0';
			} else if (temp <= 'f' && temp >= 'a') {
				temp -= 'a';
				temp += 10;
			} else if (temp <= 'F' && temp >= 'A') {
				temp -= 'A';
				temp += 10;
			} else {	
				break;
			}
			value <<= 4;
			value |= temp;
		}
		p ++;
	}

	mac[i] = value;

	return 0;
}


// ARP回复函数 echo_arp_pkt：

// 这个函数用于生成一个ARP回应数据包。
void echo_arp_pkt(struct arppkt *arp, struct arppkt *arp_rt, char *mac) {

	memcpy(arp_rt, arp, sizeof(struct arppkt));

	memcpy(arp_rt->eh.h_dst, arp->eh.h_src, ETH_ADDR_LENGTH);
	str2mac(arp_rt->eh.h_src, mac);
	arp_rt->eh.h_proto = arp->eh.h_proto;

	arp_rt->arp.h_addrlen = 6;
	arp_rt->arp.h_protolen = 4;
	arp_rt->arp.oper = htons(2);
	
	str2mac(arp_rt->arp.smac, mac);
	arp_rt->arp.sip = arp->arp.dip;
	
	memcpy(arp_rt->arp.dmac, arp->arp.smac, ETH_ADDR_LENGTH);
	arp_rt->arp.dip = arp->arp.sip;

}

//主函数 main：

// 使用 netmap 打开网络接口 eth0。
// 设置 pollfd 结构体并开始循环，监听网络接口上的数据。
// 使用 poll 系统调用等待数据包到来。
// 一旦收到数据包，根据以太网帧头中的协议字段来判定数据包类型并进行相应处理。
// 对于 IP 数据包，进一步判断具体的协议（UDP、TCP或ICMP）并处理。
// 对于 ARP 请求，如果请求的 IP 地址与指定的 IP 地址匹配，则调用 echo_arp_pkt 函数生成 ARP 响应。
// 使用 nm_inject 函数发送生成的响应数据包。
int main() {

	struct nm_pkthdr h;
	struct nm_desc *nmr = nm_open("netmap:eth0", NULL, 0, NULL);
	if (nmr == NULL) return -1;

	struct pollfd pfd = {0};
	pfd.fd = nmr->fd;
	pfd.events = POLLIN;

	struct ntcb tcb;

	while (1) {

		int ret = poll(&pfd, 1, -1);
		if (ret < 0) continue;

		if (pfd.revents & POLLIN) {

			unsigned char *stream = nm_nextpkt(nmr, &h);

			struct ethhdr *eh = (struct ethhdr *)stream;
			if (ntohs(eh->h_proto) ==  PROTO_IP) {

				struct udppkt *udp = (struct udppkt *)stream;

				if (udp->ip.type == PROTO_UDP) { //

					int udplength = ntohs(udp->udp.length);

					udp->data[udplength-8] = '\0';

					printf("udp --> %s\n", udp->data);

				} else if (udp->ip.type == PROTO_ICMP) { //

					

				} else if (udp->ip.type == PROTO_TCP) {

					struct tcppkt *tcp = (struct tcppkt *)stream;
/*
					unsigned int sip = tcp->ip.sip;
					unsigned int dip = tcp->ip.dip;

					unsigned short sport = tcp->tcp.sport;
					unsigned short dport = tcp->tcp.dport;

					tcb = search_tcb();
				*/	
					if (tcb->status == TCP_STATUS_LISTEN) { //
						
						if (tcp->tcp.flag & TCP_SYN_FLAG) {

							tcb->status = TCP_STATUS_SYN_REVD;

							// send syn, ack pkt
							// seqnum, ack 

							

						} 
						
					} else if (tcb->status == TCP_STATUS_SYN_REVD) {

						if (tcp->tcp.flag & TCP_ACK_FLAG) {

							tcb->status = TCP_STATUS_ESTABLISHED;

						}
						
					}

				}
				

			} else if (ntohs(eh->h_proto) ==  PROTO_ARP) {

				struct arppkt *arp = (struct arppkt *)stream;

				struct arppkt arp_rt;

				if (arp->arp.dip == inet_addr("192.168.0.123")) { //

					echo_arp_pkt(arp, &arp_rt, "00:50:56:33:1c:ca");

					nm_inject(nmr, &arp_rt, sizeof(arp_rt));

					printf("arp ret\n");
				
				}

			}

		}

	}
	
	

}





// 这段代码主要展现了如何使用 netmap 库来捕获和处理数据包。但是它不包括完整的错误处理和资源管理。例如，没有释放 netmap 资源，也没有完整处理 TCP 连接建立和断开的过程。如果要用于生产环境，代码需要做进一步完善。此外，代码中的硬编码值（比如 IP 地址 "192.168.0.123" 和 MAC 地址 "00:50:56:33:1c:ca"）应该根据实际情况进行替换。

