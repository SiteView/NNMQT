/*
 *	SvPing.cpp
 */
//---------------------------------------------------------------------------
#pragma hdrstop

#include "StdAfx.h"
#include "SvPing.h"

#include <QtDebug>

//---------------------------------------------------------------------------
//
//// add by zhangyan 2008-08-28
//#define   UDP_MAX_PACKET   1024  
//#define   UDP_MAX_MSG   512  
//#define   UDP_MSG_LEN   2  
////#define   SRC_IP   "192.168.0.93"  
//#define   SRC_PORT   43995  
//#define   DST_PORT   43995  
//#define   MAX_HOSTNAME_LAN 255 //最大主机名长度
//
//using namespace std;
//
//// add by zhangyan 2008-08-25
////获取本机IP地址
//char * GetLocalIP(SOCKET &SockRaw)
//{
//	char name[MAX_HOSTNAME_LAN];
//	int iErrorCode;
//	struct hostent * pHostent;	
//	iErrorCode = gethostname(name, MAX_HOSTNAME_LAN);
//	if(iErrorCode==SOCKET_ERROR)
//	{
//		printf("gethostname Error:%d\n",  GetLastError());
//		closesocket(SockRaw);
//		exit(0);
//	}
//	
//	pHostent = (struct hostent * )malloc(sizeof(struct hostent));
//	pHostent = gethostbyname(name);
//	return inet_ntoa(*(struct in_addr*)pHostent->h_addr_list[0]);
//}
//

SvPing::SvPing()
{
//    WSAData wsaData;
//    if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
//    {
//        //winsocket版本错误
//        qDebug() << "ping failed for version";
//        //return SYSERROR;
//    }
}

SvPing::~SvPing()
{
//    WSACleanup();
}

USHORT SvPing::ip_checksum(USHORT* buffer, int size)
{
    unsigned long cksum = 0;

    while (size > 1) 
	{
        cksum += *buffer++;
        size -= sizeof(USHORT);
    }

    if (size) 
	{
		cksum += *(UCHAR*)buffer;
    }

    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);

    // 返回校验和
    return (USHORT)(~cksum);
}

int SvPing::setup_for_ping(const char* host, int ttl, SOCKET& sd, sockaddr_in& dest, int iTimeOut)
{
    // 创建socket
    sd = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, 0, 0, WSA_FLAG_OVERLAPPED);

    if (sd == INVALID_SOCKET)
    {
        int i = WSAGetLastError();
        qDebug() << "create socked failed : " << i;
        if (WSA_NOT_ENOUGH_MEMORY == i)
        {

        }

        return SYSERROR;
    }

    if (setsockopt( sd,IPPROTO_IP,IP_TTL, (const char*)&ttl, sizeof(ttl)) == SOCKET_ERROR)
    {
        qDebug() << "set sock opt failed";
        return SYSERROR;
    }

    //设置超时限制
    int rtn = setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeOut, sizeof(iTimeOut));
    if( rtn != 0)
    {
        qDebug() << "set sock opt timeout failed : " << rtn;
        return SYSERROR;
    }
    // 初始化主机地址
    memset(&dest, 0, sizeof(dest));

    // 转换IP地址
    unsigned int addr = inet_addr(host);
    if (addr != INADDR_NONE)
    {
        // 填入主机地址
        dest.sin_addr.s_addr = addr;
        dest.sin_family = AF_INET;
    }
    else
    {
        // 获取主机信息
        hostent* hp = gethostbyname(host);
        if (hp != 0)
        {
            // 获得主机地址
            memcpy(&(dest.sin_addr), hp->h_addr, hp->h_length);
            dest.sin_family = hp->h_addrtype;
        }
        else
        {
            // 出错
            qDebug() << "get host by name failed";
            return SYSERROR;
        }
    }
    return 0;
}

void SvPing::init_ping_packet(ICMPHeader* icmp_hdr, int packet_size, int seq_no)
{
    // 设置ICMP包头
    icmp_hdr->type = ICMP_ECHO_REQUEST;
    icmp_hdr->code = 0;
    icmp_hdr->checksum = 0;
    icmp_hdr->id = (USHORT)GetCurrentProcessId();
    icmp_hdr->seq = seq_no;
    icmp_hdr->timestamp = GetTickCount();

    //填充DATA数据部
    const unsigned long int deadmeat = 0xDEADBEEF;
    char* datapart = (char*)icmp_hdr + sizeof(ICMPHeader);
    int bytes_left = packet_size - sizeof(ICMPHeader);
    while (bytes_left > 0)
    {
        memcpy(datapart, &deadmeat, min(int(sizeof(deadmeat)),bytes_left));
        bytes_left -= sizeof(deadmeat);
        datapart += sizeof(deadmeat);
    }

    // 计算校验和
    icmp_hdr->checksum = ip_checksum((USHORT*)icmp_hdr, packet_size);
}

//发送数据
int SvPing::send_ping(SOCKET sd, const sockaddr_in& dest, ICMPHeader* send_buf,int packet_size)
{
    if(sendto(sd, (char*)send_buf, packet_size, 0, (sockaddr*)&dest, sizeof(dest)) == SOCKET_ERROR)
    {
        return SENDERR;
    }
    return 0;
}

int SvPing::recv_ping(SOCKET sd, sockaddr_in& source, IPHeader* recv_buf,int packet_size)
{
    // 接收icmp包
    int fromlen = sizeof(source);
    int bread = recvfrom(sd, (char*)recv_buf, packet_size + sizeof(IPHeader), 0, (sockaddr*)&source, &fromlen);
    //接收出错处理
    if (bread == SOCKET_ERROR)
    {
        int  iErr = WSAGetLastError();
        if (iErr == WSAEMSGSIZE)
        {
            return SYSERROR;
        }
        else  if( iErr == 10060 )
        {
            return OVERTIME;
        }
        return RECVERR;
    }
	return bread;
}

int SvPing::decode_reply(IPHeader* reply, int bytes, sockaddr_in* from)
{
    // 跳过IP包头, 找到ICMP的包头
    unsigned short header_len = reply->h_len * 4;
    ICMPHeader* icmphdr = (ICMPHeader*)((char*)reply + header_len);

    // 包的长度合法, header_len + ICMP_MIN为最小ICMP包的长度
    if (bytes < header_len + ICMP_MIN)
    {//接收到的数据包长度过小
        return RECVERR;
    }
    else if (icmphdr->type != ICMP_ECHO_REPLY)
    {
        if (icmphdr->type != ICMP_TTL_EXPIRE )
        {//ttl减为零 表示 主机不可达或非法的ICMP包类型
            return NETERROR;
        }
    }	
	else if (from->sin_addr.S_un.S_addr != destMachine.sin_addr.S_un.S_addr) 
	{
		return -10;
	}
	return 1;
}

int SvPing::allocate_buffers(ICMPHeader*& send_buf, IPHeader*& recv_buf,int packet_size)
{
    // 发送缓冲区 初始化
    send_buf = (ICMPHeader*)new char[packet_size];
    if (send_buf == 0)
    {
        return SYSERROR;
    }
    // 接收缓冲区 初始化
    recv_buf = (IPHeader*)new char[MAX_PING_PACKET_SIZE];
    if (recv_buf == 0)
    {
        return SYSERROR;
    }
    return 0;
}

int SvPing::Ping(const char * cHost ,unsigned int iTimeOut,  int iReTry)
{
    //用在发送和接受的ICMP包头中
    int seq_no = 0;
    ICMPHeader* send_buf = 0;
    IPHeader* recv_buf = 0;
    int rtn;
    int iCount = 0;
    int packet_size = DEFAULT_PACKET_SIZE;
    int ttl = DEFAULT_TTL;

    packet_size = max(sizeof(ICMPHeader), min((unsigned int)MAX_PING_DATA_SIZE, (unsigned int)packet_size));
    qDebug() << "ping start";
    // 启动 Winsocket
//    WSAData wsaData;
//    if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
//    {
//        //winsocket版本错误
//        qDebug() << "ping failed for version";
//        return SYSERROR;
//    }

    SOCKET sd; // RAW Socket句柄
    sockaddr_in dest, source;

    // 三个任务(创建sd, 设置ttl, 初试dest的值)
    rtn = setup_for_ping( cHost, ttl, sd, dest, iTimeOut);
    if ( rtn < 0)
    {
        //WSACleanup();		// 清理winsock

        return rtn;
//        qDebug() << "ping set failed";
//        goto cleanup; //释放资源并退出
    }

	destMachine = dest;
    // 为send_buf和recv_buf分配内存
//    rtn = allocate_buffers(send_buf, recv_buf, packet_size);
//    if ( rtn < 0)
//    {
//        qDebug() << "relocate failed";
//        goto cleanup;
//    }

        send_buf = (ICMPHeader*)new char[packet_size];
        if (send_buf == 0)
        {
            return SYSERROR;
        }
        // 接收缓冲区 初始化
        recv_buf = (IPHeader*)new char[MAX_PING_PACKET_SIZE];
        if (recv_buf == 0)
        {
            return SYSERROR;
        }

    // 初始化IMCP数据包(type=8,code=0)
    init_ping_packet(send_buf, packet_size, seq_no);

    // 发送ICMP数据包
    rtn = send_ping(sd, dest, send_buf, packet_size);
    if ( rtn >= 0)
    {//用循环来保证接到其他的现成的icmp包时能忽略并继续进行icmp监听
        while(1)
        {
            // 接受回应包
			Sleep(10);
            rtn = recv_ping(sd, source, recv_buf, MAX_PING_PACKET_SIZE);
            if ( rtn <= 0)
            {
                if( iCount > iReTry) goto cleanup;
                iCount++;
                continue;
            }
            //接收成功，返回耗时，或者错误号码
            rtn = decode_reply(recv_buf, packet_size, &source);
			if( rtn == -10 ) // 不是本进程发出的ping包
				continue;
            if( rtn < 0 )
			{
				if( iCount > iReTry) // 解读数据出错
					goto cleanup;
				else
					iCount++;
			}			
            if( GetTickCount() - send_buf->timestamp >= iTimeOut )
			{
				rtn = OVERTIME;
				goto cleanup;
			}
			goto cleanup;
        }
   }

cleanup:
    delete[]send_buf;	//释放分配的内存
    delete[]recv_buf;
    //if (0 != WSACleanup())		// 清理winsock
    //{
    //    qDebug() << "cleanup failed";
    //}

    return rtn;
}


char *SvPing::GetError( int iErr)
{
    switch( iErr )
    {
        case    SYSERROR:
                            return "SYSERROR";
                            break;
        case    NETERROR:
                            return  "NETERROR";
                            break;
        case    OVERTIME:
                            return  "OVERTIME";
                            break;
        case    SENDERR:
                            return  "Send ICMP Packet Error";
                            break;
        case    RECVERR:
                            return  "receive ICMP packet error";
                            break;
        default:
                            return  "OTHERERR";
                            break;
    }
}

