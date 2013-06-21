#include "StdAfx.h"
#include "CiscoDevice.h"
#include <QtDebug>
#include <iostream>
using namespace std;

CiscoDevice::CiscoDevice(void)
{
}

CiscoDevice::~CiscoDevice(void)
{
}

//获得逻辑共同体
void CiscoDevice::getLogicEntity(SnmpDG& snmp, const SnmpPara& spr)
{
	//逻辑共同体1.3.6.1.2.1.47.1.2.1.1.4 (ciscio4006不支持这个OID)
        list<pair<string,string> > entity_list = snmp.GetMibTable(spr,"1.3.6.1.2.1.47.1.2.1.1.4");
	if(entity_list.empty())
	{
		entity_list = snmp.GetMibTable(spr,"1.3.6.1.4.1.9.9.68.1.2.2.1.2");//取得Vlan号
		if(entity_list.empty())
		{
			return;
		}
                for(list<pair<string,string> >::iterator i = entity_list.begin(); i != entity_list.end(); ++i)
		{
			i->second = string(spr.community) + "@" + i->second;
		}
	}
	
        for(list<pair<string,string> >::iterator i = entity_list.begin(); i != entity_list.end(); ++i)
	{
		//added by zhangyan 2008-12-10
		string vlanID = i->second.substr(i->second.find("@") + 1);
		if(vlanID == "1" || vlanID == "1002" || vlanID == "1003" || vlanID == "1004" || vlanID == "1005")
		{
			continue;
		}
		if(find(list_cmty.begin(), list_cmty.end(), i->second) == list_cmty.end())
		{
			list_cmty.push_back(i->second);
			qDebug() << "comm : " << (i->second).c_str();
		}
	}
}
// 根据逻辑共同体获取转发表数据
void CiscoDevice::getAftByLogicEntity(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList)
{
	//remarked by zhangyan 2008-12-15
	////逻辑共同体1.3.6.1.2.1.47.1.2.1.1.4 (ciscio4006不支持这个OID)
        //list<pair<string,string> > entity_list = snmp.GetMibTable(spr,"1.3.6.1.2.1.47.1.2.1.1.4");
	//if(entity_list.empty())
	//{
	//	entity_list = snmp.GetMibTable(spr,"1.3.6.1.4.1.9.9.68.1.2.2.1.2");
	//	if(entity_list.empty())
	//	{
	//		return;
	//	}
        //	for(list<pair<string,string> >::iterator i = entity_list.begin(); i != entity_list.end(); ++i)
	//	{
	//		i->second = string(spr.community) + "@" + i->second;
	//	}
	//}
	//list<string> list_cmty;
        //for(list<pair<string,string> >::iterator i = entity_list.begin(); i != entity_list.end(); ++i)
	//{
	//	//added by zhangyan 2008-12-10
	//	string vlanID = i->second.substr(i->second.find("@") + 1);
	//	if(vlanID == "1002" || vlanID == "1003" || vlanID == "1004" || vlanID == "1005")
	//	{
	//		continue;
	//	}
	//	if(find(list_cmty.begin(), list_cmty.end(), i->second) == list_cmty.end())
	//	{
	//		list_cmty.push_back(i->second);
	//	}
	//}
	for(list<string>::iterator i = list_cmty.begin(); i != list_cmty.end(); ++i)
	{
	    qDebug() << "test cisco123";
		if(getAftByDtp(snmp, SnmpPara(spr.ip, *i, spr.timeout,spr.retry), oidIndexList))
			{
				qDebug() << "true";
			}
			else
				{
					qDebug() << "false";
				}
	}
}


// 获取特定设备的AFT数据{devip,{port,[mac]}}
AFT_LIST& CiscoDevice::getAftData(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList)
{
    qDebug() << "test cisco";
	aft_list.clear();
	port_macs.clear();
	getAftByDtp(snmp, spr, oidIndexList);//1.3.6.1.2.1.17.4.3.1.2
	getAftByLogicEntity(snmp, spr, oidIndexList);	
//	getAftByQtp(snmp, spr);//1.3.6.1.2.1.17.7.1.2.2.1.2

	if(!port_macs.empty())
	{
		aft_list.insert(make_pair(spr.ip, port_macs));
	}
	return aft_list;
}

//added by zhangyan 2008-12-04
RouterStandby_LIST& CiscoDevice::getHsrpData(SnmpDG& snmp, const SnmpPara& spr)
{	
	//routeStandby_list;.clear();
	//1.3.6.1.4.1.9.9.106.1.2.1.1.11 :cHsrpGrpVirtualIpAddr	
        list<pair<string,string> > hsrpvirip = snmp.GetMibTable(spr, "1.3.6.1.4.1.9.9.106.1.2.1.1.1");
	if(!hsrpvirip.empty())
	{
		//1.3.6.1.4.1.9.9.106.1.2.1.1.16 :cHsrpGrpVirtualMacAddr
                list<pair<string,string> > hsrpvirmacs = snmp.GetMibTable(spr, "1.3.6.1.4.1.9.9.106.1.2.1.1.16");
		////1.3.6.1.4.1.9.9.106.1.2.1.1.13 :cHsrpGrpActiveRouter  	
                //list<pair<string,string> > vrmasterips = snmp.GetMibTable(spr, "1.3.6.1.4.1.9.9.106.1.2.1.1.13");
		////1.3.6.1.4.1.9.9.106.1.2.1.1.14 :cHsrpGrpStandbyRouter
                //list<pair<string,string> > vrprimaryips = snmp.GetMibTable(spr, "1.3.6.1.4.1.9.9.106.1.2.1.1.14");
		
		RouterStandbyItem vrrpItem;
		//填充虚拟MAC
                for(list<pair<string,string> >::iterator i = hsrpvirmacs.begin(); i != hsrpvirmacs.end(); ++i)
		{			
			string mac = replaceAll(i->second, " ", "").substr(0, 12);
			vrrpItem.virtualMacs.push_back(mac);
		}
		//填充虚拟IP
                for(list<pair<string,string> >::iterator i = hsrpvirip.begin(); i != hsrpvirip.end(); ++i)
		{
			vrrpItem.virtualIps.push_back(i->second);						
		}

		routeStandby_list.insert(make_pair(spr.ip, vrrpItem));
	}
	return routeStandby_list;;
}


DIRECTDATA_LIST CiscoDevice::getDirectData(SnmpDG& snmp, const SnmpPara& spr)
{
	//cdp
	directdata_list.clear();
	//PeerIP 1.3.6.1.4.1.9.9.23.1.2.1.1.4(localportindex,peerip)
        list<pair<string,string> > peerIPs = snmp.GetMibTable(spr,"1.3.6.1.4.1.9.9.23.1.2.1.1.4");
	if(!peerIPs.empty())
	{
		//PeerPort 1.3.6.1.4.1.9.9.23.1.2.1.1.7(localportindex,peerport)
                list<pair<string,string> > peerPorts = snmp.GetMibTable(spr, "1.3.6.1.4.1.9.9.23.1.2.1.1.7");
		if(!peerPorts.empty())
		{
			std::list<DIRECTITEM> item_list; //[localportindex,peerid,peerip,peerportdesc]
                        for(list<pair<string,string> >::iterator iip = peerIPs.begin(); iip != peerIPs.end(); ++iip)
			{
				vector<string> ip_indexs = tokenize(iip->first, ".");
				if(ip_indexs.size() < 14)
				{
					continue;
				}
                                for(list<pair<string,string> >::iterator iport = peerPorts.begin(); iport != peerPorts.end(); ++iport)
				{
					vector<string> pt_indexs = tokenize(iport->first, ".");
					if(pt_indexs.size() < 14)
					{
						continue;
					}
					if(ip_indexs[14] == pt_indexs[14] && ip_indexs[15] == pt_indexs[15])
					{
						string ip_str = replaceAll(iip->second, " ","");
						if(ip_str.length() > 10)
						{
							string ip_tmp = int2str(str2int(ip_str.substr(0,2), 16)) + "." 
										  + int2str(str2int(ip_str.substr(2,2), 16)) + "."
										  + int2str(str2int(ip_str.substr(4,2), 16)) + "."
										  + int2str(str2int(ip_str.substr(6,2), 16));
							bool bNew = true;
							for(std::list<DIRECTITEM>::iterator item = item_list.begin();
								item != item_list.end();
								++item)
							{
								if(item->localPortInx == ip_indexs[14]
									&& item->PeerIP == ip_tmp
									&& item->PeerPortDsc == iport->second)
								{
									bNew = false;
									break;
								}
							}
							if(bNew)
							{
								DIRECTITEM item_tmp;
								item_tmp.localPortInx = ip_indexs[14];
								item_tmp.localPortDsc = "";
								item_tmp.PeerID = "";
								item_tmp.PeerIP = ip_tmp;
								item_tmp.PeerPortDsc = iport->second;
								item_tmp.PeerPortInx = "";
								item_list.push_back(item_tmp);
							}
						}
					}
				}
			}
			if(!item_list.empty())
			{
				directdata_list.insert(make_pair(spr.ip, item_list));
			}
		}
	}
	return directdata_list;
}

// added by zhangyan 2008-12-11
// 获取特定设备的接口信息
IFPROP_LIST& CiscoDevice::getInfProp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList, bool bRouter)
{
	ifprop_list.clear();
	//接口数量 1.3.6.1.2.1.2.1
	string infAmount = snmp.GetMibObject(spr, "1.3.6.1.2.1.2.1");	
	if(!infAmount.empty())
	{
		//接口类型 1.3.6.1.2.1.2.2.1.3
                list<pair<string,string> > infTypes = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.3");
		//接口描述 1.3.6.1.2.1.2.2.1.2
                list<pair<string,string> > infDescs = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.2");
		//端口接口对应关系1.3.6.1.2.1.17.1.4.1.2
                list<pair<string,string> > infPorts;
                list<pair<string,string> > infPortsEx;
		if(!bRouter)
		{
			getLogicEntity(snmp, spr);
			//用接口描述补充逻辑共同体列表
			list_cmty.push_back(string(spr.community) + "@0");//增加vlan0的逻辑共同体
                        for(list<pair<string,string> >::iterator idesc = infDescs.begin(); idesc != infDescs.end(); ++idesc)
			{
				string prefix_vlan = idesc->second.substr(0, 4);
                                std::transform(prefix_vlan.begin(), prefix_vlan.end(), prefix_vlan.begin(), (int(*)(int))toupper);
				if(prefix_vlan == "VLAN")
				{
					string vlanID = idesc->second.substr(4);
					if(vlanID == "1" || vlanID == "1002" || vlanID == "1003" || vlanID == "1004" || vlanID == "1005")
					{
						continue;
					}
					string community_tmp = string(spr.community) + "@" + vlanID;		
					if(find(list_cmty.begin(), list_cmty.end(), community_tmp) == list_cmty.end())
					{
						list_cmty.push_back(community_tmp);
						qDebug() << "commuinity_tmp : " << community_tmp.c_str();
					}
					break;
				}
			}
			
			//1.用原有的共同体读Port与Ifindex的对应关系
			infPorts= snmp.GetMibTable(spr,"1.3.6.1.2.1.17.1.4.1.2");
			list<string> ifindexList;
                        for(list<pair<string,string> >::iterator iinfPort = infPorts.begin(); iinfPort != infPorts.end(); ++iinfPort)
			{
				ifindexList.push_back(iinfPort->second);
			}	
			//2.用逻辑共同体补充Port与Ifindex的对应关系
			for(list<string>::iterator i = list_cmty.begin(); i != list_cmty.end(); ++i)
			{
                                list<pair<string,string> > infPorts_tmp = snmp.GetMibTable(SnmpPara(spr.ip, *i, spr.timeout, spr.retry), "1.3.6.1.2.1.17.1.4.1.2");
                                for(list<pair<string,string> >::iterator iinfPort = infPorts_tmp.begin(); iinfPort != infPorts_tmp.end(); ++iinfPort)
				{
					if(find(ifindexList.begin(), ifindexList.end(), iinfPort->second) == ifindexList.end())
					{
						infPorts.push_back(*iinfPort);
						ifindexList.push_back(iinfPort->second);
					}
				}				
			}						
			//cisco私有oid  Ifindex-Port
			infPortsEx= snmp.GetMibTable(spr,"1.3.6.1.4.1.9.9.276.1.5.1.1.1");			
		}
		//接口MAC地址 1.3.6.1.2.1.2.2.1.6
                list<pair<string,string> > infMacs = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.6");
		//接口速度 1.3.6.1.2.1.2.2.1.5
                list<pair<string,string> > infSpeeds = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.5");

		std::list<IFREC> infprops;//(ifindex,ifType,ifDescr,ifMac,ifPort,ifSpeed)
                for(list<pair<string,string> >::iterator itype = infTypes.begin(); itype != infTypes.end(); ++itype)
		{
			IFREC inf_tmp;
			inf_tmp.ifIndex = itype->first.substr(20);
			inf_tmp.ifType = itype->second;
                        for(list<pair<string,string> >::iterator idesc = infDescs.begin(); idesc != infDescs.end(); ++idesc)
			{
				if(idesc->first.substr(20) == inf_tmp.ifIndex)
				{
					inf_tmp.ifDesc = idesc->second;
					break;
				}
			}
			inf_tmp.ifPort = inf_tmp.ifIndex;
			bool reConvert = true;
                        for(list<pair<string,string> >::iterator iport = infPorts.begin(); iport != infPorts.end(); ++iport)
			{
				if(iport->second == inf_tmp.ifIndex)
				{
					inf_tmp.ifPort = iport->first.substr(23);
					reConvert = false;
					break;
				}
			}
			if (reConvert)
			{
                                for(list<pair<string,string> >::iterator iport = infPortsEx.begin(); iport != infPortsEx.end(); ++iport)
				{
					if(iport->first.substr(30) == inf_tmp.ifIndex)
					{
						inf_tmp.ifPort = iport->second;
						break;
					}
				}
			}
                        for(list<pair<string,string> >::iterator imac = infMacs.begin(); imac != infMacs.end(); ++imac)
			{
				if(imac->first.substr(20) == inf_tmp.ifIndex)
				{
					inf_tmp.ifMac = replaceAll(imac->second, " ","").substr(0,12);
                                        std::transform(inf_tmp.ifMac.begin(), inf_tmp.ifMac.end(), inf_tmp.ifMac.begin(), (int(*)(int))toupper);
					break;
				}
			}
                        for(list<pair<string,string> >::iterator ispeed = infSpeeds.begin(); ispeed != infSpeeds.end(); ++ispeed)
			{
				if(ispeed->first.substr(20) == inf_tmp.ifIndex)
				{
					inf_tmp.ifSpeed = ispeed->second;
					break;
				}
			}
			infprops.push_back(inf_tmp);
		}
		if(!infprops.empty())
		{
			ifprop_list.insert(make_pair(spr.ip,make_pair(infAmount,infprops)));
		}
	}
	return ifprop_list;
}

//add by jiangshanwen 20100910
STP_LIST& CiscoDevice::getStpData(SnmpDG& snmp, const SnmpPara& spr)
{
	stp_list.clear();
	for(list<string>::iterator i = list_cmty.begin(); i != list_cmty.end(); ++i)
	{
		STP_LIST::iterator p;
                list<pair<string,string> > stpport = snmp.GetMibTable(spr,"1.3.6.1.2.1.17.2.15.1.1");
                for (list<pair<string,string> >::iterator item = stpport.begin(); item != stpport.end(); ++item)
		{
			 p = stp_list.find(spr.ip);
			if (p != stp_list.end())
			{
				if (find(p->second.begin(),p->second.end(),item->second) == p->second.end())
				{
					p->second.push_back(item->second);
				}
			}
			else
			{
				std::list<std::string> portlist;
				portlist.push_back(item->second);
				stp_list.insert(make_pair(spr.ip, portlist));
			}
		}
	}
	return stp_list;
}
