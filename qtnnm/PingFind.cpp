#include "StdAfx.h"
#include <QtDebug>
#include "svping.h"
#include "SnmpDG.h"
#include "snmpping.h"

#include "PingFind.h"


//#include <boost/thread.hpp>
//#include <boost/function.hpp>
//#include <boost/bind.hpp>
//#include <boost/lexical_cast.hpp>
#include "CommonDef.h"

//extern bool isStop;


#include <iostream>

PingFind::PingFind(unsigned int thrdamount)
{
	m_thrdAmount = thrdamount;
        //tp = new pool(m_thrdAmount);
}

PingFind::~PingFind(void)
{
        //delete tp;
}

// 单线程Ping
void PingFind::singlePing(const string ip, const unsigned int retrys, const unsigned int timeout)
{
	SvPing svp = SvPing();
	if(svp.Ping(ip.c_str(), timeout, retrys) > 0)
	{
                qDebug() << "aliveip" << ip.c_str();
                //mutex::scoped_lock lock(m_ping_mutex);
		aliveIp_list.push_back(ip);
	}
        else
        {
            qDebug() << "ping failed";
        }
}

//add by zhangyan 2008-10-09
void PingFind::singlePing_simple(const string ip, const unsigned int retrys, const unsigned int timeout)
{
	SvPing svp = SvPing();
	svp.Ping(ip.c_str(), timeout, retrys);
}

// 多线程PING
bool PingFind::multiPing(const vector<string>& iplist, const unsigned int retrys, const unsigned int timeout)
{
	aliveIp_list.clear();
	if(iplist.empty())
	{
                return true;
	}
	for(vector<string>::const_iterator i = iplist.begin(); i != iplist.end(); i++) 
	{
                //if (isStop)
                //{
                //    return false;
                //}
                //else
                //{
                //    //tp->schedule((boost::bind(&PingFind::singlePing, this, *i, retrys, timeout)));
                //}
	}
//        tp->wait();
        return true;
}

//add by zhangyan 2008-10-09
void PingFind::multiPing_simple(const vector<string>& iplist, const unsigned int retrys, const unsigned int timeout)
{	
	if(iplist.empty())
	{
		return;
	}
	for(vector<string>::const_iterator i = iplist.begin(); i != iplist.end(); i++) 
	{
                //tp->schedule((boost::bind(&PingFind::singlePing_simple, this, *i, retrys, timeout)));
	}
        //tp->wait();
}

// 单线程Ping
void PingFind::singlePing_SNMP(const string& ip, const string& community, const int retrys, const int timeout)
{
	SnmpPing sp;
	if(sp.Ping(2, ip, community, retrys, timeout))
	{
                //mutex::scoped_lock lock(m_ping_mutex);
		snmpAliveIp_list.push_back(make_pair(ip,make_pair(community,2)));
		return;
	}
	if(sp.Ping(1, ip, community, retrys, timeout))
	{
                //mutex::scoped_lock lock(m_ping_mutex);
		snmpAliveIp_list.push_back(make_pair(ip,make_pair(community,1)));
	}
}

// 多线程PING
void PingFind::multiPing_SNMP(const list<pair<string,string> >& iplist, const int retrys, const  int timeout)
{
	snmpAliveIp_list.clear();
	if(iplist.empty())
	{
		return;
	}
        for (list<pair<string,string> >::const_iterator i = iplist.begin();
		i != iplist.end(); 
		i++) 
	{
                //tp->schedule((boost::bind(&PingFind::singlePing_SNMP, this, i->first, i->second, retrys, timeout)));
	}
        //tp->wait();
}


// 获取活动的IP地址列表
list<string>& PingFind::getAliveIPList(void)
{
	return aliveIp_list;
}

// 获取snmp有效的IP地址列表
list<pair<string,pair<string,int> > >& PingFind::getSnmpAliveIPList(void)
{
	return snmpAliveIp_list;
}
