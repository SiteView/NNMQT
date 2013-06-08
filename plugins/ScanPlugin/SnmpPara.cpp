//#include "StdAfx.h"
#include "SnmpPara.h"

SnmpPara::SnmpPara(void)
{
}

SnmpPara::SnmpPara(const string& ip, const string& community, int timeout, int retry, string snmpver)
{
	this->ip = ip;
	this->community = community;
	this->timeout = timeout;
	this->retry = retry;
	this->snmpver = snmpver; //added by zhangyan 2009-01-06
}

SnmpPara::~SnmpPara(void)
{
}
