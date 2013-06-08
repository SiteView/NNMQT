/*
 * Topo scan console program
 *
 * Copyright (c) 2007-2008 DragonFlow
 *
 * Created date: 2007-11-28
 *
 * Autor: Tang guofu 
 * 
 * last modified 2007-11-28 by Tang guofu 
 *
 */

#pragma once

#include <map>
#include <iostream>
#include <string>
#include <list>

#include "device.h"
#include "SnmpDG.h"
//#include "svlog.h"
#include "scanutils.h"

using namespace std;

//to do
typedef struct
{
	string i;
}APPENDDATA;

class UnivDevice : public Device
{
	typedef void result_type; 

protected:
        map<string, list<pair<string,string> > > inf_macs; //{inf,[(ip,mac)]}
        map<string, list<string> > port_macs; //{port,[mac]}
	//设备AFT数据列表{sourceIP,[port,[MAC]]}
	AFT_LIST aft_list;
	//设备的OSPF邻居表{sourceIP,{infInx,[destIP]}}
	OSPFNBR_LIST ospfnbr_list;
	//设备的路由表{sourceIP,{infInx,[nextIP]}}
	ROUTE_LIST route_list;
	BGP_LIST bgp_list;

	//VRRP_LIST vrrp_list;
	RouterStandby_LIST routeStandby_list;//added by zhangyan 2009-01-09

	//设备ARP数据列表{sourceIP,[infInx,[(MAC,destIP)]]}
	ARP_LIST arp_list;
	//设备接口属性列表 {devIP,(ifAmount,[(ifindex,ifType,ifDescr,ifMac,portNum,ifSpeed)])}
	IFPROP_LIST ifprop_list;

	DIRECTDATA_LIST directdata_list;

	STP_LIST stp_list;	//add by jiangshanwen 20100910

	// 读取设备转发表
	bool getAftByDtp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList);
	// 通过Bridge共同体获取设备转发表
	void getAftByQtp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList);

private:
	//消息函数
	callback_functor_type m_callback;
	//remarked by zhangyan 2009-01-14
	/*SCAN_PARAM scanPara;
	AUX_PARAM auxPara;*/

public:
	UnivDevice(){}
	//remarked by zhangyan 2009-01-14
	/*UnivDevice(const SCAN_PARAM scanParam, const AUX_PARAM auxParam);
	virtual void Init(const SCAN_PARAM scanParam, const AUX_PARAM auxParam);*/
	~UnivDevice(void);

	virtual IFPROP_LIST& getInfProp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList,bool bRouter=false);
	virtual AFT_LIST& getAftData(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList);
	virtual ARP_LIST& getArpData(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList);
	virtual OSPFNBR_LIST& getOspfNbrData(SnmpDG& snmp, const SnmpPara& spr);	
	virtual ROUTE_LIST& getRouteData(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList);
	virtual BGP_LIST& getBgpData(SnmpDG& snmp, const SnmpPara& spr);
	/*virtual VRRP_LIST& getVrrpData(SnmpDG& snmp, const SnmpPara& spr);*/
	virtual RouterStandby_LIST& getVrrpData(SnmpDG& snmp, const SnmpPara& spr); //by zhangyan
	virtual RouterStandby_LIST& getHsrpData(SnmpDG& snmp, const SnmpPara& spr); // added by zhangyan 2008-12-04
	virtual STP_LIST& getStpData(SnmpDG& snmp, const SnmpPara& spr);	//add by jiangshanwen 20100910

	// 获取特定设备的接口流量
	virtual void getInfFlow(SnmpDG& snmp, const SnmpPara& spr){}

private:
	//设备AFT数据列表{sourceIP,[port,[MAC]]}
        pair<std::string, std::map<std::string, std::list<std::string> > > aft_cur;
	//设备ARP数据列表{sourceIP,[infInx,[(MAC,destIP)]]}
        pair<std::string, std::map<std::string, std::list<std::pair<std::string,std::string> > > > arp_cur;
	//设备接口属性列表 {devIP,(ifAmount,[(ifindex,ifType,ifDescr,ifMac,portNum,ifSpeed)])}
        pair<std::string, std::pair<std::string, std::list<IFREC> > > ifprop_cur;
	//CDP数据列表{ip,(index,peerid,peerip,peerportdesc)}
	pair<std::string, std::list<DIRECTITEM> > directdata_cur;

private:
	// 获取一台设备的附加拓扑信息数据
	DIRECTDATA_LIST getDirectData(SnmpDG& snmp, const SnmpPara& spr);

};
