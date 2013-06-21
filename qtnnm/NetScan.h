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
	// ��������ip��ַ�б�
	std::list<std::string> localip_list;
	std::string localip;//added by zhangyan 2009-01-12
        std::map<std::string, std::list<std::pair<std::string,std::string> > > localport_macs;

	ReadService* siReader;

	// ɨ�����
	SCAN_PARAM scanParam;
	//ɨ�貹�����
	AUX_PARAM myParam;
	//��Ϣ����
	callback_functor_type m_callback;

	bool m_IsCdpScan;

	//�豸������Ϣ�б�{devIP,(TYPE,SNMP,[IP],[MAC],[MASK],SYSOID,sysname)}
	DEVID_LIST devid_list;
	//�豸AFT�����б�{sourceIP,[port,[MAC]]}
	AFT_LIST aft_list;
	//�豸ARP�����б�{sourceIP,[infInx,[(MAC,destIP)]]}
	ARP_LIST arp_list;
	//�豸��OSPF�ھӱ�{sourceIP,{infInx,[destIP]}}
	OSPFNBR_LIST ospfnbr_list;
	//�豸��Route��{sourceIP,{infInx,[destIP]}}
	ROUTE_LIST rttbl_list;
	BGP_LIST bgp_list;
	/*VRRP_LIST vrrp_list;*/
	RouterStandby_LIST routeStandby_list;
	//�淶���豸AFT�����б�{sourceIP,[infInx,[destIP]]}
	FRM_AFTARP_LIST aft_list_frm;
	//�淶���豸ARP�����б�{sourceIP,[infInx,[destIP]]}
	FRM_AFTARP_LIST arp_list_frm;
	//�豸�ӿ������б� {devIP,(ifAmount,[(ifindex,ifType,ifDescr,ifMac,portNum,ifSpeed)])}
	IFPROP_LIST ifprop_list;
	//·�ɸ���·���б�
	// add by zhangyan 2008-08-27
	ROUTEPATH_LIST rtpath_list;
	//CDP�����б�{ip,(index,peerid,peerip,peerportdesc)}
	DIRECTDATA_LIST directdata_list;

	//add by jiangshanwen 20100910
	STP_LIST stp_list;
		
	// topo���ӱ��б�
	EDGE_LIST topo_edge_list;
	// topoʵ���б�
	DEVID_LIST topo_entity_list;

        //bool isStop;

public:
	// ִ��ɨ��
        void Start();
	void Scan(void);
        void SetStop(bool stop);
        void ReadConfig(SCAN_PARAM param);

	// ��ȡtopo���б�
	EDGE_LIST& getTopoEdgeList(void);
	// ��ȡtopoʵ���б�
	DEVID_LIST& getTopoEntityList(void);
	IFPROP_LIST& getInfPropList(void);

	SCALE_LIST m_scale_list_toscan;//��ɨ�跶Χ[ip0,ip1]
	SCALE_LIST m_scale_list_scaned;//��ɨ�跶Χ[ip0,ip1]
	list<string> m_ip_list_visited;//�ѷ���ip
	vector<string> seeds_arp;

       // pool* tp;

protected:
        void run();
private:
	// ��ICE�ṹת���ɲ�������
        //bool FormatConfig(SliceNNM::ScanConfig & config);

	// ��ȡɨ�������Ĳ��������ļ�
	void ReadMyScanConfigFile(void);

	bool ExistNetDevice(const DEVID_LIST& dev_list);

	// ����Χ����ɨ��
        bool ScanByScale(SCALE_LIST scales);
	// �����ӽ���ɨ��
        bool ScanBySeeds(std::list<std::string> seed_list);

	//���ݽӿ������б���ӿڱ�,��ȡ��ͬǰ׺
	string getInfDescPrex(const list<string>& infIndex_list, const list<IFREC>& inf_list);

	// ��ȡ��������IP��ַ
	bool getLocalhostIPs(void);
	// ��ȡIP��Ӧ�Ĺ�ͬ����
	std::string getCommunity_Get(const std::string& ip);
	// ��ȡIP��Ӧ��SNMP Version 
	std::string getSNMPVersion(const std::string& ip);
	// ��ȡ������ARP����
	list<string> m_loacal_ip_from_arp;
	void getLocalArp(unsigned long ipnumMin, unsigned long ipnumMax);
	// �������豸
	int GenerateDumbDevice(EDGE_LIST& edge_list, DEVID_LIST& device_list);
	// ������Ϣ
	void FillEdge(EDGE_LIST& edge_list);

	string getIpsofDevID(const IDBody& devid);

	string findInfInxFromDescr(const list<IFREC>& inf_list, const string& port);
	string findInfPortFromDescr(const list<IFREC>& inf_list, const string& port);
	// �淶�������ļ�
	bool FormatData(void);

	//add by zhangyan 2008-10-09
	void getLocalNet(SCALE_LIST& localip_scales, string& localip);
	void PingLocalNet(SCALE_LIST& localip_scales, string& localip);

	//icmp ping iplist
        bool IcmpPing(const vector<string>& iplist,bool bGetla, const string& msg, vector<string>& iplist_alive);
	//snmp ping iplist
        list<pair<string,pair<string,int> > >& SnmpPing(const list<pair<string,string> >&,const string&,list<pair<string,pair<string,int> > >&);
	// ����ip��ַ��ɨ��
	void scanByIplist();
	// ɨ��һ����Χ
        bool scanOneScale(std::pair<std::string,std::string>& scale, bool bChange);
        bool scanByIps(const vector<string>& ip_list, bool bChange);
	// ����ɨ����ԭʼ����
	bool saveOriginData(void);
	// ���ļ���ȡɨ����ԭʼ����
	bool readOriginData(void);
	// ����淶���������
	bool saveFormatData(void);

	bool isScaleBInA(const pair<string,string>& scaleA, const pair<string,string>& scaleB);
	void addScaleFromDevID(const DEVID_LIST& devlist);
	void addScaleFromRouteItems(const ROUTE_LIST& routelist);
	void addScaleFromDirectData(const DIRECTDATA_LIST& directlist);
	
	//add by wings 09-12-15
	/*****************ͨ��arp���·���**************/
	void ScanBySeedsByArp(std::list<std::string> seed_list);
	void ScanSeeds_Arp();

	/******************** cdp scan ******************/
	void Scan_cdp(void);
    
	////�淶���豸����
	//bool FormatData_cdp();
	//��������ͷ���
	//bool DataAnalyse_cdp();

	// ����ɨ����ԭʼ����
	bool saveOriginData_cdp(void);
	// ���ļ���ȡɨ����ԭʼ����
	bool readOriginData_cdp(void);

};

#endif
