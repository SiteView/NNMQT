#ifndef TELNETREADER_H
#define TELNETREADER_H
#pragma once

//#include "SvLog.h"
#include "stdafx.h"
#include "scanutils.h"




using namespace std;

//telnet读数结果的数据结构
typedef struct {
    char valueStr1[256];//port,infindex
    char valueStr2[256];//mac
    char valueStr3[256];//ip  (对AFT无效)
    char valueStr4[256];
}AftData;

#define TELNET_REC_COUNT 512

class TelnetReader
{
public:
	TelnetReader();
public:
	~TelnetReader(void);

public:
        map<string, list<string> > getAftData(const string & ips);
        map<string, list<pair<string,string> > > getArpData(const string & ips);
	
	//added by zhangyan 2009-01-13
	list<string> telnetIPList_Arp;
	list<string> telnetIPList_Aft;

private:
	// 获取注册表的值
	string getRegValue();
	string path; //added by zhangyan 2009-01-13
	void GetTelnetIPS();//added by zhangyan 2009-01-13

	HMODULE DeviceDll;
	
	//ip地址, 数组长度(in-分配的长度,out-实际的记录数), 数据结果
	//return - success:记录数, fail:-1
	typedef int (GetAftByTelnet)(const char *, int &, AftData*);
	//ip地址, 数组长度(in-分配的长度,out-实际的记录数), 数据结果
	//return - success:记录数, fail:-1
	typedef int (GetArpByTelnet)(const char *, int &, AftData*);
	//sysoid, factory, model, typeName, shortType(设备类型)
	//设备类型：0-三层交换，1-二层交换，2-路由器，3-防火墙，4-服务器，5-PC，6-其他
	//return - success: >=0, fail:< 0
	typedef int (GetTypeByOid)(const char * , char *, char *, char *, char *);

	GetAftByTelnet* getAftByTelnet;
	GetArpByTelnet* getArpByTelnet;
	//GetTypeByOid* getTypeByOid;
};
#endif