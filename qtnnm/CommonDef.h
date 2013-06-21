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





//��Ϣ���ͼ���������			
#define START_SCAN_MSG   "START_SCAN"   //������Ϣ:	"NULL"                  [��ʼɨ��]
#define FIND_SUBNET_MSG  "FIND_SUBNET"  //������Ϣ:	"ip-ip"				    [��������(��ʼ��ַip-ip)]
#define SCAN_SUBNET_MSG  "SCAN_SUBNET"  //������Ϣ:	"ip-ip"				    [ɨ�跶Χ(����)(��ʼ��ַip-ip)]      
#define FOUND_DEVICE_MSG "FOUND_DEVICE" //������Ϣ:	"ip, typename, Factory" [�����豸(ip,����,����)[����pc]
#define SAVE_DEVICE_MSG	 "SAVE_DEVICE"  //������Ϣ:	"success(or fail) xxx"	[�����豸����: �ɹ�(����ʧ��)xxx̨]
//#define FINISHED_SUBNET_MSG "FINISHED_SUBNET"  //������Ϣ:	"ip-ip"			 [���ɨ�跶Χ(����)(��ʼ��ַip-ip)] 

#define TITLE_MSG       "TITLE_MSG"     //������Ϣ
#define TITLE_MSG_010   "10"            //���⸽����Ϣ:����ֹͣNnmMonitor����
#define TITLE_MSG_011   "11"            //���⸽����Ϣ:ֹͣNnmMonitor����ʧ��,��Ҫ�ֹ�ֹͣ
#define TITLE_MSG_012   "12"            //���⸽����Ϣ:��������NnmMonitor����
#define TITLE_MSG_013   "13"            //���⸽����Ϣ:����NnmMonitor����ʧ��,��Ҫ�ֹ�����

#define TITLE_MSG_100   "100"           //���⸽����Ϣ:û����Ч����
#define TITLE_MSG_110   "110"           //���⸽����Ϣ:�����������˷���
#define TITLE_MSG_120   "120"           //���⸽����Ϣ:��ʼ���˷���
#define TITLE_MSG_121   "121"           //���⸽����Ϣ:���˷����ɹ����
#define TITLE_MSG_122   "122"           //���⸽����Ϣ:���˷���ʧ��
#define TITLE_MSG_200   "200"           //���⸽����Ϣ:�����ύʧ��

#define COMMON_MSG        "COMMON_MSG"  //��ͨ��Ϣ
#define COMMON_MSG_1100   "1100"        //��ͨ������Ϣ:����û�п���SNMP����,���ᶪʧ�豸������

#define MINUS_THREAD_AMOUNT 200
//#define BATCH_SUBMIT_AMOUNT 10
#define BATCH_SUBMIT_AMOUNT 150 //by zhangyan
#define BATCH_SUBMIT_RETRYS 5



using namespace std;

typedef struct{
	string devType;     //�豸����
	string devTypeName; //�豸��������
	string devFac;      //�豸����
	string devModel;    //�豸�ͺ�
}DEVICE_PRO;
typedef std::map<std::string, DEVICE_PRO> DEVICE_TYPE_MAP;//<sysoid,<devType,devTypeName,devFac,devModel> >


//�豸������������ (TYPE,SNMP,[IP],[MAC],[MASK],SYSOID,sysname)
typedef struct {
	std::string baseMac;//����Mac��ַ
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
	std::vector<std::string> ips;//IP��ַ1.3.6.1.2.1.4.20.1.3
	std::vector<std::string> infinxs;//IP��ַ��Ӧ������1.3.6.1.2.1.4.20.1.2
	std::vector<std::string> msks;//MASK��ַ1.3.6.1.2.1.4.20.1.3
	std::vector<std::string> macs;//MAC��ַ1.3.6.1.2.1.2.2.1.6
}IDBody;

//�豸�ӿ�����(ifindex,ifType,ifDescr,ifMac,ifPort,ifSpeed)
typedef struct{
	std::string ifIndex;
	std::string ifType;
	std::string ifDesc;
	std::string ifMac;
	std::string ifPort;
	std::string ifSpeed;
}IFREC;

//����topo������Ŀ
typedef struct
{
	string localPortInx;//���˶˿�����
	string localPortDsc;//���˶˿�����
	string PeerID;      //�Զ��豸ID
	string PeerIP;      //�Զ��豸IP
	string PeerPortInx; //�Զ˶˿�����
	string PeerPortDsc; //�Զ˶˿�����
}DIRECTITEM;

//�߽ṹ
typedef struct{
	std::string ip_left;//��˵�ip
	std::string pt_left;//��˵�port
	std::string inf_left;//��˵�inf
	std::string dsc_left;//��˵�������Ϣ
	std::string ip_right;//�Ҷ˵�ip
	std::string pt_right;//�Ҷ˵�port
	std::string inf_right;//�Ҷ˵�inf
	std::string dsc_right;//�Ҷ˵�������Ϣ
}EDGE;

//·����Ŀ
typedef struct{
	std::string dest_net;//Ŀ������
	std::string next_hop;//��һ��IP
	std::string dest_msk;//Ŀ�����������
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

typedef map<string, RouterStandbyItem> RouterStandby_LIST; //vrrp,hsrp��

//�豸������Ϣ�б�{devIP,(TYPE,SNMP,[IP],[MAC],[MASK],SYSOID,sysname)}
typedef std::map<std::string, IDBody> DEVID_LIST;
//�豸�ӿ������б� {devIP,(ifAmount,[(ifindex,ifType,ifDescr,ifMac,ifPort,ifSpeed)])}
typedef std::map<std::string, std::pair<std::string, std::list<IFREC> > > IFPROP_LIST;
//�豸AFT�����б� {sourceIP,{port,[MAC]}}
typedef std::map<std::string, std::map<std::string, std::list<std::string> > > AFT_LIST;
//�豸ARP�����б� {sourceIP,{infInx,[(MAC,destIP)]}}
typedef std::map<std::string, std::map<std::string, std::list<std::pair<std::string,std::string> > > > ARP_LIST;
//�豸OSPF�ھ��б� {sourceIP,{infInx,[destIP]}}
typedef std::map<std::string, std::map<std::string, std::list<std::string> > > OSPFNBR_LIST;
//�豸·�ɱ� {sourceIP,{infInx,[nextIP]}}
typedef std::map<std::string, std::map<std::string, std::list<ROUTEITEM> > > ROUTE_LIST;

//typedef map<string, VRRPITEM> VRRP_LIST;// added by tgf 20080922

typedef std::list<BGPITEM> BGP_LIST;
//�豸·�ɱ� {sourceIP,{infInx,[nextIP]}}
typedef std::map<std::string, std::map<std::string, std::list<std::string> > > ROUTE_LIST_FRM;
//�淶������豸AFT��ARP���� {sourceIP,{infInx,[destIP]}}
typedef std::map<std::string, std::map<std::string, std::list<std::string> > > FRM_AFTARP_LIST;

typedef std::map<std::string, std::list<DIRECTITEM> > DIRECTDATA_LIST;

//�豸stp�б�
typedef std::map<std::string,std::list<string> > STP_LIST; //add by jiangshanwen 20100910
//���б�
typedef std::list<EDGE> EDGE_LIST;

typedef std::list<std::pair<std::string,std::string> > SCALE_LIST;//[ip0,ip1]

// add by zhangyan 2008-08-27
//·�ɸ���·���� [[path1],[path2],...,[pathn]]
typedef std::list<list<string> > ROUTEPATH_LIST;

typedef struct {
	//ȱʡ����ͬ����
	std::string community_get_dft;
	//ȱʡд��ͬ����
	std::string community_set_dft;
	//��ʱʱ��
	unsigned int timeout;
	//�߳�����
	unsigned int thrdamount;
	//ɨ�����
	unsigned int depth;
	//���Դ���
	unsigned int retrytimes;
	//<����IP>
	std::list<std::string> scan_seeds;
	//<��ʼIP,����IP>
        std::list<std::pair<std::string, std::string> > scan_scales;
        std::list<std::pair<unsigned long, unsigned long> > scan_scales_num;
        //<<��Χ>,<��д��ͬ����> >
        std::list<std::pair<std::pair<std::string, std::string>, std::pair<std::string,std::string> > > communitys;
        std::list<std::pair<std::pair<unsigned long, unsigned long>, std::pair<std::string,std::string> > > communitys_num;
	//�ų��ķ�Χ
        std::list<std::pair<std::string,std::string> > filter_scales;
        std::list<std::pair<unsigned long, unsigned long> > filter_scales_num;
}SCAN_PARAM;

typedef struct
{
	std::string scan_type;//�Ƿ���豸����
	std::string seed_type;//����ɨ�跽ʽ�����������»���arp���� add by wings 2009-12-15
	std::string ping_type;//�Ƿ�ping
	std::string dumb_type;//�Ƿ񴴽����豸
	std::string comp_type;//ʹ��ARP���ݲ���AFT���ݵĲ�������
	std::string arp_read_type;//�Ƿ��ȡ���㽻������arp����
	std::string nbr_read_type;//�Ƿ��ȡ�ھӱ�
	std::string rt_read_type; //�Ƿ��ȡ·�ɱ�	�ָ�·�ɱ� add by wings 2009-11-12
	std::string vrrp_read_type; //�Ƿ��ȡVRRP,HSRP  //added by zhangyan 2009-01-16
	std::string bgp_read_type; //�Ƿ��ȡBGP
	std::string snmp_version;	// ָ���ض�SNMP Version���豸
        list<pair<string, string> > SNMPV_list; // add by zhangyan 2009-01-04
	std::string tracert_type;//�Ƿ�ִ��trace route  //add by zhangyan 2008-10-16
	std::string filter_type;//�Ƿ����ɨ�跶Χ���ip  //add by zhangyan 2008-10-30
	std::string commit_pc;//�Ƿ��ύPC��SVDB //add by zhangyan 2009-07-15
}AUX_PARAM;

typedef bool (*callback_functor_type)(string, string);

//�����豸��oid���� [{����oid,[{ָ�����,ָ��oid}]}] add by jiangshanwen 2010-7-21 
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
