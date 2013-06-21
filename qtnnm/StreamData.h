#ifndef STREAMDATA_H
#define STREAMDATA_H
// StreamData.h

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
#include "CommonDef.h"
//#include "SvLog.h"
#include "scanutils.h"


using namespace std;

class StreamData
{
public:
	StreamData(void);
	~StreamData(void);
	// 用文件保存设备标识体数据列表
	bool saveIDBodyData(const DEVID_LIST& devid_list, const string& cdpPrex = "");
	bool saveFrmDevIDList(const DEVID_LIST& devid_list, const string& cdpPrex = "");
	// 从文件读取设备标识体数据列表
	bool readIDBodyData(DEVID_LIST& devid_list, const string& cdpPrex = "");
	// 保存接口数据列表
	bool saveInfPropList(const IFPROP_LIST& inf_list, const string& cdpPrex = "");
	// 读取原始的接口数据列表
	bool readInfPropList(IFPROP_LIST& inflist, const string& cdpPrex = "");
	// 保存原始aft数据列表
	bool saveAftList(const AFT_LIST& aft_list);
	// 读取原始的转发表数据
	bool readAftList(AFT_LIST& aftlist);
	// 保存arp数据列表
	bool saveArpList(const ARP_LIST& art_list);
	// 读取原始的ARP数据列表
	bool readArpList(ARP_LIST& arplist);
	// 保存Ospf邻居数据列表
	bool saveOspfNbrList(const OSPFNBR_LIST& ospfnbr_list);	
	// 读取设备ip列表
	bool readDeviceIpList(vector<string>& devip_list);
	// 读取Ospf邻居数据列表
	bool readOspfNbrList(OSPFNBR_LIST& ospfnbr_list);
	// 读取Route数据列表
	bool saveBgpList(const BGP_LIST& bgp_list);
	bool saveVrrpList(const RouterStandby_LIST& vrrp_list);//added by tgf 2008-09-22
	bool readVrrpList(RouterStandby_LIST& vrrp_list);//added by tgf 2008-09-22

	// add by zhangyan 2008-08-29
	bool saveFrmBgpList(const BGP_LIST& bgp_list_frm);
        bool saveTraceDestData(const list<pair<pair<string, string>, string> > RouteIPPairList);
	bool readBgpList(BGP_LIST& bgp_list);
	// add by zhangyan 2008-08-27
	// 保存Trace route数据列表
	bool saveTracertList(const ROUTEPATH_LIST& tracert_list);
	bool saveRenewTracertList(const ROUTEPATH_LIST& tracert_list);
	// 读取Trace route数据列表
	bool readTracertList(ROUTEPATH_LIST& tracert_list);

	// 保存规范化后的转发表
	bool saveFrmAftList(const FRM_AFTARP_LIST& aft_list_frm);
	// 保存规范化后的arp数据列表
	bool saveFrmArpList(const FRM_AFTARP_LIST& arp_list_frm);
	// 保存原始的CDP数据列表
	bool saveDirectData(const DIRECTDATA_LIST& directdata_list);
	// 读取原始的接口数据列表
	bool readDirectData(DIRECTDATA_LIST& directdata_list);
	bool readRouteList(ROUTE_LIST& route_list);
	// add by zhangyan 2008-10-30
	bool saveConfigData(const SCAN_PARAM& scanPara);
	bool readConfigData(SCAN_PARAM& scanPara);

	// add by wings for test 2009-10-29
	bool saveEdge_SSData(const EDGE_LIST& edge_list);
	bool saveFrmRtList(const ROUTE_LIST_FRM& rt_list_frm);
	bool saveRouteList(const ROUTE_LIST& route_list);// 保存Route数据列表
	bool saveFrmSSAftList(const FRM_AFTARP_LIST& aft_list_frm,int test);
	//add by wings 2009-11-13
	bool savaDevidIps(const DEVID_LIST& devid_list);
	//add by wings 2009-11-26
	bool readSwIpList(vector<string>& swip_list);
};

#endif