#include "stdafx.h"
#include "TraceReader.h"
//#include "svlog.h"



// add by zhangyan 2008-08-25
//��ȡ����IP��ַ
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
	// ���traceĿ���豸	
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
		{//�Զ�Ϊ���ɹ����豸ʱ
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
//����ICMP������·�ɸ���
int TraceReader::TraceRouteICMP(const char * cHost, int ttl)
{
	//���ڷ��ͺͽ��ܵ�ICMP��ͷ��
    int seq_no = 0;
    ICMPHeader* send_buf = 0;
    IPHeader* recv_buf = 0;
    int rtn;
    int iCount = 0;
    int packet_size = DEFAULT_PACKET_SIZE;
    
    //packet_size = max(sizeof(ICMPHeader), min(MAX_PING_DATA_SIZE, (unsigned int)packet_size));

    // ���� Winsocket
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        //winsocket�汾����
		//cout<<"winsocket�汾����\n");
        return SYSERROR;
    }

    SOCKET sd; // RAW Socket���
    sockaddr_in dest, source;

    // ��������(����sd, ����ttl, ����dest��ֵ)
    rtn = setup_for_ping( cHost, ttl, sd, dest, timeout);
    if ( rtn < 0)
    {
		//cout<<"���� ping ����\n");
        goto cleanup; //�ͷ���Դ���˳�
    }

	destMachine = dest;
	
    // Ϊsend_buf��recv_buf�����ڴ�
    rtn = allocate_buffers(send_buf, recv_buf, packet_size);
    if ( rtn < 0)
    {
		//cout<<"���䷢�ͺͽ��ջ������\n");
        goto cleanup;
    }

    // ��ʼ��IMCP���ݰ�(type=8,code=0)
    init_ping_packet(send_buf, packet_size, seq_no);

    // ����ICMP���ݰ�
    rtn = send_ping(sd, dest, send_buf, packet_size);

	//cout<<"rtn: "<<rtn<<endl;	
    if ( rtn >= 0)
    {        		
        while(1)
        {
            // ���ܻ�Ӧ��
            rtn = recv_ping(sd, source, recv_buf, MAX_PING_PACKET_SIZE);
            if ( rtn <= 0)
            {				
                if( ++iCount < retrys)
				{
					// (�����Դ�����)����ICMP���ݰ�
					send_ping(sd, dest, send_buf, packet_size);					
					continue;
				}
				else
				{			
					//cout<<"�������Դ���\n");
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
				//·�ɸ������
				rtn = 11;
			}
		
            //if( GetTickCount() - send_buf->timestamp >= iTimeOut )
			//	rtn = OVERTIME;

			goto cleanup;
        }
   }

cleanup:
    delete[]send_buf;	//�ͷŷ�����ڴ�
    delete[]recv_buf;
    WSACleanup();		// ����winsock

    return rtn;
}

// add by zhangyan 2008-08-25
//����UDP������·�ɸ���
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

	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)  //��ʼ��SOCKET �汾2.2
	{  
		//cout<<"��ʼ��SOCKETʧ��\n");  
		return SYSERROR;  
	}  
	s = WSASocket(AF_INET, SOCK_RAW, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);  //WSA_FLAG_OVERLAPPED�������ó�ʱ
	if (s == INVALID_SOCKET)  
	{  
		//cout<<"�����׽���ʧ��\n");  
		return SYSERROR;  
	}  
	//����IP_HDRINCLѡ��ΪTRUE�ɳ�����IPͷ
	bOpt = TRUE;  
	if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, (char *)&bOpt, sizeof(bOpt)) == SOCKET_ERROR)  
	{  
		//cout<<"�����׽������� IP_HDRINCL ʧ��\n");  
		return SYSERROR;  
	}  

	//�����׽������� "TTL"����
	if (setsockopt(s, IPPROTO_IP, IP_TTL, (const char*)&ttl, sizeof(ttl)) == SOCKET_ERROR)
    {        
		//cout<<"�����׽������� TTL ����\n");
        return SYSERROR;
    }

    //���ý��ճ�ʱ����
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) != 0)
    {
		//cout<<"���ý��ճ�ʱʧ��\n");
        return SYSERROR;
    }

	recv_buf = (IPHeader*)new char[UDP_MAX_PACKET];
	if (recv_buf == 0)
	{
		//cout<<"��������ڴ�ʧ��\n");
	}
	
	char strMessage[UDP_MSG_LEN] = {0x02,0x01}; 
	string localIP = GetLocalIP(s);

	iTotalSize = sizeof(ipHdr) + sizeof(udpHdr) + UDP_MSG_LEN; //IP����С
	
	iIPVersion = 4;  
	iIPSize = sizeof(ipHdr) / sizeof(unsigned long);  //5
	//����IP��ͷ
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

	iUdpSize = sizeof(udpHdr) + UDP_MSG_LEN;  //UDP����С
	//����UDP��ͷ
	udpHdr.sport = htons(SRC_PORT);  
	udpHdr.dport = htons(DST_PORT);  
	udpHdr.len = htons(iUdpSize);  
	udpHdr.crc = 0;  
	
	//����UDPУ����
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

	//UDPУ���
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
	
	//����socket���ջ�ӦICMP��
    SOCKET sd = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (sd == INVALID_SOCKET)
    {       
		//cout<<"����ԭʼ�׽���ʧ��\n");
        return SYSERROR;
    }

	//�����׽������� "TTL"����
    if (setsockopt( sd,IPPROTO_IP,IP_TTL, (const char*)&ttl, sizeof(ttl)) == SOCKET_ERROR)
    {        
		//cout<<"�����׽������� TTL ����\n");
        return SYSERROR;
    }

    //���ó�ʱ����
    int retern = setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    if( retern != 0)
    {
		//cout<<"���ó�ʱʧ��\n");
        return SYSERROR;
    }

	source.sin_family = AF_INET;
    source.sin_port = htons(0);//�����˿� 
    source.sin_addr.s_addr = inet_addr(localIP.c_str()); //��������

    //�����bind���޷����հ���(��Ϊ�����Ѿ�����һ��UDPSOCK)
    bind(sd, (struct sockaddr*)&source, sizeof(source));

	//����UDP���ݰ�
	if (sendto(s, buf, iTotalSize, 0, (SOCKADDR *)&remote, sizeof(remote)) == SOCKET_ERROR)  
	{
		//cout<<"����UDP��ʧ��\n"); 
	}
	else    
	{	
		while(1)
        {
            //���ܻ�ӦICMP��
			int fromlen = sizeof(source);			
			int bread = recvfrom(sd, (char*)recv_buf, MAX_PING_PACKET_SIZE, 0, (sockaddr*)&source, &fromlen);			
			//���ճ�����
			if (bread == SOCKET_ERROR)
			{				
				int  iErr = WSAGetLastError();
				if( iErr == WSAETIMEDOUT )
				{
					//cout<<"���ճ�����ʱ"<<endl;	
					if( ++iCount < retrys)
					{		
						// (�����Դ�����)����UDP���ݰ�
						//sendto(s, buf, iTotalSize, 0, (SOCKADDR *)&remote, sizeof(remote));		
						continue;
					}
					else
					{						
						//cout<<"��ʱ:�������Դ���"<<endl;			
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
				//·�ɸ������
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
//���һ��������·�ɸ���·��
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
	//��ʱδ�ö��߳�
        for(list<pair<pair<string, string>, string> >::const_iterator i = RouteDESTIPPairList.begin(); i != RouteDESTIPPairList.end(); i++)
	{
		//cout<<"i->first.first:"<<i->first.first<<endl;
		getOneTraceRouteData(i->first.first);	
	}	
	return traceRoute_list;
}

// ��ȡ�豸��trace route��Ϣ
// add by zhangyan 2008-08-25
// list[[ips]]
void TraceReader::getOneTraceRouteData(const std::string& ip)
{
	//��ȡһ��traceroute·��
	list<string> onePath = getOneRoutePath(ip);	
	if (onePath.size() > 0)
	{
		traceRoute_list.push_back(onePath);
	}
}

//�÷���
//TraceReader traceR = new TraceReader(retrys, timeout, hops, devid, bgp);
//ROUTEPATH_LIST rtpath_list = traceR.getTraceRoute();
//ROUTEPATH_LIST& TraceReader::getTraceRoute(void)
//{
//	//if(myParam.scan_type == "0")
//	//{
//	//	//��·��Ŀ��ip����·�ɸ���
//	//	siReader->getTraceRouteByIPs(RouteDESTIPPairList, 30);//��RouteDESTIPPairList��local_ip��Ϊ·��Ŀ��ip
//	//	ROUTEPATH_LIST RoutePathList = devReader->getTraceRoute();
//	//	rtpath_list.assign(RoutePathList.begin(), RoutePathList.end());//�õ�·�ɸ���·����
//	//}
//	return traceRoute_list;
//}	
