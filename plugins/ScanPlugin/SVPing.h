//---------------------------------------------------------------------------
/*
* 程序名: SvPing.h
* 说明:
* 主要函数库头文件
*/
#ifndef SVPPING_H
#define SVPPING_H
//---------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <string>
#include <list>
//#include <boost/thread/mutex.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ws2tcpip.h>
#include <iostream>


using namespace std;

// ICMP 包类型, 具体参见本文的第一节
#define ICMP_ECHO_REPLY 0 
#define ICMP_DEST_UNREACH 3
#define ICMP_TTL_EXPIRE 11
#define ICMP_ECHO_REQUEST 8

#define DEFAULT_PACKET_SIZE 32                                          // 默认ICMP包字节数
#define DEFAULT_TTL 30                                                  // 默认TTL值
#define MAX_PING_DATA_SIZE 1024                                         // 最大数据块
#define MAX_PING_PACKET_SIZE (MAX_PING_DATA_SIZE + sizeof(IPHeader))    //最大ICMP包长度
#define OVERTIME    -2
#define SYSERROR    -1
#define OTHERERR    -3
#define SENDERR     -4
#define RECVERR     -5
#define NETERROR    -6

#define ICMP_MIN 8                  // 最小的ICMP包大小

#pragma comment(lib,"Ws2_32.lib")

// IP 包头结构
struct IPHeader {
    BYTE h_len:4;       // IP头长度
    BYTE version:4;     // IP版本
    BYTE tos;           // 服务类型
    USHORT total_len;   // 数据报长度
    USHORT ident;       // ID
    USHORT flags;       // Flags
    BYTE ttl;           // 生存时间,
    BYTE proto;         // 协议(TCP, UDP etc)
    USHORT checksum;    // IP 校验和
    ULONG source_ip;
    ULONG dest_ip;
};

// ICMP 包头（实际的包不包括timestamp字段,这里用来记录时间）
struct ICMPHeader {
    BYTE type;          // ICMP 类型
    BYTE code;          // 代码
    USHORT checksum;    //校验和
    USHORT id;          //id
    USHORT seq;         //序号
    ULONG timestamp;    //记录时间
};

//伪UDP 包头（用于计算UDP校验和）
struct PSHeader
{
	u_short srcaddr;
	u_short destaddr;

	u_short zero;
	u_short protocol;
	u_short len;
}; 

class SvPing
{
public:
	SvPing();
	~SvPing();

	int Ping(const char * cHost ,unsigned int iTimeOut,  int iReTry); 
	char *GetError( int iErr );
	
protected:
	int COUNT;
	sockaddr_in destMachine;

	int setup_for_ping(const char* host, int ttl, SOCKET& sd, sockaddr_in& dest, int iTimeOut);
	int allocate_buffers(ICMPHeader*& send_buf, IPHeader*& recv_buf,int packet_size);
	USHORT ip_checksum(USHORT* buffer, int size);
	void init_ping_packet(ICMPHeader* icmp_hdr, int packet_size, int seq_no);
	int send_ping(SOCKET sd, const sockaddr_in& dest, ICMPHeader* send_buf,int packet_size);
	int recv_ping(SOCKET sd, sockaddr_in& source, IPHeader* recv_buf,int packet_size);
	int decode_reply(IPHeader* reply, int bytes, sockaddr_in* from);


};

#endif
