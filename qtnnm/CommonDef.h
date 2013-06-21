#ifndef COMMONDEF_H
#define COMMONDEF_H
/****************************************************************************
 *	$id:CommonDef.h															*
 *																			*
 *	Copyright (c) 2007 Dragon Flow Networks,Inc. All rights reserved.		* 
 *																			*
 *	Author: Steffens <ming.wen@dragonflow.com>								*
 *																			*
 *	Created on 2007-03-29													*
 *																			*
 *	Last modified: 2008-09-22 by tgf										*
 *																			*
 ****************************************************************************/


#pragma once

#include "stdafx.h"
#include <snmp_pp/snmp_pp.h>

#define QUEUE_STATUS_OK 800
//#define		QUEUE_OK			800
//#define		QUEUE_EMPTY			801
//#define		QUEUE_TIMEOUT		802
//#define		QUEUE_BUFFERSMALL	803
//#define		QUEUE_NOEXIST		804
//#define		QUEUE_HASEXIST		805
//#define		QUEUE_IOFAILED		806
//#define		QUEUE_SYNERROR		807
//#define		QUEUE_UNKNOWERROR	808
//#define		QUEUE_GETPAGEERROR	809
//#define		QUEUE_MEMORYERROR	810





//消息类型及附加内容			
#define START_SCAN_MSG   "START_SCAN"   //附加消息:	"NULL"                  [开始扫描]
#define FIND_SUBNET_MSG  "FIND_SUBNET"  //附加消息:	"ip-ip"				    [发现子网(起始地址ip-ip)]
#define SCAN_SUBNET_MSG  "SCAN_SUBNET"  //附加消息:	"ip-ip"				    [扫描范围(子网)(起始地址ip-ip)]      
#define FOUND_DEVICE_MSG "FOUND_DEVICE" //附加消息:	"ip, typename, Factory" [发现设备(ip,类型,厂家)[忽略pc]
#define SAVE_DEVICE_MSG	 "SAVE_DEVICE"  //附加消息:	"success(or fail) xxx"	[保存设备数据: 成功(或者失败)xxx台]
//#define FINISHED_SUBNET_MSG "FINISHED_SUBNET"  //附加消息:	"ip-ip"			 [完成扫描范围(子网)(起始地址ip-ip)] 

#define TITLE_MSG       "TITLE_MSG"     //标题消息
#define TITLE_MSG_010   "10"            //标题附加消息:正在停止NnmMonitor服务
#define TITLE_MSG_011   "11"            //标题附加消息:停止NnmMonitor服务失败,需要手工停止
#define TITLE_MSG_012   "12"            //标题附加消息:正在启动NnmMonitor服务
#define TITLE_MSG_013   "13"            //标题附加消息:启动NnmMonitor服务失败,需要手工启动

#define TITLE_MSG_100   "100"           //标题附加消息:没有有效种子
#define TITLE_MSG_110   "110"           //标题附加消息:继续进行拓扑分析
#define TITLE_MSG_120   "120"           //标题附加消息:开始拓扑分析
#define TITLE_MSG_121   "121"           //标题附加消息:拓扑分析成功完成
#define TITLE_MSG_122   "122"           //标题附加消息:拓扑分析失败
#define TITLE_MSG_200   "200"           //标题附加消息:拓扑提交失败

#define COMMON_MSG        "COMMON_MSG"  //普通消息
#define COMMON_MSG_1100   "1100"        //普通附加消息:本机没有开启SNMP服务,将会丢失设备或主机

#define MINUS_THREAD_AMOUNT 200
//#define BATCH_SUBMIT_AMOUNT 10
#define BATCH_SUBMIT_AMOUNT 150 //by zhangyan
#define BATCH_SUBMIT_RETRYS 5



using namespace std;

typedef struct{
	string devType;     //设备类型
	string devTypeName; //设备类型名称
	string devFac;      //设备厂家
	string devModel;    //设备型号
}DEVICE_PRO;
typedef std::map<std::string, DEVICE_PRO> DEVICE_TYPE_MAP;//<sysoid,<devType,devTypeName,devFac,devModel> >


//设备基本属性数据 (TYPE,SNMP,[IP],[MAC],[MASK],SYSOID,sysname)
typedef struct {
	std::string baseMac;//基本Mac地址
	std::string snmpflag;//
	std::string community_get;
	std::string community_set;
	std::string devType;//
	std::string devFactory;//
	std::string devModel;//
	std::string devTypeName;//
	std::string sysOid;//sysObjectID 1.3.6.1.2.1.1.2
	std::string sysSvcs;//sysServices 1.3.6.1.2.1.1.7
	std::string sysName;//sysName 1.3.6.1.2.1.1.5 
	std::vector<std::string> ips;//IP地址1.3.6.1.2.1.4.20.1.3
	std::vector<std::string> infinxs;//IP地址对应的索引1.3.6.1.2.1.4.20.1.2
	std::vector<std::string> msks;//MASK地址1.3.6.1.2.1.4.20.1.3
	std::vector<std::string> macs;//MAC地址1.3.6.1.2.1.2.2.1.6
}IDBody;

//设备接口属性(ifindex,ifType,ifDescr,ifMac,ifPort,ifSpeed)
typedef struct{
	std::string ifIndex;
	std::string ifType;
	std::string ifDesc;
	std::string ifMac;
	std::string ifPort;
	std::string ifSpeed;
}IFREC;

//附加topo数据条目
typedef struct
{
	string localPortInx;//本端端口索引
	string localPortDsc;//本端端口描述
	string PeerID;      //对端设备ID
	string PeerIP;      //对端设备IP
	string PeerPortInx; //对端端口索引
	string PeerPortDsc; //对端端口描述
}DIRECTITEM;

//边结构
typedef struct{
	std::string ip_left;//左端点ip
	std::string pt_left;//左端点port
	std::string inf_left;//左端点inf
	std::string dsc_left;//左端点描述信息
	std::string ip_right;//右端点ip
	std::string pt_right;//右端点port
	std::string inf_right;//右端点inf
	std::string dsc_right;//右端点描述信息
}EDGE;

//路由条目
typedef struct{
	std::string dest_net;//目标网络
	std::string next_hop;//下一跳IP
	std::string dest_msk;//目标网络的掩码
}ROUTEITEM;

//BGP
typedef struct{
	std::string local_ip;
	std::string local_port;
	std::string peer_ip;
	std::string peer_port;
}BGPITEM;

typedef struct{
	string vrId;
	string masterIp;
	string primaryIp;
	string virtualMac;
}VRID;

//VRRP
typedef struct{
	vector<VRID> vrids;
	vector<string> assoips;
}VRRPITEM;

//by zhangyan 2009-01-09 (for vrrp,hsrp)
typedef struct
{
	vector<string> virtualIps;
	vector<string> virtualMacs;
}RouterStandbyItem;

typedef map<string, RouterStandbyItem> RouterStandby_LIST; //vrrp,hsrp等

//设备基本信息列表{devIP,(TYPE,SNMP,[IP],[MAC],[MASK],SYSOID,sysname)}
typedef std::map<std::string, IDBody> DEVID_LIST;
//设备接口属性列表 {devIP,(ifAmount,[(ifindex,ifType,ifDescr,ifMac,ifPort,ifSpeed)])}
typedef std::map<std::string, std::pair<std::string, std::list<IFREC> > > IFPROP_LIST;
//设备AFT数据列表 {sourceIP,{port,[MAC]}}
typedef std::map<std::string, std::map<std::string, std::list<std::string> > > AFT_LIST;
//设备ARP数据列表 {sourceIP,{infInx,[(MAC,destIP)]}}
typedef std::map<std::string, std::map<std::string, std::list<std::pair<std::string,std::string> > > > ARP_LIST;
//设备OSPF邻居列表 {sourceIP,{infInx,[destIP]}}
typedef std::map<std::string, std::map<std::string, std::list<std::string> > > OSPFNBR_LIST;
//设备路由表 {sourceIP,{infInx,[nextIP]}}
typedef std::map<std::string, std::map<std::string, std::list<ROUTEITEM> > > ROUTE_LIST;

//typedef map<string, VRRPITEM> VRRP_LIST;// added by tgf 20080922

typedef std::list<BGPITEM> BGP_LIST;
//设备路由表 {sourceIP,{infInx,[nextIP]}}
typedef std::map<std::string, std::map<std::string, std::list<std::string> > > ROUTE_LIST_FRM;
//规范化后的设备AFT或ARP数据 {sourceIP,{infInx,[destIP]}}
typedef std::map<std::string, std::map<std::string, std::list<std::string> > > FRM_AFTARP_LIST;

typedef std::map<std::string, std::list<DIRECTITEM> > DIRECTDATA_LIST;

//设备stp列表
typedef std::map<std::string,std::list<string> > STP_LIST; //add by jiangshanwen 20100910
//边列表
typedef std::list<EDGE> EDGE_LIST;

typedef std::list<std::pair<std::string,std::string> > SCALE_LIST;//[ip0,ip1]

// add by zhangyan 2008-08-27
//路由跟踪路径表 [[path1],[path2],...,[pathn]]
typedef std::list<list<string> > ROUTEPATH_LIST;

typedef struct {
	//缺省读共同体名
	std::string community_get_dft;
	//缺省写共同体名
	std::string community_set_dft;
	//超时时间
	unsigned int timeout;
	//线程数量
	unsigned int thrdamount;
	//扫描深度
	unsigned int depth;
	//重试次数
	unsigned int retrytimes;
	//<种子IP>
	std::list<std::string> scan_seeds;
	//<起始IP,结束IP>
        std::list<std::pair<std::string, std::string> > scan_scales;
        std::list<std::pair<unsigned long, unsigned long> > scan_scales_num;
        //<<范围>,<读写共同体名> >
        std::list<std::pair<std::pair<std::string, std::string>, std::pair<std::string,std::string> > > communitys;
        std::list<std::pair<std::pair<unsigned long, unsigned long>, std::pair<std::string,std::string> > > communitys_num;
	//排除的范围
        std::list<std::pair<std::string,std::string> > filter_scales;
        std::list<std::pair<unsigned long, unsigned long> > filter_scales_num;
}SCAN_PARAM;

typedef struct
{
	std::string scan_type;//是否从设备读数
	std::string seed_type;//种子扫描方式，是子网向下还是arp向下 add by wings 2009-12-15
	std::string ping_type;//是否ping
	std::string dumb_type;//是否创建哑设备
	std::string comp_type;//使用ARP数据补足AFT数据的操作类型
	std::string arp_read_type;//是否读取二层交换机的arp数据
	std::string nbr_read_type;//是否读取邻居表
	std::string rt_read_type; //是否读取路由表	恢复路由表 add by wings 2009-11-12
	std::string vrrp_read_type; //是否读取VRRP,HSRP  //added by zhangyan 2009-01-16
	std::string bgp_read_type; //是否读取BGP
	std::string snmp_version;	// 指定特定SNMP Version的设备
        list<pair<string, string> > SNMPV_list; // add by zhangyan 2009-01-04
	std::string tracert_type;//是否执行trace route  //add by zhangyan 2008-10-16
	std::string filter_type;//是否清除扫描范围外的ip  //add by zhangyan 2008-10-30
	std::string commit_pc;//是否提交PC到SVDB //add by zhangyan 2009-07-15
}AUX_PARAM;

typedef bool (*callback_functor_type)(string, string);

//特殊设备的oid集合 [{厂商oid,[{指标代码,指标oid}]}] add by jiangshanwen 2010-7-21 
typedef std::map<std::string, map<std::string, std::string> > SPECIAL_OID_LIST;


const string ROUTE_SWITCH = "0";
const string SWITCH = "1";
const string ROUTER = "2";
const string FIREWALL = "3";
const string SERVER = "4";
const string PC = "5";
const string OTHER = "6";


const string ERR_AFT_LOG = "Err_Aft_log.txt";
const string ERR_ARP_LOG = "Err_Arp_log.txt";
const string ERR_OID_LOG = "Err_Oid_log.txt";
const string ERR_INF_LOG = "Err_Inf_log.txt";
const string ERR_NBR_LOG = "Err_Nbr_log.txt";
const string ERR_RT_LOG  = "Err_Rt_log.txt";
const string ERR_BGP_LOG = "Err_Bgp_log.txt";
const string ERR_VRRP_LOG = "Err_Vrrp_log.txt";
const string ERR_DRC_LOG = "Err_DRC_log.txt";



#endif
