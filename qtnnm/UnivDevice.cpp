/*
 * Topo scan console program
 * Copyright (c) 2007-2008 DragonFlow
 * Created date: 2007-11-28
 * Autor: Tang guofu 
 * last modified 2008-09-22 by Tang guofu 
 *
 */

#include "StdAfx.h"
//#include "stdio.h"
#include "UnivDevice.h"
#include <QtDebug>

//remarked by zhangyan 2009-01-14
//UnivDevice::UnivDevice(const SCAN_PARAM scanpr, const AUX_PARAM auxpr)
//{
//	scanPara.community_get_dft = scanpr.community_get_dft;
//	scanPara.thrdamount = scanpr.thrdamount;
//	scanPara.retrytimes = scanpr.retrytimes;
//	scanPara.timeout = scanpr.timeout;
//
//	auxPara.arp_read_type = auxpr.arp_read_type;
//	auxPara.nbr_read_type = auxpr.nbr_read_type;
//	auxPara.rt_read_type  = auxpr.rt_read_type;
//	auxPara.ping_type     = auxpr.ping_type;
//	auxPara.bgp_read_type = auxpr.bgp_read_type;
//}
//
//void UnivDevice::Init(const SCAN_PARAM scanpr, const AUX_PARAM auxpr)
//{
//	scanPara.community_get_dft = scanpr.community_get_dft;
//	scanPara.thrdamount = scanpr.thrdamount;
//	scanPara.retrytimes = scanpr.retrytimes;
//	scanPara.timeout = scanpr.timeout;
//
//	auxPara.arp_read_type = auxpr.arp_read_type;
//	auxPara.nbr_read_type = auxpr.nbr_read_type;
//	auxPara.rt_read_type  = auxpr.rt_read_type;
//	auxPara.ping_type     = auxpr.ping_type;
//	auxPara.bgp_read_type = auxpr.bgp_read_type;
//}


UnivDevice::~UnivDevice(void)
{
}

// 获取特定设备的接口信息
IFPROP_LIST& UnivDevice::getInfProp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList,bool bRouter)
{
	ifprop_list.clear();
	//接口数量 1.3.6.1.2.1.2.1 (可能为空)
	string infAmount = snmp.GetMibObject(spr, "1.3.6.1.2.1.2.1");

	//if(!infAmount.empty())
	{
		//接口类型 1.3.6.1.2.1.2.2.1.3 (可能为空)
                list<pair<string,string> > infTypes = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.3");

		int length = 23;
		//接口描述 1.3.6.1.2.1.2.2.1.2
                list<pair<string,string> > infDescs = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.2");
		//端口接口对应关系1.3.6.1.2.1.17.1.4.1.2 (可能为空)
                list<pair<string,string> > infPorts;
		if(!bRouter)
		{
			if (oidIndexList.find(INF_INDEXPORT_MACRO) != oidIndexList.end())
			{
				infPorts= snmp.GetMibTable(spr,oidIndexList[INF_INDEXPORT_MACRO]);
				length = oidIndexList[INF_INDEXPORT_MACRO].size() + 1;
			}
			else
			{
				infPorts= snmp.GetMibTable(spr,"1.3.6.1.2.1.17.1.4.1.2");
			}
			

			//if (infPorts.empty() && (oidIndexList.find(INF_INDEXPORT_MACRO) != oidIndexList.end()))
			//{

			//}

			////add by jiangshanwen 支持私有
			//if(infPorts.empty() && !infDescs.empty())
			//{
                        //	//SvLog::writeLog("read inf");
                        //	for(list<pair<string, string> >::iterator item = infDescs.begin(); item != infDescs.end(); ++item)
			//	{
			//		if(item->second.compare(0,3,"eth") == 0)
			//		{
			//			string port = item->second.substr(3);
                        //			////SvLog::writeLog("port:"+port+"infInde:"+infIndex);
			//			pair<string,string> inpo;
			//			inpo.first = "00000000000000000000000"+port;
			//			inpo.second = item->first.substr(20);
			//			infPorts.push_back(inpo);
			//		}
			//		else
			//		{
			//			string port = item->second;
			//			pair<string,string> inpo;
			//			inpo.first = "00000000000000000000000"+port;
			//			inpo.second = item->first.substr(20);
			//			infPorts.push_back(inpo);
			//		}
			//	}
			//}
		}
		//接口MAC地址 1.3.6.1.2.1.2.2.1.6 (只要Mac不为空)
                list<pair<string,string> > infMacs = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.6");
		//接口速度 1.3.6.1.2.1.2.2.1.5
                list<pair<string,string> > infSpeeds = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.5");

		std::list<IFREC> infprops;//(ifindex,ifType,ifDescr,ifMac,ifPort,ifSpeed)
		
		//added by zhangyan 2009-04-29
		//增加对infAmount为空，但接口Mac地址不空的处理
		if (infTypes.empty())
		{
                        for(list<pair<string,string> >::iterator imac = infMacs.begin(); imac != infMacs.end(); ++imac)
			{
				IFREC inf_tmp;
				if (imac->first.length() < 21)
				{
					continue;
				}
				inf_tmp.ifIndex = imac->first.substr(20);
				inf_tmp.ifType = "";
				inf_tmp.ifMac = replaceAll(imac->second, " ","").substr(0,12);
                                std::transform(inf_tmp.ifMac.begin(), inf_tmp.ifMac.end(), inf_tmp.ifMac.begin(), (int(*)(int))toupper);
                                for(list<pair<string,string> >::iterator idesc = infDescs.begin(); idesc != infDescs.end(); ++idesc)
				{
					if(idesc->first.length() > 20 && idesc->first.substr(20) == inf_tmp.ifIndex)
					{
						inf_tmp.ifDesc = idesc->second;
						break;
					}
				}

				inf_tmp.ifPort = inf_tmp.ifIndex;
                                for(list<pair<string,string> >::iterator iport = infPorts.begin(); iport != infPorts.end(); ++iport)
				{
					if(iport->second == inf_tmp.ifIndex)
					{
						inf_tmp.ifPort = iport->first.substr(23);
						break;
					}
				}

                                for(list<pair<string,string> >::iterator ispeed = infSpeeds.begin(); ispeed != infSpeeds.end(); ++ispeed)
				{
					if(ispeed->first.length() > 20 && ispeed->first.substr(20) == inf_tmp.ifIndex)
					{
						inf_tmp.ifSpeed = ispeed->second;
						break;
					}
				}
				infprops.push_back(inf_tmp);
			}
		}
		else
		{
                        for(list<pair<string,string> >::iterator itype = infTypes.begin(); itype != infTypes.end(); ++itype)
			{
				IFREC inf_tmp;
				if (itype->first.length() < 21)
				{
					continue;
				}
				inf_tmp.ifIndex = itype->first.substr(20);
				inf_tmp.ifType = itype->second;
                                for(list<pair<string,string> >::iterator idesc = infDescs.begin(); idesc != infDescs.end(); ++idesc)
				{
					if(idesc->first.length() > 20 && idesc->first.substr(20) == inf_tmp.ifIndex)
					{
						inf_tmp.ifDesc = idesc->second;
						break;
					}
				}
				inf_tmp.ifPort = inf_tmp.ifIndex;
                                for(list<pair<string,string> >::iterator iport = infPorts.begin(); iport != infPorts.end(); ++iport)
				{
					if(iport->first.length() > length && iport->second == inf_tmp.ifIndex)
					{
						inf_tmp.ifPort = iport->first.substr(length);
						break;
					}
				}
                                for(list<pair<string,string> >::iterator imac = infMacs.begin(); imac != infMacs.end(); ++imac)
				{
					if(imac->first.length() > 20 && imac->first.substr(20) == inf_tmp.ifIndex)
					{
						inf_tmp.ifMac = replaceAll(imac->second, " ","").substr(0,12);
//						std::transform(inf_tmp.ifMac.begin(), inf_tmp.ifMac.end(), inf_tmp.ifMac.begin(), toupper);
						break;
					}
				}
                                for(list<pair<string,string> >::iterator ispeed = infSpeeds.begin(); ispeed != infSpeeds.end(); ++ispeed)
				{
					if(ispeed->first.length() > 20 && ispeed->first.substr(20) == inf_tmp.ifIndex)
					{
						inf_tmp.ifSpeed = ispeed->second;
						break;
					}
				}
				infprops.push_back(inf_tmp);
			}
		}
		
		if(!infprops.empty())
		{
			ifprop_list.insert(make_pair(spr.ip,make_pair(infAmount,infprops)));
		}
	}
	return ifprop_list;
}

// 读取设备转发表
bool UnivDevice::getAftByDtp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList)
{
        //SvLog::writeLog("Start read " + spr.ip + " aft by " + spr.community);
	////端口状态 1.3.6.1.2.1.17.4.3.1.3
	//PORT-MAC  1.3.6.1.2.1.17.4.3.1.2
        list<pair<string,string> > portMacs;
        qDebug() << " test source ip: " << spr.ip.c_str() << " portMacs size : " << portMacs.size();
                qDebug() << "community : " << spr.community.c_str() << " snmpversion " << spr.snmpver.c_str();
        portMacs = snmp.GetMibTable(spr, "1.3.6.1.2.1.17.4.3.1.2");
        qDebug() << "source ip: " << spr.ip.c_str() << " portMacs size : " << portMacs.size();

	//add by jiangshanwen 2010-7-21
	if(portMacs.empty() && (oidIndexList.find(AFT_MAC_DTP_MACRO) != oidIndexList.end()) && (oidIndexList.find(AFT_MAC_DTP_PORT) != oidIndexList.end()))
	{
                qDebug()  << "test12345";
                list<pair<string,string> > ports = snmp.GetMibTable(spr, oidIndexList[AFT_MAC_DTP_PORT]);
                list<pair<string,string> > macs = snmp.GetMibTable(spr, oidIndexList[AFT_MAC_DTP_MACRO]);
		int oidLength = oidIndexList[AFT_MAC_DTP_PORT].length();
                std::map<std::string, std::list<std::string> >::iterator i_pm;
                //SvLog::writeLog("oidIndexList[AFT_MAC]:" + oidIndexList[AFT_MAC_DTP_MACRO]);

		if (ports.empty() || macs.empty())
		{
			return false;
		}

                for (list<pair<string,string> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
                        for (list<pair<string,string> >::iterator j = macs.begin(); j != macs.end(); ++j)
			{
                                ////SvLog::writeLog("i->first:"+i->first+"i->second"+i->second+"j->first"+j->first+"j->second"+j->second);
				if (i->first.substr(oidLength) == j->first.substr(oidLength))
				{
					/*std::transform((j->second).begin(), (j->second).end(), (j->second).begin(), toupper);
                                        //SvLog::writeLog("port:"+i->second+"mac:"+j->second);*/
					string aftport = i->second;
					string aftmac = replaceAll(j->second,":","");
					if (aftport.compare(0,3,"eth") == 0)
					{
						aftport = i->second.substr(3);
					}
					i_pm = port_macs.find(aftport);
					if (i_pm != port_macs.end())
					{
						if (find(i_pm->second.begin(), i_pm->second.end(), aftmac) == i_pm->second.end())
						{
							i_pm->second.push_back(aftmac);
						}
					}
					else
					{
						std::list<std::string> maclist;
						maclist.push_back(aftmac);
						port_macs.insert(make_pair(aftport, maclist));
					}
					break;
				}
			}
		}
	}
	else if (portMacs.empty() && oidIndexList.find(AFT_MAC_DTP_PORT) != oidIndexList.end())
	{
                qDebug()  << "test123456";
		cout<<"start read aft use private oid"<<endl;
                std::map<std::string, std::list<std::string> >::iterator i_pm;
                list<pair<string,string> > ports = snmp.GetMibTable(spr, oidIndexList[AFT_MAC_DTP_PORT]);
		if(ports.empty()) return false;
                for (list<pair<string,string> >::iterator i = ports.begin(); i != ports.end(); ++i)
		{
			int oidLength = oidIndexList[AFT_MAC_DTP_PORT].length();
			vector<string> v1 = tokenize(i->first.substr(oidLength), ".", true);

			char mac_str[13];

			//vector<string> v1 = tokenize(imac->first.substr(22), ".", true);
			//add by wings 2009-11-13 
			//防止溢出
			if(v1.size()<6)
			{
				continue ;
			}
                        sprintf(mac_str, "%02X%02X%02X%02X%02X%02X",
				str2int(v1[0]), str2int(v1[1]), str2int(v1[2]), str2int(v1[3]), str2int(v1[4]), str2int(v1[5]));

			i_pm = port_macs.find(i->second);
			if(i_pm != port_macs.end())
			{//存在该端口
				if(find(i_pm->second.begin(), i_pm->second.end(), mac_str) == i_pm->second.end())
				{//不存在该mac
					i_pm->second.push_back(mac_str);
				}
			}
			else
			{//不存在该端口
				std::list<std::string> maclist;
				maclist.push_back(mac_str);
				port_macs.insert(make_pair(i->second, maclist));
			}
		}
	}
	else
	{
                qDebug()  << "test1234567";
		if (portMacs.empty())
		{
                        qDebug() << "testxxx exit";
			return false;
		}
                std::map<std::string, std::list<std::string> >::iterator i_pm;

		//add by jiangshanwen 2010-8-11 有些设备必需直接取端口，不能靠截取oid
                list<pair<string,string> >::iterator oneItem = portMacs.begin();
                vector<string> v0 = tokenize(oneItem->first, ".", true);
		if (v0.size()>18)
		{
                        list<pair<string,string> > Macs = snmp.GetMibTable(spr, "1.3.6.1.2.1.17.4.3.1.1");
                        for (list<pair<string,string> >::iterator i = portMacs.begin(); i != portMacs.end(); ++i)
			{
                                for (list<pair<string,string> >::iterator j = Macs.begin(); j != Macs.end(); ++j)
				{
                                        ////SvLog::writeLog("i->first:"+i->first+"i->second"+i->second+"j->first"+j->first+"j->second"+j->second);
					if(j->second.length() < 11) continue;
					if (i->first.substr(22) == j->first.substr(22))
					{
						/*std::transform((j->second).begin(), (j->second).end(), (j->second).begin(), toupper);
                                                //SvLog::writeLog("port:"+i->second+"mac:"+j->second);*/
						i_pm = port_macs.find(i->second);
						if (i_pm != port_macs.end())
						{
							if (find(i_pm->second.begin(), i_pm->second.end(), j->second) == i_pm->second.end())
							{
								i_pm->second.push_back(j->second);
							}
						}
						else
						{
							std::list<std::string> maclist;
							maclist.push_back(j->second);
							port_macs.insert(make_pair(i->second, maclist));
						}
						break;
					}
				}
			}
		}
		else
		{
                        for(list<pair<string,string> >::iterator imac = portMacs.begin(); imac != portMacs.end(); ++imac)
			{
				if(imac->first.length() <= 22) continue; //add by jiangshanwen 2010-7-19
				vector<string> v1 = tokenize(imac->first.substr(22), ".", true);
                                qDebug() << "imac first: " << imac->first.c_str() << " second:" << imac->second.c_str();
				char mac_str[13];

				//vector<string> v1 = tokenize(imac->first.substr(22), ".", true);
				//add by wings 2009-11-13 
				//防止溢出
				if(v1.size()<6)
				{
					continue ;
				}
                                sprintf(mac_str, "%02X%02X%02X%02X%02X%02X",
					str2int(v1[0]), str2int(v1[1]), str2int(v1[2]), str2int(v1[3]), str2int(v1[4]), str2int(v1[5]));

				i_pm = port_macs.find(imac->second);
				if(i_pm != port_macs.end())
				{//存在该端口
					if(find(i_pm->second.begin(), i_pm->second.end(), mac_str) == i_pm->second.end())
					{//不存在该mac
						i_pm->second.push_back(mac_str);
					}
				}
				else
				{//不存在该端口
					std::list<std::string> maclist;
					maclist.push_back(mac_str);
					port_macs.insert(make_pair(imac->second, maclist));
				}
			}	
		}
	}

	/*if(portMacs.empty())
	{
		return false;
	}*/
	
	return true;
}

// 通过Bridge共同体获取设备转发表
void UnivDevice::getAftByQtp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList)
{
        qDebug() << "qtp test";
	//端口状态 1.3.6.1.2.1.17.7.1.2.2.1.3(mac,sta)
	//端口MAC 1.3.6.1.2.1.17.7.1.2.2.1.2(mac,port)
        list<pair<string,string> > portMacs = snmp.GetMibTable(spr, "1.3.6.1.2.1.17.7.1.2.2.1.2");

	//add by jiangshanwen 2010-7-21
	if(portMacs.empty() && (oidIndexList.find(AFT_MAC_QTP_MACRO) != oidIndexList.end()))
	{
		portMacs = snmp.GetMibTable(spr, oidIndexList[AFT_MAC_QTP_MACRO]);
	}

	if(!portMacs.empty())
	{
                std::map<std::string, std::list<std::string> >::iterator i_pm;
                for(list<pair<string,string> >::iterator imac = portMacs.begin(); imac != portMacs.end(); ++imac)
		{
			char mac_str[13];
			if(imac->first.length() <= 28) continue; //add by jiangshanwen 2010-7-19
			vector<string> v1 = tokenize(imac->first.substr(28), ".", true);
			//add by wings 2009-11-13 
			//防止溢出
			if(v1.size()<6)
			{
				continue ;
			}
                        sprintf(mac_str, "%02X%02X%02X%02X%02X%02X",
				str2int(v1[v1.size()-6]), str2int(v1[v1.size()-5]), str2int(v1[v1.size()-4]), 
				str2int(v1[v1.size()-3]), str2int(v1[v1.size()-2]), str2int(v1[v1.size()-1]));
			string mac(mac_str);
			i_pm = port_macs.find(imac->second);
			if(i_pm != port_macs.end())
			{
				if(find(i_pm->second.begin(), i_pm->second.end(), mac) == i_pm->second.end())
				{
					i_pm->second.push_back(mac);
				}
			}
			else
			{
				std::list<std::string> maclist;
				maclist.push_back(mac);
				port_macs.insert(make_pair(imac->second, maclist));
			}
		}
	}
}

// 获取特定设备的AFT数据{devip,{port,[mac]}}
AFT_LIST& UnivDevice::getAftData(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList)
{
    qDebug () << "get aft data!!!";
	aft_list.clear();
	port_macs.clear();
	getAftByDtp(snmp, spr, oidIndexList);//1.3.6.1.2.1.17.4.3.1.2
	getAftByQtp(snmp, spr, oidIndexList);//1.3.6.1.2.1.17.7.1.2.2.1.2

	if(!port_macs.empty())
	{
		aft_list.insert(make_pair(spr.ip, port_macs));
                qDebug() << "aft table source ip: " << spr.ip.c_str();
                map<string, list<string> >::iterator it = port_macs.begin();
                for (; it != port_macs.end(); it++)
                {
                    qDebug() << "source port : " << (*it).first.c_str();
                }
	}
	return aft_list;
}


// 获取特定设备的ARP的数据
ARP_LIST& UnivDevice::getArpData(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList)
{
        //SvLog::writeLog("Start read " + spr.ip + " arp by " + spr.community);
	arp_list.clear();
	inf_macs.clear();
	//IP-端口索引1.3.6.1.2.1.4.22.1.1
        //list<pair<string,string> > IpInfInx = snmp.GetMibTable(spr, "1.3.6.1.2.1.4.22.1.1");

	//add by jiangshanwen 2010-7-21
	bool isSpecial = false;
	/*if(IpInfInx.empty() && (oidIndexList.find(ARP_INFINDEX_MACRO) != oidIndexList.end()))
	{
		IpInfInx = snmp.GetMibTable(spr, oidIndexList[ARP_INFINDEX_MACRO]);
		isSpecial = true;
	}

	if(!IpInfInx.empty())
	{*/
                std::map<std::string, std::list<pair<std::string,std::string> > >::iterator i_pm;
		//IP-MAC地址1.3.6.1.2.1.4.22.1.2 
		//update by jiangshanwen 2010-7-21
                list<pair<string,string> > IpMacs;
		if(isSpecial && (oidIndexList.find(ARP_MAC_MACRO) != oidIndexList.end()))
		{
			IpMacs = snmp.GetMibTable(spr, oidIndexList[ARP_MAC_MACRO]);
		}
		else 
		{
			IpMacs = snmp.GetMibTable(spr, "1.3.6.1.2.1.4.22.1.2");
		}
                //list<pair<string,string> > IpMacs = snmp.GetMibTable(spr, "1.3.6.1.2.1.4.22.1.2");

                for(list<pair<string,string> >::iterator imac = IpMacs.begin(); imac != IpMacs.end(); ++imac)
		{
			//cout<<"IpMacs:"<<IpMacs.size()<<endl;
			if (imac->first.size()<=20)
			{
				continue;
			}
			vector<string> v1 = tokenize(imac->first.substr(21), ".", true);
			size_t len = v1.size();
			if(len >= 4)
			{
				string ip_tmp  = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
				string mac_tmp = replaceAll(imac->second, " ","").substr(0,12);
				if(v1[len-1]=="0"||v1[len-1]=="255")
				{
					continue;
				}
				string inf_tem = v1[len-5];
				/*cout<<"ip:"<<ip_tmp<<",mac:"<<mac_tmp<<endl;
				cout<<"imac->first:"<<imac->first<<endl;
				cout<<"infindex:"<<inf_tem<<endl;*/
                                //for(list<pair<string,string> >::iterator iInf = IpInfInx.begin(); iInf != IpInfInx.end(); ++iInf)
				//{
				//	cout<<"IpInfInx:"<<IpInfInx.size()<<",ip:"<<ip_tmp<<endl;
				//	size_t iPlace = iInf->first.rfind(ip_tmp);
				//	//remarked by zhangyan 2008-10-14
				//	//if(iPlace > 0)
				//	if(iPlace != string::npos)
				//	{
						//i_pm = inf_macs.find(iInf->second);
				i_pm = inf_macs.find(inf_tem);
						if(i_pm != inf_macs.end())
						{//存在该接口
							bool bExisted = false;
                                                        for(std::list<pair<std::string,std::string> >::iterator j = i_pm->second.begin();
								j != i_pm->second.end();
								++j)
							{
								if(j->second == mac_tmp)
								{
									bExisted = true;
									break;
								}
							}
							if(!bExisted)
							{//该接口的ARP中不存在该MAC
								i_pm->second.push_back(make_pair(ip_tmp, mac_tmp));
							}
						}
						else
						{//新的接口
                                                        std::list<pair<std::string,std::string> > ipmac_list;
							ipmac_list.push_back(make_pair(ip_tmp, mac_tmp));
							//inf_macs.insert(make_pair(iInf->second, ipmac_list));
							inf_macs.insert(make_pair(inf_tem, ipmac_list));
						}
						//break;
					//}
				//}
			}
		}
		if(!inf_macs.empty())
		{
			arp_list.insert(make_pair(spr.ip,inf_macs));
		}
	//}
	cout<<"end read "<<spr.ip<<" arp"<<endl;
	return arp_list;
}

// 获取特定设备的OSPF邻居表的数据
OSPFNBR_LIST& UnivDevice::getOspfNbrData(SnmpDG& snmp, const SnmpPara& spr)
{
        //SvLog::writeLog("Start read " + spr.ip + " nbr by " + spr.community);
	ospfnbr_list.clear();
	//邻居IP所在接口索引(oid,infinx) 1.3.6.1.2.1.14.10.1.2
        list<pair<string,string> > IpInfInx = snmp.GetMibTable(spr, "1.3.6.1.2.1.14.10.1.2");
	if(!IpInfInx.empty())
	{
                std::map<std::string, std::list<std::string> > infinx_nbrips; //{infindex,[nbrip]}
                for(list<pair<string,string> >::iterator iInf = IpInfInx.begin(); iInf != IpInfInx.end(); ++iInf)
		{
			if (iInf->first.size() <= 22)
			{
				continue;
			}
			vector<string> v1 = tokenize(iInf->first.substr(22), ".", true);
			size_t len = v1.size();
			if(len >= 5)
			{
				string ip_tmp = v1[len-5] + "." + v1[len-4] + "." + v1[len-3] + "." + v1[len-2];
				string inx_tmp = iInf->second;
                                std::map<std::string, std::list<std::string> >::iterator i_pm;
				i_pm = infinx_nbrips.find(iInf->second);
				if(i_pm != infinx_nbrips.end())
				{//存在该接口
					if(find(i_pm->second.begin(), i_pm->second.end(),ip_tmp) == i_pm->second.end())
					{//该接口的ARP中不存在该MAC
						i_pm->second.push_back(ip_tmp);
					}
				}
				else
				{//新的接口
					std::list<std::string> iplist_tmp;
					iplist_tmp.push_back(ip_tmp);
					infinx_nbrips.insert(make_pair(iInf->second, iplist_tmp));
				}
			}
		}
		if(!infinx_nbrips.empty())
		{
			ospfnbr_list.insert(make_pair(spr.ip,infinx_nbrips));
		}
	}
	return ospfnbr_list;
}

// 获取特定设备的路由表的数据
ROUTE_LIST& UnivDevice::getRouteData(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList)
{
        //SvLog::writeLog("Start read " + spr.ip + " route by " + spr.community);
	route_list.clear();
	//目的网络的接口索引(oid,infinx) 1.3.6.1.2.1.4.21.1.2
        list<pair<string,string> > dstNetInfInx = snmp.GetMibTable(spr, "1.3.6.1.2.1.4.21.1.2");
        list<pair<string,string> > dstNetNextHopIp;
        list<pair<string,string> > dstStates;
        list<pair<string,string> > dstMasks;

	//add by jiangshanwen 2010-7-21
	bool isSpecial = false;
	if(dstNetInfInx.empty() && (oidIndexList.find(ROUTE_INFINDEX_MACRO) != oidIndexList.end()))
	{
		dstNetInfInx = snmp.GetMibTable(spr, oidIndexList[ROUTE_INFINDEX_MACRO]); 
		isSpecial = true;
	}

	if(!dstNetInfInx.empty())
	{
		//目的网络的下一跳IP(oid,nbrip) 1.3.6.1.2.1.4.21.1.7
		if(isSpecial && (oidIndexList.find(ROUTE_NEXTHOPIP_MACRO) != oidIndexList.end()))
			dstNetNextHopIp = snmp.GetMibTable(spr, oidIndexList[ROUTE_NEXTHOPIP_MACRO]); 
		else
			dstNetNextHopIp = snmp.GetMibTable(spr,"1.3.6.1.2.1.4.21.1.7");
		if(!dstNetNextHopIp.empty())
		{
			//到下一跳类型(oid,state) 1.3.6.1.2.1.4.21.1.8
			if(isSpecial && (oidIndexList.find(ROUTE_ROUTETYPE_MACRO) != oidIndexList.end()))
				dstNetNextHopIp = snmp.GetMibTable(spr, oidIndexList[ROUTE_ROUTETYPE_MACRO]); 
			else
				dstStates = snmp.GetMibTable(spr,"1.3.6.1.2.1.4.21.1.8");
			if(!dstStates.empty())
			{
				//目的网络mask(oid,mask) 1.3.6.1.2.1.4.21.1.11
				if(isSpecial && (oidIndexList.find(ROUTE_ROUTEMASK_MACRO) != oidIndexList.end()))
					dstNetNextHopIp = snmp.GetMibTable(spr, oidIndexList[ROUTE_ROUTEMASK_MACRO]); 
				else
					dstMasks  = snmp.GetMibTable(spr, "1.3.6.1.2.1.4.21.1.11");
				if(!dstMasks.empty())
				{
                                        list<pair<string, ROUTEITEM> > ipinx_list;//[(index,<nextip,dest_net,dest_msk>)]
                                        for(list<pair<string,string> >::iterator i = dstNetInfInx.begin();
						i != dstNetInfInx.end(); 
						++i)
					{
						if (i->first.size()<=21)
						{
							continue;
						}
						vector<string> v1 = tokenize(i->first.substr(21), ".", true);
						size_t len = v1.size();
						if(len >= 4)
						{
							string ip_dest_i = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
							string next_hop = "";
							string dest_msk = "";
                                                        for(list<pair<string,string> >::iterator j = dstNetNextHopIp.begin();
								j != dstNetNextHopIp.end(); 
								++j)
							{
								v1 = tokenize(j->first.substr(21), ".", true);
								len = v1.size();
								if(len >= 4)
								{
									string ip_dest_j = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
//									if(ip_dest_j != spr.ip && ip_dest_j != "0.0.0.0" && ip_dest_j == ip_dest_i) //by tgf 2008-10-10
									if(ip_dest_j != spr.ip && ip_dest_j == ip_dest_i)
									{
                                                                                for(list<pair<string,string> >::iterator k = dstStates.begin();
											k != dstStates.end(); 
											++k)
										{
											v1 = tokenize(k->first.substr(21), ".", true);
											len = v1.size();
											if(len >= 4)
											{
												string ip_dest_k = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
												if( ip_dest_k == ip_dest_i && ((k->second == "4") || (k->second == "1")))
												{
													next_hop = j->second;
												}
											}
										}
									}
								}
							}
                                                        for(list<pair<string,string> >::iterator j = dstMasks.begin();
								j != dstMasks.end(); 
								++j)
							{
								v1 = tokenize(j->first.substr(21), ".", true);
								len = v1.size();
								if(len >= 4)
								{
									string ip_dest_j = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
//									if(ip_dest_j != spr.ip && ip_dest_j != "0.0.0.0" && ip_dest_j == ip_dest_i) //by tgf 2008-10-10
									if(ip_dest_j != spr.ip && ip_dest_j == ip_dest_i)
									{
                                                                                for(list<pair<string,string> >::iterator k = dstStates.begin();
											k != dstStates.end(); 
											++k)
										{
											v1 = tokenize(k->first.substr(21), ".", true);
											len = v1.size();
											if(len >= 4)
											{
												string ip_dest_k = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
												if( ip_dest_k == ip_dest_i && ((k->second == "4") || (k->second == "1")))
												{
													dest_msk = j->second;
												}
											}
										}
									}
								}
							}
							if(!next_hop.empty() && !dest_msk.empty())
							{
								ROUTEITEM rtitem;
								rtitem.dest_net = ip_dest_i;
								rtitem.next_hop = next_hop;
								rtitem.dest_msk = dest_msk;
								ipinx_list.push_back(make_pair(i->second, rtitem));
							}					
						}
					}

					if(!ipinx_list.empty())
					{
                                                std::map<std::string, std::list<ROUTEITEM> > infinx_nbrips; //{infindex,[ROUTEITEM]}
                                                for(list<pair<string,ROUTEITEM> >::iterator i = ipinx_list.begin();
							i != ipinx_list.end();
							++i)
						{
							string inx_tmp = i->first;
							string dest_net = i->second.dest_net;
							string dest_msk = i->second.dest_msk;
							string next_hop = i->second.next_hop;
							
                                                        std::map<std::string, std::list<ROUTEITEM> >::iterator i_pm;
							i_pm = infinx_nbrips.find(inx_tmp);
							if(i_pm != infinx_nbrips.end())
							{//存在该接口
								bool bNewRec = true;
								for(list<ROUTEITEM>::iterator j = i_pm->second.begin();
									j != i_pm->second.end();
									++j)
								{
									if(j->dest_net == dest_net || j->dest_msk == dest_msk || j->next_hop == next_hop)
									{
										bNewRec = false;
										break;
									}
								}
								if(bNewRec)
								{
									i_pm->second.push_back(i->second);
								}
							}
							else
							{//新的接口
								std::list<ROUTEITEM> iplist_tmp;
								iplist_tmp.push_back(i->second);
								infinx_nbrips.insert(make_pair(inx_tmp, iplist_tmp));
							}
						}
						if(!infinx_nbrips.empty())
						{
							route_list.insert(make_pair(spr.ip,infinx_nbrips));
						}
					}
				}
			}
		}
	}
	return route_list;
}

BGP_LIST& UnivDevice::getBgpData(SnmpDG& snmp, const SnmpPara& spr)
{
        //SvLog::writeLog("Start read " + spr.ip + " bgp by " + spr.community);
	bgp_list.clear();
	//1.3.6.1.2.1.15.3.1.5 :bgpPeerLocalAddr
        list<pair<string,string> > localips = snmp.GetMibTable(spr, "1.3.6.1.2.1.15.3.1.5");
	if(!localips.empty())
	{
		//1.3.6.1.2.1.15.3.1.6 :bgpPeerLocalPort
                list<pair<string,string> > localports = snmp.GetMibTable(spr, "1.3.6.1.2.1.15.3.1.6");
		//1.3.6.1.2.1.15.3.1.8 :bgpPeerRemotePort
                list<pair<string,string> > peerports  = snmp.GetMibTable(spr, "1.3.6.1.2.1.15.3.1.8");
                for(list<pair<string,string> >::iterator i = localips.begin(); i != localips.end(); ++i)
		{
			vector<string> v1 = tokenize(i->first, ".", true);
			size_t len = v1.size();
			if(len > 4)
			{
				string ip_dest_i = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
				if(ip_dest_i == "0.0.0.0") continue;
				string local_ip = i->second; //add by zhangyan 2008-09-02
				string local_port = "";
				string peer_port = "";
                                for(list<pair<string,string> >::iterator j = localports.begin(); j != localports.end(); ++j)
				{
					v1 = tokenize(j->first, ".", true);
					len = v1.size();
					if(len > 4)
					{
						string ip_dest_j = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
						if(ip_dest_j != spr.ip && ip_dest_j == ip_dest_i)
						{
                                                        for(list<pair<string,string> >::iterator k = peerports.begin();	k != peerports.end(); ++k)
							{
								v1 = tokenize(k->first, ".", true);
								len = v1.size();
								if(len > 4)
								{
									string ip_dest_k = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
									if(ip_dest_k == ip_dest_i)
									{
										local_port = j->second;
										peer_port = k->second;

										bool bNew = true;
										for(BGP_LIST::iterator m = bgp_list.begin(); m != bgp_list.end(); ++m)
										{
											if((m->local_ip == spr.ip && m->local_port == local_port && m->peer_ip == ip_dest_i && m->peer_port == peer_port) 
												|| (m->peer_ip == spr.ip && m->peer_port == local_port && m->local_ip == ip_dest_i && m->local_port == peer_port))
											{
												bNew = false;
												break;
											}
										}									
										if (bNew)
										{
											BGPITEM bgpitem;
											bgpitem.peer_ip = ip_dest_i;
											bgpitem.peer_port = peer_port;
											//bgpitem.local_ip = ip; //remaked by zhangyan 2008-09-02
											bgpitem.local_ip = local_ip;
											bgpitem.local_port = local_port;
											bgp_list.push_back(bgpitem);										
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return bgp_list;
}

//added by tgf 2008-09-22
//redesigned by zhangyan 2009-01-09
//VRRP_LIST& UnivDevice::getVrrpData(SnmpDG& snmp, const SnmpPara& spr)
//{
//	//SvLog::writeLog("Start read " + spr.ip + " vrrp by " + spr.community);
//	vrrp_list.clear();
//	//1.3.6.1.2.1.68.1.3.1.1 :vrrpOperVrId
//	//list<pair<string,string> > vrids = snmp.GetMibTable(spr, "1.3.6.1.2.1.68.1.3.1.1");
//	//1.3.6.1.2.1.68.1.3.1.2 :vrrpOperVirtualMacAddr
//	list<pair<string,string> > vrmacs = snmp.GetMibTable(spr, "1.3.6.1.2.1.68.1.3.1.2");
//	if(!vrmacs.empty())
//	{
//		//1.3.6.1.2.1.68.1.3.1.7 :vrrpOperMasterIpAddr
//		list<pair<string,string> > vrmasterips = snmp.GetMibTable(spr, "1.3.6.1.2.1.68.1.3.1.7");
//		//1.3.6.1.2.1.68.1.3.1.8 :vrrpOperPrimaryIpAddr
//		list<pair<string,string> > vrprimaryips = snmp.GetMibTable(spr, "1.3.6.1.2.1.68.1.3.1.8");
//		//1.3.6.1.2.1.68.1.4.1.1 :vrrpAssoIpAddr
//		//list<pair<string,string> > vrassoips = snmp.GetMibTable(spr, "1.3.6.1.2.1.68.1.4.1.1");//可能不支持vrrpAssoIpAddr，故改为用vrrpAssoIpAddrRowStatus代替
//		//1.3.6.1.2.1.68.1.4.1.2 :vrrpAssoIpAddrRowStatus
//		list<pair<string,string> > vrassoipstatus = snmp.GetMibTable(spr, "1.3.6.1.2.1.68.1.4.1.2");
//		VRRPITEMS vrrpItems;
//		for(list<pair<string,string> >::iterator i = vrmacs.begin(); i != vrmacs.end(); ++i)
//		{
//			string index = (i->first).substr(23);
//			VRRPITEM vrrpItem;
//			vrrpItem.vrId = index.substr(index.rfind(".") + 1);
//			vrrpItem.virtualMac = replaceAll(i->second, " ", "").substr(0, 12);
//			
//			for(list<pair<string,string> >::iterator j = vrmasterips.begin(); j != vrmasterips.end(); ++j)
//			{
//				if(index == (j->first).substr(23))
//				{
//					vrrpItem.masterIp = j->second;					
//					break;
//				}
//			}
//			for(list<pair<string,string> >::iterator j = vrprimaryips.begin(); j != vrprimaryips.end(); ++j)
//			{
//				if(index == (j->first).substr(23))
//				{
//					vrrpItem.primaryIp = j->second;
//					break;
//				}
//			}
//			for(list<pair<string,string> >::iterator i = vrassoipstatus.begin(); i != vrassoipstatus.end(); ++i)
//			{		
//				vector<string> v1 = tokenize((i->first).substr(23), ".", true);
//				size_t len = v1.size();
//				if((len > 4) && (v1[len-5] == vrrpItem.vrId))
//				{
//					vrrpItem.virtualIp = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
//					break;
//				}				
//			}
//			vrrpItems.push_back(vrrpItem);
//		}
//		
//		vrrp_list.insert(make_pair(spr.ip, vrrpItems));		
//	}
//
//	getHsrpData(snmp, spr); //added by zhangyan 2008-12-04
//	cout<<"get hsrp over"<<endl;
//
//	return vrrp_list;
//}
//
//
RouterStandby_LIST& UnivDevice::getVrrpData(SnmpDG& snmp, const SnmpPara& spr)
{
        //SvLog::writeLog("Start read " + spr.ip + " vrrp by " + spr.community);
	routeStandby_list.clear();
	//1.3.6.1.2.1.68.1.3.1.2 :vrrpOperVirtualMacAddr
        list<pair<string,string> > vrmacs = snmp.GetMibTable(spr, "1.3.6.1.2.1.68.1.3.1.2");
	if(!vrmacs.empty())
	{
		//1.3.6.1.2.1.68.1.4.1.2 :vrrpAssoIpAddrRowStatus
                list<pair<string,string> > vrassoipstatus = snmp.GetMibTable(spr, "1.3.6.1.2.1.68.1.4.1.2");
		RouterStandbyItem vrrpItem;
		//填充虚拟MAC
                for(list<pair<string,string> >::iterator i = vrmacs.begin(); i != vrmacs.end(); ++i)
		{			
			string mac = replaceAll(i->second, " ", "").substr(0, 12);
			vrrpItem.virtualMacs.push_back(mac);
		}
		//填充虚拟IP
                for(list<pair<string,string> >::iterator i = vrassoipstatus.begin(); i != vrassoipstatus.end(); ++i)
		{		
			if (i->first.size() <= 23)
			{
				continue;
			}
			vector<string> v1 = tokenize((i->first).substr(23), ".", true);
			size_t len = v1.size();
			if(len > 4)
			{
				string ip = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
				vrrpItem.virtualIps.push_back(ip);					
			}				
			
		}

		routeStandby_list.insert(make_pair(spr.ip, vrrpItem));
	}
	else
	{
		getHsrpData(snmp, spr); //added by zhangyan 2008-12-04
	}
	
	return routeStandby_list;
}
//added by zhangyan 2008-12-04
RouterStandby_LIST& UnivDevice::getHsrpData(SnmpDG& snmp, const SnmpPara& spr)
{
	return routeStandby_list;	
}

DIRECTDATA_LIST UnivDevice::getDirectData(SnmpDG& snmp, const SnmpPara& spr)
{
	return directdata_list;
}



//add by jiangshanwen 20100910
STP_LIST& UnivDevice::getStpData(SnmpDG& snmp, const SnmpPara& spr)
{
	return stp_list;
}
