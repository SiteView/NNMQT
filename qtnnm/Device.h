#ifndef DEVICE_H
#define DEVICE_H

#pragma once
#include "commondef.h"
#include "SnmpPara.h"
#include "SnmpDG.h"

//add by jiangshanwen �����豸ָ����� 2010-7-21
#define DEVICEMAC_MACRO				"15010"		//�豸MAC��ַ									"1.3.6.1.2.1.17.1.1"

#define INF_AMOUNT_MACRO			"15011"		//�ӿڱ��˿�����								"1.3.6.1.2.1.2.1"
#define INF_TYPE_MACRO				"16036"		//�ӿڱ��ӿ�����								"1.3.6.1.2.1.2.2.1.3"
#define INF_DCR_MACRO				"16041"		//�ӿڱ��˿�����								"1.3.6.1.2.1.2.2.1.2"
#define	INF_INDEXPORT_MACRO			"15012"		//�ӿڱ�����˿ں��߼������Ŷ�Ӧ��ϵ			"1.3.6.1.2.1.17.4.3.1.2"
#define INF_MAC_MACRO				"16057"		//�ӿڱ��ӿ�MAC��ַ							"1.3.6.1.2.1.2.2.1.6"
#define INF_SPEED_MACRO				"15013"		//�ӿڱ��ӿ��ٶ�								"1.3.6.1.2.1.2.2.1.5"

#define IPADDR_MASK_MACRO			"16075"		//IP��ַ�����ʵ���ip��ַ���������������	"1.3.6.1.2.1.4.20.1.3"
#define IPADDR_INFINDEX				"16074"		//IP��ַ��Ψһ��ʾ��ʵ������Ӧ�Ľӿڵ�����ֵ	"1.3.6.1.2.1.4.20.1.2"

#define ARP_INFINDEX_MACRO			"16028"		//ARP���˿�������								"1.3.6.1.2.1.4.22.1.1"
#define ARP_MAC_MACRO				"16029"		//ARP�����������������ַ						"1.3.6.1.2.1.4.22.1.2"
#define ARP_IP_MACRO				"16030"		//ARP������������������ַ���Ӧ��ip��ַ		"1.3.6.1.2.1.4.22.1.3"

#define AFT_MAC_DTP_PORT			"15021"		//AFT���˿�������								"1.3.6.1.2.1.17.4.3.1.2"
#define AFT_MAC_DTP_MACRO			"16032"		//AFT��MAC��ַ(DTP)							"1.3.6.1.2.1.17.4.3.1.1"
#define AFT_MAC_QTP_MACRO			"15014"		//AFT��MAC��ַ(QTP)							"1.3.6.1.2.1.17.7.1.2.2.1.2"

#define ROUTE_INFINDEX_MACRO		"16017"		//·�ɱ�����Ŀ������Ľӿ�������				"1.3.6.1.2.1.4.21.1.2"
#define ROUTE_NEXTHOPIP_MACRO		"16022"		//·�ɱ�Ŀ���������һ��IP��ַ				"1.3.6.1.2.1.4.21.1.7"
#define ROUTE_ROUTETYPE_MACRO		"16023"		//·�ɱ�·������								"1.3.6.1.2.1.4.21.1.8"
#define ROUTE_ROUTEMASK_MACRO		"16026"		//·�ɱ�·��Ŀ��ip��ַ����������				"1.3.6.1.2.1.4.21.1.11"

#define OSPF_INFINDEX_MACRO			"15015"		//OSPF���ھ�IP���ڵĽӿ�����					"1.3.6.1.2.1.14.10.1.2"

#define BGP_LOCALADDR_MACRO			"15016"		//BGP������IP��ַ								"1.3.6.1.2.1.15.3.1.5"
#define BGP_LOCALPORT_MACRO			"15017"		//BGP�����˶˿�								"1.3.6.1.2.1.15.3.1.6"
#define BGP_REMOTEPORT_MACRO		"15018"		//BGP���Զ˶˿�								"1.3.6.1.2.1.15.3.1.8"

#define VRRP_VIRTUALMAC_MACRO		"15019"		//VRRP������MAC								"1.3.6.1.2.1.68.1.3.1.2"
#define VRRP_VIRTUALIP_MACRO		"15020"		//VRRP������IP								"1.3.6.1.2.1.68.1.4.1.2"

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
