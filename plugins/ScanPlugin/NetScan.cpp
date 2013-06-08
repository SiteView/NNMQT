//NetScan.cpp
/*
 * Topo scan console program
 *
 * Copyright (c) 2007-2008 DragonFlow
 *
 * Created date: 2007-12-06
 * Autor: Tang guofu 
 * last modified 2008-04-21 by Tang guofu 
 *
 */

#include "StdAfx.h"
#include "NetScan.h"
#include "TraceAnalyse.h"
#include "TraceReader.h"
#include "cpucount.h"
#include <QtDebug>
#include "svlog.h"
//#include "dlgscaninfo.h"

bool isStop = false;



NetScan::NetScan(const DEVICE_TYPE_MAP& devtypemap, const SPECIAL_OID_LIST& specialoidlist, SCAN_PARAM param, QObject *parent): QThread(parent)
{
        //isStop = false;
        //m_IsCdpScan = isCdpScan;
	getLocalhostIPs();
        //FormatConfig(config);

//        scanParam.depth = 5;
//        scanParam.timeout = 200;
//        scanParam.thrdamount = 50;
//        scanParam.retrytimes = 3;
        ReadConfig(param);
        //m_callback = callback_function;
	ReadMyScanConfigFile();
        siReader = new ReadService(devtypemap, scanParam, myParam, specialoidlist);	//update by jiangshanwen 2010-7-21
}

NetScan::~NetScan(void)
{
	delete siReader;
}

void NetScan::Start()
{
    start(HighestPriority);
}

void NetScan::run()
{
    Scan();
}

void NetScan::SetStop(bool stop)
{
    isStop = stop;
    qDebug() << "set stoped : " << stop << " : " << isStop;
}


//add by zhangyan 2008-10-09
void NetScan::getLocalNet(SCALE_LIST& localip_scales, string& localip)
{		
        std::list<pair<std::string, std::string> >::iterator j;
        std::list<pair<std::string, std::string> > ipmsk_list;
	//get ip-mask
	siReader->getIpMaskList(SnmpPara(localip, getCommunity_Get(localip), scanParam.timeout, scanParam.retrytimes), ipmsk_list);	
	for (j = ipmsk_list.begin(); j != ipmsk_list.end(); ++j)
	{
		//排除虚拟IP－Mask(如VPPN)
		if (find(localip_list.begin(), localip_list.end(), j->first) == localip_list.end())
		{
			continue;
		}
		//if(j->first.find("0.0.0.") == 0) continue;
		std::pair<string,string> scale_cur = getScaleByIPMask(*j);
		bool bExist = false;
                for (std::list<std::pair<std::string,std::string> >::iterator k = localip_scales.begin();
			k != localip_scales.end();
			++k)
		{
			if (k->first == scale_cur.first && k->second == scale_cur.second)
			{
				bExist = true;
				break;
			}
		}
		if (!bExist )
		{
			localip_scales.push_back(scale_cur);
			string ipscale = scale_cur.first + "-" + scale_cur.second;
                        ////SvLog::writeLog("add local host scale:" + ipscale );
		}		
	}
}

void NetScan::PingLocalNet(SCALE_LIST& localip_scales, string& localip)
{	
        ////SvLog::writeLog("Ping thread amount: " + int2str(scanParam.thrdamount));

	vector<string> ip_list_all;//范围内有效ips
	if ((myParam.filter_type == "1") && (!scanParam.scan_scales_num.empty()))
	{
		for (SCALE_LIST::iterator scale_cur = localip_scales.begin(); scale_cur != localip_scales.end(); ++scale_cur)
		{			
                        SvLog::writeLog("start ping local scale: " + scale_cur->first + "-" + scale_cur->second);
			unsigned long ipnumMin = ntohl(inet_addr(scale_cur->first.c_str()));
			unsigned long ipnumMax = ntohl(inet_addr(scale_cur->second.c_str()));
			struct in_addr addr;		
			for (unsigned long i = ipnumMin; i <= ipnumMax; ++i)
			{
				addr.S_un.S_addr = htonl(i);
				string ipStr = inet_ntoa(addr);
				if (ipStr.size() < 7 || ipStr.substr(ipStr.size()-4) == ".255" || ipStr.substr(ipStr.size()-2) == ".0" || 
					(find(localip_list.begin(),localip_list.end(), ipStr) != localip_list.end()))
				{//排除广播、缺省地址及本地ip
					continue;
				}				
				bool bExcluded = false;
				for (DEVID_LIST::iterator id = devid_list.begin(); id != devid_list.end(); ++id)
				{				
					if (find(id->second.ips.begin(), id->second.ips.end(), ipStr) != id->second.ips.end())
					{//排除已存在的设备ip
						bExcluded = true;
						break;						
					}			
				}
				if (bExcluded)
				{
					continue;
				}
				// added by zhangyan 2008-10-30	
				bool bAllowed = false;				
                                for (std::list<std::pair<unsigned long, unsigned long> >::iterator j = scanParam.scan_scales_num.begin(); j != scanParam.scan_scales_num.end(); ++j)
				{//允许的范围
					if (i <= j->second && i >= j->first)
					{					
						bAllowed = true;
                                                for (std::list<std::pair<unsigned long, unsigned long> >::iterator j = scanParam.filter_scales_num.begin();
							j != scanParam.filter_scales_num.end();
							++j)
						{//排除的范围ip
							if (i <= j->second && i >= j->first)
							{					
								bAllowed = false;
								break;
							}
						}
						break;
					}
				}
			
				if (bAllowed)
				{										
					ip_list_all.push_back(ipStr);					
				}			
			}
		}
	}
	else
	{
		for (SCALE_LIST::iterator scale_cur = localip_scales.begin(); scale_cur != localip_scales.end(); ++scale_cur)
		{			
                        SvLog::writeLog("start ping local scale: " + scale_cur->first + "-" + scale_cur->second);
			unsigned long ipnumMin = ntohl(inet_addr(scale_cur->first.c_str()));
			unsigned long ipnumMax = ntohl(inet_addr(scale_cur->second.c_str()));
			struct in_addr addr;		
			for (unsigned long i = ipnumMin; i <= ipnumMax; ++i)
			{
				addr.S_un.S_addr = htonl(i);
				string ipStr = inet_ntoa(addr);
				if (ipStr.size() < 7 || ipStr.substr(ipStr.size()-4) == ".255" || ipStr.substr(ipStr.size()-2) == ".0" || 
					(find(localip_list.begin(),localip_list.end(), ipStr) != localip_list.end()))
				{//排除广播、缺省地址及本地ip
					continue;
				}				
				bool bExcluded = false;
				for (DEVID_LIST::iterator id = devid_list.begin(); id != devid_list.end(); ++id)
				{				
					if (find(id->second.ips.begin(), id->second.ips.end(), ipStr) != id->second.ips.end())
					{//排除已存在的设备ip
						bExcluded = true;
						break;						
					}			
				}
				if (bExcluded)
				{
					continue;
				}
                                for (std::list<std::pair<unsigned long, unsigned long> >::iterator j = scanParam.filter_scales_num.begin();
					j != scanParam.filter_scales_num.end();
					++j)
				{//排除的范围ip
					if (i <= j->second && i >= j->first)
					{					
						bExcluded = true;
						break;
					}
				}
			
				if (!bExcluded)
				{										
					ip_list_all.push_back(ipStr);					
				}			
			}
		}
	}

	if (!ip_list_all.empty())
	{
		/*ofstream output("ping_iplist.txt", ios::out);
		string line = "";
		for (vector<string>::iterator iter = ip_list_all.begin(); iter != ip_list_all.end(); ++iter)
		{
			line += " "+ *iter; 
		}
		output << line << endl;*/

		//do ping			
		vector<string> iplist_to_ping;	
		PingFind myPing(scanParam.thrdamount);
		SnmpPara snmpPara(localip, getCommunity_Get(localip), scanParam.timeout, scanParam.retrytimes);
		size_t iTotal = ip_list_all.size();
		int istart = 0, iend = 0;
		int iBatchs = (iTotal + 99) / 100;
		for (int btchs = 0; btchs < iBatchs; ++btchs)
		{//ping 100 devices at a time
			iend += 100;
			if(iend > iTotal)
			{
				iend = iTotal;
			}
			iplist_to_ping.assign(ip_list_all.begin() + istart, ip_list_all.begin() + iend);
			istart = iend;
			/*myPing.multiPing_simple(iplist_to_ping, scanParam.retrytimes, scanParam.timeout);*/
			myPing.multiPing_simple(iplist_to_ping, 2, 1000);

			//get local arp
			siReader->getOneArpData(snmpPara, localport_macs);
		}			
	}
	
	if(!localport_macs.empty())
	{
		arp_list.insert(make_pair(localip, localport_macs));
	}
}

// 获取本地主机IP地址
bool NetScan::getLocalhostIPs(void)
{
	localip_list.clear();

        char ac[80];
        int i = gethostname(ac, sizeof(ac));
        if (i == SOCKET_ERROR) {
                /*
		cerr << "Error " << WSAGetLastError() << 
		" when getting local host name." << endl; 
                */
		return 1; 
	} 
	struct hostent *phe = gethostbyname(ac); 
	if (phe == 0) 
	{ 
		return false; 
	} 
	for (int i = 0; phe->h_addr_list[i] != 0; ++i) 
	{ 
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr)); 
		localip_list.push_back(inet_ntoa(addr));
                SvLog::writeLog(string("Local IP")+ int2str(i) + ": " + inet_ntoa(addr));
	}

	//added by zhangyan 2009-01-12
	if (!localip_list.empty())
	{
		localip = *(localip_list.begin());
	}
	else
	{
		localip = "";
	}
	localport_macs.clear();
	return true;
}

// 将ICE结构转换成参数类型
/*
bool NetScan::FormatConfig(SliceNNM::ScanConfig & config)
{
	if(config.DefaultCommunityGet.empty())
	{
		scanParam.community_get_dft = "public";
	}
	else
	{
		scanParam.community_get_dft = Utf8ToString(config.DefaultCommunityGet);
	}
	//scanParam.community_set_dft = Utf8ToString(config.DefaultCommunitySet);
	if(config.Depth <= 0)
	{
		scanParam.depth = 5;
	}
	else
	{
		scanParam.depth = int(config.Depth);
	}
	if(config.TryTimes <= 0)
	{
		scanParam.retrytimes = 1;
	}
	else
	{
		scanParam.retrytimes = int(config.TryTimes);
	}
	if(config.Timeout < 100)
	{
		scanParam.timeout = 200;//==2000ms 低于100ms的超时设置启用默认值200 *10ms
	}
	else
	{
		scanParam.timeout = int(config.Timeout)/10;
	}
	// remarked by zhangyan 2008-10-10
	//if(config.MaxTreadCount <= 0)
	//{
	//	scanParam.thrdamount = 50;
	//}
	//else
	//{
	//	scanParam.thrdamount = min(MINUS_THREAD_AMOUNT, int(config.MaxTreadCount));
	//}
	int cpu_total = getCPUCount();
	if (cpu_total > 0)
	{
		scanParam.thrdamount = min(MINUS_THREAD_AMOUNT, cpu_total * 20);
	}
	else
	{
		scanParam.thrdamount = 40;
	}	

	for (SliceNNM::SeedIpLst::iterator i = config.SeedIpList.begin();
		i != config.SeedIpList.end();
		++i) 
	{
		std::string seed_cur = Utf8ToString(*i);
		scanParam.scan_seeds.push_back(seed_cur);
	}

	// remarked by zhangyan 2008-10-27
	/*if(!scanParam.scan_seeds.empty() && !localip_list.empty())
	{
		string localip = *(localip_list.begin());
		if( find(scanParam.scan_seeds.begin(), scanParam.scan_seeds.end(), localip) == scanParam.scan_seeds.end())
		{
			scanParam.scan_seeds.push_back(localip);
		}
	}*/
        /*
	if (!localip.empty())
	{
		if (find(scanParam.scan_seeds.begin(), scanParam.scan_seeds.end(), localip) == scanParam.scan_seeds.end())
		{
			scanParam.scan_seeds.push_back(localip);
		}
	}

	scanParam.scan_scales.clear();
	scanParam.scan_scales_num.clear();
	
	for (SliceNNM::AddStartEndLst::iterator i = config.AddStartEndList.begin();
		i != config.AddStartEndList.end();
		++i) 
	{
                std::string startip = Utf8ToString(i->StartIp);
		std::string endip   = Utf8ToString(i->EndIp);
		//scanParam.scan_scales.push_back(make_pair(startip,endip));
		unsigned long startnum = ntohl(inet_addr(startip.c_str()));
		unsigned long endnum   = ntohl(inet_addr(endip.c_str()));
		//scanParam.scan_scales_num.push_back(make_pair(startnum,endnum));
		// begin added by zhangyan 2008-12-30
		// 将大范围分成小范围
		unsigned long endnum_temp;
		struct in_addr addr;
		while ((endnum-startnum) > 256)
		{						
			addr.S_un.S_addr = htonl(startnum);
			startip = inet_ntoa(addr);
			endnum_temp = startnum + 256;			
			addr.S_un.S_addr = htonl(endnum_temp);
			endip = inet_ntoa(addr);
			scanParam.scan_scales.push_back(make_pair(startip, endip));
			scanParam.scan_scales_num.push_back(make_pair(startnum, endnum_temp));
			//cout<<"ip: "<<startip.c_str()<<" "<<endip.c_str()<<endl;
			//cout<<"num: "<<startnum<<" "<<endnum_temp<<endl;
			startnum = endnum_temp + 1;
		}
		if ((endnum-startnum) >= 0)
		{
			addr.S_un.S_addr = htonl(startnum);
			startip = inet_ntoa(addr);
			addr.S_un.S_addr = htonl(endnum);
			endip = inet_ntoa(addr);
			scanParam.scan_scales.push_back(make_pair(startip, endip));
			scanParam.scan_scales_num.push_back(make_pair(startnum, endnum));
			//cout<<"last ip: "<<startip.c_str()<<" "<<endip.c_str()<<endl;
			//cout<<"last num: "<<startnum<<" "<<endnum<<endl;
		}
		// end added by zhangyan 2008-12-30
	}

	scanParam.communitys.clear();
	scanParam.communitys_num.clear();
	for (SliceNNM::CommunityLst::iterator i = config.CommunityList.begin();
		i != config.CommunityList.end();
		++i) 
	{
		std::string startip = Utf8ToString(i->StartIp);
		std::string endip   = Utf8ToString(i->EndIp);
		unsigned long startnum = ntohl(inet_addr(startip.c_str()));
		unsigned long endnum   = ntohl(inet_addr(endip.c_str()));
		std::pair<std::string, std::string> cmnty = make_pair(Utf8ToString(i->Get),Utf8ToString(i->Set));

		scanParam.communitys.push_back(make_pair(make_pair(startip,endip), cmnty));
		scanParam.communitys_num.push_back(make_pair(make_pair(startnum,endnum), cmnty));
	}

	scanParam.filter_scales.clear();
	scanParam.filter_scales_num.clear();
	for (SliceNNM::FilterStartEndLst::iterator i = config.FilterStartEndList.begin();
		i != config.FilterStartEndList.end();
		++i) 
	{
		std::string startip = Utf8ToString(i->StartIp);
		std::string endip   = Utf8ToString(i->EndIp);
		unsigned long startnum = ntohl(inet_addr(startip.c_str()));
		unsigned long endnum   = ntohl(inet_addr(endip.c_str()));
		scanParam.filter_scales.push_back(make_pair(startip, endip));
		scanParam.filter_scales_num.push_back(make_pair(startnum, endnum));
	}

	return true;
}
*/

//读取界面配置
void NetScan::ReadConfig(SCAN_PARAM param)
{
    scanParam = param;
//    scanParam.community_get_dft = q2s(config->getDefaultCommunityGet());
//    scanParam.community_get_dft = q2s(wizard->communityOfRangeConfigWizard->DefaultGetCommunity);

//    int iDepth = wizard->scanPerformanceConfigWizard->ScanDeep;
//    int iTryTimes = wizard->scanPerformanceConfigWizard->Retry;
//    int iTimeOut = wizard->scanPerformanceConfigWizard->TimeOut;
//    //int iThread = wizard->scanPerformanceConfigWizard->SynThreadCount;

//    if(iDepth <= 0)
//    {
//            scanParam.depth = 5;
//    }
//    else
//    {
//            scanParam.depth = iDepth;
//    }

//    if(iTryTimes <= 0)
//    {
//            scanParam.retrytimes = 1;
//    }
//    else
//    {
//            scanParam.retrytimes = iTryTimes;
//    }


//    if(iTimeOut < 100)
//    {
//            scanParam.timeout = 200;//==2000ms 低于100ms的超时设置启用默认值200 *10ms
//    }
//    else
//    {
//            scanParam.timeout = iTimeOut/10;
//    }

//    int cpu_total = getCPUCount();
//    qDebug() << "cpu count : " << cpu_total;
//    if (cpu_total > 0)
//    {
//            scanParam.thrdamount = min(MINUS_THREAD_AMOUNT, cpu_total * 20);
//            qDebug() << "thrdamount : " << scanParam.thrdamount;
//    }
//    else
//    {
//            scanParam.thrdamount = 40;
//    }

//    scanParam.scan_scales.clear();
//    scanParam.scan_scales_num.clear();
//    foreach(QString scanscales, wizard->scanRangeConfigWizard->ScanRangeList)
//    {
//        QStringList startendip = scanscales.split("-", QString::SkipEmptyParts);
//        string startip = q2s(startendip.at(0));
//        string endip = q2s(startendip.at(1));
//        unsigned long startnum = ntohl(inet_addr(startip.c_str()));
//        unsigned long endnum   = ntohl(inet_addr(endip.c_str()));

//        // 将大范围分成小范围
//        unsigned long endnum_temp;
//        struct in_addr addr;
//        while ((endnum-startnum) > 256)
//        {
//                addr.S_un.S_addr = htonl(startnum);
//                startip = inet_ntoa(addr);
//                endnum_temp = startnum + 256;
//                addr.S_un.S_addr = htonl(endnum_temp);
//                endip = inet_ntoa(addr);
//                scanParam.scan_scales.push_back(make_pair(startip, endip));
//                scanParam.scan_scales_num.push_back(make_pair(startnum, endnum_temp));
//                startnum = endnum_temp + 1;
//        }
//        if ((endnum-startnum) >= 0)
//        {
//                addr.S_un.S_addr = htonl(startnum);
//                startip = inet_ntoa(addr);
//                addr.S_un.S_addr = htonl(endnum);
//                endip = inet_ntoa(addr);
//                scanParam.scan_scales.push_back(make_pair(startip, endip));
//                scanParam.scan_scales_num.push_back(make_pair(startnum, endnum));
//        }
//    }



//    scanParam.communitys.clear();
//    scanParam.communitys_num.clear();
//    foreach(QString communityRangeList, wizard->communityOfRangeConfigWizard->CommunityOfRangeList)
//    {
//        QStringList community = communityRangeList.split("-", QString::SkipEmptyParts);
//        string startip = q2s(community.at(0));
//        string endip = q2s(community.at(1));
//        unsigned long startnum = ntohl(inet_addr(startip.c_str()));
//        unsigned long endnum   = ntohl(inet_addr(endip.c_str()));

//        std::pair<std::string, std::string> cmnty = make_pair( q2s(community.at(2)),"");

//        qDebug() << "start : " <<startip.c_str() << " end : " << endip.c_str() << " get: " << q2s(community.at(2)).c_str();

//        scanParam.communitys.push_back(make_pair(make_pair(startip,endip), cmnty));
//        scanParam.communitys_num.push_back(make_pair(make_pair(startnum,endnum), cmnty));
//    }

//    //QStringList ExcludeRangeList = startElem.firstChildElement("ExcludeRange").text().split(";",QString::SkipEmptyParts);

//    scanParam.scan_seeds.clear();
//    foreach(QString seeds, wizard->seedsConfigWizard->SeedsList)
//    {
//        scanParam.scan_seeds.push_back(q2s(seeds));
//    }

//    scanParam.filter_scales.clear();
//    scanParam.filter_scales_num.clear();
//    foreach(QString str, wizard->excludeRangeConfigWizard->ExcludeRangeList)
//    {
//            QStringList startendIp = str.split("-", QString::SkipEmptyParts);
//            string excludestartip = q2s(startendIp.at(0));
//            string excludeendip   = q2s(startendIp.at(1));
//            unsigned long startnum = ntohl(inet_addr(excludestartip.c_str()));
//            unsigned long endnum   = ntohl(inet_addr(excludeendip.c_str()));
//            scanParam.filter_scales.push_back(make_pair(excludestartip, excludeendip));
//            scanParam.filter_scales_num.push_back(make_pair(startnum, endnum));
//    }
}

// 获取IP对应的共同体名
std::string NetScan::getCommunity_Get(const std::string& ip)
{
	std::string community_ret(scanParam.community_get_dft);
        std::list<std::pair<std::pair<unsigned long, unsigned long>, std::pair<std::string,std::string> > >::iterator i;
	unsigned long ipnum = ntohl(inet_addr(ip.c_str()));
	for(i = scanParam.communitys_num.begin(); i != scanParam.communitys_num.end(); ++i)
	{
		if(ipnum >= i->first.first && ipnum <= i->first.second)
		{
			return i->second.first;
		}
	}


	return community_ret;
}

// 获取IP对应的SNMP Version
std::string NetScan::getSNMPVersion(const std::string& ip)
{
	/*std::string sVersion(scanParam.snmpv_dft);
	if (scanParam.specSNMPVList.empty())
	{
		return sVersion;
	}
        for (std::list<std::pair<std::string,std::string> >::iterator iter = scanParam.specSNMPVList.begin(); iter != scanParam.specSNMPVList.end(); ++iter)
	{
		if (iter->first == ip)
		{
			return iter->second;
		}		
	}*/
	std::string sVersion(myParam.snmp_version);
	if (myParam.SNMPV_list.empty())
	{
		return sVersion;
	}
        for (std::list<std::pair<std::string,std::string> >::iterator iter = myParam.SNMPV_list.begin(); iter != myParam.SNMPV_list.end(); ++iter)
	{
		if (iter->first == ip)
		{
			return iter->second;
		}		
	}
	return sVersion;
}
//主扫描函数
void NetScan::Scan(void)
{
        /*
	if(m_IsCdpScan)
	{
		Scan_cdp();
		return;
	}
        */

    devid_list.clear();
    ifprop_list.clear();
    aft_list_frm.clear();
    arp_list_frm.clear();
    ospfnbr_list.clear();
    rttbl_list.clear();
    bgp_list.clear();
    directdata_list.clear();

        start(LowPriority);
	m_ip_list_visited.clear();

	std::string msg = "";
	if(myParam.scan_type == "2"||myParam.scan_type == "1")
	{
                //SvLog::writeLog("analyse through old data...", COMMON_MSG, m_callback);
                SvLog::writeLog("analyse through old data...");
		readOriginData();
	}
	if(myParam.scan_type == "0"||myParam.scan_type == "2")
	{//进行全新扫描
                    if(myParam.ping_type == "2")
		{
                        SvLog::writeLog("scan by ips...");
			scanByIplist();
		}
		else
		{
			if(!scanParam.scan_scales.empty())
			{//按范围扫描
                                SvLog::writeLog("scan by scales...");
                                if (!ScanByScale(scanParam.scan_scales))
                                {
                                    qDebug() << "stop scan";
                                    emit StopScan();
                                    return;
                                }
			}
			else
			{//按种子扫描
                                SvLog::writeLog("Scan by seeds...");
				std::list<std::string> seeds_cur;				
				seeds_cur.assign(scanParam.scan_seeds.begin(), scanParam.scan_seeds.end());
				// by zhangyan 2008-10-27				
				/*if(seeds_cur.empty())
				{
					if(!localip_list.empty())
					{
						seeds_cur.push_back(*(localip_list.begin()));
					}
				}*/
				if(seeds_cur.empty())
				{
                                        //SvLog::writeLog("There existed no valided seeds.", COMMON_MSG, m_callback);
                                        SvLog::writeLog("There existed no valided seeds.");
					return;
				}
				if(myParam.seed_type == "0")
                                {
                                        if (!ScanBySeeds(seeds_cur))
                                        {
                                           emit StopScan();
                                           return ;
                                        }

                                }
				else
					ScanBySeedsByArp(seeds_cur);
			}
		}	
                SvLog::writeLog("End Scan.");

		//将接口表中的MAC添加到设备的mac地址表中
		for(DEVID_LIST::iterator i = devid_list.begin();
			i != devid_list.end();
			++i)
		{
			IFPROP_LIST::iterator iinf =  ifprop_list.find(i->first);
			if(iinf != ifprop_list.end())
			{
				for(list<IFREC>::iterator j = iinf->second.second.begin();
					j != iinf->second.second.end();
					++j)
				{
					if(j->ifMac != "" && j->ifMac != "000000000000" && j->ifMac != "FFFFFFFFFFFF")
					{
						if(find(i->second.macs.begin(), i->second.macs.end(), j->ifMac) == i->second.macs.end())
						{
							i->second.macs.push_back(j->ifMac);
						}
					}
				}
			}
		}
		
	//delete by wings 2009-11-13
		/*if (!localport_macs.empty())
		{
			arp_list.insert(make_pair(localip, localport_macs));
		}
		else
		{
			//add by zhangyan 2008-10-09
			//todo: 读取本机arp(先ping本网段，再读)
			for (list<string>::iterator iter = localip_list.begin(); iter != localip_list.end(); ++iter)
			{
				SCALE_LIST localip_scales;	
				getLocalNet(localip_scales, *iter);
				if (!localip_scales.empty())
				{
					PingLocalNet(localip_scales, *iter);
					break;
				}				
			}
			
		}*/

                SvLog::writeLog("To Save.");
		saveOriginData();
                SvLog::writeLog("End Save.");
	}
	//delete by wings 2009-11-13
	/*else
	{//从文件获取数据
                //SvLog::writeLog("analyse through old data...", COMMON_MSG, m_callback);
		readOriginData();
	}*/

        SvLog::writeLog("To Format data.");
	FormatData();
        SvLog::writeLog("To save Format data.");
	saveFormatData();

        //SvLog::writeLog("Analyse data...", COMMON_MSG, m_callback);
		SvLog::writeLog("Analyse data...");

	if(devid_list.empty())
	{
		topo_edge_list.clear();		
	}
	else if(ExistNetDevice(devid_list))
	{
		topoAnalyse TA(devid_list, ifprop_list, aft_list_frm, arp_list_frm, ospfnbr_list, rttbl_list, bgp_list, directdata_list, myParam);
		if(TA.EdgeAnalyse())
		{
			topo_edge_list = TA.getEdgeList();
			// add by zhangyan 2008-09-24
			if ((myParam.tracert_type == "1") && (TraceAnalyse::getConnection(topo_edge_list) > 1))
			{
				//TODO(read,analyse)
				TraceReader traceR(devid_list, bgp_list, scanParam.retrytimes, scanParam.timeout, 30);				
				traceR.TracePrepare();
				if (myParam.scan_type == "0")
				{
					rtpath_list = traceR.getTraceRouteByIPs();
					//将trace path保存到文件
					StreamData myfile;
					myfile.saveTracertList(rtpath_list);

					TraceAnalyse traceA(devid_list, rtpath_list, traceR.RouteDESTIPPairList, traceR.unManagedDevices, scanParam);
					traceA.AnalyseRRByRtPath(topo_edge_list);
				}
				else
				{//从文件扫描时
					TraceAnalyse traceA(devid_list, rtpath_list, traceR.RouteDESTIPPairList, traceR.unManagedDevices);
					traceA.AnalyseRRByRtPath_Direct(topo_edge_list);
				}				
				
			}		

			//补充边的附加信息
			FillEdge(topo_edge_list);
			//创建哑设备
			GenerateDumbDevice(topo_edge_list,devid_list);
			if(myParam.commit_pc == "0")//not commit pc to svdb
			{
                                SvLog::writeLog("delete pc from entities.");
				for(DEVID_LIST::iterator j = devid_list.begin();j != devid_list.end();++j)
				{
					/*if(j->second.devType != "5" && j->second.devType != "4")*/
					if(j->second.devType != "5")
					{
						topo_entity_list.insert(*j);						
					}
				}			
			}
			else
			{
				topo_entity_list = devid_list;
			}
                        SvLog::writeLog("success to analyse.");
                        emit FinishAnalyse();
		}
		else
		{
                        SvLog::writeLog("fail to analyse.");
		}
	}
}

bool NetScan::ExistNetDevice(const DEVID_LIST& dev_list)
{
	for(DEVID_LIST::const_iterator i = dev_list.begin();
		i != dev_list.end();
		++i)
	{
		if(i->second.devType == "0" || i->second.devType == "1" || i->second.devType == "2" || i->second.devType == "3" || i->second.devType == "4")
		{
			return true;
		}
	}
	return false;
}


// 按范围进行扫描
bool NetScan::ScanByScale(SCALE_LIST scales)
{
	for(SCALE_LIST::iterator i = scales.begin(); i != scales.end(); ++i)
	{
                if (scanOneScale(*i, false))
                {

                }
                else
                {
                    return false;
                }
	}

	devid_list = siReader->devid_list_visited;//by zhangyan 2008-10-20
        return true;
}

// 按种子进行扫描
bool NetScan::ScanBySeeds(std::list<std::string> seed_list)
{
	m_scale_list_scaned.clear();
	m_scale_list_toscan.clear();
	//首先从种子发现子网
        list<pair<std::string, std::string> >::iterator j;
	for(std::list<std::string>::iterator i = seed_list.begin(); i != seed_list.end(); ++i)
	{
                list<pair<std::string, std::string> > ipmsk_list;
		siReader->getIpMaskList(SnmpPara(*i, getCommunity_Get(*i), scanParam.timeout, scanParam.retrytimes), ipmsk_list);
		for(j = ipmsk_list.begin(); j != ipmsk_list.end(); ++j)
		{
			//if(j->first.find("0.0.0.") == 0) continue; //by zhangyan 2008-10-21
			std::pair<string,string> scale_cur = getScaleByIPMask(*j);
			bool bExist = false;
                        for(std::list<std::pair<std::string,std::string> >::iterator k = m_scale_list_toscan.begin();
				k != m_scale_list_toscan.end();
				++k)
			{
				if( k->first == scale_cur.first && k->second == scale_cur.second)
				{
					bExist = true;
					break;
				}
			}
			if( !bExist )
			{
				m_scale_list_toscan.push_back(scale_cur);
                                //SvLog::writeLog(scale_cur.first + "-" + scale_cur.second, FIND_SUBNET_MSG, m_callback);
								SvLog::writeLog(scale_cur.first + "-" + scale_cur.second);
			}
		}
	}
	
	for(unsigned int depth = 0; depth < scanParam.depth; ++depth)
	{
                SvLog::writeLog(string("start depth ") + int2str(depth) + " scan.");
                list<pair<string,string> > scale_list_cur;
		scale_list_cur.assign(m_scale_list_toscan.begin(), m_scale_list_toscan.end());
		while(!scale_list_cur.empty())
		{
			std::pair<std::string, std::string> scale_cur = scale_list_cur.front();
			scale_list_cur.pop_front();
			m_scale_list_toscan.pop_front();
			m_scale_list_scaned.push_back(scale_cur);
                        if (scanOneScale(scale_cur,true))
                        {

                        }
                        else
                        {
                            return false;
                        }
		}
	}

	devid_list = siReader->devid_list_visited;//by zhangyan 2008-10-20

        return true;
}

//add by wings 2009-12-15
void NetScan::ScanBySeedsByArp(std::list<std::string> seed_list)
{
	seeds_arp.clear();
	for(std::list<std::string>::iterator i = seed_list.begin(); i != seed_list.end(); ++i)
	{
		seeds_arp.push_back(*i);
	}
	for(unsigned int depth = 0; depth < scanParam.depth; ++depth)
	{
                SvLog::writeLog(string("start depth ") + int2str(depth) + " scan.");
		if(seeds_arp.empty())
			break;
		scanByIps(seeds_arp,false);
		ScanSeeds_Arp();
	}
	devid_list = siReader->devid_list_visited;//add by wings 2009-12-17
}

//add by wings 2009-12-15
void NetScan::ScanSeeds_Arp()
{
	ARP_LIST arp_list_cur;
	std::list<std::string> ip_list;
	for(std::vector<std::string>::iterator i = seeds_arp.begin();
		i != seeds_arp.end();
		i++)
	{
		ARP_LIST::iterator arp = arp_list.find(*i);
		if(arp != arp_list.end())
		{
                        for(std::map<std::string, std::list<std::pair<std::string,std::string> > >::
				iterator pmi = arp->second.begin();
				pmi != arp->second.end();
				++pmi)
			{
                                for(std::list<std::pair<std::string,std::string> >
					::iterator mi = pmi->second.begin();
					mi != pmi->second.end();
					++mi)
				{
					string ip = mi->first;
					if(find(ip_list.begin(),ip_list.end(),ip) == ip_list.end()
						&&find(m_ip_list_visited.begin(),m_ip_list_visited.end(),ip) 
							== m_ip_list_visited.end())
					{
						ip_list.push_back(ip);
					}
				}
			}
		}

		//add by jiangshanwen20100831
		ROUTE_LIST::iterator route = rttbl_list.find(*i);
		if (route != rttbl_list.end())
		{
                        for(map<string,list<ROUTEITEM> >::iterator k = route->second.begin();
				k != route->second.end();
				++k)
			{
				for(list<ROUTEITEM>::iterator dst = k->second.begin();
					dst != k->second.end();
					++dst)
				{
					string ip = dst->next_hop;
					if(find(ip_list.begin(),ip_list.end(),ip) == ip_list.end()
						&&find(m_ip_list_visited.begin(),m_ip_list_visited.end(),ip) 
						== m_ip_list_visited.end())
					{
						ip_list.push_back(ip);
					}
				}
			}
		}
	}

	
	seeds_arp.clear();
	if(ip_list.empty())
		return;
	for(std::list<std::string>::iterator i = ip_list.begin();
		i != ip_list.end();
		++i)
	{
		seeds_arp.push_back(*i);
	}
}

//根据接口索引列表与接口表,获取共同前缀
string NetScan::getInfDescPrex(const list<string>& infIndex_list, const list<IFREC>& inf_list)
{
	string prex("");
	list<string> prex_list;
	for(list<string>::const_iterator i = infIndex_list.begin(); i != infIndex_list.end(); ++i)
	{
		string port = *i;
		string d=port.substr(0,1);
		port = port.substr(1);
		while(port.substr(0,1) == "0" && port.substr(0,2) != "0/")
		{
			port = port.substr(1);
		}
		

		string prex_tmp = "";
		for(list<IFREC>::const_iterator j = inf_list.begin(); j != inf_list.end(); ++j)
		{
			string vlan = j->ifDesc;
                        transform(vlan.begin(),vlan.end(),vlan.begin(),(int(*)(int))toupper);
			if (vlan.find("VLAN") != string::npos || vlan.compare(0,1,d) != 0 || (j->ifDesc.find("/")!=string::npos && port.find("/")==string::npos))
			{
				continue;
			}
			size_t iPlace = j->ifDesc.find(port);
			//remarked by zhangyan 2008-10-14
			//if(iPlace > 0)
			if(iPlace != string::npos)
			{
				prex_tmp = j->ifDesc.substr(0, iPlace);
				break;
			}
			else
			{
				iPlace = j->ifDesc.find(replaceAll(port, ":", "/"));
				//remarked by zhangyan 2008-10-14
				//if(iPlace > 0)
				if(iPlace != string::npos)
				{
					prex_tmp = j->ifDesc.substr(0, iPlace);
					break;
				}
			}
		}
		if(!prex_tmp.empty())
		{
			prex_list.push_back(prex_tmp);
		}
	}
	int iMinLen = 100000;
	for(list<string>::iterator i = prex_list.begin(); i != prex_list.end(); ++i)
	{
		if((int)((*i).size()) < iMinLen)
		{
			iMinLen = (int)((*i).size());
			prex = *i;
		}
	}
	return prex;
}

string NetScan::findInfInxFromDescr(const list<IFREC>& inf_list, const string& port)
{
	for(list<IFREC>::const_iterator m = inf_list.begin();	m != inf_list.end(); ++m)
	{//通过接口描述信息寻找对应的接口索引
		size_t iPlace = m->ifDesc.find(port);
		//remarked by zhangyan 2008-10-14
		//if(iPlace > 0)
		if(iPlace != string::npos)
		{//需要修改端口
			return m->ifIndex;
		}
		else
		{
			string toport = port;
			iPlace = m->ifDesc.find(replaceAll(toport,"/", ":"));
			if(iPlace >= 0)
			{
				return m->ifIndex;
			}
		}
	}
	return port;
}

string NetScan::findInfPortFromDescr(const list<IFREC>& inf_list, const string& port)
{
	for(list<IFREC>::const_iterator m = inf_list.begin();	m != inf_list.end(); ++m)
	{//通过接口描述信息寻找对应的接口索引
		size_t iPlace = m->ifDesc.find(port);
		//remarked by zhangyan 2008-10-14
		//if(iPlace > 0)
		if(iPlace != string::npos)
		{//需要修改端口
			return m->ifPort;
		}
		else
		{
			string toport = port;
			iPlace = m->ifDesc.find(replaceAll(toport,"/", ":"));
			//remarked by zhangyan 2008-10-14
			//if(iPlace > 0)
			if(iPlace != string::npos)
			{
				return m->ifPort;
			}
		}
	}
	return port;
}

// 规范化数据文件
bool NetScan::FormatData(void)
{
	//begin added by tgf 2008-09-22
	//处理vrrp的数据:删除vrrp的ip-mac数据
	list<string> iplist_virtual;
	list<string> maclist_virtual;
	//by zhangyan 2009-01-09
	for(RouterStandby_LIST::iterator iter = routeStandby_list.begin(); iter != routeStandby_list.end(); ++iter)
	{		
		for(vector<string>::iterator ii = iter->second.virtualIps.begin(); ii != iter->second.virtualIps.end(); ++ii)
		{
			if(find(iplist_virtual.begin(), iplist_virtual.end(), *ii) == iplist_virtual.end())
			{
				iplist_virtual.push_back(*ii);
			}
		}
		for(vector<string>::iterator ii = iter->second.virtualMacs.begin(); ii != iter->second.virtualMacs.end(); ++ii)
		{
			if(find(maclist_virtual.begin(), maclist_virtual.end(), *ii) == maclist_virtual.end())
			{
				maclist_virtual.push_back(*ii);
			}
		}		
	}

	//for(VRRP_LIST::iterator i = vrrp_list.begin(); i != vrrp_list.end(); ++i)
	//{
	//	for(vector<string>::iterator j = i->second.assoips.begin(); j !=  i->second.assoips.end(); ++j)
	//	{
	//		if(find(iplist_virtual.begin(), iplist_virtual.end(), *j) == iplist_virtual.end())
	//		{
	//			iplist_virtual.push_back(*j);
	//		}
	//	}
	//	for(vector<VRID>::iterator j = i->second.vrids.begin(); j !=  i->second.vrids.end(); ++j)
	//	{
	//		if(find(maclist_virtual.begin(), maclist_virtual.end(), j->virtualMac) == maclist_virtual.end())
	//		{
	//			maclist_virtual.push_back(j->virtualMac);
	//		}
	//	}
	//}

	//format directdata	    
	DIRECTDATA_LIST drc_list_tmp;
	for(DIRECTDATA_LIST::iterator i = directdata_list.begin(); i != directdata_list.end(); ++i)
	{
		string left_ip = i->first;		
		for(DEVID_LIST::iterator s = devid_list.begin(); s != devid_list.end();	++s)
		{
			if(find(s->second.ips.begin(), s->second.ips.end(), left_ip) != s->second.ips.end())
			{
				left_ip = s->first;
				break;
			}
		}
		for(list<DIRECTITEM>::iterator j = i->second.begin(); j != i->second.end();)// ++j)
		{
			//add by zhangyan 2008-10-6
			string right_ip = j->PeerIP;			
			if (right_ip == "0.0.0.0")
			{
				i->second.erase(j++);
				continue;
			}
			if (right_ip.find(".") == string::npos)
			{
				bool bExist = false;
				for(DEVID_LIST::iterator iter = devid_list.begin(); iter != devid_list.end(); ++iter)
				{
					if (find(iter->second.macs.begin(), iter->second.macs.end(), right_ip) != iter->second.macs.end())
					{//mac-to-ip
						j->PeerIP = iter->first;
						bExist = true;
						break;
					}				
				}
				if(!bExist)
				{
					i->second.erase(j++);
				}
				else
					++j;
			}
			else
			{				
				for(DEVID_LIST::iterator s = devid_list.begin(); s != devid_list.end();	++s)
				{
					if(find(s->second.ips.begin(), s->second.ips.end(), right_ip) != s->second.ips.end())
					{						
						j->PeerIP = s->first;	
						break;
					}
				}
				++j;
			}			
		}
		drc_list_tmp[left_ip] = i->second;
	}
	directdata_list = drc_list_tmp;
	//end added by tgf 2008-09-22

	// added by zhangyan 2008-10-30	
	if ((myParam.filter_type == "1") && (!scanParam.scan_scales_num.empty()))
	{
                SvLog::writeLog("delete ips, while not in scan scales");
		for(ARP_LIST::iterator m_srcip = arp_list.begin(); m_srcip != arp_list.end(); ++m_srcip)
		{//对source ip 循环			
			for(std::map<std::string, std::list<pair<string,string> > >::iterator m_srcport = m_srcip->second.begin(); 
				m_srcport != m_srcip->second.end(); 
				++m_srcport)
			{//对source port 循环
                                for(list<pair<string,string> >::iterator destip_mac = m_srcport->second.begin();
					destip_mac != m_srcport->second.end(); )					
				{//对destip-mac	循环
					bool bAllowed = false;					
					unsigned long ipnum = ntohl(inet_addr(destip_mac->first.c_str()));					
                                        for (std::list<std::pair<unsigned long, unsigned long> >::iterator j = scanParam.scan_scales_num.begin();
						j != scanParam.scan_scales_num.end(); 
						++j)
					{//允许的范围
						if (ipnum <= j->second && ipnum >= j->first)
						{					
							bAllowed = true;
                                                        for (std::list<std::pair<unsigned long, unsigned long> >::iterator k = scanParam.filter_scales_num.begin();
								k != scanParam.filter_scales_num.end(); 
								++k)
							{//排除的范围ip
								if (ipnum <= k->second && ipnum >= k->first)
								{					
									bAllowed = false;
									break;
								}
							}
							break;
						}
					}
					if (!bAllowed)
					{						
						//删除不在允许范围内的ip-mac
						m_srcport->second.erase(destip_mac++);
					}
					else
					{
						destip_mac++;
					}
				}
			}
		}
		
	} // end by zhangyan 2008-10-30	

	//规范化接口, begin added by tgf 2008-07-04
	for(ARP_LIST::iterator i = arp_list.begin(); i != arp_list.end(); ++i)
	{//对source ip 循环
                list<pair<string,list<pair<string,string> > > > infindex_list;//[<oldport,[ipmac]>]
		for(std::map<std::string, std::list<pair<string,string> > >::iterator m_it = i->second.begin(); 
			m_it != i->second.end(); 
			++m_it)
		{//对source port 循环
			string port = m_it->first;
			if(port.length() == 1 || (port.compare(0,1,"G") != 0 && port.compare(0,1,"E") != 0))
			{
				continue;
			}
			if(!(i->second.empty()))
			{
				bool bValidPort = false;
				for(std::list<pair<string,string> >::iterator pi = m_it->second.begin();
					pi != m_it->second.end();
					)
				{
					if(pi->second.length() != 12)
					{
						m_it->second.erase(pi++);
					}
					else
					{
						++pi;
						bValidPort = true;
					}
				}
				if(bValidPort)
				{
					port = port.substr(1);
					while(port.substr(0,1) == "0" && port.substr(0,2) != "0/")
					{
						port = port.substr(1);
					}
					infindex_list.push_back(make_pair(port, m_it->second));//delete G,E
				}
			}
		}
		if(!infindex_list.empty())
		{
			i->second.clear();
                        for(list<pair<string,list<pair<string,string> > > >::iterator k = infindex_list.begin();
				k != infindex_list.end();
				++k)
			{
				i->second.insert(*k);
			}
		}
	}

	for(AFT_LIST::iterator i = aft_list.begin(); i != aft_list.end(); ++i)
	{//对source ip 循环
                list<pair<string,list<string> > > infindex_list;//[<oldport,[mac]>]
                list<pair<string,list<string> > > Validinfindex_list;//[<validport,[mac]>]  // added by zhangyan 2008-12-05
		for(std::map<std::string, std::list<string> >::iterator m_it = i->second.begin(); 
			m_it != i->second.end(); 
			++m_it)
		{//对source port 循环: telnetport -> infPort
			string port = m_it->first;
			if(port.length() == 1 || (port.compare(0,1,"G") != 0 && port.compare(0,1,"E") != 0))
			{
				Validinfindex_list.push_back(make_pair(port, m_it->second)); // added by zhangyan 2008-12-05
				continue;
			}
			if(!(i->second.empty()))
			{
				bool bValidPort = false;
				for(std::list<string>::iterator pi = m_it->second.begin();
					pi != m_it->second.end();
					)
				{//mac
					if((*pi).length() != 12)
					{
						m_it->second.erase(pi++);
					}
					else
					{
						++pi;
						bValidPort = true;
					}
				}
				if(bValidPort)
				{
					/*port = port.substr(1);
					while(port.substr(0,1) == "0" && port.substr(0,2) != "0/")
					{
						port = port.substr(1);
					}*/
					infindex_list.push_back(make_pair(port, m_it->second));//delete G,E
				}
			}
		}
		if(!infindex_list.empty())
		{
			IFPROP_LIST::iterator iif = ifprop_list.find(i->first);
			if(iif != ifprop_list.end())
			{
				string myPrex = "";
				//list<string>str_list;//port
                                //for(list<pair<string,list<string> > >::iterator k = infindex_list.begin();
				//	k != infindex_list.end();
				//	++k)
				//{
				//	str_list.push_back(k->first);
				//}
				//myPrex = getInfDescPrex(str_list, iif->second.second);
                                for(list<pair<string,list<string> > >::iterator k = infindex_list.begin();
					k != infindex_list.end();
					++k)
				{
					list<string>str_list;//port
					str_list.push_back(k->first);
					myPrex = getInfDescPrex(str_list, iif->second.second);
					string port_inf = k->first;
					port_inf = port_inf.substr(1);
					while(port_inf.substr(0,1) == "0" && port_inf.substr(0,2) != "0/")
					{
						port_inf = port_inf.substr(1);
					}
					string myport = myPrex + port_inf;
					k->first = findInfPortFromDescr(iif->second.second, myport);
				}
				i->second.clear();
                                for(list<pair<string,list<string> > >::iterator k = infindex_list.begin();
					k != infindex_list.end();
					++k)
				{
					i->second.insert(*k);
				}
				// begin added by zhangyan 2008-12-05
				//合并端口集
                                for (list<pair<string,list<string> > >::iterator port_mac = Validinfindex_list.begin();
					port_mac != Validinfindex_list.end(); 
					++port_mac)
				{//port-macs
					if (i->second.find(port_mac->first) != i->second.end())
					{//存在该端口						
						for (list<string>::iterator idestmac = port_mac->second.begin(); 
							idestmac != port_mac->second.end(); 
							++idestmac)
						{
							if (find(i->second[port_mac->first].begin(), i->second[port_mac->first].end(), *idestmac) == i->second[port_mac->first].end())
							{//不存在该mac
								i->second[port_mac->first].push_back(*idestmac);
							}
						}
					}
					else
					{
						i->second.insert(*port_mac);
					}
				}
				// end added by zhangyan 2008-12-05
			}
		}
	}
	//规范化接口, end added by tgf 2008-07-04

	//在arp中出现的新的ip-mac作为host加入到设备列表
        list<std::pair<string,string> > ipmac_list;
	list<string> deleteIPS; // added by zhangyan 2008-12-04
	for(ARP_LIST::iterator i = arp_list.begin(); i != arp_list.end(); ++i)
	{//对source ip 循环
		for(std::map<std::string, std::list<pair<string,string> > >::iterator m_it = i->second.begin(); 
			m_it != i->second.end(); 
			++m_it)
		{//对source port 循环
			for(std::list<pair<string,string> >::iterator ip_mac_new = m_it->second.begin();
				ip_mac_new != m_it->second.end();
				++ip_mac_new)
			{//对dest ip 循环
				//begin added by tgf 2008-09-23
				if(find(maclist_virtual.begin(), maclist_virtual.end(), ip_mac_new->second) != maclist_virtual.end())
				{//忽略vrrp 虚拟ip-mac
					continue;
				}				
				//end added by tgf 2008-09-23
				
				// added by zhangyan 2008-12-04
				if((ip_mac_new->first.compare(0, 3, "127") == 0) 
					//add by wings 2009-11-13
					||(ip_mac_new->first.compare(0, 5, "0.255") == 0) 
					|| (find(deleteIPS.begin(), deleteIPS.end(), ip_mac_new->first) != deleteIPS.end()))
				{
					continue;
				}

				bool bNew = true;
				for(std::list<pair<string,string> >::iterator ip_mac = ipmac_list.begin();
					ip_mac != ipmac_list.end();
					++ip_mac)
				{
					if(ip_mac_new->first == ip_mac->first && ip_mac_new->second == ip_mac->second)
					{
						bNew = false;
						break;
					}
					// added by zhangyan 2008-10-23
					if(ip_mac_new->first == ip_mac->first && ip_mac_new->second != ip_mac->second)
					{//删除此IP-MAC
						bNew = false;
						deleteIPS.push_back(ip_mac->first);
						//cout<<"deleteIPS;"<<ip_mac->first<<endl;
						ipmac_list.remove(*ip_mac);
						break;
					}
				}
				if(bNew)
				{
					ipmac_list.push_back(*ip_mac_new);
				}
			}
		}
	}
	for(std::list<pair<string,string> >::iterator i = ipmac_list.begin(); i != ipmac_list.end(); ++i)
	{
		bool bExist = false;
		DEVID_LIST::iterator iid;
		for(iid = devid_list.begin(); iid != devid_list.end(); ++iid)
		{
			if(find(iid->second.ips.begin(), iid->second.ips.end(), i->first) != iid->second.ips.end())
			{
				iid->second.baseMac = i->second;// added by zhangyan 2008-10-23
				bExist = true;
				break;
			}
		}
		if(!bExist)
		{//作为host加入
			IDBody id_tmp;
			id_tmp.snmpflag = "0";
			id_tmp.baseMac = i->second;
			id_tmp.devType = "5";//host
			id_tmp.devModel = "";
			id_tmp.devFactory = "";
			id_tmp.devTypeName = "";
			id_tmp.ips.push_back(i->first);
			id_tmp.msks.push_back("");
			id_tmp.infinxs.push_back("0");
			id_tmp.macs.push_back(i->second);
			devid_list.insert(make_pair(i->first, id_tmp));
		}
		else if(iid->second.macs.empty())
		{//将MAC地址加入设备的
			iid->second.macs.push_back(i->second);
			iid->second.baseMac = i->second;
		}
	}
	
	//规范化arp数据表
	arp_list_frm.clear();	
	for(ARP_LIST::iterator i = arp_list.begin(); i != arp_list.end(); ++i)
	{
		string src_ip = i->first;
		bool bDevice = false;
		for(DEVID_LIST::iterator j = devid_list.begin(); j != devid_list.end(); ++j)
		{
			if(find(j->second.ips.begin(), j->second.ips.end(), src_ip) != j->second.ips.end())
			{
				src_ip = j->first;
				bDevice = true;
				break;
			}
		}
		if(!bDevice)
		{
			continue;
		}
		if(arp_list_frm.find(src_ip) == arp_list_frm.end())
		{//忽略已经存在的src_ip
			string myPrex = "";
			list<string> infindex_list;
			if(!(i->second.empty()))
			{
				if( i->second.begin()->first.compare(0, 1, "G") == 0 ||
					i->second.begin()->first.compare(0, 1, "E") == 0)
				{
                                        for(std::map<string,list<pair<string,string> > >::iterator j = i->second.begin();
						j != i->second.end();
						++j)
					{
						string str_tmp = j->first.substr(1);
						if(str_tmp.length() > 1 && str_tmp.substr(0,2) != "0/")
						{
							str_tmp = lTrim(lTrim(str_tmp),"0");
						}
						infindex_list.push_back(str_tmp);
					}
				}
			}
			IFPROP_LIST::iterator iinf = ifprop_list.find(src_ip);
			if(!infindex_list.empty() && iinf != ifprop_list.end())
			{
				myPrex = getInfDescPrex(infindex_list, iinf->second.second);
			}

                        std::map<string, list<string> > pset_tmp;
                        for(std::map<string,list<pair<string,string> > >::iterator j = i->second.begin();
				j != i->second.end();
				++j)
			{				
				string myport = j->first;//缺省接口				
				list<string> destip_list;
				//dest_ip->dev_ip
                                for(list<pair<string,string> >::iterator k = j->second.begin(); k != j->second.end(); ++k)
				{					
					//begin added by tgf 2008-09-23
					if(find(iplist_virtual.begin(), iplist_virtual.end(), k->first) != iplist_virtual.end())
					{//忽略vrrp 虚拟ip-mac
						continue;
					}
					//end added by tgf 2008-09-23

					// added by zhangyan 2008-12-04
					if(find(deleteIPS.begin(), deleteIPS.end(), k->first) != deleteIPS.end())
					{
						continue;
					}

					for(DEVID_LIST::iterator m = devid_list.begin(); m != devid_list.end(); ++m)
					{
						if(find(m->second.ips.begin(), m->second.ips.end(), k->first) != m->second.ips.end())
						{//忽略不在设备列表中的条目
							if(m->first == src_ip)
							{//忽略转发到自身的条目
								break;
							}
							if(find(destip_list.begin(), destip_list.end(), m->first) == destip_list.end())
							{
								destip_list.push_back(m->first); 
								break;
							}
						}
					}
				}
				if(!destip_list.empty() && pset_tmp.find(myport) == pset_tmp.end())
				{
					pset_tmp.insert(make_pair(myport,destip_list));
				}
			}
			if(!pset_tmp.empty())
			{
				arp_list_frm.insert(make_pair(src_ip, pset_tmp));
			}
		}
	}

	//规范化aft数据表
	aft_list_frm.clear();
	for(AFT_LIST::iterator i = aft_list.begin(); i != aft_list.end(); ++i)
	{
		string src_ip = i->first;
		bool bDevice = false;
		for(DEVID_LIST::iterator j = devid_list.begin(); j != devid_list.end(); ++j)
		{//src_ip -> dev_ip
			if(find(j->second.ips.begin(), j->second.ips.end(), src_ip) != j->second.ips.end())
			{
				src_ip = j->first;
				bDevice = true;
				break;
			}
		}
		if(!bDevice)
		{
			continue;
		}
		if(aft_list_frm.find(src_ip) == aft_list_frm.end())
		{//忽略已经存在的src_ip
			string myPrex = "";
			list<string> infindex_list;
			if(!(i->second.empty()))
			{
                                ////SvLog::writeLog(src_ip.c_str()+string("  ") + i->second.begin()->first.c_str());
				if( i->second.begin()->first.compare(0, 1, "G") == 0 ||
					i->second.begin()->first.compare(0, 1, "E") == 0)
				{
                                        for(std::map<string,list<string> >::iterator j = i->second.begin();
						j != i->second.end();
						++j)
					{
						string str_tmp = j->first.substr(1);
                                                ////SvLog::writeLog(string("str_tmp:")+str_tmp.c_str());
						/*if(str_tmp.length() > 1 && str_tmp.substr(1,2) != "/")
						{
							str_tmp = lTrim(lTrim(str_tmp),"0");
						}*/
						infindex_list.push_back(str_tmp);
					}
				}
			}
			IFPROP_LIST::iterator iinf = ifprop_list.find(src_ip);
			if(!infindex_list.empty() && iinf != ifprop_list.end())
			{
				myPrex = getInfDescPrex(infindex_list, iinf->second.second);
			}

                        std::map<string, list<string> > pset_tmp;
                        for(std::map<string,list<string> >::iterator j = i->second.begin();
				j != i->second.end();
				++j)
			{//port -> infindex
				string myport = j->first;//缺省接口号
				
				if(iinf != ifprop_list.end())
				{
					for(list<IFREC>::iterator k = iinf->second.second.begin();
						k != iinf->second.second.end();
						++k)
					{//通过端口寻找对应的接口索引
						if(k->ifPort == myport && k->ifIndex != myport)
						{//需要修改端口
							myport = k->ifIndex;
							break;
						}
					}
				}
				
				list<string> destip_list;
				//mac->dev_ip
				for(list<string>::iterator k = j->second.begin(); k != j->second.end(); ++k)
				{
					//begin added by tgf 2008-09-23
					if(find(maclist_virtual.begin(), maclist_virtual.end(), *k) != maclist_virtual.end())
					{//忽略vrrp 虚拟ip-mac
						continue;
					}
					//end added by tgf 2008-09-23
                                        std::transform((*k).begin(), (*k).end(), (*k).begin(), (int(*)(int))toupper);
					for(DEVID_LIST::iterator m = devid_list.begin(); m != devid_list.end(); ++m)
					{
						if(find(m->second.macs.begin(), m->second.macs.end(), *k) != m->second.macs.end())
						{//忽略不在设备列表中的条目
							if(m->first != src_ip && find(destip_list.begin(), destip_list.end(), m->first) == destip_list.end())
							{//忽略转发到自身的条目
								destip_list.push_back(m->first);
							}
							break;
						}
					}
				}
				
				if(!destip_list.empty() && pset_tmp.find(myport) == pset_tmp.end())
				{
					pset_tmp.insert(make_pair(myport, destip_list));
				}
			}
			if(!pset_tmp.empty())
			{
				aft_list_frm.insert(make_pair(src_ip, pset_tmp));
			}
		}
	}
	return true;
}

//icmp ping iplist
bool NetScan::IcmpPing(const vector<string>& iplist, bool bGetla, const string& msg, vector<string>& iplist_alive)
{
	iplist_alive.clear();

	vector<string> iplist_to_ping;
        SvLog::writeLog(string("start ping scale: ") + msg);

	PingFind myPing(scanParam.thrdamount);

	size_t iTotal = iplist.size();
	int istart = 0, iend = 0;
	/*int iBatchs = (iTotal + 253) / 254;*/
	int iBatchs = (iTotal + 99) / 100;
        for(int btchs = 0; btchs < iBatchs; ++btchs)
        {//ping 100 devices at a time
                iend += 100;
                if(iend > iTotal)
                {
                        iend = iTotal;
                }
                iplist_to_ping.assign(iplist.begin() + istart, iplist.begin() + iend);
                istart = iend;
		//myPing.multiPing(iplist_to_ping, scanParam.retrytimes, scanParam.timeout);
                if (!myPing.multiPing(iplist_to_ping, 2, 1000))//Ping重试及超时固定为2，1000ms
                {
                    return false;
                }
                //先用单线程ping
                //iplist_to_ping.assign(iplist.begin(), iplist.begin()+iplist.size());
               // vector<string>::iterator i;
                //for (i = iplist_to_ping.begin(); i != iplist_to_ping.end(); ++i)
                //{
                //    myPing.singlePing(*i, 2, 100);//Ping重试及超时固定为2，1000ms
                //}
		list<string> list_tmp = myPing.getAliveIPList();
		for(list<string>::iterator i = list_tmp.begin();
			i != list_tmp.end();
			++i)
		{
                        QString msg = "alive ip : " + s2q(*i);
                        emit SendScanMessage(msg);
			iplist_alive.push_back(*i);
		}
		if (bGetla)
		{	
			//获取本机arp表
                        string localcmty = getCommunity_Get(localip);
			siReader->getOneArpData(SnmpPara(localip, localcmty, scanParam.timeout, scanParam.retrytimes), localport_macs);		
		}
		/*getLocalArp(ipMin, ipMax);
		for(list<string>::iterator i = m_loacal_ip_from_arp.begin();
			i != m_loacal_ip_from_arp.end();
			++i)
		{
			if(find(iplist_alive.begin(), iplist_alive.end(), *i) == iplist_alive.end())
			{
				iplist_alive.push_back(*i);
			}
		}*/
        }
	/*if (bGetla)
	{
                for (std::map<std::string, std::list<std::pair<std::string,std::string> > >::iterator iter = localport_macs.begin(); iter != localport_macs.end(); ++iter)
		{
                        for (std::list<std::pair<std::string,std::string> >::iterator ii = iter->second.begin(); ii != iter->second.end(); ++ii)
			{
				if (find(iplist_alive.begin(), iplist_alive.end(), ii->first) == iplist_alive.end())
				{
					iplist_alive.push_back(ii->first);
				}
			}
		}
	}*/
        SvLog::writeLog(string("End ping: there are ") + int2str((int)iplist_alive.size()) + " alive ips in " + msg);
        return true;
}

//snmp ping iplist
list<pair<string,pair<string,int> > >& NetScan::SnmpPing(const list<pair<string,string> >& ipcmtylist,const string& msg,
                                                                                                           list<pair<string,pair<string,int> > >& iplist_alive)
{
	iplist_alive.clear();
        SvLog::writeLog(string("Start SnmpPing ") + msg);

	PingFind myPing(scanParam.thrdamount);

	myPing.multiPing_SNMP(ipcmtylist, scanParam.retrytimes, scanParam.timeout);
	iplist_alive = myPing.getSnmpAliveIPList();
        list<pair<string,string> > ipcmtys_sp_retry;
        for(list<pair<string,string> >::const_iterator i = ipcmtylist.begin();	i != ipcmtylist.end(); ++i)
	{
		bool bRetry = true;
                for(list<pair<string,pair<string,int> > >::iterator j = iplist_alive.begin();
			j != iplist_alive.end();
			++j)
		{

			if(j->first == i->first)
			{
				bRetry = false;
				break;
			}
		}
		if(bRetry)
		{
			ipcmtys_sp_retry.push_back(*i);
		}
	}
	myPing.multiPing_SNMP(ipcmtys_sp_retry, scanParam.retrytimes, scanParam.timeout);
        list<pair<string,pair<string,int> > > snmpAliveIp_list_retry = myPing.getSnmpAliveIPList();

        for(list<pair<string,pair<string,int> > >::iterator i = snmpAliveIp_list_retry.begin();
		i != snmpAliveIp_list_retry.end();
		++i)
	{
		iplist_alive.push_back(*i);
	}
        SvLog::writeLog(string("End SnmpPing: there are ") + int2str((int)iplist_alive.size()) + " alive ips in " + msg);
	return iplist_alive;
}

// 根据ip地址表扫描
void NetScan::scanByIplist()
{
	StreamData sd;
	vector<string> aliveIp_list;
	if((!sd.readDeviceIpList(aliveIp_list)) || aliveIp_list.empty())
	{
                SvLog::writeLog(string("There none device ip: File DeviceIps.txt is not existed or No ips."));
		return;
	}
	scanByIps(aliveIp_list, false);
	//add by wings 2009-11-13
	devid_list = siReader->devid_list_visited;
}

//从设备信息添加新的扫描范围
void NetScan::addScaleFromDevID(const DEVID_LIST& devlist)
{
	for(DEVID_LIST::const_iterator i = devlist.begin(); i != devlist.end(); ++i)
	{
		if(i->second.devType == ROUTE_SWITCH || i->second.devType == ROUTER)// || i->second.devType == SWITCH)
		{//r-s,r,//s
			vector<string>::const_iterator ip_j = i->second.ips.begin();
			vector<string>::const_iterator msk_j = i->second.msks.begin();
			for(; ip_j != i->second.ips.end() && msk_j != i->second.msks.end();	++ip_j, ++msk_j)
			{
				//if((*ip_j).find("0.0.0.")==0) continue; //by zhangyan 2008-10-21
				if ((*msk_j).empty())
				{
					continue;
				}

				bool bNew = true;
				pair<string,string> scale_j = getScaleByIPMask(make_pair(*ip_j, *msk_j));
				for(SCALE_LIST::iterator k = m_scale_list_toscan.begin();
					k != m_scale_list_toscan.end();	
					++k)
				{
					if(isScaleBInA(*k, scale_j))
					{
						bNew = false;
						break;
					}
				}
				if(bNew)
				{
					for(SCALE_LIST::iterator k = m_scale_list_scaned.begin(); 
						k != m_scale_list_scaned.end();	
						++k)
					{
						if(isScaleBInA(*k, scale_j))
						{
							bNew = false;
							break;
						}
					}
				}
				if(bNew)
				{//发现新子网
					m_scale_list_toscan.push_back(scale_j);
                                        //SvLog::writeLog(scale_j.first + "-" + scale_j.second, FIND_SUBNET_MSG, m_callback);
                                        SvLog::writeLog(scale_j.first + "-" + scale_j.second);
				}
			}
		}
	}
}

void NetScan::addScaleFromRouteItems(const ROUTE_LIST& routelist)
{
	for(ROUTE_LIST::const_iterator i = routelist.begin(); i != routelist.end(); ++i)
	{
                for(map<string,list<ROUTEITEM> >::const_iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{
			for(list<ROUTEITEM>::const_iterator k = j->second.begin();
				k != j->second.end();
				++k)
			{
				if(k->dest_net.find("0.0.0.")==0) continue;
				pair<string,string> scale_k = getScaleByIPMask(make_pair(k->dest_net, k->dest_msk));
				if(scale_k.first == "" || scale_k.first == "0.0.0.0"
					|| scale_k.first.compare(0,3,"127") == 0 //排除环回地址
					//add by wings 2009-11-13
					|| scale_k.first.compare(0,5,"0.255") == 0
//					|| (scale_k.first.compare(0,3,"224") >= 0 && scale_k.first.compare(0,3,"239") <= 0) //排除组播地址
					)
				{
					continue;
				}

				bool bNew = true;
				for(SCALE_LIST::iterator s = m_scale_list_toscan.begin(); 
					s != m_scale_list_toscan.end();	
					++s)
				{
					if(isScaleBInA(*s, scale_k))
					{
						bNew = false;
						break;
					}
				}
				if(bNew)
				{
					for(SCALE_LIST::iterator s = m_scale_list_scaned.begin(); 
						s != m_scale_list_scaned.end();	
						++s)
					{
						if(isScaleBInA(*s, scale_k))
						{
							bNew = false;
							break;
						}
					}
				}
				if(bNew)
				{//发现新子网
					m_scale_list_toscan.push_back(scale_k);
                                        //SvLog::writeLog(scale_k.first + "-" + scale_k.second, FIND_SUBNET_MSG, m_callback);
                                        SvLog::writeLog(scale_k.first + "-" + scale_k.second);
				}
			}
		}
	}
}

bool NetScan::isScaleBInA(const pair<string,string>& scaleA, const pair<string,string>& scaleB)
{
	unsigned long numMin0 = ntohl(inet_addr(scaleA.first.c_str()));
	unsigned long numMax0 = ntohl(inet_addr(scaleA.second.c_str()));
	unsigned long numMin1 = ntohl(inet_addr(scaleB.first.c_str()));
	unsigned long numMax1 = ntohl(inet_addr(scaleB.second.c_str()));
	return (numMin0 <= numMin1 && numMax1 <= numMax0);
}

//added by tgf 2008-09-23
void NetScan::addScaleFromDirectData(const DIRECTDATA_LIST& directlist)
{
	for(DIRECTDATA_LIST::const_iterator i = directlist.begin(); i != directlist.end(); ++i)
	{
		for(list<DIRECTITEM>::const_iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{
			pair<string,string> pair_new = make_pair(j->PeerIP, j->PeerIP);
			bool bNew = true;
			for(SCALE_LIST::iterator s = m_scale_list_toscan.begin(); 
				s != m_scale_list_toscan.end();	
				++s)
			{
				if(isScaleBInA(*s, pair_new))
				{
					bNew = false;
					break;
				}
			}
			if(bNew)
			{
				for(SCALE_LIST::iterator s = m_scale_list_scaned.begin(); 
					s != m_scale_list_scaned.end();	
					++s)
				{
					if(isScaleBInA(*s, pair_new))
					{
						bNew = false;
						break;
					}
				}
			}
			if(bNew)
			{//发现新子网
				m_scale_list_toscan.push_back(make_pair(j->PeerIP, j->PeerIP));
                                //SvLog::writeLog(j->PeerIP + "-" + j->PeerIP, FIND_SUBNET_MSG, m_callback);
                                SvLog::writeLog(j->PeerIP + "-" + j->PeerIP);
			}
		}
	}
}

string NetScan::getIpsofDevID(const IDBody& devid)
{
	string ips = "";
	for(vector<string>::const_iterator i = devid.ips.begin();
		i != devid.ips.end();
		++i)
	{
		ips += *i + ",";
	}
	if(!ips.empty())
	{
		ips = ips.substr(0, ips.length() - 1);
	}
	return ips;
}

bool NetScan::scanByIps(const vector<string>& aliveIp_list, bool bChange)
{
	vector<SnmpPara> spr_list;
	for(vector<string>::const_iterator i = aliveIp_list.begin(); 
	    i != aliveIp_list.end(); 
	    ++i)
	{
		//remarked by zhangyan2008-10-20
		//if(find(m_ip_list_visited.begin(), m_ip_list_visited.end(), *i) == m_ip_list_visited.end())
		{//增加到已访问列表
			m_ip_list_visited.push_back(*i);
			string ipCmt = getCommunity_Get(*i);
			string snmpVer = getSNMPVersion(*i);//"2" or "1" or "0"	 added by zhangyan 2009-01-06
			spr_list.push_back(SnmpPara(*i, ipCmt, scanParam.timeout, scanParam.retrytimes, snmpVer));
		}
	}
	//vector<string>::const_iterator i;
	//i = "172.25.1.1";
	//spr_list.push_back(SnmpPara(*i, "public", scanParam.timeout, scanParam.retrytimes, "2"));

	siReader->ip_visited_list = m_ip_list_visited;
	//siReader->devid_list_visited = devid_list;//by zhangyan 2008-10-20
        if (!siReader->getDeviceData(spr_list))
        {
            return false;
        }
	//devid_list = siReader->devid_list_visited;//by zhangyan 2008-10-20
	m_ip_list_visited = siReader->ip_visited_list;//更新后的已访问ip地址表
	DEVID_LIST devlist_cur = siReader->devid_list_valid;//在当前范围中发现的新设备

	AFT_LIST aftlist_cur = siReader->getAftData();
	ARP_LIST arplist_cur = siReader->getArpData();
	IFPROP_LIST inflist_cur = siReader->getInfProps();
	OSPFNBR_LIST nbrlist_cur = siReader->getOspfNbrData();
	ROUTE_LIST rttbl_cur = siReader->getRouteData();  //changed by zhang 2009-03-26 去掉路由表的取数
													 //changed again by wings 2009-11-13 恢复路由表
	BGP_LIST bgplist_cur = siReader->getBgpData();
	RouterStandby_LIST vrrplist_cur = siReader->getVrrpData();

	DIRECTDATA_LIST directlist_cur = siReader->getDirectData();

	if(!aftlist_cur.empty())
	{
		aft_list.insert(aftlist_cur.begin(), aftlist_cur.end());
	}
	if(!arplist_cur.empty())
	{
		arp_list.insert(arplist_cur.begin(), arplist_cur.end());
	}
	if(!inflist_cur.empty())
	{
		ifprop_list.insert(inflist_cur.begin(), inflist_cur.end());
	}
	if(!nbrlist_cur.empty())
	{
		ospfnbr_list.insert(nbrlist_cur.begin(), nbrlist_cur.end());
	}
	if(!rttbl_cur.empty())		//changed by zhang 2009-03-26 去掉路由表的取数
		//changed again by wings 2009-11-13 恢复路由表
	{
		rttbl_list.insert(rttbl_cur.begin(), rttbl_cur.end());
	}
	if(!bgplist_cur.empty())
	{
		bgp_list.insert(bgp_list.end(), bgplist_cur.begin(), bgplist_cur.end());
	}	
	if(!vrrplist_cur.empty())
	{
		routeStandby_list.insert(vrrplist_cur.begin(), vrrplist_cur.end());
	}
	if(!directlist_cur.empty())
	{
		directdata_list.insert(directlist_cur.begin(), directlist_cur.end());
	}

	if(bChange)
	{//增加新范围
		addScaleFromDevID(devlist_cur);
//		addScaleFromRouteItems(rttbl_cur);			//by zhangyan 2008-12-18
//		addScaleFromDirectData(directlist_cur);		//by zhangyan 2008-10-15
	}

        return true;
}

// 扫描一个范围
bool NetScan::scanOneScale(std::pair<std::string, std::string>& scale, bool bChng)
{
        std::string msg = "start scan scale : " + scale.first + "-" + scale.second;
        //SvLog::writeLog(msg, SCAN_SUBNET_MSG, m_callback);
        emit SendScanMessage(s2q(msg));
        SvLog::writeLog(msg);

	unsigned long ipnumMin = ntohl(inet_addr(scale.first.c_str()));
	unsigned long ipnumMax = ntohl(inet_addr(scale.second.c_str())) + 1;
	struct in_addr addr;
	vector<string> ip_list_all;
	for(unsigned long i = ipnumMin; i < ipnumMax; ++i)
	{
                qDebug() << "test12";
		addr.S_un.S_addr = htonl(i);
		string ipStr = inet_ntoa(addr);
		if(ipStr.size() < 7 || ipStr.substr(ipStr.size()-4) == ".255" || ipStr.substr(ipStr.size()-2) == ".0")
		{//排除广播和缺省地址
			continue;
		}
		bool bExcluded = false;
                for(std::list<std::pair<unsigned long, unsigned long> >::iterator j = scanParam.filter_scales_num.begin();
			j != scanParam.filter_scales_num.end();
			++j)
		{
			if(i <= j->second && i >= j->first)
			{
				bExcluded = true;
				break;
			}
		}
		if((!bExcluded) && find(m_ip_list_visited.begin(), m_ip_list_visited.end(), ipStr) == m_ip_list_visited.end())
		{
                        ip_list_all.push_back(ipStr.c_str());
		}
	}
	if(!ip_list_all.empty())
        {
                qDebug() << "test123";
		vector<string> aliveIp_list;
		//altered by zhangyan 2009-01-12
		if(myParam.ping_type == "1")
		{
			bool bGetLocalArp = false;
			for(list<string>::iterator iter = localip_list.begin(); iter != localip_list.end(); ++iter)
			{
				string local_ip = *iter;
				unsigned long ipnumLocal = ntohl(inet_addr(local_ip.c_str()));
				if(ipnumLocal < ipnumMax  && ipnumLocal >= ipnumMin)
				{
					//is local net;
					bGetLocalArp = true;
					break;
				}
			}
                        if (!IcmpPing(ip_list_all, bGetLocalArp, msg, aliveIp_list))
                        {
                            return false;
                        }
			
                        if (!scanByIps(aliveIp_list, bChng))
                        {
                            return false;
                        }

		}
		else
		{
                        if (!scanByIps(ip_list_all, bChng))
                        {
                            qDebug() << "return false";
                            return false;
                        }
		}
	}

	msg = string("end scan scale:") + scale.first + "-" + scale.second;
        //SvLog::writeLog(msg, COMMON_MSG, m_callback);
        SvLog::writeLog(msg);

        return true;
}

// 读取扫描程序本身的补充配置文件
void NetScan::ReadMyScanConfigFile(void)
{
        //myParam.scan_type = "1"; //扫描类型
        myParam.scan_type = "0"; //扫描类型
	myParam.seed_type = "0";//种子方式 add by wings 2009-12-15
	myParam.ping_type  = "1";//执行ping
	myParam.comp_type = "1";//补充类型
        //myParam.dumb_type = "0"; //生成dumb
        myParam.dumb_type = "1"; //生成dumb
	myParam.arp_read_type = "0";//不读取2层交换机的arp数据
	myParam.nbr_read_type = "0";//不读取邻居表
	myParam.rt_read_type  = "0";//不读取路由表  恢复路由表 add by wings 2009-11-12
	myParam.vrrp_read_type  = "0";//不读取VRRP,HSRP // by zhangyan 2009-01-16
	myParam.bgp_read_type  = "0";
	myParam.snmp_version  = "0";//自适应SNMP版本
	myParam.tracert_type = "0";//不执行trace route
	myParam.filter_type = "0"; //不清除扫描范围外的ip  //add by zhangyan 2008-10-30
	myParam.commit_pc = "1"; //提交PC到SVDB  //add by zhangyan 2009-07-15

	ifstream input("scanconfig.txt", ios::in);
	string line; 
    if (input.is_open())
    {
		while(1) 
		{ 
			getline(input,line);
			if(!line.empty())
			{
				if(line.find("#") != 0)
				{
					vector<string> vstr = tokenize(line, "=");
					if(vstr.size() == 2)
					{
						vstr[0] = replaceAll(vstr[0], " ","");
						vstr[1] = replaceAll(vstr[1], " ","");
                                                transform(vstr[0].begin(), vstr[0].end(), vstr[0].begin(), (int(*)(int))toupper);
						if(vstr[0] == "SCAN_TYPE")
						{
							myParam.scan_type = vstr[1];
						}
						else if(vstr[0] == "SEED_TYPE")
						{
							myParam.seed_type = vstr[1];
						}
						else if(vstr[0] == "PING_TYPE")
						{
							myParam.ping_type = vstr[1];
						}
						else if(vstr[0] == "ARP_READ_TYPE")
						{
							myParam.arp_read_type = vstr[1];
						}
						else if(vstr[0] == "COMP_TYPE")
						{
							myParam.comp_type = vstr[1];
						}
						else if(vstr[0] == "DUMB_TYPE")
						{
							myParam.dumb_type = vstr[1];
						}
						else if(vstr[0] == "NBR_READ_TYPE")
						{
							myParam.nbr_read_type = vstr[1];
						}
						else if(vstr[0] == "RT_READ_TYPE")
						{
							myParam.rt_read_type = vstr[1];
						}
						else if(vstr[0] == "VRRP_READ_TYPE")
						{
							myParam.vrrp_read_type = vstr[1];
						}
						else if(vstr[0] == "BGP_READ_TYPE")
						{
							myParam.bgp_read_type = vstr[1];
						}
						else if(vstr[0] == "TRACERT_TYPE")
						{
							myParam.tracert_type = vstr[1];
						}//add by zhangyan 2008-10-16
						else if(vstr[0] == "FILTER_TYPE")
						{
							myParam.filter_type = vstr[1];
						}//add by zhangyan 2008-10-30
						else if(vstr[0] == "COMMIT_PC")
						{
							myParam.commit_pc = vstr[1];
						}//add by zhangyan 2009-07-15
						else if(vstr[0] == "SNMP_VERSION")
						{
							myParam.snmp_version = vstr[1];
						}						
					}
				}
			}
			if(input.eof()) break; 
		} 
		input.close();
    }

	// added by zhangyan 2009-01-04
	myParam.SNMPV_list.clear();
	if (myParam.snmp_version == "1")
	{
		//从SNMPV2_List.txt中取出需要用SNMPV2探测SNMP信息的特定设备
		ifstream input("SNMPV2_List.txt", ios::in);
		if (input.is_open())
		{
			while(1) 
			{ 
				getline(input, line);
				line = replaceAll(line, " ", "");
				if(!line.empty())
				{
					myParam.SNMPV_list.push_back(make_pair(line, "2"));
				}
				if(input.eof()) break; 
			}
			input.close();
		}
	}
	else if (myParam.snmp_version == "2")
	{
		//从SNMPV1_List.txt中取出需要用SNMPV1探测SNMP信息的特定设备
		ifstream input("SNMPV1_List.txt", ios::in);
		if (input.is_open())
		{
			while(1) 
			{ 
				getline(input, line);
				line = replaceAll(line, " ", "");
				if(!line.empty())
				{
					myParam.SNMPV_list.push_back(make_pair(line, "1"));
				}
				if(input.eof()) break; 
			}
			input.close();
		}
	}
}

// 保存扫描后的原始数据
bool NetScan::saveOriginData(void)
{	
	StreamData myfile;//扫描数据文件处理器
	if(myParam.ping_type != "2")
		myfile.savaDevidIps(devid_list);
	myfile.saveIDBodyData(devid_list);
	myfile.saveAftList(aft_list);
	myfile.saveArpList(arp_list);
	myfile.saveInfPropList(ifprop_list);
	myfile.saveOspfNbrList(ospfnbr_list);
	myfile.saveRouteList(rttbl_list);
	myfile.saveBgpList(bgp_list);
	myfile.saveVrrpList(routeStandby_list);
    myfile.saveDirectData(directdata_list);
	myfile.saveConfigData(scanParam);// added by zhangyan 2008-10-30

	return true;
}

// 从文件读取扫描后的原始数据
bool NetScan::readOriginData(void)
{
	if(!devid_list.empty()) devid_list.clear();
	if(!aft_list.empty()) aft_list.clear();
	if(!arp_list.empty()) arp_list.clear();
	if(!ifprop_list.empty()) ifprop_list.clear();
	if(!ospfnbr_list.empty()) ospfnbr_list.clear();
	if(!rttbl_list.empty()) rttbl_list.clear();
	if(!bgp_list.empty()) bgp_list.clear();
	if(!routeStandby_list.empty()) routeStandby_list.clear();

	if(!rtpath_list.empty()) rtpath_list.clear();// add by zhangyan 2008-08-27
	if(!directdata_list.empty()) directdata_list.clear();

	StreamData myfile;//扫描数据文件处理器
	myfile.readIDBodyData(devid_list);
	//add by wings 2009-11-13
	if(!devid_list.empty())
	{
                SvLog::writeLog("test devid_list length"+int2str(devid_list.size()));
		for(DEVID_LIST::const_iterator i = devid_list.begin();
			i != devid_list.end(); 
			i++)
		{
			IDBody devid = i->second;
			m_ip_list_visited.insert
				(m_ip_list_visited.end(), devid.ips.begin(), devid.ips.begin());
		}
		if(myParam.scan_type == "2")
		{
			siReader->devid_list_visited = devid_list;
			siReader->ip_visited_list    = m_ip_list_visited;
		}
	}
	myfile.readAftList(aft_list);
	myfile.readArpList(arp_list);
	myfile.readInfPropList(ifprop_list);
	myfile.readOspfNbrList(ospfnbr_list);
	myfile.readRouteList(rttbl_list);
	myfile.readBgpList(bgp_list);
	myfile.readVrrpList(routeStandby_list);
	myfile.readTracertList(rtpath_list);// add by zhangyan 2008-08-27
	myfile.readDirectData(directdata_list);
	//add by wings 2009-11-13
	if(myParam.scan_type == "1")
	{
		myfile.readConfigData(scanParam); // added by zhangyan 2008-10-30
	}
	return true;
}

// 保存规范化后的数据
bool NetScan::saveFormatData(void)
{
	StreamData myfile;//扫描数据文件处理器
	myfile.saveFrmDevIDList(devid_list);
	myfile.saveFrmAftList(aft_list_frm);
	myfile.saveFrmArpList(arp_list_frm);	
	return true;
}

// 获取topo边列表
EDGE_LIST& NetScan::getTopoEdgeList(void)
{
	return topo_edge_list;
}

// 获取topo实体列表
DEVID_LIST& NetScan::getTopoEntityList(void)
{
	return topo_entity_list;
}

IFPROP_LIST& NetScan::getInfPropList(void)
{
	return ifprop_list;
}

// 获取本机的ARP数据
void NetScan::getLocalArp(unsigned long ipnumMin, unsigned long ipnumMax)
{
	if(!localip_list.empty())
	{
		string localip = *(localip_list.begin());
		unsigned long ipnumLocal = ntohl(inet_addr(localip.c_str()));
		if((ipnumLocal < ipnumMax  && ipnumLocal >= ipnumMin) || (ipnumMax==0 && ipnumMin == 0))
		{
			string localcmty = getCommunity_Get(localip);
			ARP_LIST arplist_local;
			siReader->getOneArpData(SnmpPara(localip, localcmty, scanParam.timeout, scanParam.retrytimes), arplist_local);
			m_loacal_ip_from_arp.clear();
			for(ARP_LIST::iterator i = arplist_local.begin();
				i != arplist_local.end();
				++i)
			{
				m_loacal_ip_from_arp.push_back(i->first);
			}
			if(arplist_local.empty())
			{
                                SvLog::writeLog("Warnning: Must start local snmp service.");
			}
			else
			{
				arp_list.insert(arplist_local.begin(), arplist_local.end());
			}
		}
	}
}

// 创建哑设备
int NetScan::GenerateDumbDevice(EDGE_LIST& edge_list, DEVID_LIST& device_list)
{
	if( myParam.dumb_type == "0" || edge_list.empty())
	{//不创建哑设备
		return 0;
	}

	int amountAdded = 0;
	EDGE_LIST edge_list_dumb;
	DEVID_LIST device_list_dumb;
	EDGE_LIST::iterator i_end = edge_list.end();
	i_end--;
	EDGE_LIST::iterator j_start;
	for(EDGE_LIST::iterator i = edge_list.begin();
		i != i_end;
		++i)
	{		
		if(i->ip_left.compare(0,4,"DUMB") == 0)
		{
			continue;
		}
		DEVID_LIST::iterator iii = device_list.find(i->ip_right);
		if(iii != device_list.end() && iii->second.devType != "5" && iii->second.devType != "4")
		{//只对主机和服务器添加dumb
			continue;
		}

		j_start = i;
		j_start++;
		string DumbIP = "";
		IDBody DumbBody;
		for(EDGE_LIST::iterator j = j_start;
			j != edge_list.end();
			++j)
		{
			DEVID_LIST::iterator iii = device_list.find(j->ip_right);
			if(iii != device_list.end() && iii->second.devType != "5" && iii->second.devType != "4")
			{
				continue;
			}
			if(j->ip_left == i->ip_left && j->inf_left == i->inf_left)
			{//左连设备相同
				if(DumbIP == "")
				{//添加哑设备
					DumbIP = "DUMB" + int2str(amountAdded);
					DumbBody.baseMac = "";
					DumbBody.devType = "6";//其他设备
					DumbBody.sysOid = "HUB";
					DumbBody.sysName = "dumb device";
					device_list_dumb.insert(make_pair(DumbIP, DumbBody));
					amountAdded++;
				}
				//改变边的关联关系
				j->ip_left = DumbIP;
				j->dsc_left = "";
				j->inf_left = "0";
				j->pt_left = "0";
			}
		}
		if(DumbIP != "")
		{
			//添加一条边
			EDGE edge_tmp;
			edge_tmp.ip_left = i->ip_left;
			edge_tmp.inf_left = i->inf_left;
			edge_tmp.pt_left = i->pt_left;
			edge_tmp.dsc_left = i->dsc_left;
			edge_tmp.ip_right = DumbIP;
			edge_tmp.inf_right = "0";
			edge_tmp.pt_right = "0";
			edge_tmp.dsc_right = "";
			edge_list_dumb.push_back(edge_tmp);

			//更改第一条边的左边信息
			i->ip_left = DumbIP;
			i->dsc_left = "";
			i->inf_left = "0";
			i->pt_left = "0";
		}
	}
	for(EDGE_LIST::iterator i = edge_list_dumb.begin();
		i != edge_list_dumb.end();
		++i)
	{
		edge_list.push_back( *i );
	}
	for(DEVID_LIST::iterator i = device_list_dumb.begin();
		i != device_list_dumb.end();
		++i)
	{
		device_list.insert( *i );
	}
	return amountAdded;
}

// 填充边信息
void NetScan::FillEdge(EDGE_LIST& edge_list)
{
	if(edge_list.empty()) return;
	for(EDGE_LIST::iterator i = edge_list.begin();
		i != edge_list.end();
		++i)
	{
		//处理左边端口
		if(i->inf_left == "PX") i->inf_left = "0";
		else
		{//
			IFPROP_LIST::iterator iinf = ifprop_list.find(i->ip_left);
			if(iinf != ifprop_list.end())
			{
				for(list<IFREC>::iterator jj = iinf->second.second.begin();
					jj != iinf->second.second.end();
					++jj)
				{//通过端口寻找对应的接口索引
					if(jj->ifIndex == i->inf_left)
					{//需要修改端口
						i->pt_left = jj->ifPort;
						i->dsc_left = jj->ifDesc;
						break;
					}
				}
			}
		}
		//处理右边端口
		if(i->pt_right == "PX")
			i->inf_right = "0";
		else
		{//
			IFPROP_LIST::iterator iinf = ifprop_list.find(i->ip_right);
			if(iinf != ifprop_list.end())
			{
				for(list<IFREC>::iterator jj = iinf->second.second.begin();
					jj != iinf->second.second.end();
					++jj)
				{//通过端口寻找对应的接口索引
					if(jj->ifIndex == i->inf_right)
					{//需要修改端口
						i->pt_right = jj->ifPort;
						i->dsc_right = jj->ifDesc;
						break;
					}
				}
			}
		}
	}
}

/*** cdp scan ***/
// 保存扫描后的原始数据
bool NetScan::saveOriginData_cdp(void)
{	
	StreamData myfile;//扫描数据文件处理器
	myfile.saveDirectData(directdata_list);
	myfile.saveIDBodyData(devid_list);
	myfile.saveInfPropList(ifprop_list);
	myfile.saveAftList(aft_list);
	myfile.saveArpList(arp_list);
	return true;
}

// 从文件读取扫描后的原始数据
bool NetScan::readOriginData_cdp(void)
{
	if(!directdata_list.empty()) directdata_list.clear();
	if(!devid_list.empty()) devid_list.clear();
	if(!ifprop_list.empty()) ifprop_list.clear();
	if(!aft_list.empty()) aft_list.clear();
	if(!arp_list.empty()) arp_list.clear();
	
	StreamData myfile;
	myfile.readDirectData(directdata_list);
	myfile.readIDBodyData(devid_list);
	myfile.readInfPropList(ifprop_list);
	myfile.readAftList(aft_list);
	myfile.readArpList(arp_list);
	return true;
}

// 执行扫描
void NetScan::Scan_cdp(void)
{		
	m_ip_list_visited.clear();
		
	if(myParam.scan_type == "0")
	{//进行全新扫描
		list<string> iplist_to_scan = scanParam.scan_seeds;
		if(iplist_to_scan.empty())
		{			
                        //SvLog::writeLog("There existed no valided seeds.", COMMON_MSG, m_callback);
                        SvLog::writeLog("There existed no valided seeds.");
			return;
		}		
		
		while(!iplist_to_scan.empty())
		{			
			vector<SnmpPara> spr_list;
			for (list<string>::iterator iter = iplist_to_scan.begin(); iter != iplist_to_scan.end(); ++iter)
			{//增加到已访问列表
				m_ip_list_visited.push_back(*iter);
				string ipCmt = getCommunity_Get(*iter);
				spr_list.push_back(SnmpPara(*iter, ipCmt, scanParam.timeout, scanParam.retrytimes));
			}

			siReader->ip_visited_list = m_ip_list_visited;
			//siReader->devid_list_visited = devid_list;//by zhangyan 2008-10-20
			siReader->getCdpDeviceData(spr_list);
			//devid_list = siReader->devid_list_visited;//by zhangyan 2008-10-20
			m_ip_list_visited = siReader->ip_visited_list;//更新后的已访问ip地址表
			//DEVID_LIST devlist_cur = siReader->devid_list_valid;//在当前范围中发现的新设备

			AFT_LIST aftlist_cur = siReader->getAftData();
                        qDebug() << "aft size : " << aftlist_cur.size();
			ARP_LIST arplist_cur = siReader->getArpData();
			IFPROP_LIST inflist_cur = siReader->getInfProps();			
			DIRECTDATA_LIST directlist_cur = siReader->getDirectData();

			if(!aftlist_cur.empty())
			{
				aft_list.insert(aftlist_cur.begin(), aftlist_cur.end());
			}
			if(!arplist_cur.empty())
			{
				arp_list.insert(arplist_cur.begin(), arplist_cur.end());
			}
			if(!inflist_cur.empty())
			{
				ifprop_list.insert(inflist_cur.begin(), inflist_cur.end());
			}			
			if(!directlist_cur.empty())
			{
				directdata_list.insert(directlist_cur.begin(), directlist_cur.end());
			}

			iplist_to_scan.clear();
			for(DIRECTDATA_LIST::iterator i = directlist_cur.begin(); i != directlist_cur.end(); ++i)
			{
				for(list<DIRECTITEM>::iterator j = i->second.begin(); j != i->second.end(); ++j)
				{					
					if ((j->PeerIP != "0.0.0.0") && (find(m_ip_list_visited.begin(), m_ip_list_visited.end(), j->PeerIP) == m_ip_list_visited.end()))
					{
						iplist_to_scan.push_back(j->PeerIP);
						m_ip_list_visited.push_back(j->PeerIP);
					}
				}
			}		
						
		}//end while
		devid_list = siReader->devid_list_visited;//获得所有设备列表

		////add by zhangyan 2008-10-31
		////todo: 读取本机arp(先ping本网段，再读)
		//if (!localip_list.empty())
		//{			
		//	for (list<string>::iterator iter = localip_list.begin(); iter != localip_list.end(); ++iter)
		//	{
		//		SCALE_LIST localip_scales;	
		//		getLocalNet(localip_scales, *iter);
		//		if (!localip_scales.empty())
		//		{
		//			PingLocalNet(localip_scales, *iter);
		//			break;
		//		}
		//		
		//	}
		//}		

                SvLog::writeLog("end scan, to save data...");
		saveOriginData_cdp();
	}
	else
	{
                SvLog::writeLog("analyse through old data..");
                //m_callback(TITLE_MSG, StringToUtf8(TITLE_MSG_110));
                m_callback(TITLE_MSG, TITLE_MSG_110);
		readOriginData_cdp();
	}

        SvLog::writeLog("To Format data.");
	FormatData();
        SvLog::writeLog("To save Format data.");
	saveFormatData();
        SvLog::writeLog("Analyse data...");

	if(devid_list.empty())
	{
                topo_edge_list.clear();
	}
	else if(ExistNetDevice(devid_list))
	{		
		topoAnalyse TA(devid_list, ifprop_list, aft_list_frm, arp_list_frm, ospfnbr_list, rttbl_list, bgp_list, directdata_list, myParam);
		if(TA.EdgeAnalyseDirect())
		{
			topo_edge_list = TA.getEdgeList();
			//补充边的附加信息
			FillEdge(topo_edge_list);//根据端口索引填充端口描述
			//创建哑设备
			GenerateDumbDevice(topo_edge_list,devid_list);
			topo_entity_list = devid_list;//已添加了哑设备
                        SvLog::writeLog("success to analyse.");
		}
		else
		{
                        SvLog::writeLog("fail to analyse.");
		}
	}
}
