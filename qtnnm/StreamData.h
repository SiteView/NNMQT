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
	// ���ļ������豸��ʶ�������б�
	bool saveIDBodyData(const DEVID_LIST& devid_list, const string& cdpPrex = "");
	bool saveFrmDevIDList(const DEVID_LIST& devid_list, const string& cdpPrex = "");
	// ���ļ���ȡ�豸��ʶ�������б�
	bool readIDBodyData(DEVID_LIST& devid_list, const string& cdpPrex = "");
	// ����ӿ������б�
	bool saveInfPropList(const IFPROP_LIST& inf_list, const string& cdpPrex = "");
	// ��ȡԭʼ�Ľӿ������б�
	bool readInfPropList(IFPROP_LIST& inflist, const string& cdpPrex = "");
	// ����ԭʼaft�����б�
	bool saveAftList(const AFT_LIST& aft_list);
	// ��ȡԭʼ��ת��������
	bool readAftList(AFT_LIST& aftlist);
	// ����arp�����б�
	bool saveArpList(const ARP_LIST& art_list);
	// ��ȡԭʼ��ARP�����б�
	bool readArpList(ARP_LIST& arplist);
	// ����Ospf�ھ������б�
	bool saveOspfNbrList(const OSPFNBR_LIST& ospfnbr_list);	
	// ��ȡ�豸ip�б�
	bool readDeviceIpList(vector<string>& devip_list);
	// ��ȡOspf�ھ������б�
	bool readOspfNbrList(OSPFNBR_LIST& ospfnbr_list);
	// ��ȡRoute�����б�
	bool saveBgpList(const BGP_LIST& bgp_list);
	bool saveVrrpList(const RouterStandby_LIST& vrrp_list);//added by tgf 2008-09-22
	bool readVrrpList(RouterStandby_LIST& vrrp_list);//added by tgf 2008-09-22

	// add by zhangyan 2008-08-29
	bool saveFrmBgpList(const BGP_LIST& bgp_list_frm);
        bool saveTraceDestData(const list<pair<pair<string, string>, string> > RouteIPPairList);
	bool readBgpList(BGP_LIST& bgp_list);
	// add by zhangyan 2008-08-27
	// ����Trace route�����б�
	bool saveTracertList(const ROUTEPATH_LIST& tracert_list);
	bool saveRenewTracertList(const ROUTEPATH_LIST& tracert_list);
	// ��ȡTrace route�����б�
	bool readTracertList(ROUTEPATH_LIST& tracert_list);

	// ����淶�����ת����
	bool saveFrmAftList(const FRM_AFTARP_LIST& aft_list_frm);
	// ����淶�����arp�����б�
	bool saveFrmArpList(const FRM_AFTARP_LIST& arp_list_frm);
	// ����ԭʼ��CDP�����б�
	bool saveDirectData(const DIRECTDATA_LIST& directdata_list);
	// ��ȡԭʼ�Ľӿ������б�
	bool readDirectData(DIRECTDATA_LIST& directdata_list);
	bool readRouteList(ROUTE_LIST& route_list);
	// add by zhangyan 2008-10-30
	bool saveConfigData(const SCAN_PARAM& scanPara);
	bool readConfigData(SCAN_PARAM& scanPara);

	// add by wings for test 2009-10-29
	bool saveEdge_SSData(const EDGE_LIST& edge_list);
	bool saveFrmRtList(const ROUTE_LIST_FRM& rt_list_frm);
	bool saveRouteList(const ROUTE_LIST& route_list);// ����Route�����б�
	bool saveFrmSSAftList(const FRM_AFTARP_LIST& aft_list_frm,int test);
	//add by wings 2009-11-13
	bool savaDevidIps(const DEVID_LIST& devid_list);
	//add by wings 2009-11-26
	bool readSwIpList(vector<string>& swip_list);
};

#endif