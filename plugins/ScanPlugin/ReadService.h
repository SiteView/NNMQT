#pragma once

#include <string>

#include "CommonDef.h"
#include "SnmpPara.h"
#include "svlog.h"
#include "scanutils.h"
#include "DeviceRegister.h"

#include "TelnetReader.h"


#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

#include "threadpool.hpp"
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace boost::threadpool;


using namespace std;

class ReadService
{
public:
        ReadService(const DEVICE_TYPE_MAP& devtypemap, const SCAN_PARAM scanpr, const AUX_PARAM auxpr, const SPECIAL_OID_LIST& specialoidlist);
public:
	~ReadService(void);

public:
	void getCdpDeviceData(vector<SnmpPara>& spr_list);
	//获取设备数据
        bool getDeviceData(vector<SnmpPara>& spr_list);
	AFT_LIST& getAftData(void){return aft_list;}
	ARP_LIST& getArpData(void){return arp_list;}
	OSPFNBR_LIST& getOspfNbrData(void){return ospfnbr_list;}
	ROUTE_LIST& getRouteData(void){return route_list;}
	BGP_LIST& getBgpData(void){return bgp_list;}
	/*VRRP_LIST& getVrrpData(void){return vrrp_list;}*/
	RouterStandby_LIST& getVrrpData(void){return routeStandby_list;}//by zhangyan 2009-01-09
	IFPROP_LIST& getInfProps(){return ifprop_list;}
	DIRECTDATA_LIST& getDirectData(){return directdata_list;}

	STP_LIST& getStpData(){return stp_list;}
	
	void getOneArpData(const SnmpPara& spr, ARP_LIST& arp_list);
	//added by zhangyan 2008-10-10
        void getOneArpData(const SnmpPara& spr, std::map<std::string, std::list<std::pair<std::string,std::string> > >& inf_ipmacs);
        void getIpMaskList(const SnmpPara& spr, list<pair<string, string> >& ipcm_result);
	IDBody getOneSysInfo_NJ(SnmpPara& spr);
	//add by wings 09-11-13
	bool testIP(const SnmpPara& spr);

public:
	DEVICE_TYPE_MAP dev_type_list;
	DEVID_LIST devid_list_valid;
	DEVID_LIST devid_list_visited;
	list<string> ip_visited_list;
	SPECIAL_OID_LIST special_oid_list; //add by jiangshanwen 2010-7-21
	map<string, string> oid_index_list;	//add by jiangshanwen 2010-7-21

        list<pair<SnmpPara,pair<string,string> > > sproid_list;

private:
	void getOneSysInfo(SnmpPara& spr);
        bool getSysInfos(vector<SnmpPara>& spr_list);
	void getDevTypeByOid(const string& sysOid, const string& sysSvcs, const string& ip,  map<string,string>& map_res);
	void addDevID(const SnmpPara& spr, const IDBody& devid);
	bool isNewIp(const string& ip);

	//获取一台设备的普通数据
	void getOneDeviceData(const SnmpPara& spr, const string& devType, const string& sysOid);
	//获取一台设备的CDP数据
	void getOneCdpDeviceData(const SnmpPara& spr, const string& devType, const string& sysOid);

	string getDeviceIps(const string& ip);

	//获取某一台设备的oid索引 add by jiangshanwen
	void getOidIndex(map<string, string>& oidIndexList, string sysOid);


private:
        boost::mutex m_data_mutex;

	SCAN_PARAM scanPara;
	AUX_PARAM auxPara;
	callback_functor_type m_callback;
	map<string,string> map_devType;

	TelnetReader* telnetReader;

private://返回数据
	//设备AFT数据列表{sourceIP,[port,[MAC]]}
	AFT_LIST aft_list;
	//设备的OSPF邻居表{sourceIP,{infInx,[destIP]}}
	OSPFNBR_LIST ospfnbr_list;
	//设备的路由表{sourceIP,{infInx,[nextIP]}}
	ROUTE_LIST route_list;
	//设备ARP数据列表{sourceIP,[infInx,[(MAC,destIP)]]}
	ARP_LIST arp_list;
	//设备接口属性列表 {devIP,(ifAmount,[(ifindex,ifType,ifDescr,ifMac,portNum,ifSpeed)])}
	IFPROP_LIST ifprop_list;
	BGP_LIST bgp_list;
	/*VRRP_LIST vrrp_list;*/
	RouterStandby_LIST routeStandby_list; //by zhangyan 2009-01-09
	//直接边数据列表{ip,(index,peerid,peerip,peerportdesc)}
	DIRECTDATA_LIST directdata_list;
	STP_LIST stp_list;
};
