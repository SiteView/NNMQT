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
	
	bool EdgeAnalyse(void);// 进行数据分析
	bool EdgeAnalyseDirect(void);// 进行CDP数据分析

private:
	AUX_PARAM m_param;
	EDGE_LIST edge_list_rs_byaft;
	// topo连接边列表
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
	list<string> rt_list;//路由器ip地址表
	list<string> sw_list;//交换机ip地址表
	list<string> rs_list;//交换路由,路由器ip地址表
	//add by wings 2009-11-26
	list<string> sw2_list;//记录2层交换机ip
	list<string> sw3_list;//记录3层交换机ip

	IFPROP_LIST ifprop_list;

public:
	// 获取topo边列表
	EDGE_LIST& getEdgeList(void);

private:
	// 分析路由器与交换机之间的连接关系
	bool AnalyseRS(void);
	bool AnalyseRS_COMP(void);
	// 分析交换机与主机之间的连接关系
	bool AnalyseSH(void);
	bool AnalyseSH_COMP(void);
	// 分析交换机之间的连接关系
	bool AnalyseSS(void);
	bool AnalyseSS_COMP(void);
	// 分析路由器之间的连接关系
	bool AnalyseRRByArp(void);
	bool AnalyseRRByNbr(void);
	// 分析路由器之间的连接关系
	bool AnalyseRRByRt(void);
	bool AnalyseRRByBgp(void);
	// added by zhangyan 2009-01-15
	bool AnalyseRRBySubnet(void);

	bool isSameFactory(const DEVID_LIST& device_list);

	EDGE_LIST getDirectEdge();
	EDGE_LIST getDirectEdgeByAft(void);
	EDGE_LIST getDirectEdgeByArp(void);

	// 判定一个端口集合(port_set_A)中的ip是否包含在另一个端口集合(port_set_B)中
	// port_set_A -- 被包含集合
	// port_set_B -- 包含集合
	// excludeIp -- 从port_set_A排除的ip
        bool IsInclude(const map<string,list<string> >& port_set_A, const map<string,list<string> >& port_set_B, const string excludeIp);
	// 判定一个端口集是否叶子
	bool IsLeafPort(const list<string>& iplist, const list<string>& sw_list);
	// 判定一个端口是否仅连接了一台交换设备
	bool IsSWLeafPort(const list<string>& iplist, const list<string>& sw_list);
	// 判定一个交换设备是否为一个叶子交换设备
        bool IsSWLeafDevice(const string remoteip, const map<string,list<string> >& port_set);
	void printdata(const FRM_AFTARP_LIST& data_list);
	// 用arp数据补充aft
	void CompAftWithArp(const string& stype);
};
