//NetScan.h 
#ifndef NETSCAN_H
#define NETSCAN_H



/*
 * Topo scan console program
 *
 * Copyright (c) 2007-2008 DragonFlow
 *
 * Created date: 2007-12-06
 *
 * Autor: Tang guofu 
 * 
 * last modified 2007-12-06 by Tang guofu 
 *
 */

#pragma once

#include <string>
#include <list>

#include "CommonDef.h"
#include "scanutils.h"

#include "StreamData.h"
//#include "SliceDataWrap.h"

#include "PingFind.h"
#include "ReadService.h"

#include "topoAnalyse.h"
#include "QConfig.h"
#include <QObject>
#include <QThread>

using namespace std;


//using namespace SliceNNM;

class NetScan : public QThread
{
        Q_OBJECT
public:
        NetScan(const DEVICE_TYPE_MAP& devtypemap, const SPECIAL_OID_LIST& specialoidlist, SCAN_PARAM param, QObject *parent = 0);
	~NetScan(void);

signals:
        void SendScanMessage(QString msg);
        void FinishAnalyse();
        void StopScan();

private:
	DEVICE_TYPE_MAP dev_type_list;
	// 本地主机ip地址列表
	std::list<std::string> localip_list;
	std::string localip;//added by zhangyan 2009-01-12
        std::map<std::string, std::list<std::pair<std::string,std::string> > > localport_macs;

	ReadService* siReader;

	// 扫描参数
	SCAN_PARAM scanParam;
	//扫描补充参数
	AUX_PARAM myParam;
	//消息函数
	callback_functor_type m_callback;

	bool m_IsCdpScan;

	//设备基本信息列表{devIP,(TYPE,SNMP,[IP],[MAC],[MASK],SYSOID,sysname)}
	DEVID_LIST devid_list;
	//设备AFT数据列表{sourceIP,[port,[MAC]]}
	AFT_LIST aft_list;
	//设备ARP数据列表{sourceIP,[infInx,[(MAC,destIP)]]}
	ARP_LIST arp_list;
	//设备的OSPF邻居表{sourceIP,{infInx,[destIP]}}
	OSPFNBR_LIST ospfnbr_list;
	//设备的Route表{sourceIP,{infInx,[destIP]}}
	ROUTE_LIST rttbl_list;
	BGP_LIST bgp_list;
	/*VRRP_LIST vrrp_list;*/
	RouterStandby_LIST routeStandby_list;
	//规范化设备AFT数据列表{sourceIP,[infInx,[destIP]]}
	FRM_AFTARP_LIST aft_list_frm;
	//规范化设备ARP数据列表{sourceIP,[infInx,[destIP]]}
	FRM_AFTARP_LIST arp_list_frm;
	//设备接口属性列表 {devIP,(ifAmount,[(ifindex,ifType,ifDescr,ifMac,portNum,ifSpeed)])}
	IFPROP_LIST ifprop_list;
	//路由跟踪路径列表
	// add by zhangyan 2008-08-27
	ROUTEPATH_LIST rtpath_list;
	//CDP数据列表{ip,(index,peerid,peerip,peerportdesc)}
	DIRECTDATA_LIST directdata_list;

	//add by jiangshanwen 20100910
	STP_LIST stp_list;
		
	// topo连接边列表
	EDGE_LIST topo_edge_list;
	// topo实体列表
	DEVID_LIST topo_entity_list;

        //bool isStop;

public:
	// 执行扫描
        void Start();
	void Scan(void);
        void SetStop(bool stop);
        void ReadConfig(SCAN_PARAM param);

	// 获取topo边列表
	EDGE_LIST& getTopoEdgeList(void);
	// 获取topo实体列表
	DEVID_LIST& getTopoEntityList(void);
	IFPROP_LIST& getInfPropList(void);

	SCALE_LIST m_scale_list_toscan;//待扫描范围[ip0,ip1]
	SCALE_LIST m_scale_list_scaned;//已扫描范围[ip0,ip1]
	list<string> m_ip_list_visited;//已访问ip
	vector<string> seeds_arp;

       // pool* tp;

protected:
        void run();
private:
	// 将ICE结构转换成参数类型
        //bool FormatConfig(SliceNNM::ScanConfig & config);

	// 读取扫描程序本身的补充配置文件
	void ReadMyScanConfigFile(void);

	bool ExistNetDevice(const DEVID_LIST& dev_list);

	// 按范围进行扫描
        bool ScanByScale(SCALE_LIST scales);
	// 按种子进行扫描
        bool ScanBySeeds(std::list<std::string> seed_list);

	//根据接口索引列表与接口表,获取共同前缀
	string getInfDescPrex(const list<string>& infIndex_list, const list<IFREC>& inf_list);

	// 获取本地主机IP地址
	bool getLocalhostIPs(void);
	// 获取IP对应的共同体名
	std::string getCommunity_Get(const std::string& ip);
	// 获取IP对应的SNMP Version 
	std::string getSNMPVersion(const std::string& ip);
	// 获取本机的ARP数据
	list<string> m_loacal_ip_from_arp;
	void getLocalArp(unsigned long ipnumMin, unsigned long ipnumMax);
	// 创建哑设备
	int GenerateDumbDevice(EDGE_LIST& edge_list, DEVID_LIST& device_list);
	// 填充边信息
	void FillEdge(EDGE_LIST& edge_list);

	string getIpsofDevID(const IDBody& devid);

	string findInfInxFromDescr(const list<IFREC>& inf_list, const string& port);
	string findInfPortFromDescr(const list<IFREC>& inf_list, const string& port);
	// 规范化数据文件
	bool FormatData(void);

	//add by zhangyan 2008-10-09
	void getLocalNet(SCALE_LIST& localip_scales, string& localip);
	void PingLocalNet(SCALE_LIST& localip_scales, string& localip);

	//icmp ping iplist
        bool IcmpPing(const vector<string>& iplist,bool bGetla, const string& msg, vector<string>& iplist_alive);
	//snmp ping iplist
        list<pair<string,pair<string,int> > >& SnmpPing(const list<pair<string,string> >&,const string&,list<pair<string,pair<string,int> > >&);
	// 根据ip地址表扫描
	void scanByIplist();
	// 扫描一个范围
        bool scanOneScale(std::pair<std::string,std::string>& scale, bool bChange);
        bool scanByIps(const vector<string>& ip_list, bool bChange);
	// 保存扫描后的原始数据
	bool saveOriginData(void);
	// 从文件读取扫描后的原始数据
	bool readOriginData(void);
	// 保存规范化后的数据
	bool saveFormatData(void);

	bool isScaleBInA(const pair<string,string>& scaleA, const pair<string,string>& scaleB);
	void addScaleFromDevID(const DEVID_LIST& devlist);
	void addScaleFromRouteItems(const ROUTE_LIST& routelist);
	void addScaleFromDirectData(const DIRECTDATA_LIST& directlist);
	
	//add by wings 09-12-15
	/*****************通过arp向下发现**************/
	void ScanBySeedsByArp(std::list<std::string> seed_list);
	void ScanSeeds_Arp();

	/******************** cdp scan ******************/
	void Scan_cdp(void);
    
	////规范化设备数据
	//bool FormatData_cdp();
	//数据整理和分析
	//bool DataAnalyse_cdp();

	// 保存扫描后的原始数据
	bool saveOriginData_cdp(void);
	// 从文件读取扫描后的原始数据
	bool readOriginData_cdp(void);

};

#endif
