#ifndef PINGFIND_H
#define PINGFIND_H
#pragma once
#include "stdafx.h"
//#include <boost/thread/mutex.hpp>
//#include <boost/shared_ptr.hpp>
//
//#include "threadpool.hpp"

using namespace std;
//using namespace boost;
//using namespace boost::threadpool;

class PingFind
{
private:
	unsigned int m_thrdAmount;
	list<string> aliveIp_list;
        //pool* tp;
//	boost::shared_ptr<pool> tp;

public:
        list<pair<string,pair<string,int> > > snmpAliveIp_list;
        //boost::mutex m_ping_mutex;
	PingFind(unsigned int thrdamount);
	~PingFind(void);
	// 单线程Ping
	void singlePing(const string ip, const unsigned int retrys, const unsigned int timeout);
	// 多线程PING
        bool multiPing(const vector<string>& iplist, const unsigned int retrys, const unsigned int timeout);
	//add by zhangyan 2008-10-09
	void singlePing_simple(const string ip, const unsigned int retrys, const unsigned int timeout);	
	void multiPing_simple(const vector<string>& iplist, const unsigned int retrys, const unsigned int timeout);

	// 单线程Ping
	void singlePing_SNMP(const string& ip, const string& community, const int retrys, const int timeout);
	// 多线程PING
        void multiPing_SNMP(const list<pair<string,string> >& iplist, const int retrys, const int timeout);

	// 获取活动的IP地址列表
	list<string>& getAliveIPList(void);
	// 获取snmp有效的IP地址列表[ip,cmty,ver]
        list<pair<string,pair<string,int> > >& getSnmpAliveIPList(void);


};

#endif
