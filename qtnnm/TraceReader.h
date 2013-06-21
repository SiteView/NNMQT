#ifndef TRACEREADER_H
#define TRACEREADER_H

#pragma once


#include "commondef.h"
#include "SvPing.h"

// add by zhangyan 2008-08-28
#define   UDP_MAX_PACKET   1024  
#define   UDP_MAX_MSG   512  
#define   UDP_MSG_LEN   2  
#define   SRC_PORT   43995  
#define   DST_PORT   43995  
#define   MAX_HOSTNAME_LAN 255 //最大主机名长度


//UDP 包头
struct UDPHeader
{
	u_short sport;			// Source port
	u_short dport;			// Destination port
	u_short len;			// Datagram length
	u_short crc;			// Checksum
};


class TraceReader : public SvPing
{
public:
	TraceReader(unsigned int u_retrys, unsigned int u_timeout, unsigned int u_Hops);
	TraceReader(const DEVID_LIST& devlist, const BGP_LIST& bgp_list, unsigned int u_retrys, unsigned int u_timeout, unsigned int u_Hops);
public:
	~TraceReader(void);

public:
	ROUTEPATH_LIST& getTraceRouteByIPs();
	//ROUTEPATH_LIST& getTraceRoute(void);
	list<string> unManagedDevices;//不可网管设备IP列表
        list<pair<pair<string, string>, string> > RouteDESTIPPairList;//[<<localip,localport>, peerip>]
	void TracePrepare(void);

private:
	int TraceRouteICMP(const char * cHost, int ttl);
	//发送UDP包进行路由跟踪
	int TraceRouteUDP(const char * cHost, int ttl);
	//获得一次完整的路由跟踪路径
	list<string> getOneRoutePath(const string& ip);
	void getOneTraceRouteData(const string& ip);	

private:
	DEVID_LIST devid_list;
	BGP_LIST bgp_list;
	unsigned int retrys;
	unsigned int timeout;
	unsigned int Hops;
	
	//路由跟踪路径列表
	ROUTEPATH_LIST traceRoute_list;//add by zhangyan 2008-08-25
	list<string> oneRoutePath;
};

#endif