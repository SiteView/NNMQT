// StreamData.cpp

/*
 * Topo scan console program
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

#include "stdafx.h"
#include "StreamData.h"

#include <iostream> 
#include <fstream> 
#include <strstream> 
#include <string> 
#include <vector>

#include "scanutils.h"

using namespace std; 

#define SPLIT_TOP "[:::]"
#define SPLIT_MAIN "[::]"
#define SPLIT_SUB  "[:]"

StreamData::StreamData(void)
{
}

StreamData::~StreamData(void)
{
}

// 用文件保存设备标识体数据列表
bool StreamData::saveIDBodyData(const DEVID_LIST& devid_list, const string& cdpPrex)
{
//	ofstream output("DeviceInfos.txt",ios::out | ios::app);
	ofstream output((cdpPrex+"DeviceInfos.txt").c_str(),ios::out);
	for(DEVID_LIST::const_iterator i = devid_list.begin(); i != devid_list.end(); i++)
	{
		string line = i->first + SPLIT_MAIN; //管理IP
		IDBody id = i->second;
		line += id.snmpflag + SPLIT_MAIN;
		line += id.community_get + SPLIT_MAIN;//
		line += id.community_set + SPLIT_MAIN;//
		line += id.sysOid + SPLIT_MAIN;
		line += id.devType + SPLIT_MAIN; //设备类型
		line += id.devFactory + SPLIT_MAIN; //设备厂家
		line += id.devModel + SPLIT_MAIN; //设备型号
		line += id.devTypeName + SPLIT_MAIN; //设备类型名称
		line += id.baseMac + SPLIT_MAIN; //基本Mac地址
		line += id.sysName + SPLIT_MAIN;
		line += id.sysSvcs + SPLIT_MAIN;
		if(!id.ips.empty())
		{
			std::vector<std::string>::iterator ip_j_end = id.ips.end();
			ip_j_end--;
			std::vector<std::string>::const_iterator ip_j, msk_j, inx_j;
			for(ip_j = id.ips.begin(), msk_j = id.msks.begin(), inx_j = id.infinxs.begin();
				ip_j != id.ips.end() && msk_j != id.msks.end() && inx_j != id.infinxs.end(); 
				ip_j++, msk_j++, inx_j++)
			{//ip-mask-index
				line += *ip_j + "/" + *msk_j + "/" + *inx_j;
				if(ip_j != ip_j_end)
				{
					line += SPLIT_SUB;
				}
			}
		}
		line += SPLIT_MAIN;
		if(!id.macs.empty())
		{
			std::vector<std::string>::iterator mac_j_end = id.macs.end();
			mac_j_end--;
			for(std::vector<std::string>::const_iterator mac_j = id.macs.begin();
				mac_j != id.macs.end(); 
				mac_j++)
			{//macs
				line += *mac_j;
				if(mac_j != mac_j_end)
				{
					line += SPLIT_SUB;
				}
			}
		}
		output << line << endl;
	}
	output.close();

	return true;
}

// 保存原始aft数据列表
bool StreamData::saveAftList(const AFT_LIST& aft_list)
{
	ofstream output("Aft_ORG.txt",ios::out);
	string line = "";
	for(AFT_LIST::const_iterator i = aft_list.begin(); i != aft_list.end(); i++)
	{
		if(i->second.empty())
		{
			continue;
		}
		line = i->first + "::"; //管理IP
		std::map<std::string, std::list<std::string> >::const_iterator j_end = i->second.end();
		j_end--;
		for(std::map<std::string, std::list<std::string> >::const_iterator j = i->second.begin();
			j != i->second.end();
			j++)
		{//port 循环
			line += j->first + ":";
			std::list<std::string>::const_iterator k_end =  j->second.end();
			k_end--;
			for(std::list<std::string>::const_iterator k = j->second.begin(); 
				k != j->second.end(); 
				k++)
			{//mac循环
				line += *k;
				if(k != k_end)
				{
					line += ",";
				}
			}
			if(j != j_end)
			{
				line += ";";
			}
		}
		output << line << endl;
	}
	output.close();
	return true;
}

// 保存arp数据列表
bool StreamData::saveArpList(const ARP_LIST& art_list)
{
	ofstream output("Arp_ORG.txt",ios::out);
	string line = "";
	for(ARP_LIST::const_iterator i = art_list.begin(); i != art_list.end(); i++)
	{
		if(i->second.empty())
		{
			continue;
		}
		line = i->first + "::"; //管理IP
                std::map<std::string, std::list<pair<std::string,std::string> > >::const_iterator j_end = i->second.end();
		j_end--;
                for(std::map<std::string, std::list<pair<std::string,std::string> > >::const_iterator j = i->second.begin();
			j != i->second.end();
			j++)
		{//port 循环
			line += j->first + ":";
                        std::list<pair<std::string,std::string> >::const_iterator k_end =  j->second.end();
			k_end--;
                        for(std::list<pair<std::string,std::string> >::const_iterator k = j->second.begin();
				k != j->second.end(); 
				k++)
			{//mac循环
				line += k->first + "-" + k->second;
				if(k != k_end)
				{
					line += ",";
				}
			}
			if(j != j_end)
			{
				line += ";";
			}
		}
		output << line << endl;
	}
	output.close();
	return true;
}

// 读取设备ip列表
bool StreamData::readDeviceIpList(vector<string>& devip_list)
{
	ifstream input("DeviceIps.txt",ios::in);
	devip_list.clear();
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input,line);
			line = replaceAll(line, " ","");			
			if(!line.empty())
			{
				devip_list.push_back(line);
			}
			if(input.eof()) break; 
		} 
		input.close();
		return true;
    }
    else
	{
        cout << "Open DeviceIps.txt error!" << endl; 
		return false;
	}
}

// add by wings 2009-11-26
// 读取默认交换机设备ip
bool StreamData::readSwIpList(vector<string>& swip_list)
{
	ifstream input("SwIps.txt",ios::in);
	swip_list.clear();
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input,line);
			line = replaceAll(line, " ","");			
			if(!line.empty())
			{
				swip_list.push_back(line);
			}
			if(input.eof()) break; 
		} 
		input.close();
		return true;
    }
    else
	{
        cout << "Open DeviceIps.txt error!" << endl; 
		return false;
	}
}

// 保存Ospf邻居数据列表
bool StreamData::saveOspfNbrList(const OSPFNBR_LIST& ospfnbr_list)
{
	ofstream output("OspfNbr_ORG.txt",ios::out);
	string line = "";
	for(OSPFNBR_LIST::const_iterator i = ospfnbr_list.begin(); i != ospfnbr_list.end(); i++)
	{
		if(i->second.empty())
		{
			continue;
		}
		line = i->first + "::"; //管理IP
		std::map<std::string, std::list<std::string> >::const_iterator j_end = i->second.end();
		j_end--;
		for(std::map<std::string, std::list<std::string> >::const_iterator j = i->second.begin();
			j != i->second.end();
			j++)
		{//port 循环
			line += j->first + ":";
			std::list<std::string>::const_iterator k_end =  j->second.end();
			k_end--;
			for(std::list<std::string>::const_iterator k = j->second.begin(); 
				k != j->second.end(); 
				k++)
			{//ip循环
				line += *k;
				if(k != k_end)
				{
					line += ",";
				}
			}
			if(j != j_end)
			{
				line += ";";
			}
		}
		output << line << endl;
	}
	output.close();
	return true;
}


// 读取Ospf邻居数据列表
bool StreamData::readOspfNbrList(OSPFNBR_LIST& ospfnbr_list)
{
	ifstream input("OspfNbr_ORG.txt",ios::in);
	ospfnbr_list.clear();
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input,line);
			if(!line.empty())
			{
				vector<string> vstr = tokenize(line, "::");
				if(vstr.size() == 2)
				{
					string ip = vstr[0];
					if(!vstr[1].empty())
					{//存在端口集合
                                                map<string, list<string> > map_ps;
						vector<string> vpms = tokenize(vstr[1],";");
						for(vector<string>::iterator i = vpms.begin();
							i != vpms.end();
							i++)
						{//ports
							if(!(*i).empty())
							{
								vector<string> vps = tokenize(*i, ":");
								string port = vps[0];
								if(!vps[1].empty())
								{//ips
									vector<string> ips = tokenize(vps[1], ",");
									list<string> ip_list;
									for(vector<string>::iterator j = ips.begin();
										j != ips.end();
										++j)
									{
										if(find(ip_list.begin(), ip_list.end(), *j) == ip_list.end())
										{
											ip_list.push_back(*j);
										}
									}
									if(!ip_list.empty())
									{
										map_ps.insert(make_pair(port, ip_list));
									}
								}
							}
						}
						ospfnbr_list.insert(make_pair(ip, map_ps));
					}
				}
			}
			if(input.eof()) break; 
		} 
		input.close();
    }
    else
        cout << "Open OspfNbr_ORG.txt eroor!" << endl; 
	return true;
}

// 保存Route数据列表
bool StreamData::saveRouteList(const ROUTE_LIST& route_list)
{
	ofstream output("Route_ORG.txt",ios::out);
	string line = "";
	for(ROUTE_LIST::const_iterator i = route_list.begin(); i != route_list.end(); i++)
	{
		if(i->second.empty())
		{
			continue;
		}
		line = i->first + "::"; //管理IP
		std::map<std::string, std::list<ROUTEITEM> >::const_iterator j_end = i->second.end();
		j_end--;
		for(std::map<std::string, std::list<ROUTEITEM> >::const_iterator j = i->second.begin();
			j != i->second.end();
			j++)
		{//port 循环
			line += j->first + ":";
			std::list<ROUTEITEM>::const_iterator k_end =  j->second.end();
			k_end--;
			for(std::list<ROUTEITEM>::const_iterator k = j->second.begin(); 
				k != j->second.end(); 
				k++)
			{//ip循环
				line += k->next_hop + "/" + k->dest_net + "/" + k->dest_msk;
				if(k != k_end)
				{
					line += ",";
				}
			}
			if(j != j_end)
			{
				line += ";";
			}
		}
		output << line << endl;
	}
	output.close();
	return true;
}
// 读取Route数据列表
bool StreamData::readRouteList(ROUTE_LIST& route_list)
{
	ifstream input("Route_ORG.txt",ios::in);
	route_list.clear();
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input,line);
			if(!line.empty())
			{
				vector<string> vstr = tokenize(line, "::");
				if(vstr.size() == 2)
				{
					string ip = vstr[0];
					if(!vstr[1].empty())
					{//存在端口集合
                                                map<string, list<ROUTEITEM> > map_ps;
						vector<string> vpms = tokenize(vstr[1],";");
						for(vector<string>::iterator i = vpms.begin();
							i != vpms.end();
							i++)
						{//ports
							if(!(*i).empty())
							{
								vector<string> vps = tokenize(*i, ":");
								string port = vps[0];
								if(!vps[1].empty())
								{//ips
									vector<string> ips = tokenize(vps[1], ",");
									list<ROUTEITEM> ip_list;
									for(vector<string>::iterator j = ips.begin();
										j != ips.end();
										++j)
									{
										//line += k->next_hop + "/" + k->dest_net + "/" + k->dest_msk;
										vector<string> dests = tokenize(*j, "/");
										ROUTEITEM rtitem;
										if(dests.size() == 3)
										{
											rtitem.next_hop = dests[0];
											rtitem.dest_net = dests[1];
											rtitem.dest_msk = dests[2];
										}
										else
										{
											rtitem.next_hop = dests[0];
											rtitem.dest_net = "";
											rtitem.dest_msk = "";
										}
										ip_list.push_back(rtitem);
									}
									if(!ip_list.empty())
									{
										map_ps.insert(make_pair(port, ip_list));
									}
								}
							}
						}
						route_list.insert(make_pair(ip, map_ps));
					}
				}
			}
			if(input.eof()) break; 
		} 
		input.close();
    }
    else
        cout << "打开文件：Route_ORG.txt" << " 时出错！" << endl; 
	return true;
}

// 保存bgp数据列表
bool StreamData::saveBgpList(const BGP_LIST& bgp_list)
{
	ofstream output("Bgp_ORG.txt",ios::out);
	string line = "";
	for(BGP_LIST::const_iterator i = bgp_list.begin(); i != bgp_list.end(); i++)
	{
		line = i->local_ip + "::" + i->local_port + "::"  + i->peer_ip + "::" + i->peer_port;
		output << line << endl;
	}
	output.close();
	return true;
}

// add by zhangyan 2008-08-29
// 保存规范化后的bgp表
bool StreamData::saveFrmBgpList(const BGP_LIST& bgp_list_frm)
{
	ofstream output("Bgp_FRM.txt",ios::out);
	string line = "";
	for(BGP_LIST::const_iterator i = bgp_list_frm.begin(); i != bgp_list_frm.end(); i++)
	{
		line = i->local_ip + "::" + i->local_port + "::"  + i->peer_ip + "::" + i->peer_port;
		output << line << endl;
	}
	output.close();
	return true;
}

// 读取bgp邻居数据列表
bool StreamData::readBgpList(BGP_LIST& bgp_list)
{
	ifstream input("Bgp_ORG.txt",ios::in);
	bgp_list.clear();
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input,line);
			if(!line.empty())
			{
				vector<string> vstr = tokenize(line, "::");
				if(vstr.size() == 4)
				{
					BGPITEM bgpitem;
					bgpitem.local_ip = vstr[0];
					bgpitem.local_port = vstr[1];
					bgpitem.peer_ip = vstr[2];
					bgpitem.peer_port = vstr[3];
					bgp_list.push_back(bgpitem);
				}
			}
			if(input.eof()) break; 
		} 
		input.close();
    }
    else
        cout << "Open Bgp_ORG.txt error!" << endl; 
	return true;
}

bool StreamData::saveVrrpList(const RouterStandby_LIST& vrrp_list)
{
	ofstream output("Vrrp_ORG.txt",ios::out);
	string line = "";	
	for(RouterStandby_LIST::const_iterator i = vrrp_list.begin(); i != vrrp_list.end(); i++)
	{
		line = i->first + SPLIT_TOP;
		vector<string>::const_iterator j_end = i->second.virtualIps.end();
		j_end--;
		for(vector<string>::const_iterator j = i->second.virtualIps.begin(); j != j_end; ++j)
		{
			line += *j + SPLIT_SUB;
		}
		line += *j_end + SPLIT_MAIN;

		j_end = i->second.virtualMacs.end();
		j_end--;
		for(vector<string>::const_iterator j = i->second.virtualMacs.begin(); j != i->second.virtualMacs.end(); ++j)
		{
			line += *j + SPLIT_SUB;
		}
		line += *j_end;
		output << line << endl;
	}
	output.close();
	return true;
}

bool StreamData::readVrrpList(RouterStandby_LIST& vrrp_list)
{
	ifstream input("Vrrp_ORG.txt",ios::in);
	vrrp_list.clear();
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input,line);
			if(!line.empty())
			{
				vector<string> vstr = tokenize(line, SPLIT_TOP);
				if(vstr.size() == 2)
				{
					RouterStandbyItem vrrpItem;	
					vector<string> vipmacs = tokenize(vstr[1], SPLIT_MAIN);
					if(vipmacs.size() == 2)
					{
						vector<string> vips = tokenize(vipmacs[0], SPLIT_SUB);
						vector<string> vmacs = tokenize(vipmacs[1], SPLIT_SUB);
						vrrpItem.virtualIps.assign(vips.begin(), vips.end());
						vrrpItem.virtualMacs.assign(vmacs.begin(), vmacs.end());
					}

					vrrp_list[vstr[0]] = vrrpItem;
				}
			}
			if(input.eof()) break; 
		} 
		input.close();
    }
    else
        cout << "Open file Vrrp_ORG.txt error!" << endl; 
	return true;
}

// add by zhangyan 2008-09-05
// 保存重新得到的Trace route数据列表
bool StreamData::saveRenewTracertList(const ROUTEPATH_LIST& tracert_list)
{
	ofstream output("Tracert_renew.txt",ios::out);
	string line;
	for (ROUTEPATH_LIST::const_iterator iter = tracert_list.begin(); iter != tracert_list.end(); iter++)
	{
		line = "";
		for (list<string>::const_iterator i = iter->begin(); i != iter->end(); ++i)
		{
			 line += *i + ",";
		}
		line = line.substr(0, line.length() - 1);
		output << line << endl;	
	}
	output.close();
	return true;
}

// add by zhangyan 2008-08-27
// 保存Trace route数据列表
bool StreamData::saveTracertList(const ROUTEPATH_LIST& tracert_list)
{
	ofstream output("Tracert.txt",ios::out);
	string line;
	for (ROUTEPATH_LIST::const_iterator iter = tracert_list.begin(); iter != tracert_list.end(); iter++)
	{
		line = "";
		for (list<string>::const_iterator i = iter->begin(); i != iter->end(); ++i)
		{
			 line += *i + ",";
		}
		line = line.substr(0, line.length() - 1);
		output << line << endl;	
	}
	output.close();
	return true;
}

// add by zhangyan 2008-08-27
// 读取Trace route数据列表
bool StreamData::readTracertList(ROUTEPATH_LIST& tracert_list)
{
	ifstream input("Tracert.txt",ios::in);
	tracert_list.clear();
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input,line);
			if (!line.empty())
			{
				vector<string> vstr = tokenize(line, ",");
				list<string> lstr;
				lstr.assign(vstr.begin(),vstr.end());
				tracert_list.push_back(lstr);				
			}
			if (input.eof()) break; 
		} 
		input.close();
    }
    else
	{
        cout << "打开文件：Tracert.txt" << " 时出错！" << endl; 
	}
	
	// add by zhangyan 2008-09-05
	//打开Tracert_renew.txt
	ifstream re_input("Tracert_renew.txt", ios::in);	
    if (re_input.is_open())
    {
		while(1) 
		{ 
			getline(re_input, line);
			if (!line.empty())
			{
				vector<string> vstr = tokenize(line, ",");
				list<string> lstr;
				lstr.assign(vstr.begin(), vstr.end());
				tracert_list.push_back(lstr);				
			}
			if (re_input.eof()) break; 
		} 
		re_input.close();
    }    
	return true;
}

bool StreamData::saveTraceDestData(const list<pair<pair<string, string>, string> > RouteIPPairList)
{
	ofstream output("TracertDest.txt",ios::out);
	string line = "";
        for (list<pair<pair<string, string>, string> >::const_iterator i = RouteIPPairList.begin(); i != RouteIPPairList.end(); i++)
	{
		line = i->first.first + "::" + i->first.second + "::" + i->second;
		output << line << endl;
	}
	output.close();
	return true;
}

// 保存接口数据列表
bool StreamData::saveInfPropList(const IFPROP_LIST& inf_list, const string& cdpPrex)
{
	ofstream output((cdpPrex+"InfProps.txt").c_str(),ios::out);
	string line = "";
	for(IFPROP_LIST::const_iterator i = inf_list.begin(); i != inf_list.end(); i++)
	{
		if(i->second.second.empty())
		{
			continue;
		}
		line = i->first + SPLIT_TOP; //管理IP
		line += i->second.first + SPLIT_TOP;//接口数量
		std::list<IFREC>::const_iterator j_end = i->second.second.end();
		j_end--;
		for(std::list<IFREC>::const_iterator j = i->second.second.begin();
			j != i->second.second.end();
			j++)
		{//inf 循环
			line += j->ifIndex + SPLIT_SUB;//接口索引
			line += j->ifType + SPLIT_SUB;//接口类型
			line += j->ifMac + SPLIT_SUB;//接口Mac
			line += j->ifPort + SPLIT_SUB;//接口端口
			line += j->ifDesc + SPLIT_SUB;//接口描述
			line += j->ifSpeed;
			if(j != j_end)
			{
				line += SPLIT_MAIN;
			}
		}
		output << line << endl;
	}
	output.close();
	return true;
}

// 用文件保存设备标识体数据列表
bool StreamData::saveFrmDevIDList(const DEVID_LIST& devid_list, const string& cdpPrex)
{
//	ofstream output("DeviceInfos.txt",ios::out | ios::app);
	ofstream output((cdpPrex+"DeviceInfos_Frm.txt").c_str(),ios::out);
	for(DEVID_LIST::const_iterator i = devid_list.begin(); i != devid_list.end(); i++)
	{
		string line = i->first + SPLIT_MAIN; //管理IP
		IDBody id = i->second;
		line += id.snmpflag + SPLIT_MAIN;
		line += id.community_get + SPLIT_MAIN;//
		line += id.community_set + SPLIT_MAIN;//
		line += id.sysOid + SPLIT_MAIN;
		line += id.devType + SPLIT_MAIN; //设备类型
		line += id.devFactory + SPLIT_MAIN; //设备厂家
		line += id.devModel + SPLIT_MAIN; //设备型号
		line += id.devTypeName + SPLIT_MAIN; //设备类型名称
		line += id.baseMac + SPLIT_MAIN; //基本Mac地址
		line += id.sysName + SPLIT_MAIN;
		line += id.sysSvcs + SPLIT_MAIN;
		if(!id.ips.empty())
		{
			std::vector<std::string>::iterator ip_j_end = id.ips.end();
			ip_j_end--;
			std::vector<std::string>::const_iterator ip_j, msk_j, inf_j;
			for(ip_j = id.ips.begin(), msk_j = id.msks.begin(), inf_j = id.infinxs.begin();
				ip_j != id.ips.end() && msk_j != id.msks.end() && inf_j != id.infinxs.end(); 
				ip_j++, msk_j++, inf_j++)
			{//ip-mask
				line += *ip_j + "/" + *msk_j + "/" + *inf_j;
				if(ip_j != ip_j_end)
				{
					line += SPLIT_SUB;
				}
			}
		}
		line += SPLIT_MAIN;
		if(!id.macs.empty())
		{
			std::vector<std::string>::iterator mac_j_end = id.macs.end();
			mac_j_end--;
			for(std::vector<std::string>::const_iterator mac_j = id.macs.begin();
				mac_j != id.macs.end(); 
				mac_j++)
			{//macs
				line += *mac_j;
				if(mac_j != mac_j_end)
				{
					line += SPLIT_SUB;
				}
			}
		}
		output << line << endl;
	}
	output.close();

	return true;
}

// 保存规范化后的转发表
bool StreamData::saveFrmAftList(const FRM_AFTARP_LIST& aft_list_frm)
{
	ofstream output("Aft_FRM.txt",ios::out);
	string line = "";
	for(FRM_AFTARP_LIST::const_iterator i = aft_list_frm.begin(); i != aft_list_frm.end(); i++)
	{
		if(i->second.empty())
		{
			continue;
		}
		line = i->first + "::"; //管理IP
		std::map<std::string, std::list<std::string> >::const_iterator j_end = i->second.end();
		j_end--;
		for(std::map<std::string, std::list<std::string> >::const_iterator j = i->second.begin();
			j != i->second.end();
			j++)
		{//port 循环
			line += j->first + ":";
			std::list<std::string>::const_iterator k_end =  j->second.end();
			k_end--;
			for(std::list<std::string>::const_iterator k = j->second.begin(); 
				k != j->second.end(); 
				k++)
			{//ip循环
				line += *k;
				if(k != k_end)
				{
					line += ",";
				}
			}
			if(j != j_end)
			{
				line += ";";
			}
		}
		output << line << endl;
	}
	output.close();
	return true;
}


bool StreamData::saveFrmRtList(const ROUTE_LIST_FRM& rt_list_frm)
{
	ofstream output("Rt_FRM.txt",ios::out);
	string line = "";
	for(ROUTE_LIST_FRM::const_iterator i = rt_list_frm.begin(); i != rt_list_frm.end(); i++)
	{
		if(i->second.empty())
		{
			continue;
		}
		line = i->first + "::"; //管理IP
		std::map<std::string, std::list<std::string> >::const_iterator j_end = i->second.end();
		j_end--;
		for(std::map<std::string, std::list<std::string> >::const_iterator j = i->second.begin();
			j != i->second.end();
			j++)
		{//port 循环
			line += j->first + ":";
			std::list<std::string>::const_iterator k_end =  j->second.end();
			k_end--;
			for(std::list<std::string>::const_iterator k = j->second.begin(); 
				k != j->second.end(); 
				k++)
			{//ip循环
				line += *k;
				if(k != k_end)
				{
					line += ",";
				}
			}
			if(j != j_end)
			{
				line += ";";
			}
		}
		output << line << endl;
	}
	output.close();
	return true;
}


bool StreamData::saveEdge_SSData(const EDGE_LIST& edge_list) 
{
	ofstream output("test/edge_ss.txt",ios::out);
	for(EDGE_LIST::const_iterator i = edge_list.begin(); i != edge_list.end(); i++)
	{
		string line = i->ip_left+":"+i->inf_left+"--"+i->ip_right+":"+i->inf_right+";";
		output << line << endl;
	}
	output.close();
	return true;
}

//add by wings for test 2009-10-29

bool StreamData::saveFrmSSAftList(const FRM_AFTARP_LIST& aft_list_frm,int test)
{
	char mac_str[30] = "";
        sprintf(mac_str,"test/Aft_FRM_SS_%d.txt",test);
        //SvLog::writeLog(mac_str);
	ofstream output(mac_str,ios::out);
	string line = "";
	for(FRM_AFTARP_LIST::const_iterator i = aft_list_frm.begin(); i != aft_list_frm.end(); i++)
	{
		if(i->second.empty())
		{
			continue;
		}
		line = i->first + "::"; //管理IP
		std::map<std::string, std::list<std::string> >::const_iterator j_end = i->second.end();
		j_end--;
		for(std::map<std::string, std::list<std::string> >::const_iterator j = i->second.begin();
			j != i->second.end();
			j++)
		{//port 循环
			if(j->second.empty())
				continue;
			line += j->first + ":";
			std::list<std::string>::const_iterator k_end =  j->second.end();
			k_end--;
			for(std::list<std::string>::const_iterator k = j->second.begin(); 
				k != j->second.end(); 
				k++)
			{//ip循环
				line += *k;
				if(k != k_end)
				{
					line += ",";
				}
			}
			if(j != j_end)
			{
				line += ";";
			}
		}
		output << line << endl;
	}
	output.close();
	return true;
}

// 保存规范化后的arp数据列表
bool StreamData::saveFrmArpList(const FRM_AFTARP_LIST& arp_list_frm)
{
	ofstream output("Arp_FRM.txt",ios::out);
	string line = "";
	for(FRM_AFTARP_LIST::const_iterator i = arp_list_frm.begin(); i != arp_list_frm.end(); i++)
	{
		if(i->second.empty())
		{
			continue;
		}
		line = i->first + "::"; //管理IP
		std::map<std::string, std::list<std::string> >::const_iterator j_end = i->second.end();
		j_end--;
		for(std::map<std::string, std::list<std::string> >::const_iterator j = i->second.begin();
			j != i->second.end();
			j++)
		{//port 循环
			line += j->first + ":";
			std::list<std::string>::const_iterator k_end =  j->second.end();
			k_end--;
			for(std::list<std::string>::const_iterator k = j->second.begin(); 
				k != j->second.end(); 
				k++)
			{//ip循环
				line += *k;
				if(k != k_end)
				{
					line += ",";
				}
			}
			if(j != j_end)
			{
				line += ";";
			}
		}
		output << line << endl;
	}
	output.close();
	return true;
}

// 保存原始的CDP数据列表
bool StreamData::saveDirectData(const DIRECTDATA_LIST& directdata_list)
{
	ofstream output("Direct_Data.txt", ios::out);
	string line = "";
	for(DIRECTDATA_LIST::const_iterator i = directdata_list.begin(); i != directdata_list.end(); i++)
	{
		if(i->second.empty())
		{
			continue;
		}
		line = i->first + SPLIT_TOP; //管理IP
		std::list<DIRECTITEM>::const_iterator j_end = i->second.end();
		j_end--;
		for(std::list<DIRECTITEM>::const_iterator j = i->second.begin();
			j != i->second.end();
			j++)
		{//inf 循环
			line += j->localPortInx + SPLIT_SUB;
			line += j->localPortDsc + SPLIT_SUB;
			line += j->PeerID + SPLIT_SUB;
			line += j->PeerIP + SPLIT_SUB;
			line += j->PeerPortInx + SPLIT_SUB;
			line += j->PeerPortDsc;
			if(j != j_end)
			{
				line += SPLIT_MAIN;
			}
		}
		output << line << endl;
	}
	output.close();
	return true;
}


// 从文件读取设备标识体数据列表
bool StreamData::readIDBodyData(DEVID_LIST& devid_list, const string& cdpPrex)
{
	ifstream input((cdpPrex+"DeviceInfos.txt").c_str(),ios::in);
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input, line);
			if(!line.empty())
			{
				vector<string> vstr = tokenize(line, SPLIT_MAIN);
				if(vstr.size() == 14)
				{
					string ip = vstr[0]; //管理IP
					IDBody id;
					id.snmpflag = vstr[1];//
					id.community_get = vstr[2];
					id.community_set = vstr[3]; 
					id.sysOid = vstr[4];
					id.devType = vstr[5]; //设备类型
					id.devFactory = vstr[6]; //设备厂家
					id.devModel = vstr[7]; //设备型号
					id.devTypeName = vstr[8]; //设备类型名称
					id.baseMac = vstr[9]; //基本Mac地址
					id.sysName = vstr[10];
					id.sysSvcs = vstr[11];
					if(!vstr[12].empty())
					{
						vector<string> vipmsks = tokenize(vstr[12], SPLIT_SUB);
						for(vector<string>::iterator i = vipmsks.begin();
							i != vipmsks.end();
							++i)
						{
							if(!(*i).empty())
							{
								vector<string> ipmskinx_tmp = tokenize(*i, "/");
								id.ips.push_back(ipmskinx_tmp[0]);
								id.msks.push_back(ipmskinx_tmp[1]);
								if(ipmskinx_tmp.size() == 3)
								{
									id.infinxs.push_back(ipmskinx_tmp[2]);
								}
								else
								{
									id.infinxs.push_back(string("0"));
								}
							}
						}
					}
					if(!vstr[13].empty())
					{
						vector<string> vmacs = tokenize(vstr[13], SPLIT_SUB);
						for(vector<string>::iterator i = vmacs.begin();
							i != vmacs.end();
							++i)
						{
							id.macs.push_back(*i);
						}
					}
					devid_list.insert(make_pair(ip,id));
				}
			}
			if(input.eof()) break;
		} 
		input.close();
    }
    else
        cout << "打开文件：deviceInfos.txt" << " 时出错！" << endl; 
	return true;
}

// 读取原始的转发表数据
bool StreamData::readAftList(AFT_LIST& aftlist)
{
	ifstream input("Aft_ORG.txt",ios::in);
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input, line);
			if(!line.empty())
			{
				vector<string> vstr = tokenize(line, "::");
				if(vstr.size() == 2)
				{
					string ip = vstr[0];
					if(!vstr[1].empty())
					{//存在端口集合
                                                map<string, list<string> > map_ps;
						vector<string> vpms = tokenize(vstr[1],";");
						for(vector<string>::iterator i = vpms.begin();
							i != vpms.end();
							i++)
						{//ports
							if(!(*i).empty())
							{
								vector<string> vps = tokenize(*i, ":");
								string port = vps[0];
								if(!vps[1].empty())
								{//macs
									vector<string> macs = tokenize(vps[1], ",");
									list<string> mac_list;
									for(vector<string>::iterator j = macs.begin();
										j != macs.end();
										++j)
									{
										mac_list.push_back(*j);
									}
									map_ps.insert(make_pair(port, mac_list));
								}
							}
						}
						aftlist.insert(make_pair(ip, map_ps));
					}
				}
			}
			if(input.eof()) break; 
		} 
		input.close();
    }
    else
        cout << "打开文件：AFT_ORG.txt" << " 时出错！" << endl; 

	return true;
}

// 读取原始的ARP数据列表
bool StreamData::readArpList(ARP_LIST& arplist)
{
	ifstream input("Arp_ORG.txt",ios::in);
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input,line);
			if(!line.empty())
			{
				vector<string> vstr = tokenize(line, "::");
				if(vstr.size() == 2)
				{
					string ip = vstr[0];
					if(!vstr[1].empty())
					{//存在端口集合
                                                map<string, list<pair<string,string> > > map_ps;
						vector<string> vpms = tokenize(vstr[1],";");
						for(vector<string>::iterator i = vpms.begin();
							i != vpms.end();
							i++)
						{//ports
							if(!(*i).empty())
							{
								vector<string> vps = tokenize(*i, ":");
								string port = vps[0];
								if(!vps[1].empty())
								{//ip-macs
									vector<string> macs = tokenize(vps[1], ",");
                                                                        list<pair<string,string> > ipmac_list;
									for(vector<string>::iterator j = macs.begin();
										j != macs.end();
										++j)
									{
										if(!(*j).empty())
										{
											vector<string> ipmac_tmp = tokenize(*j, "-");
											ipmac_list.push_back(make_pair(ipmac_tmp[0],ipmac_tmp[1]));
										}
									}
									map_ps.insert(make_pair(port, ipmac_list));
								}
							}
						}
						arplist.insert(make_pair(ip, map_ps));
					}
				}
			}
			if(input.eof()) break; 
		} 
		input.close();
    }
    else
        cout << "打开文件：Arp_ORG.txt" << " 时出错！" << endl; 
	return true;
}

// 读取原始的接口数据列表
bool StreamData::readInfPropList(IFPROP_LIST& inflist, const string& cdpPrex)
{
	ifstream input((cdpPrex+"InfProps.txt").c_str(),ios::in);
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input,line);
			if(!line.empty())
			{
				vector<string> vstr = tokenize(line, SPLIT_TOP);
				if(vstr.size() == 3)
				{
					string ip = vstr[0];//管理IP
					string amount = vstr[1];//接口数量
					if(!vstr[2].empty())
					{//
						list<IFREC> inf_list;
						vector<string> vinfs = tokenize(vstr[2], SPLIT_MAIN);
						for(vector<string>::iterator i = vinfs.begin();
							i != vinfs.end();
							i++)
						{//infs
							if(!(*i).empty())
							{
								vector<string> props = tokenize(*i, SPLIT_SUB);
								if(props.size() == 6)
								{//infprops
									IFREC ifrec;
									ifrec.ifIndex = props[0];
									ifrec.ifType = props[1];
									ifrec.ifMac = props[2];
									ifrec.ifPort = props[3];
									ifrec.ifDesc = props[4];
									ifrec.ifSpeed = props[5];
									inf_list.push_back(ifrec);
								}
							}
						}
						if(!inf_list.empty())
						{
							inflist.insert(make_pair(ip,make_pair(amount,inf_list)));
						}
					}
				}
			}
			if(input.eof()) break; 
		} 
		input.close();
    }
    else
        cout << "打开文件：infprops.txt" << " 时出错！" << endl; 
	return true;
}

// 读取原始的原始的CDP数据列表
bool StreamData::readDirectData(DIRECTDATA_LIST& directdata_list)
{
	directdata_list.clear();
	ifstream input("Direct_Data.txt", ios::in);
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input,line);
			if(!line.empty())
			{
				vector<string> vstr = tokenize(line, SPLIT_TOP);
				if(vstr.size() == 2)
				{
					string ip = vstr[0];//管理IP
					if(!vstr[1].empty())
					{//
						list<DIRECTITEM> item_list;
						vector<string> vcdps = tokenize(vstr[1], SPLIT_MAIN);
						for(vector<string>::iterator i = vcdps.begin();
							i != vcdps.end();
							i++)
						{//cdps
							if(!(*i).empty())
							{
								vector<string> items = tokenize(*i, SPLIT_SUB);
								if(items.size() == 6)
								{//cdpitems
									DIRECTITEM item_tmp;
									item_tmp.localPortInx = items[0];
									item_tmp.localPortDsc = items[1];
									item_tmp.PeerID = items[2];
									item_tmp.PeerIP = items[3];
									item_tmp.PeerPortInx = items[4];
									item_tmp.PeerPortDsc = items[5];
									item_list.push_back(item_tmp);
								}
								else if(items.size() == 4)
								{
									DIRECTITEM item_tmp;
									item_tmp.localPortInx = items[0];
									item_tmp.localPortDsc = items[0];
									item_tmp.PeerID = items[1];
									item_tmp.PeerIP = items[2];
									item_tmp.PeerPortInx = items[3];
									item_tmp.PeerPortDsc = items[3];
									item_list.push_back(item_tmp);
								}
							}
						}
						if(!item_list.empty())
						{
							directdata_list.insert(make_pair(ip, item_list));
						}
					}
				}
			}
			if(input.eof()) break; 
		} 
		input.close();
    }
    else
        cout << "打开文件：Direct_Data.txt" << " 时出错！" << endl; 
	return true;
}

// add by zhangyan 2008-10-30
// 保存配置信息
bool StreamData::saveConfigData(const SCAN_PARAM& scanPara)
{
	ofstream output("Scan_Para.txt",ios::out);
	string line = "";
        for(std::list<std::pair<string, string> >::const_iterator iter = scanPara.scan_scales.begin(); iter != scanPara.scan_scales.end(); iter++)
	{		
		line = "SCAN:" + iter->first + ":" + iter->second;
		output << line << endl;
	}
        for(std::list<std::pair<string, string> >::const_iterator iter = scanPara.filter_scales.begin(); iter != scanPara.filter_scales.end(); iter++)
	{		
		line = "FILTER:" + iter->first + ":" + iter->second;
		output << line << endl;
	}
	output.close();
	return true;
}

// 读取配置信息
bool StreamData::readConfigData(SCAN_PARAM& scanPara)
{
	ifstream input("Scan_Para.txt",ios::in);
	scanPara.scan_scales.clear();
	scanPara.scan_scales_num.clear();
	scanPara.filter_scales.clear();
	scanPara.filter_scales_num.clear();
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input,line);
			if (!line.empty())
			{
				vector<string> vstr = tokenize(line, ":");
				if (vstr.size() == 3)
				{
					unsigned long startnum = ntohl(inet_addr(vstr[1].c_str()));
					unsigned long endnum = ntohl(inet_addr(vstr[2].c_str()));
					if (vstr[0] == "SCAN")
					{
						scanPara.scan_scales.push_back(make_pair(vstr[1], vstr[2]));
						scanPara.scan_scales_num.push_back(make_pair(startnum, endnum));
					}
					else
					{
						scanPara.filter_scales.push_back(make_pair(vstr[1], vstr[2]));
						scanPara.filter_scales_num.push_back(make_pair(startnum, endnum));
					}	
				}
			}
			if (input.eof()) break; 
		} 
		input.close();
    }
    else
	{
        cout << "打开文件：Scan_Para.txt" << " 时出错！" << endl; 
	}
	return true;
}

//add by wings 2009-11-13
bool StreamData::savaDevidIps(const DEVID_LIST& devid_list)
{
	ofstream output("DeviceIps.txt",ios::out);
	for(DEVID_LIST::const_iterator i = devid_list.begin(); i != devid_list.end(); i++)
	{
		string line = i->first;
		output << line << endl;
	}
	output.close();
	return true;
}

