// topoAnalyse.h

/*
 *
 * Copyright (c) 2007-2008 DragonFlow
 *
 * Created date: 2007-11-30
 *
 * Autor: Tang guofu 
 * 
 * last modified 2007-11-30 by Tang guofu 
 *
 */


#pragma once
#include "stdafx.h"
#include "commondef.h"
#include "StreamData.h"


using namespace std;


class topoAnalyse
{
public:
	topoAnalyse(const DEVID_LIST& devlist, const IFPROP_LIST& ifprop_list, const FRM_AFTARP_LIST& aftlist, const FRM_AFTARP_LIST& arplist, const OSPFNBR_LIST& ospfnbr_list, const ROUTE_LIST& route_list, const BGP_LIST& bgp_list, const DIRECTDATA_LIST& direct_list, const AUX_PARAM param);

	~topoAnalyse(void);
	
	bool EdgeAnalyse(void);// �������ݷ���
	bool EdgeAnalyseDirect(void);// ����CDP���ݷ���

private:
	AUX_PARAM m_param;
	EDGE_LIST edge_list_rs_byaft;
	// topo���ӱ��б�
	EDGE_LIST topo_edge_list;
	EDGE_LIST edge_list_cur; //added by tgf 2008-09-03

	//
	DEVID_LIST device_list;
	FRM_AFTARP_LIST aft_list;
	FRM_AFTARP_LIST arp_list;
	OSPFNBR_LIST nbr_list;
	ROUTE_LIST rttbl_list;
	BGP_LIST bgp_list;
	DIRECTDATA_LIST direct_list;

        map<string, list<pair<string, string> > > IPADDR_map; //{ip,[<subnet,ifindex>]} added by zhangyan 2009-01-15

	//DEVID_LIST device_list_bak;
	//FRM_AFTARP_LIST aft_list_bak; //remarked by zhangyan 2008-10-28
	//FRM_AFTARP_LIST arp_list_bak;

	//
	list<string> rt_list;//·����ip��ַ��
	list<string> sw_list;//������ip��ַ��
	list<string> rs_list;//����·��,·����ip��ַ��
	//add by wings 2009-11-26
	list<string> sw2_list;//��¼2�㽻����ip
	list<string> sw3_list;//��¼3�㽻����ip

	IFPROP_LIST ifprop_list;

public:
	// ��ȡtopo���б�
	EDGE_LIST& getEdgeList(void);

private:
	// ����·�����뽻����֮������ӹ�ϵ
	bool AnalyseRS(void);
	bool AnalyseRS_COMP(void);
	// ����������������֮������ӹ�ϵ
	bool AnalyseSH(void);
	bool AnalyseSH_COMP(void);
	// ����������֮������ӹ�ϵ
	bool AnalyseSS(void);
	bool AnalyseSS_COMP(void);
	// ����·����֮������ӹ�ϵ
	bool AnalyseRRByArp(void);
	bool AnalyseRRByNbr(void);
	// ����·����֮������ӹ�ϵ
	bool AnalyseRRByRt(void);
	bool AnalyseRRByBgp(void);
	// added by zhangyan 2009-01-15
	bool AnalyseRRBySubnet(void);

	bool isSameFactory(const DEVID_LIST& device_list);

	EDGE_LIST getDirectEdge();
	EDGE_LIST getDirectEdgeByAft(void);
	EDGE_LIST getDirectEdgeByArp(void);

	// �ж�һ���˿ڼ���(port_set_A)�е�ip�Ƿ��������һ���˿ڼ���(port_set_B)��
	// port_set_A -- ����������
	// port_set_B -- ��������
	// excludeIp -- ��port_set_A�ų���ip
        bool IsInclude(const map<string,list<string> >& port_set_A, const map<string,list<string> >& port_set_B, const string excludeIp);
	// �ж�һ���˿ڼ��Ƿ�Ҷ��
	bool IsLeafPort(const list<string>& iplist, const list<string>& sw_list);
	// �ж�һ���˿��Ƿ��������һ̨�����豸
	bool IsSWLeafPort(const list<string>& iplist, const list<string>& sw_list);
	// �ж�һ�������豸�Ƿ�Ϊһ��Ҷ�ӽ����豸
        bool IsSWLeafDevice(const string remoteip, const map<string,list<string> >& port_set);
	void printdata(const FRM_AFTARP_LIST& data_list);
	// ��arp���ݲ���aft
	void CompAftWithArp(const string& stype);
};
