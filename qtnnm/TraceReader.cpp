#include "stdafx.h"
#include "TraceReader.h"
//#include "svlog.h"



// add by zhangyan 2008-08-25
//获取本机IP地址
char * GetLocalIP(SOCKET &SockRaw)
{
	char name[MAX_HOSTNAME_LAN];
	int iErrorCode;
	struct hostent * pHostent;	
	iErrorCode = gethostname(name, MAX_HOSTNAME_LAN);
	if(iErrorCode==SOCKET_ERROR)
	{
		printf("gethostname Error:%d\n",  GetLastError());
		closesocket(SockRaw);
		exit(0);
	}
	
	pHostent = (struct hostent * )malloc(sizeof(struct hostent));
	pHostent = gethostbyname(name);
	return inet_ntoa(*(struct in_addr*)pHostent->h_addr_list[0]);
}

TraceReader::TraceReader(unsigned int u_retrys, unsigned int u_timeout, unsigned int u_Hops)
{
	retrys = u_retrys;
	timeout = u_timeout;
	Hops = u_Hops;	
}

TraceReader::TraceReader(const DEVID_LIST& devlist, const BGP_LIST& bgpdata_list, unsigned int u_retrys, unsigned int u_timeout, unsigned int u_Hops)
{
	devid_list = devlist;
	bgp_list = bgpdata_list;
	retrys = u_retrys;
	timeout = u_timeout;
	Hops = u_Hops;	
}

TraceReader::~TraceReader(void)
{
}

void TraceReader::TracePrepare(void)
{	
	RouteDESTIPPairList.clear();
	unManagedDevices.clear();
	// 获得trace目的设备	
	for (BGP_LIST::iterator i = bgp_list.begin(); i != bgp_list.end(); ++i)
	{		
		bool bOK = true;
		for (DEVID_LIST::iterator j = devid_list.begin(); j != devid_list.end(); ++j)
		{			
			if (find(j->second.ips.begin(), j->second.ips.end(), i->peer_ip) != j->second.ips.end() && (j->second.devType != "5"))
			{					
				bOK = false;
				break;
			}			
		}
		if (bOK)
		{//对端为不可管理设备时
			string local_ip = i->local_ip;
			if (devid_list.find(local_ip) == devid_list.end())
			{
				for (DEVID_LIST::iterator j = devid_list.begin(); j != devid_list.end(); ++j)
				{			
					if (find(j->second.ips.begin(), j->second.ips.end(), local_ip) != j->second.ips.end())
					{					
						local_ip = j->first;
						break;
					}
				}
			}
			RouteDESTIPPairList.push_back(make_pair(make_pair(local_ip, i->local_port), i->peer_ip));			
			if (find(unManagedDevices.begin(), unManagedDevices.end(), i->peer_ip) == unManagedDevices.end())
			{
				unManagedDevices.push_back(i->peer_ip);				
			}	
		}	
	}
}

// add by zhangyan 2008-08-25
//发送ICMP包进行路由跟踪
int TraceReader::TraceRouteICMP(const char * cHost, int ttl)
{
	//用在发送和接受的ICMP包头中
    int seq_no = 0;
    ICMPHeader* send_buf = 0;
    IPHeader* recv_buf = 0;
    int rtn;
    int iCount = 0;
    int packet_size = DEFAULT_PACKET_SIZE;
    
    //packet_size = max(sizeof(ICMPHeader), min(MAX_PING_DATA_SIZE, (unsigned int)packet_size));

    // 启动 Winsocket
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        //winsocket版本错误
		//cout<<"winsocket版本错误\n");
        return SYSERROR;
    }

    SOCKET sd; // RAW Socket句柄
    sockaddr_in dest, source;

    // 三个任务(创建sd, 设置ttl, 初试dest的值)
    rtn = setup_for_ping( cHost, ttl, sd, dest, timeout);
    if ( rtn < 0)
    {
		//cout<<"创建 ping 错误\n");
        goto cleanup; //释放资源并退出
    }

	destMachine = dest;
	
    // 为send_buf和recv_buf分配内存
    rtn = allocate_buffers(send_buf, recv_buf, packet_size);
    if ( rtn < 0)
    {
		//cout<<"分配发送和接收缓冲错误\n");
        goto cleanup;
    }

    // 初始化IMCP数据包(type=8,code=0)
    init_ping_packet(send_buf, packet_size, seq_no);

    // 发送ICMP数据包
    rtn = send_ping(sd, dest, send_buf, packet_size);

	//cout<<"rtn: "<<rtn<<endl;	
    if ( rtn >= 0)
    {        		
        while(1)
        {
            // 接受回应包
            rtn = recv_ping(sd, source, recv_buf, MAX_PING_PACKET_SIZE);
            if ( rtn <= 0)
            {				
                if( ++iCount < retrys)
				{
					// (在重试次数内)发送ICMP数据包
					send_ping(sd, dest, send_buf, packet_size);					
					continue;
				}
				else
				{			
					//cout<<"超过重试次数\n");
					oneRoutePath.push_back("*");
					goto cleanup;
				}
                
            }
					
			char szSourceIP[16];				
                        strncpy(szSourceIP, inet_ntoa(source.sin_addr), 16);
			//cout<<szSourceIP<<endl;
			//{
			//	mutex::scoped_lock lock(m_tracert_mutex);
			//	oneRoutePath.push_back(szSourceIP);
			//}	
			oneRoutePath.push_back(szSourceIP);
			if (strcmp(szSourceIP, cHost) == 0)
			{
				//路由跟踪完成
				rtn = 11;
			}
		
            //if( GetTickCount() - send_buf->timestamp >= iTimeOut )
			//	rtn = OVERTIME;

			goto cleanup;
        }
   }

cleanup:
    delete[]send_buf;	//释放分配的内存
    delete[]recv_buf;
    WSACleanup();		// 清理winsock

    return rtn;
}

// add by zhangyan 2008-08-25
//发送UDP包进行路由跟踪
int TraceReader::TraceRouteUDP(const char * cHost, int ttl)  
{  	
	WSADATA  wsd;  
	SOCKET  s;  
	BOOL  bOpt;  
	struct sockaddr_in  remote, source;   
	IPHeader   ipHdr, *recv_buf = 0;  
	UDPHeader  udpHdr;  	
	DWORD  i;
	int rtn = 0, iCount = 0;
	unsigned short iTotalSize, iUdpSize, iUdpChecksumSize, iIPVersion, iIPSize, cksum = 0; 
	char buf[UDP_MAX_PACKET], *ptr = NULL;  

	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)  //初始化SOCKET 版本2.2
	{  
		//cout<<"初始化SOCKET失败\n");  
		return SYSERROR;  
	}  
	s = WSASocket(AF_INET, SOCK_RAW, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);  //WSA_FLAG_OVERLAPPED用于设置超时
	if (s == INVALID_SOCKET)  
	{  
		//cout<<"创建套接字失败\n");  
		return SYSERROR;  
	}  
	//设置IP_HDRINCL选项为TRUE由程序处理IP头
	bOpt = TRUE;  
	if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, (char *)&bOpt, sizeof(bOpt)) == SOCKET_ERROR)  
	{  
		//cout<<"设置套接字属性 IP_HDRINCL 失败\n");  
		return SYSERROR;  
	}  

	//设置套接字属性 "TTL"错误
	if (setsockopt(s, IPPROTO_IP, IP_TTL, (const char*)&ttl, sizeof(ttl)) == SOCKET_ERROR)
    {        
		//cout<<"设置套接字属性 TTL 错误\n");
        return SYSERROR;
    }

    //设置接收超时限制
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) != 0)
    {
		//cout<<"设置接收超时失败\n");
        return SYSERROR;
    }

	recv_buf = (IPHeader*)new char[UDP_MAX_PACKET];
	if (recv_buf == 0)
	{
		//cout<<"分配接收内存失败\n");
	}
	
	char strMessage[UDP_MSG_LEN] = {0x02,0x01}; 
	string localIP = GetLocalIP(s);

	iTotalSize = sizeof(ipHdr) + sizeof(udpHdr) + UDP_MSG_LEN; //IP包大小
	
	iIPVersion = 4;  
	iIPSize = sizeof(ipHdr) / sizeof(unsigned long);  //5
	//设置IP包头
	ipHdr.version = iIPVersion;  
	ipHdr.h_len = iIPSize;  
	ipHdr.tos = 0;    
	ipHdr.total_len = htons(iTotalSize);    
	ipHdr.ident = 0;    
	ipHdr.flags = 0;    
	ipHdr.ttl = ttl;    
	ipHdr.proto = IPPROTO_UDP;    
	ipHdr.checksum = 0;    
	ipHdr.source_ip = inet_addr(localIP.c_str());	  
	ipHdr.dest_ip = inet_addr(cHost);    

	iUdpSize = sizeof(udpHdr) + UDP_MSG_LEN;  //UDP包大小
	//设置UDP包头
	udpHdr.sport = htons(SRC_PORT);  
	udpHdr.dport = htons(DST_PORT);  
	udpHdr.len = htons(iUdpSize);  
	udpHdr.crc = 0;  
	
	//计算UDP校验码
	iUdpChecksumSize = 0;  
	ptr = buf;  
	ZeroMemory(buf, UDP_MAX_PACKET);  

	memcpy(ptr, &ipHdr.source_ip, sizeof(ipHdr.source_ip));    
	ptr += sizeof(ipHdr.source_ip);  
	iUdpChecksumSize += sizeof(ipHdr.source_ip);  

	memcpy(ptr, &ipHdr.dest_ip, sizeof(ipHdr.dest_ip));    
	ptr += sizeof(ipHdr.dest_ip);  
	iUdpChecksumSize += sizeof(ipHdr.dest_ip);  

	ptr++;  
	iUdpChecksumSize += 1;  

	memcpy(ptr, &ipHdr.proto, sizeof(ipHdr.proto));    
	ptr += sizeof(ipHdr.proto);  
	iUdpChecksumSize += sizeof(ipHdr.proto);  

	memcpy(ptr, &udpHdr.len, sizeof(udpHdr.len));    
	ptr += sizeof(udpHdr.len);  
	iUdpChecksumSize += sizeof(udpHdr.len);  

	memcpy(ptr, &udpHdr, sizeof(udpHdr));    
	ptr += sizeof(udpHdr);  
	iUdpChecksumSize += sizeof(udpHdr);  

	for(i = 0; i < UDP_MSG_LEN; i++, ptr++)
	{
		*ptr = strMessage[i];  
	}
	iUdpChecksumSize += UDP_MSG_LEN;  

	//UDP校验和
	cksum = ip_checksum((USHORT *)buf, iUdpChecksumSize);  
	udpHdr.crc = cksum;  

	ZeroMemory(buf, UDP_MAX_PACKET);  
	ptr = buf;  

	memcpy(ptr, &ipHdr, sizeof(ipHdr));   
	ptr += sizeof(ipHdr);  
	memcpy(ptr, &udpHdr, sizeof(udpHdr));   
	ptr += sizeof(udpHdr);  
	memcpy(ptr, strMessage, UDP_MSG_LEN);  

	remote.sin_family =  AF_INET;  
	remote.sin_port = htons(DST_PORT);  
	remote.sin_addr.s_addr   =   inet_addr(cHost);  
	
	//创建socket接收回应ICMP包
    SOCKET sd = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (sd == INVALID_SOCKET)
    {       
		//cout<<"创建原始套接字失败\n");
        return SYSERROR;
    }

	//设置套接字属性 "TTL"错误
    if (setsockopt( sd,IPPROTO_IP,IP_TTL, (const char*)&ttl, sizeof(ttl)) == SOCKET_ERROR)
    {        
		//cout<<"设置套接字属性 TTL 错误\n");
        return SYSERROR;
    }

    //设置超时限制
    int retern = setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    if( retern != 0)
    {
		//cout<<"设置超时失败\n");
        return SYSERROR;
    }

	source.sin_family = AF_INET;
    source.sin_port = htons(0);//监听端口 
    source.sin_addr.s_addr = inet_addr(localIP.c_str()); //监听主机

    //如果不bind就无法接收包了(因为上面已经创建一个UDPSOCK)
    bind(sd, (struct sockaddr*)&source, sizeof(source));

	//发送UDP数据包
	if (sendto(s, buf, iTotalSize, 0, (SOCKADDR *)&remote, sizeof(remote)) == SOCKET_ERROR)  
	{
		//cout<<"发送UDP包失败\n"); 
	}
	else    
	{	
		while(1)
        {
            //接受回应ICMP包
			int fromlen = sizeof(source);			
			int bread = recvfrom(sd, (char*)recv_buf, MAX_PING_PACKET_SIZE, 0, (sockaddr*)&source, &fromlen);			
			//接收出错处理
			if (bread == SOCKET_ERROR)
			{				
				int  iErr = WSAGetLastError();
				if( iErr == WSAETIMEDOUT )
				{
					//cout<<"接收出错：超时"<<endl;	
					if( ++iCount < retrys)
					{		
						// (在重试次数内)发送UDP数据包
						//sendto(s, buf, iTotalSize, 0, (SOCKADDR *)&remote, sizeof(remote));		
						continue;
					}
					else
					{						
						//cout<<"超时:超过重试次数"<<endl;			
						oneRoutePath.push_back("*");
						break;
					}
				}
				else
				{					
					break;
				}                
			}            
			
			char szSourceIP[16];				
                        strncpy(szSourceIP, inet_ntoa(source.sin_addr), 16);
			oneRoutePath.push_back(szSourceIP);			
			if (strcmp(szSourceIP, cHost) == 0)
			{
				//路由跟踪完成
				rtn = 11;
			}
            //if( GetTickCount() - udphdr->timestamp >= iTimeOut )
			//	rtn = OVERTIME;
			break;
        }
	}
	closesocket(s);
	delete[]recv_buf;
	WSACleanup();  

	return  rtn;  
}   


// add by zhangyan 2008-08-25
//获得一次完整的路由跟踪路径
list<string> TraceReader::getOneRoutePath(const string& ip)
{
	//cout<<"ip:"<<ip.c_str()<<endl;	
	oneRoutePath.clear();	
	for(int i=1; i<=Hops; i++) 
	{
		//int rtn = TraceRouteUDP(ip.c_str(), i);
		int rtn = TraceRouteICMP(ip.c_str(), i);
		if (rtn == 11)
		{
			//cout<<"Trace complete\n";						
			break;
		}		
	}
	return oneRoutePath;
}


ROUTEPATH_LIST& TraceReader::getTraceRouteByIPs()
{
        //SvLog::writeLog("Do Trace Route.");
	traceRoute_list.clear();	
	//暂时未用多线程
        for(list<pair<pair<string, string>, string> >::const_iterator i = RouteDESTIPPairList.begin(); i != RouteDESTIPPairList.end(); i++)
	{
		//cout<<"i->first.first:"<<i->first.first<<endl;
		getOneTraceRouteData(i->first.first);	
	}	
	return traceRoute_list;
}

// 获取设备的trace route信息
// add by zhangyan 2008-08-25
// list[[ips]]
void TraceReader::getOneTraceRouteData(const std::string& ip)
{
	//读取一条traceroute路径
	list<string> onePath = getOneRoutePath(ip);	
	if (onePath.size() > 0)
	{
		traceRoute_list.push_back(onePath);
	}
}

//用法：
//TraceReader traceR = new TraceReader(retrys, timeout, hops, devid, bgp);
//ROUTEPATH_LIST rtpath_list = traceR.getTraceRoute();
//ROUTEPATH_LIST& TraceReader::getTraceRoute(void)
//{
//	//if(myParam.scan_type == "0")
//	//{
//	//	//对路由目的ip尝试路由跟踪
//	//	siReader->getTraceRouteByIPs(RouteDESTIPPairList, 30);//以RouteDESTIPPairList的local_ip作为路由目的ip
//	//	ROUTEPATH_LIST RoutePathList = devReader->getTraceRoute();
//	//	rtpath_list.assign(RoutePathList.begin(), RoutePathList.end());//得到路由跟踪路径表
//	//}
//	return traceRoute_list;
//}	
