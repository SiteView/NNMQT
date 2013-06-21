#ifndef DEVICE_H
#define DEVICE_H

#pragma once
#include "commondef.h"
#include "SnmpPara.h"
#include "SnmpDG.h"

//add by jiangshanwen 特殊设备指标代号 2010-7-21
#define DEVICEMAC_MACRO				"15010"		//设备MAC地址									"1.3.6.1.2.1.17.1.1"

#define INF_AMOUNT_MACRO			"15011"		//接口表：端口总数								"1.3.6.1.2.1.2.1"
#define INF_TYPE_MACRO				"16036"		//接口表：接口类型								"1.3.6.1.2.1.2.2.1.3"
#define INF_DCR_MACRO				"16041"		//接口表：端口描述								"1.3.6.1.2.1.2.2.1.2"
#define	INF_INDEXPORT_MACRO			"15012"		//接口表：物理端口和逻辑索引号对应关系			"1.3.6.1.2.1.17.4.3.1.2"
#define INF_MAC_MACRO				"16057"		//接口表：接口MAC地址							"1.3.6.1.2.1.2.2.1.6"
#define INF_SPEED_MACRO				"15013"		//接口表：接口速度								"1.3.6.1.2.1.2.2.1.5"

#define IPADDR_MASK_MACRO			"16075"		//IP地址表：与该实体的ip地址相关联的子网掩码	"1.3.6.1.2.1.4.20.1.3"
#define IPADDR_INFINDEX				"16074"		//IP地址表：唯一标示该实体所对应的接口的索引值	"1.3.6.1.2.1.4.20.1.2"

#define ARP_INFINDEX_MACRO			"16028"		//ARP表：端口索引号								"1.3.6.1.2.1.4.22.1.1"
#define ARP_MAC_MACRO				"16029"		//ARP表：介质依赖的物理地址						"1.3.6.1.2.1.4.22.1.2"
#define ARP_IP_MACRO				"16030"		//ARP表：与介质依赖的物理地址相对应的ip地址		"1.3.6.1.2.1.4.22.1.3"

#define AFT_MAC_DTP_PORT			"15021"		//AFT表：端口索引号								"1.3.6.1.2.1.17.4.3.1.2"
#define AFT_MAC_DTP_MACRO			"16032"		//AFT表：MAC地址(DTP)							"1.3.6.1.2.1.17.4.3.1.1"
#define AFT_MAC_QTP_MACRO			"15014"		//AFT表：MAC地址(QTP)							"1.3.6.1.2.1.17.7.1.2.2.1.2"

#define ROUTE_INFINDEX_MACRO		"16017"		//路由表：连接目的网络的接口索引号				"1.3.6.1.2.1.4.21.1.2"
#define ROUTE_NEXTHOPIP_MACRO		"16022"		//路由表：目的网络的下一跳IP地址				"1.3.6.1.2.1.4.21.1.7"
#define ROUTE_ROUTETYPE_MACRO		"16023"		//路由表：路由类型								"1.3.6.1.2.1.4.21.1.8"
#define ROUTE_ROUTEMASK_MACRO		"16026"		//路由表：路由目的ip地址的子网掩码				"1.3.6.1.2.1.4.21.1.11"

#define OSPF_INFINDEX_MACRO			"15015"		//OSPF表：邻居IP所在的接口索引					"1.3.6.1.2.1.14.10.1.2"

#define BGP_LOCALADDR_MACRO			"15016"		//BGP表：本端IP地址								"1.3.6.1.2.1.15.3.1.5"
#define BGP_LOCALPORT_MACRO			"15017"		//BGP表：本端端口								"1.3.6.1.2.1.15.3.1.6"
#define BGP_REMOTEPORT_MACRO		"15018"		//BGP表：对端端口								"1.3.6.1.2.1.15.3.1.8"

#define VRRP_VIRTUALMAC_MACRO		"15019"		//VRRP表：虚拟MAC								"1.3.6.1.2.1.68.1.3.1.2"
#define VRRP_VIRTUALIP_MACRO		"15020"		//VRRP表：虚拟IP								"1.3.6.1.2.1.68.1.4.1.2"

class Device
{
public:
	Device(void);
	Device(const SCAN_PARAM scanParam, const AUX_PARAM auxParam){}
public:
	~Device(void);
public:
	//remarked by zhangyan 2009-01-14
	//virtual void Init(const SCAN_PARAM scanpr, const AUX_PARAM auxpr)=0;

	virtual DIRECTDATA_LIST getDirectData(SnmpDG& snmp, const SnmpPara& spr)=0;

	virtual AFT_LIST& getAftData(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList)=0;
	virtual ARP_LIST& getArpData(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList)=0;
	virtual OSPFNBR_LIST& getOspfNbrData(SnmpDG& snmp, const SnmpPara& spr)=0;
	virtual ROUTE_LIST& getRouteData(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList)=0;
	virtual BGP_LIST& getBgpData(SnmpDG& snmp, const SnmpPara& spr)=0;
	virtual IFPROP_LIST& getInfProp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList,bool bRouter=false)=0;
	virtual void getInfFlow(SnmpDG& snmp, const SnmpPara& spr)=0;
	virtual RouterStandby_LIST& getVrrpData(SnmpDG& snmp, const SnmpPara& spr)=0;
	virtual STP_LIST& getStpData(SnmpDG& snmp, const SnmpPara& spr)=0;	//add by jiangshanwen 20100910

};

#endif
