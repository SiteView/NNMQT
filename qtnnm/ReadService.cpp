#include "StdAfx.h"
#include "ReadService.h"
#include "SnmpDG.h"
#include <QtDebug>

//extern bool isStop;

ReadService::ReadService(const DEVICE_TYPE_MAP& devtypemap, const SCAN_PARAM scanpr, const AUX_PARAM auxpr, const SPECIAL_OID_LIST& specialoidlist)
{
	dev_type_list = devtypemap;
        //m_callback = callback_function;
	special_oid_list = specialoidlist;	//add by jiangshanwen 2010-7-21

	scanPara.community_get_dft = scanpr.community_get_dft;
	scanPara.thrdamount = scanpr.thrdamount;
	scanPara.retrytimes = scanpr.retrytimes;
	scanPara.timeout = scanpr.timeout;

	auxPara.seed_type = auxpr.seed_type;
	auxPara.arp_read_type = auxpr.arp_read_type;
	auxPara.nbr_read_type = auxpr.nbr_read_type;
	auxPara.rt_read_type  = auxpr.rt_read_type;  //恢复路由表 add by wings 2009-11-12
	auxPara.vrrp_read_type  = auxpr.vrrp_read_type; //by zhangyan 2009-01-16
	auxPara.ping_type     = auxpr.ping_type;
	auxPara.bgp_read_type = auxpr.bgp_read_type;
	// added by zhangyan 2009-01-04
	auxPara.snmp_version = auxpr.snmp_version;
	auxPara.SNMPV_list = auxpr.SNMPV_list;

	map_devType.insert(make_pair("0", "ROUTE-SWITCH"));
	map_devType.insert(make_pair("1", "SWITCH"));
	map_devType.insert(make_pair("2", "ROUTER"));
	map_devType.insert(make_pair("3", "FIREWALL"));
	map_devType.insert(make_pair("4", "SERVER"));
	map_devType.insert(make_pair("5", "HOST"));
	map_devType.insert(make_pair("6", "OTHER"));

	DeviceRegister::registerDevice();//注册所有设备类型

	ofstream output1(ERR_AFT_LOG.c_str(), ios::out);
	output1.close();
	ofstream output2(ERR_ARP_LOG.c_str(), ios::out);
	output2.close();
	ofstream output3(ERR_OID_LOG.c_str(), ios::out);
	output3.close();
	ofstream output4(ERR_INF_LOG.c_str(), ios::out);
	output4.close();
	ofstream output5(ERR_NBR_LOG.c_str(), ios::out);
	output5.close();
	ofstream output6(ERR_RT_LOG.c_str(), ios::out);
	output6.close();
	ofstream output7(ERR_BGP_LOG.c_str(), ios::out);
	output7.close();
	ofstream output8(ERR_DRC_LOG.c_str(), ios::out);
	output8.close();
	ofstream output9(ERR_VRRP_LOG.c_str(), ios::out);
	output9.close();

	telnetReader = new TelnetReader();

}

ReadService::~ReadService(void)
{
	delete telnetReader;
}

void ReadService::getOidIndex(map<string, string>& oidIndexList, string sysOid)
{
	if(tokenize(sysOid, ".", true).size() < 7) return;
	if(special_oid_list.find(sysOid) != special_oid_list.end())
	{
		oidIndexList = special_oid_list[sysOid];
		/*for(map<string, string>::iterator iter = special_oid_list[sysOid].begin(); iter != special_oid_list[sysOid].end(); ++iter)
		{
			if((iter->first != "Descr") && (oidIndexList.find(iter->first) != oidIndexList.end()))
			{
				oidIndexList[iter->first] = iter->second;
			}
		}*/
	}
	else
	{
		getOidIndex(oidIndexList, sysOid.substr(0, sysOid.find_last_of(".")));
	}
}

// 获取设备类型
void ReadService::getDevTypeByOid(const string& sysOid, const string& sysSvcs, const string& ip,  map<string,string>& map_res)
{
	string devtype_res = "5";//default to host
	string factory_res = "";
	string model_res = "";
	string typeName_res = "";

	if(dev_type_list.find(sysOid) != dev_type_list.end())//DEVICE_TYPE_MAP {sysoid,<devType,devTypeName,devFac,devModel>}
	{
		devtype_res = dev_type_list[sysOid].devType;
		factory_res = dev_type_list[sysOid].devFac;
		model_res   = dev_type_list[sysOid].devModel;
		typeName_res = dev_type_list[sysOid].devTypeName;
	}
	else
	{
		if (sysOid.substr(0,16) =="1.3.6.1.4.1.311.")//enterprises节点:1.3.6.1.4.1
		{
			devtype_res = "5";//host
		}
		else if(sysOid.substr(0,17) == "1.3.6.1.4.1.8072.")
		{
			devtype_res = "4";// "SERVER"
		}
		//else if(sysOid.substr(0,17) == "1.3.6.1.4.1.9952.") //TOPSEC  added by zhangyan 2008-11-04
		//{
		//	devtype_res = "3";// "FIREWALL"
		//}
		else if(sysOid.substr(0,21) == "1.3.6.1.4.1.6486.800.")
		{
			devtype_res = "0";//ROUTE-SWITCH
		}
		else if(sysOid == "1.3.6.1.4.1.13742.1.1.1")
		{//KVM 流量分频器  1.3.6.1.4.1.13742.1.1.1
			devtype_res = "6"; //Other device
		}
		else
		{//
                        SvLog::writeErrorLog(string(ip) + ":" + sysOid, ERR_OID_LOG);
			int isvc = str2int(sysSvcs);
			if (isvc == 0)
			{
				devtype_res = "5";//host
			}
			else if((isvc & 6) == 6)
			{
				devtype_res = "0"; //"ROUTE-SWITCH"
			}
			else if((isvc & 4) == 4)
			{
				devtype_res = "2"; //ROUTER
			}
			else if((isvc & 2) == 2)
			{
				devtype_res = "1"; //"SWITCH"
			}
			else
			{
				devtype_res = "5"; //host
                                //SvLog::writeLog("Can't identify oid=" + sysOid + ", services=" + sysSvcs + " (" + ip + ")");
			} 
		}
	}

	//added by zhangyan 2008-12-25
	if (devtype_res == "5")
	{
		devtype_res = "4"; //将开了SNMP的PC（或其它未被识别的设备）作为SERVER
	}
	map_res["devtype"] = devtype_res;
	map_res["factory"] = factory_res;
	map_res["model"] = model_res;
	map_res["typename"] = typeName_res;
}

void ReadService::getOneSysInfo(SnmpPara& spr)
{
	if(testIP(spr))
	{
		IDBody devid = getOneSysInfo_NJ(spr);
		if(devid.snmpflag == "1")//add by wings 09-11-05
			addDevID(spr, devid);
	}
}

//add by wings 09-11-13
bool ReadService::testIP(const SnmpPara& spr)
{
	if(find(ip_visited_list.begin(), ip_visited_list.end(), spr.ip) 
		== ip_visited_list.end())
		return false;
	return true;
}

IDBody ReadService::getOneSysInfo_NJ(SnmpPara& spr)
{
       // SvLog::writeLog("Start get ID of " + spr.ip + " " + spr.community);
	SnmpDG snmp;
	IDBody devid;
	// remarked by zhangyan 20009-01-06
        //list<pair<string,string> > sysInfos = snmp.GetMibTable(spr, "1.3.6.1.2.1.1");
        list<pair<string,string> > sysInfos;
	//cout<<"snmp version: "<<spr.snmpver.c_str()<<endl;
        qDebug() << "snmpversion : " << spr.snmpver.c_str();
	if (spr.snmpver == "2")
	{
		sysInfos = snmp.GetMibTable(version2c, spr, "1.3.6.1.2.1.1");

		if(sysInfos.empty())
		{
			sysInfos = snmp.GetMibTable(version1, spr, "1.3.6.1.2.1.1");
			if (!sysInfos.empty())
			{
				spr.snmpver = "1";
			}
		}
	}
	else if (spr.snmpver == "1")
	{
		sysInfos = snmp.GetMibTable(version1, spr, "1.3.6.1.2.1.1");

		if(sysInfos.empty())
		{
			sysInfos = snmp.GetMibTable(version2c, spr, "1.3.6.1.2.1.1");
			if (!sysInfos.empty())
			{
				spr.snmpver = "2";
			}
		}
	}
	else// if (spr.snmpver == "0")
	{
		sysInfos = snmp.GetMibTable(spr, "1.3.6.1.2.1.1");
	}
	string sysOid = "";//snmp.GetMibObject("1.3.6.1.2.1.1.2");
	string sysSvcs = "";//snmp.GetMibObject("1.3.6.1.2.1.1.7");
	string sysName = "";//snmp.GetMibObject("1.3.6.1.2.1.1.5");
	if(sysInfos.empty())
	{
		//修改oid重取一次 update by jiangshanwen
		if (spr.snmpver == "2")
		{
			sysOid = snmp.GetMibObject(version2c,spr,"1.3.6.1.2.1.1.2");
			if(!sysOid.empty())
			{
				sysSvcs = snmp.GetMibObject(version2c,spr,"1.3.6.1.2.1.1.7");
				sysName = snmp.GetMibObject(version2c,spr,"1.3.6.1.2.1.1.5");
			}
			else
			{
				sysOid = snmp.GetMibObject(version1,spr,"1.3.6.1.2.1.1.2");
				if(!sysOid.empty())
				{
					sysSvcs = snmp.GetMibObject(version1,spr,"1.3.6.1.2.1.1.7");
					sysName = snmp.GetMibObject(version1,spr,"1.3.6.1.2.1.1.5");
					spr.snmpver = "1";
				}
			}
		}
		else if (spr.snmpver == "1")
		{
			sysOid = snmp.GetMibObject(version1,spr,"1.3.6.1.2.1.1.2");
			if(!sysOid.empty())
			{
				sysSvcs = snmp.GetMibObject(version1,spr,"1.3.6.1.2.1.1.7");
				sysName = snmp.GetMibObject(version1,spr,"1.3.6.1.2.1.1.5");
			}
			else
			{
				sysOid = snmp.GetMibObject(version2c,spr,"1.3.6.1.2.1.1.2");
				if(!sysOid.empty())
				{
					sysSvcs = snmp.GetMibObject(version2c,spr,"1.3.6.1.2.1.1.7");
					sysName = snmp.GetMibObject(version2c,spr,"1.3.6.1.2.1.1.5");
					spr.snmpver = "2";
				}
			}
		}
		else
		{
			sysOid = snmp.GetMibObject(spr,"1.3.6.1.2.1.1.2");
			if(!sysOid.empty())
			{
				sysSvcs = snmp.GetMibObject(spr,"1.3.6.1.2.1.1.7");
				sysName = snmp.GetMibObject(spr,"1.3.6.1.2.1.1.5");
			}
		}

		if(sysOid.empty() && sysSvcs.empty() && sysName.empty())
		{
                        //SvLog::writeLog("Can not get the ID of " + spr.ip);
			return devid;
		}
                //SvLog::writeLog("newMethod**ip:"+spr.ip+" sysOid:"+sysOid+" sysSvcs:"+sysSvcs+" sysName:"+sysName);
	}
	else
	{
                for(list<pair<string,string> >::iterator i = sysInfos.begin();
			i != sysInfos.end();
			++i)
		{
                        //SvLog::writeLog("sysInfo**ip:"+spr.ip+" sysInfo->first:"+i->first);
			cout<<"i->first : "<<i->first.c_str()<<endl;
			if(i->first == "1.3.6.1.2.1.1.2.0"
				//add by wings 2009-11-13
				||i->first == "1.3.6.1.2.1.1.2")
                        {
				sysOid = i->second;
                                //qDebug() << sysOid.c_str();
			}
			else if(i->first == "1.3.6.1.2.1.1.5.0"
				//add by wings 2009-11-13
				||i->first == "1.3.6.1.2.1.1.5")
			{ 
				sysName = i->second;
                                //qDebug() << sysName.c_str();
			}
			else if(i->first == "1.3.6.1.2.1.1.7.0"
				//add by wings 2009-11-13
				||i->first == "1.3.6.1.2.1.1.7")
			{
				sysSvcs = i->second;
			}
		}
                //SvLog::writeLog("OldMethod**ip:"+spr.ip+" sysOid:"+sysOid+" sysSvcs:"+sysSvcs+" sysName:"+sysName);
	}
        //SvLog::writeLog("get sysOid of " + spr.ip + ": " + sysOid);
	cout<<"sysOid : "<<sysOid.c_str()<<endl;

	if(sysOid == "1.3.6.1.4.1.13742.1.1.1")
	{//KVM 流量分频器  1.3.6.1.4.1.13742.1.1.1
		devid.sysOid = "1.3.6.1.4.1.13742.1.1.1";
//		devid.ips.push_back(spr.ip);
		devid.devType = "6";//Other device
		devid.devTypeName = "KVM";
	}
	else
	{		
		if(sysOid==""&&sysSvcs==""&&sysName=="") return devid;
		if (sysOid == "")
		{
			cout<<"sysOid is empty! "<<endl;
			sysOid = "00";
		}
		//IP-MASK地址1.3.6.1.2.1.4.20.1.3
                list<pair<string,string> > ipmsks = snmp.GetMibTable(spr, "1.3.6.1.2.1.4.20.1.3");//[ipDes, mask]
		//IP-InfInx 1.3.6.1.2.1.4.20.1.2
                list<pair<string,string> > infinxs = snmp.GetMibTable(spr, "1.3.6.1.2.1.4.20.1.2");//[ipDes, index]

		map<string,string> map_type;
		getDevTypeByOid(sysOid, sysSvcs, spr.ip, map_type);//填充map_type
		
                //SvLog::writeLog("ip:"+spr.ip+"**map_type['devtype']:"+map_type["devtype"]); //add by jiang 20100602

		devid.sysOid = sysOid;
		devid.snmpflag = "1"; //snmp is enabled
		devid.community_get = spr.community;
		devid.devType = map_type["devtype"];
		devid.devModel = map_type["model"];
		devid.devFactory = map_type["factory"];
		devid.devTypeName = map_type["typename"];
		//	devid.baseMac = baseMac;
		devid.sysSvcs = sysSvcs;
                //devid.sysName = Utf8ToString(sysName);	//update by jiang 20100602 编码转换
                devid.sysName = sysName;	//update by jiang 20100602 编码转换

		/*update by jiang 20100602 如果没有读取到ip表，设备会被过滤掉
		//add by wings 09-11-05
		if(ipmsks.size() ==0)
		{
			return devid;
		}
		if((ipmsks.size() ==1)&&(ipmsks.begin()->second.compare(0,1,"0") == 0))
		{
			return devid;
		}
		*/
		//devid.snmpflag = "1"; update by jiang 20100602 前面已经赋值，重复，注释掉
		if(!(ipmsks.size() == 0) && !((ipmsks.size() ==1)&&(ipmsks.begin()->second.compare(0,1,"0") == 0))) //update by jiang 20100602
		{
                        for(list<pair<string,string> >::iterator i = ipmsks.begin(); i != ipmsks.end(); ++i)
			{
				cout<<"i->first : "<<i->first.c_str()<<" devid.sysOid:"<<devid.sysOid<<" devid.sysName:"<<devid.sysName<<" spr.ip:"<<spr.ip<<endl;
				if (i->first.length() < 22)
				{
					continue;
				}
				std::string ip_tmp = i->first.substr(21);
				cout<<"ip_tmp:"<<ip_tmp<<endl;
				//			if(ip_tmp != "" && ip_tmp != "0.0.0.0"    //remarked by zhangyan 2008-10-15
				if(ip_tmp.compare(0,6,"0.0.0.") != 0
					&& ip_tmp.compare(0,3,"127") != 0 //排除环回地址
					//add by wings 2009-11-13
					&& ip_tmp.compare(0,5,"0.255") != 0
					//				&& (ip_tmp.compare(0,3,"224") < 0 || ip_tmp.compare(0,3,"239") > 0) //排除组播地址  //remarked by zhangyan 2008-10-15
					)
				{
					if(find(devid.ips.begin(), devid.ips.end(), ip_tmp) == devid.ips.end())
					{
                                                for(list<pair<string,string> >::iterator j = infinxs.begin();
							j != infinxs.end();
							++j)
						{
							if(j->first.length() > 21 && j->first.substr(21) == ip_tmp)
							{
								cout<<"ip_tmp : "<<ip_tmp.c_str()<<endl;
								devid.infinxs.push_back(j->second);
								devid.ips.push_back(ip_tmp);
								devid.msks.push_back(i->second);
								break;
							}
						}
					}
				}
			}
		}
		
		// remarked by zhangyan 2008-10-23
		////IP-MAC地址1.3.6.1.2.1.4.22.1.2
                //list<pair<string,string> > IpMacs = snmp.GetMibTable(spr, "1.3.6.1.2.1.4.22.1.2");
		//if(!IpMacs.empty())
		//{
                //	for(list<pair<string,string> >::iterator imac = IpMacs.begin(); imac != IpMacs.end(); ++imac)
		//	{
		//		vector<string> v1 = tokenize(imac->first.substr(21), ".", true);
		//		size_t len = v1.size();
		//		if(len < 4)	{	continue;	}
		//		string ip_tmp  = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
		//		string mac_tmp = replaceAll(imac->second, " ","").substr(0,12);
		//		if(find(devid.ips.begin(), devid.ips.end(), ip_tmp) != devid.ips.end())
		//		{//(ip,mac) 中的 ip 地址 属于ip地址表
		//			std::transform(mac_tmp.begin(), mac_tmp.end(), mac_tmp.begin(), toupper);
		//			if(mac_tmp != "" && mac_tmp != "000000000000" && mac_tmp != "FFFFFFFFFFFF")
		//			{
		//				if(find(devid.macs.begin(), devid.macs.end(), mac_tmp) == devid.macs.end())
		//				{
		//					devid.macs.push_back(mac_tmp);
		//					devid.baseMac = mac_tmp;//added by tgf 2008-09-22
		//				}
		//			}
		//		}
		//	}
		//}

		// added by zhangyan 2008-10-23		
		// get base mac (ps:只对交换机有效)
		if (devid.devType == "0" || devid.devType == "1")
		{
			string mac_tmp = snmp.GetMibObject(spr, "1.3.6.1.2.1.17.1.1");			
			if (!mac_tmp.empty())
			{
				string baseMac = replaceAll(mac_tmp, " ","").substr(0,12);
                                std::transform(baseMac.begin(), baseMac.end(), baseMac.begin(), (int(*)(int))toupper);
				if (baseMac != "" && baseMac != "000000000000" && baseMac != "FFFFFFFFFFFF")
				{
					devid.macs.push_back(baseMac);
					devid.baseMac = baseMac;
				}
			}
		}
	}
	// added by zhangyan 2008-11-04
	if (devid.ips.empty())
	{
		devid.infinxs.push_back("0");
		devid.ips.push_back(spr.ip);
		devid.msks.push_back("");
	}
        //SvLog::writeLog("Success read the ID of " + spr.ip);
	return devid;
}

void ReadService::addDevID(const SnmpPara& spr, const IDBody& devid)
{
	if(devid.sysOid.empty())
	{
		return;
	}
	//begin added by tgf 2008-09-23
	if(find(devid.ips.begin(), devid.ips.end(), spr.ip) == devid.ips.end())
	{//忽略vrrp的虚拟设备
		return;
	}
	//end added by tgf 2008-09-23

        //mutex::scoped_lock lock(m_data_mutex);
	for(DEVID_LIST::iterator i = devid_list_visited.begin(); i != devid_list_visited.end();	++i)
	{
		if(devid.ips == i->second.ips)
		{
			return;
		}
	}
	pair<string, IDBody> devid_cur = make_pair(spr.ip, devid);
	devid_list_valid.insert(devid_cur);
	devid_list_visited.insert(devid_cur);
	ip_visited_list.insert(ip_visited_list.end(), devid.ips.begin(), devid.ips.begin());
	//remarked by zhangyan 2008-12-29
	//修改目的：将非网络设备不添加到sproid_list列表中
	//sproid_list.push_back(make_pair(spr, make_pair(devid.devType, devid.sysOid)));
	if (devid.devType == "0" || devid.devType == "1" || devid.devType == "2" || devid.devType == "3")
	{
            qDebug() << " devType : " << devid.devType.c_str();
		sproid_list.push_back(make_pair(spr, make_pair(devid.devType, devid.sysOid)));
	}
	
	string msg = spr.ip + ", " + map_devType[devid.devType]+ ", " + devid.devFactory;
        //SvLog::writeLog(msg, FOUND_DEVICE_MSG, m_callback);
        //SvLog::writeLog(msg);
}

bool ReadService::isNewIp(const string& ip)
{
        //mutex::scoped_lock lock(m_data_mutex);
	for(DEVID_LIST::iterator i = devid_list_visited.begin(); i != devid_list_visited.end();	++i)
	{
		if(find(i->second.ips.begin(), i->second.ips.end(), ip) != i->second.ips.end())
		{
			return false;
		}
	}
	return true;
}

bool ReadService::getSysInfos(vector<SnmpPara>& spr_list)
{
        //qDebug() << "isStop " << isStop;
	devid_list_valid.clear();
	sproid_list.clear();
	if(spr_list.size() > 0)
	{
		if(spr_list.size() == 1)
		{
			if(isNewIp(spr_list[0].ip))
			{
				getOneSysInfo(spr_list[0]);
			}
		}
		else
		{
			//pool tp(scanPara.thrdamount);
//                        pool tp(min(scanPara.thrdamount, spr_list.size()));//by zhangyan 2008-12-29
			for (vector<SnmpPara>::const_iterator i = spr_list.begin(); i != spr_list.end(); ++i) 
			{
				if(isNewIp(i->ip))
				{
                                        //if (isStop)
                                        //{
                                        //    return false;
                                        //}
                                        //else
                                        //{
                                        //    //tp.schedule((boost::bind(&ReadService::getOneSysInfo, this, *i)));
                                        //}

				}
			}
                        //tp.wait();
		}
	}
        return true;
}

string ReadService::getDeviceIps(const string& ip)
{
	string ips = "";
	vector<string> ips_tmp = devid_list_valid[ip].ips;
	if(find(ips_tmp.begin(), ips_tmp.end(), ip) == ips_tmp.end())
		ips_tmp.push_back(ip);
	for(vector<string>::iterator i = ips_tmp.begin();	i != ips_tmp.end();	++i)
	{
		ips += *i + ",";
	}
	return ips.substr(0, ips.length() - 1);
}

//获取一台设备的普通数据
void ReadService::getOneDeviceData(const SnmpPara& spr, const string& devType, const string& sysOid)
{
        //SvLog::writeLog("Start read the data of " + spr.ip);
	Device* device = DeviceFactory::Instance()->CreateDevice(getFacOid(sysOid));
	//device->Init(scanPara, auxPara); //by zhangyan 2008-10-28

	AFT_LIST aftlist_cur;
	ARP_LIST arplist_cur;
	IFPROP_LIST inflist_cur;
	OSPFNBR_LIST nbrlist_cur;
	ROUTE_LIST rttbl_cur;			//changed by zhang 2009-03-26 去掉路由表数据
	//changed again by wings 2009-11-13 恢复路由表
	BGP_LIST bgplist_cur;
	/*VRRP_LIST vrrplist_cur;*/
	RouterStandby_LIST vrrplist_cur;
	DIRECTDATA_LIST drctdata_cur;
	STP_LIST stplist_cur;

	bool bAft = false;
	bool bArp = false;
	bool bInf = true;
	bool bNbr = false;
	bool bRoute = false;
	bool bBgp = false;
	bool bVrrp = false; 
	bool bDirect = true;

	SnmpDG snmp;
	//remarked by zhangyan 2008-12-29
	/*if(devType == ROUTE_SWITCH || devType == SWITCH || devType == ROUTER)
	{
		inflist_cur = device->getInfProp(snmp, spr, ROUTER==devType);
		drctdata_cur = device->getDirectData(snmp, spr);
		bInf = true;
		bDirect = true;
	}*/
	//获取oid索引 add by jiangshanwen
	map<string, string> oidIndexList;
	getOidIndex(oidIndexList, sysOid);

	inflist_cur = device->getInfProp(snmp, spr, oidIndexList, ROUTER==devType);
	drctdata_cur = device->getDirectData(snmp, spr);
	
	/*for(map<string, string>::iterator iter = oidIndexList.begin(); iter != oidIndexList.end(); ++iter)
	{
                //SvLog::writeLog("key:" + iter->first + ",value:" + iter->second);
	}*/

	if(devType == ROUTE_SWITCH  || devType == FIREWALL)
	{//r-s
		bAft = true;
		bArp = true;				
		bRoute = true;	
		aftlist_cur = device->getAftData(snmp, spr, oidIndexList);
		arplist_cur = device->getArpData(snmp, spr, oidIndexList);
		rttbl_cur = device->getRouteData(snmp, spr, oidIndexList);  //changed by zhang 2009-03-26 去掉路由表的取数
														//changed again by wings 2009-11-13 恢复路由表
		if(auxPara.nbr_read_type == "1")
		{
			bNbr = true;
			nbrlist_cur = device->getOspfNbrData(snmp, spr);
		}
		if(auxPara.bgp_read_type == "1")
		{
			bBgp = true;
			bgplist_cur = device->getBgpData(snmp, spr);
		}
		if(auxPara.vrrp_read_type == "1")
		{
			bVrrp = true;
			vrrplist_cur = device->getVrrpData(snmp, spr);
		}
		//remarked by zhangyan 2008-11-28
		//if(aftlist_cur.empty())  
		//{
                //	map<string, list<string> > portmac_tmp = telnetReader->getAftData(getDeviceIps(spr.ip));
		//	if(!portmac_tmp.empty())
		//	{
		//		aftlist_cur.insert(make_pair(spr.ip, portmac_tmp));
		//	}
		//}		
		//再用telnet读数
		//added by zhangyan 2009-01-13
		vector<string> ips_tmp = devid_list_valid[spr.ip].ips;
		string ip_tmp = "";
		for (vector<string>::iterator iter = ips_tmp.begin(); iter != ips_tmp.end(); ++iter)
		{
			if (find(telnetReader->telnetIPList_Aft.begin(), telnetReader->telnetIPList_Aft.end(), *iter) != telnetReader->telnetIPList_Aft.end())
			{
				ip_tmp = *iter;
				break;
			}
		}
		if (ip_tmp != "") //该IP配置了telnet读aft表
		{
			//用telnet读该IP的aft表
                        map<string, list<string> > portmac_tmp = telnetReader->getAftData(ip_tmp);
			if(!portmac_tmp.empty())
			{
				if (aftlist_cur.empty())
				{
					aftlist_cur.insert(make_pair(spr.ip, portmac_tmp));
				}
				else
				{
					//合并端口集
                                        for (map<string, list<string> >::iterator port_mac = portmac_tmp.begin(); port_mac != portmac_tmp.end(); ++port_mac)
					{//port-macs
						if (aftlist_cur[spr.ip].find(port_mac->first) != aftlist_cur[spr.ip].end())
						{//存在该端口
							//bexits = true;
							for (list<string>::iterator idestmac = port_mac->second.begin(); idestmac != port_mac->second.end(); ++idestmac)
							{
								if (find(aftlist_cur[spr.ip][port_mac->first].begin(), aftlist_cur[spr.ip][port_mac->first].end(), *idestmac) == aftlist_cur[spr.ip][port_mac->first].end())
								{//不存在该mac
									aftlist_cur[spr.ip][port_mac->first].push_back(*idestmac);
								}
							}
						}
						else
						{
							aftlist_cur[spr.ip].insert(make_pair(port_mac->first, port_mac->second));
						}
					}
				}
			}
		}
		
		/*if(arplist_cur.empty())
		{*/
			//added by zhangyan 2009-01-13
			ip_tmp = "";
			for (vector<string>::iterator iter = ips_tmp.begin(); iter != ips_tmp.end(); ++iter)
			{
				if (find(telnetReader->telnetIPList_Arp.begin(), telnetReader->telnetIPList_Arp.end(), *iter) != telnetReader->telnetIPList_Arp.end())
				{
					ip_tmp = *iter;
					break;
				}
			}
			if (ip_tmp != "") //该IP配置了telnet读arp表
			{
				//用telnet读该IP的arp表
                                map<string, list<pair<string,string> > > portipmac_tmp = telnetReader->getArpData(ip_tmp);
				//update by jiangshanwen 20100925
				if(!portipmac_tmp.empty())
				{
					if (arplist_cur.empty())
					{
						arplist_cur.insert(make_pair(spr.ip, portipmac_tmp));
					}
					else
					{
                                                for (map<string,list<pair<string,string> > >::iterator iter = portipmac_tmp.begin();iter != portipmac_tmp.end();++iter)
						{
							if (arplist_cur[spr.ip].find(iter->first) != arplist_cur[spr.ip].end())
							{
                                                                for (list<pair<string,string> >::iterator item = iter->second.begin();item != iter->second.end();++item)
								{
                                                                        for (list<pair<string,string> >::iterator mac_ip = arplist_cur[spr.ip][iter->first].begin();mac_ip != arplist_cur[spr.ip][iter->first].end();++mac_ip)
									{
										if (item->first == mac_ip->first)
										{
											continue;
										}
										else
										{
											arplist_cur[spr.ip][iter->first].push_back(*item);
										}
									}
								}
							}
							else
							{
								arplist_cur[spr.ip].insert(make_pair(iter->first,iter->second));
							}
						}
					}
				}
			}
		/*}*/
	}
	else if(devType == SWITCH)
	{//s
		bAft = true;
		aftlist_cur = device->getAftData(snmp, spr, oidIndexList);
		//remarked by zhangyan 2008-11-28
		//if(aftlist_cur.empty())  
		//{
                //	map<string, list<string> > portmac_tmp = telnetReader->getAftData(getDeviceIps(spr.ip));
		//	if(!portmac_tmp.empty())
		//	{
		//		aftlist_cur.insert(make_pair(spr.ip, portmac_tmp));
		//	}
		//}		
		//再用telnet读数
		vector<string> ips_tmp = devid_list_valid[spr.ip].ips;
		string ip_tmp = "";
		for (vector<string>::iterator iter = ips_tmp.begin(); iter != ips_tmp.end(); ++iter)
		{
			if (find(telnetReader->telnetIPList_Aft.begin(), telnetReader->telnetIPList_Aft.end(), *iter) != telnetReader->telnetIPList_Aft.end())
			{
				ip_tmp = *iter;
				break;
			}
		}
		if (ip_tmp != "") //该IP配置了telnet读aft表
		{
			//用telnet读该IP的aft表
                        map<string, list<string> > portmac_tmp = telnetReader->getAftData(ip_tmp);
			if(!portmac_tmp.empty())
			{
				if (aftlist_cur.empty())
				{
					aftlist_cur.insert(make_pair(spr.ip, portmac_tmp));
				}
				else
				{
					//合并端口集
                                        for (map<string, list<string> >::iterator port_mac = portmac_tmp.begin(); port_mac != portmac_tmp.end(); ++port_mac)
					{//port-macs
						if (aftlist_cur[spr.ip].find(port_mac->first) != aftlist_cur[spr.ip].end())
						{//存在该端口
							//bexits = true;
							for (list<string>::iterator idestmac = port_mac->second.begin(); idestmac != port_mac->second.end(); ++idestmac)
							{
								if (find(aftlist_cur[spr.ip][port_mac->first].begin(), aftlist_cur[spr.ip][port_mac->first].end(), *idestmac) == aftlist_cur[spr.ip][port_mac->first].end())
								{//不存在该mac
									aftlist_cur[spr.ip][port_mac->first].push_back(*idestmac);
								}
							}
						}
						else
						{
							aftlist_cur[spr.ip].insert(make_pair(port_mac->first, port_mac->second));
						}
					}
				}
			}
		}
		//remarked by zhangyan 2009-01-16
		//if(auxPara.arp_read_type == "1")
		//{
		//	bArp = true;
		//	arplist_cur = device->getArpData(snmp, spr);
		//	if(arplist_cur.empty())
		//	{
		//		//added by zhangyan 2009-01-13
		//		ip_tmp = "";
		//		for (vector<string>::iterator iter = ips_tmp.begin(); iter != ips_tmp.end(); ++iter)
		//		{
		//			if (find(telnetReader->telnetIPList_Arp.begin(), telnetReader->telnetIPList_Arp.end(), *iter) != telnetReader->telnetIPList_Arp.end())
		//			{
		//				ip_tmp = *iter;
		//				break;
		//			}
		//		}
		//		if (ip_tmp != "") //该IP配置了telnet读arp表
		//		{
		//			//用telnet读该IP的arp表
                //			map<string, list<pair<string,string> > > portipmac_tmp = telnetReader->getArpData(ip_tmp);
		//			if(!portipmac_tmp.empty())
		//			{
		//				arplist_cur.insert(make_pair(spr.ip, portipmac_tmp));
		//			}
		//		}
		//	}
		//}

		//add by jiangshanwen 20100825
		if(auxPara.seed_type == "1")
		{
			bArp = true;
			arplist_cur = device->getArpData(snmp, spr, oidIndexList);
			if(arplist_cur.empty())
			{
				ip_tmp = "";
				for (vector<string>::iterator iter = ips_tmp.begin(); iter != ips_tmp.end(); ++iter)
				{
					if (find(telnetReader->telnetIPList_Arp.begin(), telnetReader->telnetIPList_Arp.end(), *iter) != telnetReader->telnetIPList_Arp.end())
					{
						ip_tmp = *iter;
						break;
					}
				}
				if (ip_tmp != "") //该IP配置了telnet读arp表
				{
					//用telnet读该IP的arp表
                                        map<string, list<pair<string,string> > > portipmac_tmp = telnetReader->getArpData(ip_tmp);
					if(!portipmac_tmp.empty())
					{
						arplist_cur.insert(make_pair(spr.ip, portipmac_tmp));
					}
				}
			}
		}
	}
	else if(devType == ROUTER)
	{//r
		bArp = true;
		bRoute = true;		
		arplist_cur = device->getArpData(snmp, spr, oidIndexList);
		rttbl_cur = device->getRouteData(snmp, spr, oidIndexList);		//changed again by wings 2009-11-13 恢复路由表
		if(auxPara.nbr_read_type == "1")
		{
			bNbr = true;
			nbrlist_cur = device->getOspfNbrData(snmp, spr);
		}
		if(auxPara.bgp_read_type == "1")
		{
			bBgp = true;
			bgplist_cur = device->getBgpData(snmp, spr);
		}
		if(auxPara.vrrp_read_type == "1")
		{
			bVrrp = true;
			vrrplist_cur = device->getVrrpData(snmp, spr);
		}
		if(arplist_cur.empty())
		{
			vector<string> ips_tmp = devid_list_valid[spr.ip].ips;
			string ip_tmp = "";
			for (vector<string>::iterator iter = ips_tmp.begin(); iter != ips_tmp.end(); ++iter)
			{
				if (find(telnetReader->telnetIPList_Arp.begin(), telnetReader->telnetIPList_Arp.end(), *iter) != telnetReader->telnetIPList_Arp.end())
				{
					ip_tmp = *iter;
					break;
				}
			}
			if (ip_tmp != "") //该IP配置了telnet读arp表
			{
				//用telnet读该IP的arp表
                                map<string, list<pair<string,string> > > portipmac_tmp = telnetReader->getArpData(ip_tmp);
				if(!portipmac_tmp.empty())
				{
					arplist_cur.insert(make_pair(spr.ip, portipmac_tmp));
				}
			}
		}
	}
	else if (devType == SERVER && auxPara.seed_type == "1")
	{
		bArp = true;
		bRoute = true;		
		arplist_cur = device->getArpData(snmp, spr, oidIndexList);
		rttbl_cur = device->getRouteData(snmp, spr, oidIndexList);		//changed again by wings 2009-11-13 恢复路由表
		if(arplist_cur.empty())
		{
			vector<string> ips_tmp = devid_list_valid[spr.ip].ips;
			string ip_tmp = "";
			for (vector<string>::iterator iter = ips_tmp.begin(); iter != ips_tmp.end(); ++iter)
			{
				if (find(telnetReader->telnetIPList_Arp.begin(), telnetReader->telnetIPList_Arp.end(), *iter) != telnetReader->telnetIPList_Arp.end())
				{
					ip_tmp = *iter;
					break;
				}
			}
			if (ip_tmp != "") //该IP配置了telnet读arp表
			{
				//用telnet读该IP的arp表
                                map<string, list<pair<string,string> > > portipmac_tmp = telnetReader->getArpData(ip_tmp);
				if(!portipmac_tmp.empty())
				{
					arplist_cur.insert(make_pair(spr.ip, portipmac_tmp));
				}
			}
		}
	}

	if(aftlist_cur.empty())
	{
		if(bAft)
		{
                        //SvLog::writeLog("Can not get aft of " + spr.ip);
                        SvLog::writeErrorLog(spr.ip, ERR_AFT_LOG);
		}
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		aft_list.insert(aftlist_cur.begin(), aftlist_cur.end());
	}
	if(arplist_cur.empty())
	{
		if(bArp)
		{
                        //SvLog::writeLog("Can not get arp of " + spr.ip);
                        SvLog::writeErrorLog(spr.ip, ERR_ARP_LOG);
		}
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		arp_list.insert(arplist_cur.begin(), arplist_cur.end());
	}

	if(inflist_cur.empty())
	{
		if(bInf)
		{
                        //SvLog::writeLog("Can not get inf of " + spr.ip);
                        SvLog::writeErrorLog(spr.ip, ERR_INF_LOG);
		}
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		ifprop_list.insert(inflist_cur.begin(), inflist_cur.end());
	}

	if(nbrlist_cur.empty())
	{
		if(bNbr)
		{
                        //SvLog::writeLog("Can not get BNR of " + spr.ip);
                        SvLog::writeErrorLog(spr.ip, ERR_NBR_LOG);
		}
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		ospfnbr_list.insert(nbrlist_cur.begin(), nbrlist_cur.end());
	}

	if(rttbl_cur.empty())
	{
		if(bRoute)
		{
                        //SvLog::writeLog("Can not get Route of " + spr.ip);
                        SvLog::writeErrorLog(spr.ip, ERR_RT_LOG);
		}
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		route_list.insert(rttbl_cur.begin(), rttbl_cur.end());
	}

	if(bgplist_cur.empty())
	{
		if(bBgp)
		{
                        //SvLog::writeLog("Can not get BGP of " + spr.ip);
                        SvLog::writeErrorLog(spr.ip, ERR_BGP_LOG);
		}
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		bgp_list.insert(bgp_list.end(), bgplist_cur.begin(), bgplist_cur.end());
	}

	if(vrrplist_cur.empty())
	{
		if(bVrrp)
		{
                        //SvLog::writeLog("Can not get VRRP of " + spr.ip);
                        SvLog::writeErrorLog(spr.ip, ERR_VRRP_LOG);
		}
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		routeStandby_list.insert(vrrplist_cur.begin(), vrrplist_cur.end());
	}

	if(drctdata_cur.empty())
	{
		if(bDirect)
		{
                        //SvLog::writeLog("Can not get DIRECT of " + spr.ip);
                        SvLog::writeErrorLog(spr.ip, ERR_DRC_LOG);
		}
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		directdata_list.insert(drctdata_cur.begin(), drctdata_cur.end());
	}
	if (stplist_cur.empty())
	{
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		stp_list.insert(stplist_cur.begin(),stplist_cur.end());
	}

	Sleep(500);
        //SvLog::writeLog("End read the data of " + spr.ip);
}
//获取一台设备的CDP数据
void ReadService::getOneCdpDeviceData(const SnmpPara& spr, const string& devType, const string& sysOid)
{
        //SvLog::writeLog("Start read the cdp data of " + spr.ip);
	CiscoDevice cisDevice;

	AFT_LIST aftlist_cur;
	ARP_LIST arplist_cur;
	IFPROP_LIST inflist_cur;
	DIRECTDATA_LIST drctdata_cur;

	SnmpDG snmp;
	bool bAft = false;
	bool bArp = false;
	bool bInf = false;
	bool bDirect = false;

	//获取oid索引 add by jiangshanwen
	map<string, string> oidIndexList = oid_index_list;
	getOidIndex(oidIndexList, sysOid);

	if(devType == ROUTE_SWITCH)
	{//r-s
		drctdata_cur = cisDevice.getDirectData(snmp, spr);
		inflist_cur = cisDevice.getInfProp(snmp, spr, oidIndexList);	
		aftlist_cur = cisDevice.getAftData(snmp, spr, oidIndexList);
		arplist_cur = cisDevice.getArpData(snmp, spr, oidIndexList);
		//remarked by zhangyan 2008-11-28
		//if(aftlist_cur.empty())  
		//{
                //	map<string, list<string> > portmac_tmp = telnetReader->getAftData(getDeviceIps(spr.ip));
		//	if(!portmac_tmp.empty())
		//	{
		//		aftlist_cur.insert(make_pair(spr.ip, portmac_tmp));
		//	}
		//}		
		//再用telnet读数
		//added by zhangyan 2009-01-13
		vector<string> ips_tmp = devid_list_valid[spr.ip].ips;
		string ip_tmp = "";
		for (vector<string>::iterator iter = ips_tmp.begin(); iter != ips_tmp.end(); ++iter)
		{
			if (find(telnetReader->telnetIPList_Aft.begin(), telnetReader->telnetIPList_Aft.end(), *iter) != telnetReader->telnetIPList_Aft.end())
			{
				ip_tmp = *iter;
				break;
			}
		}
		if (ip_tmp != "") //该IP配置了telnet读aft表
		{
			//用telnet读该IP的aft表
                        map<string, list<string> > portmac_tmp = telnetReader->getAftData(ip_tmp);
			if(!portmac_tmp.empty())
			{
				if (aftlist_cur.empty())
				{
					aftlist_cur.insert(make_pair(spr.ip, portmac_tmp));
				}
				else
				{
					//合并端口集
                                        for (map<string, list<string> >::iterator port_mac = portmac_tmp.begin(); port_mac != portmac_tmp.end(); ++port_mac)
					{//port-macs
						if (aftlist_cur[spr.ip].find(port_mac->first) != aftlist_cur[spr.ip].end())
						{//存在该端口
							//bexits = true;
							for (list<string>::iterator idestmac = port_mac->second.begin(); idestmac != port_mac->second.end(); ++idestmac)
							{
								if (find(aftlist_cur[spr.ip][port_mac->first].begin(), aftlist_cur[spr.ip][port_mac->first].end(), *idestmac) == aftlist_cur[spr.ip][port_mac->first].end())
								{//不存在该mac
									aftlist_cur[spr.ip][port_mac->first].push_back(*idestmac);
								}
							}
						}
						else
						{
							aftlist_cur[spr.ip].insert(make_pair(port_mac->first, port_mac->second));
						}
					}	
				}
			}
		}
		if(arplist_cur.empty())
		{
			//added by zhangyan 2009-01-13
			ip_tmp = "";
			for (vector<string>::iterator iter = ips_tmp.begin(); iter != ips_tmp.end(); ++iter)
			{
				if (find(telnetReader->telnetIPList_Arp.begin(), telnetReader->telnetIPList_Arp.end(), *iter) != telnetReader->telnetIPList_Arp.end())
				{
					ip_tmp = *iter;
					break;
				}
			}
			if (ip_tmp != "") //该IP配置了telnet读arp表
			{
				//用telnet读该IP的arp表
                                map<string, list<pair<string,string> > > portipmac_tmp = telnetReader->getArpData(ip_tmp);
				if(!portipmac_tmp.empty())
				{
					arplist_cur.insert(make_pair(spr.ip, portipmac_tmp));
				}
			}
		}
		bAft = true;
		bArp = true;
		bInf = true;
		bDirect = true;
	}

	if(devType == SWITCH)
	{//s
		drctdata_cur = cisDevice.getDirectData(snmp, spr);
		inflist_cur = cisDevice.getInfProp(snmp, spr, oidIndexList);	
		aftlist_cur = cisDevice.getAftData(snmp, spr, oidIndexList);
		//remarked by zhangyan 2008-11-28
		//if(aftlist_cur.empty())  
		//{
                //	map<string, list<string> > portmac_tmp = telnetReader->getAftData(getDeviceIps(spr.ip));
		//	if(!portmac_tmp.empty())
		//	{
		//		aftlist_cur.insert(make_pair(spr.ip, portmac_tmp));
		//	}
		//}		
		//再用telnet读数
		//added by zhangyan 2009-01-13
		vector<string> ips_tmp = devid_list_valid[spr.ip].ips;
		string ip_tmp = "";
		for (vector<string>::iterator iter = ips_tmp.begin(); iter != ips_tmp.end(); ++iter)
		{
			if (find(telnetReader->telnetIPList_Aft.begin(), telnetReader->telnetIPList_Aft.end(), *iter) != telnetReader->telnetIPList_Aft.end())
			{
				ip_tmp = *iter;
				break;
			}
		}
		if (ip_tmp != "") //该IP配置了telnet读aft表
		{
			//用telnet读该IP的aft表
                        map<string, list<string> > portmac_tmp = telnetReader->getAftData(ip_tmp);
			if(!portmac_tmp.empty())
			{
				if (aftlist_cur.empty())
				{
					aftlist_cur.insert(make_pair(spr.ip, portmac_tmp));
				}
				else
				{
					//合并端口集
                                        for (map<string, list<string> >::iterator port_mac = portmac_tmp.begin(); port_mac != portmac_tmp.end(); ++port_mac)
					{//port-macs
						if (aftlist_cur[spr.ip].find(port_mac->first) != aftlist_cur[spr.ip].end())
						{//存在该端口
							//bexits = true;
							for (list<string>::iterator idestmac = port_mac->second.begin(); idestmac != port_mac->second.end(); ++idestmac)
							{
								if (find(aftlist_cur[spr.ip][port_mac->first].begin(), aftlist_cur[spr.ip][port_mac->first].end(), *idestmac) == aftlist_cur[spr.ip][port_mac->first].end())
								{//不存在该mac
									aftlist_cur[spr.ip][port_mac->first].push_back(*idestmac);
								}
							}
						}
						else
						{
							aftlist_cur[spr.ip].insert(make_pair(port_mac->first, port_mac->second));
						}
					}
				}
			}	
		}
		bAft = true;		
		bInf = true;
		bDirect = true;
		//if(auxPara.arp_read_type == "1")
		//{			
		//	arplist_cur = cisDevice.getArpData(snmp, spr);
		//	if(arplist_cur.empty())
		//	{
		//		//added by zhangyan 2009-01-13
		//		ip_tmp = "";
		//		for (vector<string>::iterator iter = ips_tmp.begin(); iter != ips_tmp.end(); ++iter)
		//		{
		//			if (find(telnetReader->telnetIPList_Arp.begin(), telnetReader->telnetIPList_Arp.end(), *iter) != telnetReader->telnetIPList_Arp.end())
		//			{
		//				ip_tmp = *iter;
		//				break;
		//			}
		//		}
		//		if (ip_tmp != "") //该IP配置了telnet读arp表
		//		{
		//			//用telnet读该IP的arp表
                //			map<string, list<pair<string,string> > > portipmac_tmp = telnetReader->getArpData(ip_tmp);
		//			if(!portipmac_tmp.empty())
		//			{
		//				arplist_cur.insert(make_pair(spr.ip, portipmac_tmp));
		//			}
		//		}
		//	}
		//	bArp = true;
		//}
	}

	if(devType == ROUTER)
	{//r
		drctdata_cur = cisDevice.getDirectData(snmp, spr);
		inflist_cur = cisDevice.getInfProp(snmp, spr,oidIndexList, true);	
		arplist_cur = cisDevice.getArpData(snmp, spr, oidIndexList);		
		if(arplist_cur.empty())
		{
			vector<string> ips_tmp = devid_list_valid[spr.ip].ips;
			string ip_tmp = "";
			for (vector<string>::iterator iter = ips_tmp.begin(); iter != ips_tmp.end(); ++iter)
			{
				if (find(telnetReader->telnetIPList_Arp.begin(), telnetReader->telnetIPList_Arp.end(), *iter) != telnetReader->telnetIPList_Arp.end())
				{
					ip_tmp = *iter;
					break;
				}
			}
			if (ip_tmp != "") //该IP配置了telnet读arp表
			{
				//用telnet读该IP的arp表
                                map<string, list<pair<string,string> > > portipmac_tmp = telnetReader->getArpData(ip_tmp);
				if(!portipmac_tmp.empty())
				{
					arplist_cur.insert(make_pair(spr.ip, portipmac_tmp));
				}
			}
		}
		bArp = true;
		bInf = true;
		bDirect = true;
	}

	if(aftlist_cur.empty())
	{	
		if (bAft)
		{
                        //SvLog::writeLog("Can not get aft of " + spr.ip);
                        SvLog::writeErrorLog(spr.ip, ERR_AFT_LOG);
		}
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		aft_list.insert(aftlist_cur.begin(), aftlist_cur.end());
	}

	if(arplist_cur.empty())
	{
		if(bArp)
		{			
                        //SvLog::writeLog("Can not get arp of " + spr.ip);
                        SvLog::writeErrorLog(spr.ip, ERR_ARP_LOG);
		}
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		arp_list.insert(arplist_cur.begin(), arplist_cur.end());
	}

	if(inflist_cur.empty())
	{
		if(bInf)
		{
                        //SvLog::writeLog("Can not get inf of " + spr.ip);
                        SvLog::writeErrorLog(spr.ip, ERR_INF_LOG);
		}
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		ifprop_list.insert(inflist_cur.begin(), inflist_cur.end());
	}

	if(drctdata_cur.empty())
	{
		if(bDirect)
		{
                        //SvLog::writeLog("Can not get DIRECT of " + spr.ip);
                        SvLog::writeErrorLog(spr.ip, ERR_DRC_LOG);
		}
	}
	else
	{
                //mutex::scoped_lock lock(m_data_mutex);
		directdata_list.insert(drctdata_cur.begin(), drctdata_cur.end());
	}

	Sleep(500);
        //SvLog::writeLog("End read the cdp data of " + spr.ip);
}

//获取所有设备的CDP数据
void ReadService::getCdpDeviceData(vector<SnmpPara>& spr_list)
{
        getSysInfos(spr_list);

	aft_list.clear();
	arp_list.clear();
	ifprop_list.clear();
	
	if(sproid_list.size() > 0)
	{
		if(sproid_list.size() == 1)
		{
			getOneCdpDeviceData(sproid_list.begin()->first, sproid_list.begin()->second.first, sproid_list.begin()->second.second);
		}
		else
		{
			//pool tp(scanPara.thrdamount);//(min(thrdAmount,ip_communitys.size()));
//                        pool tp(min(scanPara.thrdamount, sproid_list.size()));//by zhangyan 2008-12-29
                        for (list<pair<SnmpPara, pair<string,string> > >::const_iterator i = sproid_list.begin(); i != sproid_list.end(); ++i)
			{
                                //tp.schedule((boost::bind(&ReadService::getOneCdpDeviceData,this, (*i).first, (*i).second.first, (*i).second.second)));
			}
                       // tp.wait();
		}
	}
}

//获取所有设备的普通数据
bool ReadService::getDeviceData(vector<SnmpPara>& spr_list)
{
        if (!getSysInfos(spr_list))
        {
            return false;
        }
	
	aft_list.clear();
	arp_list.clear();
	ifprop_list.clear();
	ospfnbr_list.clear();
	//route_list.clear(); //changed by zhang 2009-03-26 去掉路由表的取数
	bgp_list.clear();

	if(sproid_list.size() > 0)
	{
                qDebug() << "sproid size : " << sproid_list.size();
		if(sproid_list.size() == 1)
		{
			getOneDeviceData(sproid_list.begin()->first, sproid_list.begin()->second.first, sproid_list.begin()->second.second);
		}
		else
		{
			//pool tp(scanPara.thrdamount);//(min(thrdAmount,ip_communitys.size()));
//                        pool tp(min(scanPara.thrdamount, sproid_list.size()));//by zhangyan 2008-12-29
                        for (list<pair<SnmpPara, pair<string,string> > >::const_iterator i = sproid_list.begin(); i != sproid_list.end(); ++i)
			{
                                //if (isStop)
                                //{
                                //    return false;
                                //}
                                //else
                                //{
                                //    //tp.schedule((boost::bind(&ReadService::getOneDeviceData,this, (*i).first, (*i).second.first, (*i).second.second)));
                                //}
			}
                        //tp.wait();
		}
	}
        return true;
}


// 获取特定设备的ARP的数据
void ReadService::getOneArpData(const SnmpPara& spr, ARP_LIST& arplist)
{
	arplist.clear();
	SnmpDG snmp;
	//IP-端口索引1.3.6.1.2.1.4.22.1.1
        list<pair<string,string> > IpInfInx = snmp.GetMibTable(spr, "1.3.6.1.2.1.4.22.1.1");
	if(!IpInfInx.empty())
	{
                std::map<std::string, std::list<pair<std::string,std::string> > > inf_ipmacs; //{inf,[(ip,mac)]}
                std::map<std::string, std::list<pair<std::string,std::string> > >::iterator i_pm;
		//IP-MAC地址1.3.6.1.2.1.4.22.1.2
                list<pair<string,string> > IpMacs = snmp.GetMibTable(spr, "1.3.6.1.2.1.4.22.1.2");
                for(list<pair<string,string> >::iterator imac = IpMacs.begin(); imac != IpMacs.end(); ++imac)
		{
			vector<string> v1 = tokenize(imac->first.substr(21), ".", true);
			size_t len = v1.size();
			if(len < 4)
			{
				continue;
			}
			string ip_tmp  = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
			string mac_tmp = replaceAll(imac->second, " ","").substr(0,12);
                        for(list<pair<string,string> >::iterator iInf = IpInfInx.begin(); iInf != IpInfInx.end(); ++iInf)
			{
				size_t iPlace = iInf->first.rfind(ip_tmp);
				if(iPlace > 0)
				{
					i_pm = inf_ipmacs.find(iInf->second);
					if(i_pm != inf_ipmacs.end())
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
						inf_ipmacs.insert(make_pair(iInf->second, ipmac_list));
					}
					break;
				}
			}
		}
		if(!inf_ipmacs.empty())
		{
			arplist.insert(make_pair(spr.ip, inf_ipmacs));
		}
	}
	else
	{//使用telnet读取ARP表
		//if(!getOneArpByTelnet(ip))
		//{
                //	//SvLog::writeLog("Can not read the arp table of " + ip);
                //	//SvLog::writeErrorLog(ip,ERR_ARP_LOG);
		//}
		//else
		//{
                //	//SvLog::writeLog("Successe read the arp table of " + ip);
		//}
	}
}
//added by zhangyan 2008-10-10
void ReadService::getOneArpData(const SnmpPara& spr, std::map<std::string, std::list<std::pair<std::string,std::string> > >& inf_ipmacs)
{	
	SnmpDG snmp;
	//IP-端口索引1.3.6.1.2.1.4.22.1.1
        list<pair<string,string> > IpInfInx = snmp.GetMibTable(spr, "1.3.6.1.2.1.4.22.1.1");
	if(!IpInfInx.empty())
        {
                std::map<std::string, std::list<pair<std::string,std::string> > >::iterator i_pm;
		//IP-MAC地址1.3.6.1.2.1.4.22.1.2
                list<pair<string,string> > IpMacs = snmp.GetMibTable(spr, "1.3.6.1.2.1.4.22.1.2");
                for(list<pair<string,string> >::iterator imac = IpMacs.begin(); imac != IpMacs.end(); ++imac)
		{
			vector<string> v1 = tokenize(imac->first.substr(21), ".", true);
			size_t len = v1.size();
			if(len < 4)
			{
				continue;
			}
			string ip_tmp  = v1[len-4] + "." + v1[len-3] + "." + v1[len-2] + "." + v1[len-1];
			string mac_tmp = replaceAll(imac->second, " ","").substr(0,12);
                        for(list<pair<string,string> >::iterator iInf = IpInfInx.begin(); iInf != IpInfInx.end(); ++iInf)
			{
				size_t iPlace = iInf->first.rfind(ip_tmp);
				//remarked by zhangyan 2008-10-14
				//if(iPlace > 0)
				if(iPlace != string::npos)
				{
					i_pm = inf_ipmacs.find(iInf->second);
					if(i_pm != inf_ipmacs.end())
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
						inf_ipmacs.insert(make_pair(iInf->second, ipmac_list));
					}
					break;
				}
			}
		}		
	}	
}


void ReadService::getIpMaskList(const SnmpPara& spr, list<pair<string, string> >& ipcm_result)
{
	ipcm_result.clear();
	SnmpDG snmp;

	//IP-MASK地址1.3.6.1.2.1.4.20.1.3
        list<pair<string,string> > ipmsks = snmp.GetMibTable(spr,"1.3.6.1.2.1.4.20.1.3");
	if(!ipmsks.empty())
	{
                for(list<pair<string,string> >::iterator i = ipmsks.begin();	i != ipmsks.end();	++i)
		{
			string ip_cur = i->first.substr(21);
			if(ip_cur != "" && ip_cur != "0.0.0.0" //排除任意匹配地址
				&& ip_cur.compare(0,3,"127") != 0  //排除环回地址
				//add by wings 2009-11-13
				&& ip_cur.compare(0,5,"0.255") != 0
//				&& (ip_cur.compare(0,3,"224") < 0 || ip_cur.compare(0,3,"239") > 0) //排除组播地址
				)
			{
				//added by zhangyan 2009-01-15
				if(i->second.empty())
				{
					continue;
				}

				pair<string,string> ipm_tmp = make_pair(ip_cur, i->second);
				if(find(ipcm_result.begin(), ipcm_result.end(), ipm_tmp) == ipcm_result.end())
				{
					ipcm_result.push_back(ipm_tmp);
				}
			}
		}
	}
}

