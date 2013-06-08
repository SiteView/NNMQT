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
	//�豸AFT�����б�{sourceIP,[port,[MAC]]}
	AFT_LIST aft_list;
	//�豸��OSPF�ھӱ�{sourceIP,{infInx,[destIP]}}
	OSPFNBR_LIST ospfnbr_list;
	//�豸��·�ɱ�{sourceIP,{infInx,[nextIP]}}
	ROUTE_LIST route_list;
	BGP_LIST bgp_list;

	//VRRP_LIST vrrp_list;
	RouterStandby_LIST routeStandby_list;//added by zhangyan 2009-01-09

	//�豸ARP�����б�{sourceIP,[infInx,[(MAC,destIP)]]}
	ARP_LIST arp_list;
	//�豸�ӿ������б� {devIP,(ifAmount,[(ifindex,ifType,ifDescr,ifMac,portNum,ifSpeed)])}
	IFPROP_LIST ifprop_list;

	DIRECTDATA_LIST directdata_list;

	STP_LIST stp_list;	//add by jiangshanwen 20100910

	// ��ȡ�豸ת����
	bool getAftByDtp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList);
	// ͨ��Bridge��ͬ���ȡ�豸ת����
	void getAftByQtp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList);

private:
	//��Ϣ����
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

	// ��ȡ�ض��豸�Ľӿ�����
	virtual void getInfFlow(SnmpDG& snmp, const SnmpPara& spr){}

private:
	//�豸AFT�����б�{sourceIP,[port,[MAC]]}
        pair<std::string, std::map<std::string, std::list<std::string> > > aft_cur;
	//�豸ARP�����б�{sourceIP,[infInx,[(MAC,destIP)]]}
        pair<std::string, std::map<std::string, std::list<std::pair<std::string,std::string> > > > arp_cur;
	//�豸�ӿ������б� {devIP,(ifAmount,[(ifindex,ifType,ifDescr,ifMac,portNum,ifSpeed)])}
        pair<std::string, std::pair<std::string, std::list<IFREC> > > ifprop_cur;
	//CDP�����б�{ip,(index,peerid,peerip,peerportdesc)}
	pair<std::string, std::list<DIRECTITEM> > directdata_cur;

private:
	// ��ȡһ̨�豸�ĸ���������Ϣ����
	DIRECTDATA_LIST getDirectData(SnmpDG& snmp, const SnmpPara& spr);

};
