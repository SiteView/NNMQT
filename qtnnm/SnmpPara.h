#ifndef SNMPPARA_H
#define SNMPPARA_H
#pragma once

#include <string>
//#include "CommonDef.h" //added by zhangyan 2009-01-05

using namespace std;

class SnmpPara
{
public:
	string ip;
	string community;
	int timeout;
	int retry;
	string snmpver;//added by zhangyan 2009-01-05

public:
	SnmpPara(void);
	SnmpPara(const string& ip, const string& community, int timeout=300, int retry=3, string snmpver="0");
public:
	~SnmpPara(void);
};
#endif
