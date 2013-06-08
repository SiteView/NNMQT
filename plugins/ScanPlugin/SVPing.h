//---------------------------------------------------------------------------
/*
* ������: SvPing.h
* ˵��:
* ��Ҫ������ͷ�ļ�
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

// ICMP ������, ����μ����ĵĵ�һ��
#define ICMP_ECHO_REPLY 0 
#define ICMP_DEST_UNREACH 3
#define ICMP_TTL_EXPIRE 11
#define ICMP_ECHO_REQUEST 8

#define DEFAULT_PACKET_SIZE 32                                          // Ĭ��ICMP���ֽ���
#define DEFAULT_TTL 30                                                  // Ĭ��TTLֵ
#define MAX_PING_DATA_SIZE 1024                                         // ������ݿ�
#define MAX_PING_PACKET_SIZE (MAX_PING_DATA_SIZE + sizeof(IPHeader))    //���ICMP������
#define OVERTIME    -2
#define SYSERROR    -1
#define OTHERERR    -3
#define SENDERR     -4
#define RECVERR     -5
#define NETERROR    -6

#define ICMP_MIN 8                  // ��С��ICMP����С

#pragma comment(lib,"Ws2_32.lib")

// IP ��ͷ�ṹ
struct IPHeader {
    BYTE h_len:4;       // IPͷ����
    BYTE version:4;     // IP�汾
    BYTE tos;           // ��������
    USHORT total_len;   // ���ݱ�����
    USHORT ident;       // ID
    USHORT flags;       // Flags
    BYTE ttl;           // ����ʱ��,
    BYTE proto;         // Э��(TCP, UDP etc)
    USHORT checksum;    // IP У���
    ULONG source_ip;
    ULONG dest_ip;
};

// ICMP ��ͷ��ʵ�ʵİ�������timestamp�ֶ�,����������¼ʱ�䣩
struct ICMPHeader {
    BYTE type;          // ICMP ����
    BYTE code;          // ����
    USHORT checksum;    //У���
    USHORT id;          //id
    USHORT seq;         //���
    ULONG timestamp;    //��¼ʱ��
};

//αUDP ��ͷ�����ڼ���UDPУ��ͣ�
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
