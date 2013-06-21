#ifndef CISCODEVICE_H
#define CISCODEVICE_H

#pragma once
#include "UnivDevice.h"

class CiscoDevice : virtual	public UnivDevice
{
public:
	CiscoDevice();
public:
	~CiscoDevice(void);
public:
	void getLogicEntity(SnmpDG& snmp, const SnmpPara& spr);// added by zhangyan 2008-12-15

	virtual AFT_LIST& getAftData(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList);
	virtual DIRECTDATA_LIST getDirectData(SnmpDG& snmp, const SnmpPara& spr);
	virtual RouterStandby_LIST& getHsrpData(SnmpDG& snmp, const SnmpPara& spr); // added by zhangyan 2008-12-04
	virtual IFPROP_LIST& getInfProp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList,bool bRouter=false);// added by zhangyan 2008-12-11
	virtual STP_LIST& getStpData(SnmpDG& snmp, const SnmpPara& spr);	//add by jiangshanwen 20100910

private:
	void getAftByLogicEntity(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList);
	list<string> list_cmty;// added by zhangyan 2008-12-15
};

#endif
