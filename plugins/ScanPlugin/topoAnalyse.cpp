// topoAnalyse.cpp

/*
 *
 * Copyright (c) 2007-2008 DragonFlow
 *
 * Created date: 2007-11-30
 *
 * Autor: Tang guofu 
 * 
 * last modified 2007-12-19 by Tang guofu 
 *
 */


#include "StdAfx.h"
#include "topoAnalyse.h"
#include "TraceAnalyse.h"
#include "svlog.h"
#include <QtDebug>

#include "scanutils.h"

topoAnalyse::topoAnalyse(const DEVID_LIST& devlist, const IFPROP_LIST& ifprop_list, 
						 const FRM_AFTARP_LIST& aftlist, const FRM_AFTARP_LIST& arplist, 
						 const OSPFNBR_LIST& ospfnbr_list, const ROUTE_LIST& route_list, 
						 const BGP_LIST& bgp_list, const DIRECTDATA_LIST& direct_list, 
						 const AUX_PARAM param)
{
	m_param.comp_type = param.comp_type;

	device_list = devlist;
	aft_list = aftlist;
	arp_list = arplist;
	nbr_list = ospfnbr_list;
	rttbl_list = route_list;
	this->bgp_list = bgp_list;
	this->direct_list = direct_list;
	this->ifprop_list = ifprop_list;

	//device_list_bak = devlist;
	//aft_list_bak = aftlist;//by zhangyan 2008-10-28
	//arp_list_bak = arplist;

	rt_list.clear();
	sw_list.clear();
	//add by wings 2009-11-26
	sw2_list.clear();
	sw3_list.clear();
	rs_list.clear();
	
	StreamData sd;
	vector<string> swip_list;
	sd.readSwIpList(swip_list);
	if(!swip_list.empty())
	{
		for(vector<string>::iterator ip = swip_list.begin();
			ip != swip_list.end();
			ip++)
		{
			if(find(sw_list.begin(),sw_list.end(),*ip) == sw_list.end())
				sw_list.push_back(*ip);
		}
	}
	if(!device_list.empty())
	{
		for(DEVID_LIST::iterator i = device_list.begin(); i != device_list.end(); ++i)
		{
			//add by wings 2009-11-26
			if(i->second.devType == SWITCH)
			{
				if(find(sw2_list.begin(), sw2_list.end(), i->first) == sw2_list.end())
				{
					sw2_list.push_back(i->first);
				}
			}
			if(i->second.devType == ROUTE_SWITCH || i->second.devType == FIREWALL)
			{//switch-router, switch
				if(find(sw3_list.begin(), sw3_list.end(), i->first) == sw3_list.end())
				{
					sw3_list.push_back(i->first);
				}
			}

			if(i->second.devType == ROUTE_SWITCH||i->second.devType == SWITCH || i->second.devType == FIREWALL)
			{
				if(find(sw_list.begin(), sw_list.end(), i->first) == sw_list.end())
				{
					sw_list.push_back(i->first);
				}
			}
			if(i->second.devType == ROUTER)
			{//router
				if(find(rt_list.begin(), rt_list.end(), i->first) == rt_list.end())
				{
					rt_list.push_back(i->first);
					//begin added by zhangyan 2009-01-15
                                        list<pair<string, string> > subnet_ifindex;
					vector<string>::iterator ip_iter, msk_iter, ifindex_iter;
					for(ip_iter = i->second.ips.begin(), msk_iter = i->second.msks.begin(), ifindex_iter = i->second.infinxs.begin(); 
						ip_iter != i->second.ips.end() && msk_iter != i->second.msks.end(), ifindex_iter != i->second.infinxs.end(); 
						++ip_iter, ++msk_iter, ++ifindex_iter)
					{
						if((*msk_iter).empty())
						{
							continue;
						}
						//�����벻Ϊ��ʱ
						string subnet = getSubnetByIPMask(*ip_iter, *msk_iter);
						/*if(subnet == "192.168.0.0/24")
						{
							continue;
						}*/
						subnet_ifindex.push_back(make_pair(subnet, *ifindex_iter));		
					}					
					IPADDR_map.insert(make_pair(i->first, subnet_ifindex));
					//end
				}
			}
			if(i->second.devType == ROUTE_SWITCH || i->second.devType == ROUTER || i->second.devType == FIREWALL)
			{//rs
				if(find(rs_list.begin(), rs_list.end(), i->first) == rs_list.end())
				{
					rs_list.push_back(i->first);
				}
			}
		}
		for(list<string>::iterator i = sw_list.begin(); i != sw_list.end(); i++)
		{
			if(aft_list.find(*i) == aft_list.end())//���н�������aft��δ����
			{
				list<string> list_tmp;
				//list_tmp.push_back("");
                                map<string,list<string> > mm;
				mm.insert(make_pair("0", list_tmp));
				aft_list.insert( make_pair(*i, mm));
				//aft_list_bak.insert(make_pair(*i, mm));//by zhangyan 2008-10-28
			}
		}
	}
}

topoAnalyse::~topoAnalyse(void)
{
}

// �ж�һ���˿ڼ��Ƿ�Ҷ�Ӷ˿�
bool topoAnalyse::IsLeafPort(const list<string>& iplist, const list<string>& dv_list)
{
	if(iplist.empty())
	{//û��Ԫ�صĶ˿�,����Ϊ��Ҷ�Ӷ˿�
		return false;
	}
	for(list<string>::const_iterator i = dv_list.begin();
		i != dv_list.end();
		++i)
	{
		if(find(iplist.begin(), iplist.end(), *i) != iplist.end())
		{//dest ip�д��ڽ����豸
			return false;
		}
	}
	return true;
}

// �ж�һ���˿��Ƿ��������һ̨�����豸
bool topoAnalyse::IsSWLeafPort(const list<string>& iplist, const list<string>& sw_list)
{	
	if(iplist.size() == 1)
	{
		if(find(sw_list.begin(), sw_list.end(), *(iplist.begin()) ) != sw_list.end())
		{
			return true;
		}
	}
	return false;
}

// �ж�һ�������豸�Ƿ�Ϊһ��Ҷ�ӽ����豸
// remoteip -- �Զ˵�ip
// port_set -- ���˵Ķ˿ڼ���
bool topoAnalyse::IsSWLeafDevice(const string remoteip, const map<string,list<string> >& port_set)
{
	if(port_set.empty())
	{
		return true;
	}
	int mycount = 0;
	bool bFindPeer = false;
        for(map<string,list<string> >::const_iterator i = port_set.begin();
		i != port_set.end();
		++i)
	{
		if(i->second.size() >= 1)
		{
			mycount++;
			if(find(i->second.begin(), i->second.end(), remoteip) != i->second.end())
			{
				bFindPeer = true;
			}
		}
	}
	if(mycount == 0 || (mycount == 1 && bFindPeer))
	{//����û����Ч�˿�,���߱���ֻ��һ����Ч�˿��Ҹö˿��а����Զ�IP
		return true;
	}
	return false;
}

// �ж�һ���˿ڼ���(port_set_A)�е�ip�Ƿ��������һ���˿ڼ���(port_set_B)��
// port_set_A -- ����������
// port_set_B -- ��������
// excludeIp -- ��port_set_A�ų���ip
bool topoAnalyse::IsInclude(const map<string,list<string> >& port_set_A, const map<string,list<string> >& port_set_B, const string excludeIp)
{
	list<string>iplist_in_B;
        for(map<string,list<string> >::const_iterator i = port_set_B.begin();
		i != port_set_B.end();
		++i)
	{
		for(list<string>::const_iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{
			iplist_in_B.push_back(*j);
		}
	}
	iplist_in_B.push_back(excludeIp);

	int ii = 0;
        for(map<string,list<string> >::const_iterator i = port_set_A.begin();
		i != port_set_A.end();
		++i)
	{
		if(!(i->second.empty()))
		{
			ii++;
		}
		for(list<string>::const_iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{
			if(find(iplist_in_B.begin(), iplist_in_B.end(), *j) == iplist_in_B.end())
			{
				return false;
			}
		}
	}
	if(ii > 1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

// ����·�����뽻����֮������ӹ�ϵ
// remarked by zhangyan 2008-10-29
//bool topoAnalyse::AnalyseRS(void)
//{
//	edge_list_rs_byaft.clear();
//
//	bool bGoOn = true;
//	while(bGoOn)
//	{
//		list<string> iplist_del;//�������·����ip
//		bGoOn = false;
//		for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
//			i != aft_list.end();
//			++i)
//		{//��source ip ѭ��
//			string ip_src = i->first;
//			for(std::map<string,list<string> >::iterator j = i->second.begin();
//				j != i->second.end();
//				++j)
//			{//��source port ѭ��
//				bool bSkip = false;
//				for(list<string>::iterator isw = sw_list.begin();
//					isw != sw_list.end();
//					++isw)
//				{
//					if(find(j->second.begin(), j->second.end(), *isw) != j->second.end())
//					{
//						bSkip = true;
//						break;
//					}
//				}
//				if(bSkip)
//				{//�������ڽ����豸�Ľӿڼ���
//					continue;
//				}
//
//				string pt_src = j->first; 
//				for(list<string>::iterator k = j->second.begin();
//					k != j->second.end();
//					++k)
//				{//��dest ip ѭ��
//					if(find(sw_list.begin(), sw_list.end(), *k) != sw_list.end())
//					{//����Ϊ�����豸��dest_ip
//						continue;
//					}
//					string ip_dest = *k;
//					if(find(rt_list.begin(), rt_list.end(), ip_dest) != rt_list.end())
//					{//Ŀ��ipΪrouter
//						bGoOn = true;
//						string pt_dest = "PX";//Ŀ���豸�Ķ˿�ȱʡΪPX
//						if(arp_list.find(ip_dest) != arp_list.end())
//						{//dest ip������arp�����б���
//							for(std::map<string,list<string> >::iterator j_dest = arp_list[ip_dest].begin();
//								j_dest != arp_list[ip_dest].end();
//								++j_dest)
//							{//Ѱ�ҶԶ��豸�а���source ip�Ķ˿�
//								if(find(j_dest->second.begin(), j_dest->second.end(), ip_src) != j_dest->second.end())
//								{
//									pt_dest = j_dest->first;
//									break;
//								}
//							}
//						}
//						if(pt_dest != "PX")
//						{//���Բ����ҵ�Ŀ�Ķ˿ڵı�
//							EDGE edge_tmp;
//							edge_tmp.ip_left  = ip_src;
//							edge_tmp.pt_left  = pt_src;
//							edge_tmp.inf_left = pt_src;
//							edge_tmp.ip_right  = ip_dest;
//							edge_tmp.pt_right  = pt_dest;
//							edge_tmp.inf_right = pt_dest;
//							edge_list_rs_byaft.push_back(edge_tmp);
//						}
//						//�����Ѵ����ip�б�
//						if(find(iplist_del.begin(), iplist_del.end(), ip_dest) == iplist_del.end())
//						{
//							iplist_del.push_back(ip_dest);
//						}
//					}
//				}
//			}
//		}
//		//ɾ��������dest ip
//		if(!iplist_del.empty())
//		{
//			for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
//				i != aft_list.end();
//				++i)
//			{//��source ip ѭ��
//				string ip_src = i->first;
//				for(std::map<string,list<string> >::iterator j = i->second.begin();
//					j != i->second.end();
//					++j)
//				{//��source port ѭ��
//					for(list<string>::iterator k = j->second.begin();
//						k != j->second.end();)
//					{
//						if(find(iplist_del.begin(), iplist_del.end(), *k) != iplist_del.end())
//						{//ִ��ɾ��
//							j->second.erase(k++);
//						}
//						else
//						{
//							++k;
//						}
//					}
//				}
//			}
//		}
//	}
//
//	//�����ܷ�����router ip ȫ������
//	for(list<string>::iterator rt = rt_list.begin();
//		rt != rt_list.end();
//		++rt)
//	{
//		for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); ++i)
//		{//��source ip ѭ��
//			for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); ++j)
//			{//��source port ѭ��
//				j->second.remove(*rt);
//			}
//		}
//	}
//	
//	//printdata(aft_list);
//
//	return true;
//}


bool topoAnalyse::AnalyseRS(void)
{
	edge_list_rs_byaft.clear();		
		
	for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
		i != aft_list.end();
		++i)
	{//��source ip ѭ��
		string ip_src = i->first;
                for(std::map<string,list<string> >::iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{//��source port ѭ��
			bool bSkip = false;
			for(list<string>::iterator isw = sw_list.begin();
				isw != sw_list.end();
				++isw)
			{
				if(find(j->second.begin(), j->second.end(), *isw) != j->second.end())
				{
					bSkip = true;
					break;
				}
			}
			if(bSkip)
			{//�������ڽ����豸�Ľӿڼ���
				continue;
			}

			string pt_src = j->first; 
			for(list<string>::iterator k = j->second.begin();
				k != j->second.end();
				++k)
			{//��dest ip ѭ��
				string ip_dest = *k;
				if(find(rt_list.begin(), rt_list.end(), ip_dest) != rt_list.end())
                                {//Ŀ��ipΪrouter
                                        qDebug() << "sourceip : " << pt_src.c_str() << "dest router : " << ip_dest.c_str();
					string pt_dest = "PX";//Ŀ���豸�Ķ˿�ȱʡΪPX
					if(arp_list.find(ip_dest) != arp_list.end())
					{//dest ip������arp�����б���
                                                for(std::map<string,list<string> >::iterator j_dest = arp_list[ip_dest].begin();
							j_dest != arp_list[ip_dest].end();
							++j_dest)
						{//Ѱ�ҶԶ��豸�а���source ip�Ķ˿�
							if(find(j_dest->second.begin(), j_dest->second.end(), ip_src) != j_dest->second.end())
							{
								pt_dest = j_dest->first;
								break;
							}
						}
					}
					if(pt_dest != "PX")
					{//���Բ����ҵ�Ŀ�Ķ˿ڵı�
						EDGE edge_tmp;
						edge_tmp.ip_left  = ip_src;
						edge_tmp.pt_left  = pt_src;
						edge_tmp.inf_left = pt_src;
						edge_tmp.ip_right  = ip_dest;
						edge_tmp.pt_right  = pt_dest;
						edge_tmp.inf_right = pt_dest;
						edge_list_rs_byaft.push_back(edge_tmp);
						cout<<"AnalyseRS:"<<edge_tmp.ip_left<<" "<<edge_tmp.pt_left<<" "<<edge_tmp.ip_right<<" "<<edge_tmp.pt_right<<endl;
						break;//�ڽ������ĵ�ǰ�˿����ҵ���һ̨�����ӵ�·���������˳���ǰdest ipѭ��  added by zhangyan 2008-10-28
					}					
				}
			}
		}
	}
	
	//ɾ��aft�����е�router ip
	for(list<string>::iterator rt = rt_list.begin();
		rt != rt_list.end();
		++rt)
	{
		for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); ++i)
		{//��source ip ѭ��
                        for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); ++j)
			{//��source port ѭ��
				j->second.remove(*rt);
			}
		}
	}
	
	// added by zhangyan 2008-12-04 
	// ���Ȳ��뽻������·���������ӹ�ϵ
	topo_edge_list.insert(topo_edge_list.end(), edge_list_rs_byaft.begin(), edge_list_rs_byaft.end());

	//printdata(aft_list);

	return true;
}


// ����������������֮������ӹ�ϵ
// remarked by zhangyan 2008-10-29
//bool topoAnalyse::AnalyseSH(void)
//{
//	EDGE_LIST edge_list_sh;
//	bool bGoOn = true;
//	while(bGoOn)
//	{
//		bGoOn = false;
//		list<string> iplist_del;//�������ip
//		for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); ++i)
//		{//��source ip ѭ��
//			string ip_src = i->first;
//			for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); ++j)
//			{//��source port ѭ��
//				if(IsLeafPort(j->second, sw_list))
//				{//��Ҷ�Ӷ˿�
//					string pt_src = j->first; 
//					for(list<string>::iterator k = j->second.begin();
//						k != j->second.end();
//						++k)
//					{//��dest ip ѭ��,Ϊÿ��Ŀ��ip���һ����
//						if(find(sw_list.begin(), sw_list.end(), *k) != sw_list.end())
//						{//����Ϊ�����豸��dest_ip
//							continue;
//						}
//						if(find(rs_list.begin(), rs_list.end(), *k) != rs_list.end())
//						{
//							continue;
//						}
//						bGoOn = true;
//						string pt_dest = "PX";
//						if(arp_list.find(*k) != arp_list.end())
//						{//��·�����Ҵ��ڶ�Ӧ��arp��Ŀ,Ѱ�Ҷ�Ӧ�Ķ˿�(�ӿ�)
//							for(std::map<string,list<string> >::iterator i_arp = arp_list[*k].begin();
//								i_arp != arp_list[*k].end();
//								++i_arp
//								)
//							{
//								if(find(i_arp->second.begin(), i_arp->second.end(), ip_src) != i_arp->second.end())
//								{
//									pt_dest = i_arp->first;
//									break;
//								}
//							}
//						}
//						//��֤ÿ��Hostֻ����һ��
//						bool bNew = true;
//						for(EDGE_LIST::iterator ie = edge_list_sh.begin();
//							ie != edge_list_sh.end();
//							++ie)
//						{
//							if(*k == ie->ip_right && pt_dest == ie->pt_right)
//							{
//								bNew = false;
//								break;
//							}
//						}
//						if(bNew)
//						{
//							EDGE edge_tmp;
//							edge_tmp.ip_left  = ip_src;
//							edge_tmp.pt_left  = pt_src;
//							edge_tmp.inf_left = pt_src;
//							edge_tmp.ip_right  = *k;
//							edge_tmp.pt_right  = pt_dest;
//							edge_tmp.inf_right = pt_dest;
//							edge_list_sh.push_back(edge_tmp);
//
//						}
//						//����dest ip ��ӵ��Ѿ������ip��ַ����
//						if(find(iplist_del.begin(), iplist_del.end(), *k) == iplist_del.end())
//						{
//							iplist_del.push_back(*k);
//						}
//					}//��dest ip ѭ��
//				}//��Ҷ�Ӷ˿�
//			}
//		}
//		//ɾ��������dest ip
//		for(list<string>::iterator ip_del = iplist_del.begin();
//			ip_del != iplist_del.end();
//			++ip_del)
//		{
//			for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
//				i != aft_list.end();
//				++i)
//			{//��source ip ѭ��
//				string ip_src = i->first;
//				for(std::map<string,list<string> >::iterator j = i->second.begin();
//					j != i->second.end();
//					++j)
//				{//��source port ѭ��
//					j->second.remove(*ip_del);
//				}
//			}
//		}
//	}	
//	topo_edge_list.insert(topo_edge_list.end(), edge_list_sh.begin(), edge_list_sh.end());
//
//	//�����ܷ���������ipȫ������
//	for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); ++i)
//	{//��source ip ѭ��
//		string ip_src = i->first;
//		for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); ++j)
//		{//��source port ѭ��
//			for(list<string>::iterator k = j->second.begin();
//				k != j->second.end();
//				)
//			{//��dest ip ѭ��
//				if(find(sw_list.begin(), sw_list.end(), *k) == sw_list.end())
//				{
//					j->second.erase(k++);
//				}
//				else
//				{
//					++k;
//				}
//			}
//		}
//	}
//	return true;
//}

bool topoAnalyse::AnalyseSH(void)
{
	EDGE_LIST edge_list_sh;
	//add by wings 2009-11-26  
	//����ȷ������һ��pc�Ľ������˿�
	for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); ++i)
	{//��source ip ѭ��
                for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{//��source port ѭ��
			if(j->second.size() == 1)
			{
				list<string>::iterator ip_dest = j->second.begin();
				if(find(sw_list.begin(),sw_list.end(),*ip_dest) == sw_list.end())
				{
					string ip_src = i->first;
					string pt_src = j->first;
					string pt_dest = "PX";
					if(arp_list.find(*ip_dest) != arp_list.end())
					{//Ѱ�Ҷ�Ӧ�Ķ˿�(�ӿ�)
                                                qDebug() << "single pc : " << (*ip_dest).c_str();
                                                for(std::map<string,list<string> >::iterator i_arp = arp_list[*ip_dest].begin();
							i_arp != arp_list[*ip_dest].end();
							++i_arp
							)
						{
							if(find(i_arp->second.begin(), i_arp->second.end(), ip_src) != i_arp->second.end())
							{
								pt_dest = i_arp->first;
								break;
							}
						}
					}
						//��֤ÿ��Hostֻ����һ��
						bool bNew = true;
						for(EDGE_LIST::iterator ie = edge_list_sh.begin();
							ie != edge_list_sh.end();
							++ie)
						{
							if(*ip_dest == ie->ip_right && pt_dest == ie->pt_right)
							{
								bNew = false;
								break;
							}
						}
						if(bNew)
						{
							EDGE edge_tmp;
							edge_tmp.ip_left  = ip_src;
							edge_tmp.pt_left  = pt_src;
							edge_tmp.inf_left = pt_src;
							edge_tmp.ip_right  = *ip_dest;
							edge_tmp.pt_right  = pt_dest;
							edge_tmp.inf_right = pt_dest;
							edge_list_sh.push_back(edge_tmp);
                                                        qDebug() << "single pc: " << ip_src.c_str() << ",dest ip: " << (*ip_dest).c_str();
                                                        ////SvLog::writeLog("edge:"+edge_tmp.ip_left
							//	+edge_tmp.pt_left+edge_tmp.ip_right
							//	+edge_tmp.pt_right+"**************");
						}	
					

				}
			}
		}
	}
	//add by wings 2009-11-26 
	//�ڶ�ȷ�����㽻����pc��
	for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); ++i)
	{//��source ip ѭ��
		string ip_src = i->first;
		if(find(sw3_list.begin(),sw3_list.end(),ip_src)!=sw3_list.end())
		{
			continue;
		}
                for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{//��source port ѭ��
			if(IsLeafPort(j->second, sw_list))
			{//��Ҷ�Ӷ˿�
                                SvLog::writeLog("see the LeafPort"+j->first+"  "+int2str(j->second.size()));
				string pt_src = j->first; 
				for(list<string>::iterator k = j->second.begin();
					k != j->second.end();
					++k)
				{//��dest ip ѭ��,Ϊÿ��Ŀ��ip���һ����
					string pt_dest = "PX";
					if(arp_list.find(*k) != arp_list.end())
					{//Ѱ�Ҷ�Ӧ�Ķ˿�(�ӿ�)
                                                for(std::map<string,list<string> >::iterator i_arp = arp_list[*k].begin();
							i_arp != arp_list[*k].end();
							++i_arp
							)
						{
							if(find(i_arp->second.begin(), i_arp->second.end(), ip_src) != i_arp->second.end())
							{
								pt_dest = i_arp->first;
								break;
							}
						}
					}
					//��֤ÿ��Hostֻ����һ��
					bool bNew = true;
					for(EDGE_LIST::iterator ie = edge_list_sh.begin();
						ie != edge_list_sh.end();
						++ie)
					{
						if(*k == ie->ip_right && pt_dest == ie->pt_right)
						{
                                                        qDebug() << "second switch the same ip : " << (*k).c_str();
							bNew = false;
							break;
						}
					}
					if(bNew)
					{
						EDGE edge_tmp;
						edge_tmp.ip_left  = ip_src;
						edge_tmp.pt_left  = pt_src;
						edge_tmp.inf_left = pt_src;
						edge_tmp.ip_right  = *k;
						edge_tmp.pt_right  = pt_dest;
						edge_tmp.inf_right = pt_dest;
						edge_list_sh.push_back(edge_tmp);
                                                qDebug() << "second switch source ip:" << ip_src.c_str() <<  "dest ip:" << (*k).c_str();
						//cout<<"edge:"<<edge_tmp.ip_left<<" "<<edge_tmp.pt_left<<" "<<edge_tmp.ip_right<<" "<<edge_tmp.pt_right<<endl;
					}					
				}//��dest ip ѭ��
			}//��Ҷ�Ӷ˿�
		}
	}
	//add by wings 2009-11-26
	//��ȷ�����㽻������pc��
	for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); ++i)
	{//��source ip ѭ��
		string ip_src = i->first;
		if(find(sw2_list.begin(),sw2_list.end(),ip_src)!=sw2_list.end())
		{
			continue;
		}
                for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{//��source port ѭ��
			if(IsLeafPort(j->second, sw_list))
			{//��Ҷ�Ӷ˿�
				string pt_src = j->first; 
				for(list<string>::iterator k = j->second.begin();
					k != j->second.end();
					++k)
				{//��dest ip ѭ��,Ϊÿ��Ŀ��ip���һ����
					string pt_dest = "PX";
					if(arp_list.find(*k) != arp_list.end())
					{//Ѱ�Ҷ�Ӧ�Ķ˿�(�ӿ�)
                                                for(std::map<string,list<string> >::iterator i_arp = arp_list[*k].begin();
							i_arp != arp_list[*k].end();
							++i_arp
							)
						{
							if(find(i_arp->second.begin(), i_arp->second.end(), ip_src) != i_arp->second.end())
							{
								pt_dest = i_arp->first;
								break;
							}
						}
					}
					//��֤ÿ��Hostֻ����һ��
					bool bNew = true;
					for(EDGE_LIST::iterator ie = edge_list_sh.begin();
						ie != edge_list_sh.end();
						++ie)
					{
						if(*k == ie->ip_right && pt_dest == ie->pt_right)
						{
                                                        qDebug() << "third switch the same ip : " << pt_src.c_str() << ",dest ip" << (*k).c_str();
							bNew = false;
							break;
						}
					}
					if(bNew)
					{
						EDGE edge_tmp;
						edge_tmp.ip_left  = ip_src;
						edge_tmp.pt_left  = pt_src;
						edge_tmp.inf_left = pt_src;
						edge_tmp.ip_right  = *k;
						edge_tmp.pt_right  = pt_dest;
						edge_tmp.inf_right = pt_dest;
						edge_list_sh.push_back(edge_tmp);
                                                qDebug() << "third switch source ip:" << ip_src.c_str() <<  "dest ip:" << (*k).c_str();
						//cout<<"edge:"<<edge_tmp.ip_left<<" "<<edge_tmp.pt_left<<" "<<edge_tmp.ip_right<<" "<<edge_tmp.pt_right<<endl;
					}					
				}//��dest ip ѭ��
			}//��Ҷ�Ӷ˿�
		}
	}
	// remarked by zhangyan 2008-12-04
	// �ý�������·���������ӹ�ϵ�����ڽ����������������ӹ�ϵ
	//topo_edge_list.insert(topo_edge_list.end(), edge_list_sh.begin(), edge_list_sh.end());
	for(EDGE_LIST::iterator i = edge_list_sh.begin(); i != edge_list_sh.end(); ++i)
	{
		bool bNew = true;
		for(EDGE_LIST::iterator j = edge_list_rs_byaft.begin();
			j != edge_list_rs_byaft.end();
			++j)
		{
			if ((j->ip_left == i->ip_left && j->pt_left == i->pt_left)
				|| (j->ip_right == i->ip_right && j->pt_right == i->pt_right)
			    || (j->ip_left == i->ip_right && j->pt_left == i->pt_right)
				|| (j->ip_right == i->ip_left && j->pt_right == i->pt_left)
			    )
			{
				bNew = false;
				break;
			}
		}
		if(bNew)
		{
			cout<<"AnalyseSH:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
			topo_edge_list.push_back(*i);
		}
	}

	//������ipȫ������
	for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); ++i)
	{//��source ip ѭ��
		string ip_src = i->first;
                for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{//��source port ѭ��
			for(list<string>::iterator k = j->second.begin();
				k != j->second.end();
				)
			{//��dest ip ѭ��
				if(find(sw_list.begin(), sw_list.end(), *k) == sw_list.end())
				{
					j->second.erase(k++);
				}
				else
				{
					++k;
				}
			}
		}
	}
	return true;
}

// ����������֮������ӹ�ϵ
bool topoAnalyse::AnalyseSS(void)
{
	if(sw_list.size() < 2)
	{
		return true;
	}
	int test = 0;
	StreamData myfile;
	myfile.saveFrmSSAftList(aft_list,test++);
	for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
		i != aft_list.end();
		++i)
	{
		bool bEmpty = true;
                for(std::map<string,list<string> >::iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{
			for(std::list<string>::iterator k = j->second.begin();
				k != j->second.end();
				)
			{
				if(aft_list.find(*k) == aft_list.end() && find(sw_list.begin(), sw_list.end(), *k) == sw_list.end())
				{
					j->second.erase(k++);
				}
				else
				{
					++k;
				}
			}
		}
	}

	EDGE_LIST edge_list_ss;
	bool bGoOn = true;
	while(bGoOn)
	{
		bGoOn = false;
		//�ȴ���������
		bool bNormal = true;
		while(bNormal)
		{
			list<string> iplist_del;//������Ľ���ip
			bNormal = false;
			for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); ++i)
			{//��source ip ѭ��
				string ip_src = i->first;
                                for(std::map<string,list<string> >::iterator j = i->second.begin();
					j != i->second.end();
					++j)
				{//��source port ѭ��
					if(j->second.empty())
					{
						continue;
					}
					string pt_src = j->first; 

					if(IsSWLeafPort(j->second, sw_list))
					{//�ǽ���Ҷ�Ӷ˿�
						string ip_dest = *(j->second.begin());

						if(aft_list.find(ip_dest) != aft_list.end())
						{
							if(IsSWLeafDevice(ip_src, aft_list[ip_dest]))
							{//�Զ��ǽ���Ҷ��,���һ������-������
								bNormal = true;
								string pt_dest = "PX";//Ŀ���豸�Ķ˿�ȱʡΪPX
                                                                for(std::map<string,list<string> >::iterator j_dest = aft_list[ip_dest].begin();
									j_dest != aft_list[ip_dest].end();
									j_dest++)
								{//Ѱ�ҶԶ��豸�а���source ip�Ķ˿�
									if(find(j_dest->second.begin(), j_dest->second.end(), ip_src) != j_dest->second.end())
									{
										pt_dest = j_dest->first;
										j_dest->second.clear();//��նԶ��а���source ip�Ķ˿�
										break;
									}
								}
								EDGE edge_tmp;
								edge_tmp.ip_left  = ip_src;
								edge_tmp.pt_left  = pt_src;
								edge_tmp.inf_left = pt_src;
								edge_tmp.ip_right  = ip_dest;
								edge_tmp.pt_right  = pt_dest;
								edge_tmp.inf_right = pt_dest;
								edge_list_ss.push_back(edge_tmp);

                                                                qDebug() << "single switch, source ip : " << ip_src.c_str() << " dest ip : " << ip_dest.c_str();

								if(find(iplist_del.begin(), iplist_del.end(), ip_dest) == iplist_del.end())
								{                                                                  
									iplist_del.push_back(ip_dest);
								}

								j->second.clear();//���source port����
							}
							else if(IsInclude(aft_list[ip_dest], aft_list[ip_src], ip_src))
							{
								bNormal = true;
								string pt_dest = "PX";//Ŀ���豸�Ķ˿�ȱʡΪPX
                                                                for(std::map<string,list<string> >::iterator j_dest = aft_list[ip_dest].begin();
									j_dest != aft_list[ip_dest].end();
									++j_dest)
								{//Ѱ�ҶԶ��豸�зǿն˿�
									if(!(j_dest->second.empty()))
									{
										pt_dest = j_dest->first;
										j_dest->second.clear();//��նԶ��а���source ip�Ķ˿�
										break;
										//if (find(j_dest->second.begin(),j_dest->second.end(),ip_src) != j_dest->second.end())
										//{
										//	pt_dest = j_dest->first;
										//	j_dest->second.clear();//��նԶ��а���source ip�Ķ˿�
										//	break;
										//}
										
									}
								}
								EDGE edge_tmp;
								edge_tmp.ip_left  = ip_src;
								edge_tmp.pt_left  = pt_src;
								edge_tmp.inf_left = pt_src;
								edge_tmp.ip_right  = ip_dest;
								edge_tmp.pt_right  = pt_dest;
								edge_tmp.inf_right = pt_dest;
								edge_list_ss.push_back(edge_tmp);

								if(find(iplist_del.begin(), iplist_del.end(), ip_dest) == iplist_del.end())
								{
                                                                        qDebug() << "include source ip : " << ip_src.c_str() << " dest ip : " << ip_dest.c_str();
									iplist_del.push_back(ip_dest);
								}
								j->second.clear();//���source port����
							}
						}
					}
				}
			}
		
			//ɾ��������dest����ip
			for(list<string>::iterator ip_del = iplist_del.begin();
				ip_del != iplist_del.end();
				++ip_del)
			{
				for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
					i != aft_list.end();
					++i)
				{//��source ip ѭ��
					string ip_src = i->first;
                                        for(std::map<string,list<string> >::iterator j = i->second.begin();
						j != i->second.end();
						++j)
					{//��source port ѭ��
						j->second.remove(*ip_del);
					}
				}
			}
		}

		//�ٴ���·
		//ֻҪ����һ̨�豸,�䲻ͬ�˿��ܹ�����ͬһ�豸,��һ�����ڻ�·
		//�����ڻ�·,���ڴ����������ߺ�,��Ҫ��������������
		bool bCircle = true;
		//myfile.saveFrmSSAftList(aft_list,test++);
		//update by wings 2009-11-13  
		//�Ѿ����Լ��㻷·
		while(bCircle)
		{
			bCircle = false;
			for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); ++i)
			{//��source ip ѭ��
				if(i->second.size() > 1)
				{//���ܴ��ڻ�·(���ڶ���˿ڣ�
					string ip_src = i->first;//source ip
                                        std::map<string,list<string> >::iterator k_start;
                                        std::map<string,list<string> >::iterator j_end = i->second.end();
					j_end--;
                                        list<pair<string,string> > sameptip_list; //��ͬ�� <port, dest ip>���
																//��source ip�¾�����ͬ��Ŀ��ip�Ķ˿ں�Ŀ��IP�Ķ��б�
					list<string> src_pt_list;//��·��i->first�Ķ˿�
					list<string> dest_pt_list;//��·��k->first�Ķ˿�
					string dest_ip;
					string src_ip;
					bool cycleflag = false;
					int cycle = 0;
					int cycle2 = 0;
                                        for(std::map<string,list<string> >::iterator j = i->second.begin();
						j != j_end; 
						++j)
					{//��source port ѭ��
						if(cycleflag)//һ��ֻ������ip�Ļ�·
							break;
						k_start = j;
						k_start++; // = j+1
						for(std::list<string>::iterator ip_j = j->second.begin();
							ip_j != j->second.end();
							++ip_j)//ip_j��destip
						{
							if(cycleflag)//һ��ֻ������ip�Ļ�·
								break;
                                                        for(std::map<string,list<string> >::iterator k = k_start;//j+1��ʼ
								k != i->second.end();
								++k)
							{//�Ƚ� j,k �˿ڼ��ϵ�Ԫ��
								for(list<string>::iterator ip_k = k->second.begin();
									ip_k != k->second.end();
									++ip_k)
								{//k�˿��µ�destipѭ����ip_k��
									if(*ip_k == *ip_j)
									{//k,j�˿��´�����ͬ��destip
										if(!cycleflag)
										{
											dest_ip = *ip_j;
											src_ip  = i->first;
											cycleflag = true;
										}
										bool bk = true, bj = true;
										for(list<string>::iterator pt1 = src_pt_list.begin();
											pt1 != src_pt_list.end();
											++pt1)
										{
											if(*pt1 == j->first)
											{
												bj = false;
												break;
											}
										}
										for(list<string>::iterator pt2 = src_pt_list.begin();
											pt2 != src_pt_list.end();
											++pt2)
										{
											if(*pt2 == k->first)
											{
												bk = false;
												break;
											}
										}
										if(bk)
										{
											src_pt_list.push_back(k->first);
											cycle++;
										}
										if(bj)
										{
											src_pt_list.push_back(j->first);
											cycle++;
										}
										break;
									}
								}
							}
						}
					}
					if(cycleflag)
					{//���ڻ�·
						bGoOn = true; //�������������������
						//����·��ӵ��߼���
						FRM_AFTARP_LIST::iterator ps_peer = aft_list.find(dest_ip);
						if(ps_peer != aft_list.end())
						{
                                                        for(map<string,list<string> >::iterator j_peer = ps_peer->second.begin();
								j_peer != ps_peer->second.end();
								++j_peer)
							{//�Զ˶˿ڼ���
								for(list<string>::iterator ip_peer = j_peer->second.begin();
									ip_peer != j_peer->second.end();
									)
								{//�Զ˶˿ڼ����е�Ԫ��
									if(*ip_peer == ip_src)
									{//�õ�һ���Ա߶˿�
										dest_pt_list.push_back(j_peer->first);
										j_peer->second.erase(ip_peer++);
										cycle2++;
										break;
									}
									else
									{
										++ip_peer;
									}
								}
							}
							if(cycle2>0)
							{
								list<string>::iterator spt = src_pt_list.begin();
								list<string>::iterator dpt = dest_pt_list.begin();
								for(;spt != src_pt_list.end()&&dpt != dest_pt_list.end();
									++spt,++dpt)
								{
									bCircle = true;
									EDGE edge_tmp;
									edge_tmp.ip_left  = src_ip;
									edge_tmp.pt_left  = *spt;
									edge_tmp.inf_left = *spt;
									edge_tmp.ip_right  = dest_ip;
									edge_tmp.pt_right  = *dpt;
									edge_tmp.inf_right = *dpt;
                                                                        ////SvLog::writeLog("one cycle edge"+edge_tmp.ip_left+":"+edge_tmp.inf_left+
									//"--"+edge_tmp.ip_right+":"+edge_tmp.inf_right);	
									edge_list_ss.push_back(edge_tmp);
								}
							}
						}
					
						
						//��source ��ɾ����·�豸
                                                for(std::map<string,list<string> >::iterator j = i->second.begin();
							j != i->second.end();
							++j)
						{//��source port ѭ��
							for(list<string>::iterator k = j->second.begin();
								k != j->second.end();
								)
							{//��dest ip
								if(*k == dest_ip)
								{
									j->second.erase(k++);
									break;
								}
								else
								{
									++k;
								}
							}
						}
					}
				}
			}
		}
		//myfile.saveFrmSSAftList(aft_list,test++);
	}
	//todo:ʣ��Ľ�������,�������Ӽ���
	for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
		i != aft_list.end();
		++i)
	{//��source ip ѭ��
		string ip_src = i->first;
                for(std::map<string,list<string> >::iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{//��source port ѭ��
			string pt_src = j->first;
			for(list<string>::iterator k = j->second.begin();
				k != j->second.end();
				++k)
			{//��dest ip ѭ��
				FRM_AFTARP_LIST::iterator res_dest = aft_list.find(*k);
				if(res_dest != aft_list.end())
				{//�Զ˴�����aft��
                                        for(std::map<string,list<string> >::iterator m = res_dest->second.begin();
						m != res_dest->second.end();
						++m)
					{
						if(find(m->second.begin(), m->second.end(), ip_src) != m->second.end())
						{
							//���һ��ss��
							EDGE edge_tmp;
							edge_tmp.ip_left  = ip_src;
							edge_tmp.pt_left  = pt_src;
							edge_tmp.inf_left = pt_src;
							edge_tmp.ip_right  = *k;
							edge_tmp.pt_right  = m->first;
							edge_tmp.inf_right = m->first;
							edge_list_ss.push_back(edge_tmp);
							//�ӶԶ˶˿ڼ�����ɾ��Դip
                                                        qDebug() << "last edge source ip : " << ip_src.c_str() << " dest ip : " << (*k).c_str();

							m->second.remove(ip_src); //update by jiangshanwen 20100607
						}
						//else
						//{
						//	EDGE edge_tmp;
						//	edge_tmp.ip_left  = ip_src;
						//	edge_tmp.pt_left  = pt_src;
						//	edge_tmp.inf_left = pt_src;
						//	edge_tmp.ip_right  = *k;
						//	edge_tmp.pt_right  ="PX";
						//	edge_tmp.inf_right ="PX";
						//	edge_list_ss.push_back(edge_tmp);
						//	//�ӶԶ˶˿ڼ�����ɾ��Դip
						//	m->second.remove(ip_src); //update by jiangshanwen 20100607
						//}
					}
				}
			}
		}
	}
	//add by wings 2009-11-13
	ofstream output("test/edge_ss.txt",ios::out);
	for(EDGE_LIST::const_iterator i = edge_list_ss.begin(); i != edge_list_ss.end(); i++)
	{
		string line = i->ip_left+":"+i->inf_left+"--"+i->ip_right+":"+i->inf_right+";";
		output << line << endl;
	}
	output.close();
	for(EDGE_LIST::iterator i = edge_list_ss.begin(); 
		i != edge_list_ss.end(); 
		++i)
	{
		// added by zhangyan 2008-12-04
		if (i->inf_left == "PX" || i->inf_right == "PX")
		{
			//����ͨ���������Ƿ��������ӹ�ϵ
			bool bNew = true;
			TraceAnalyse::getConnection(topo_edge_list);
                        for(std::list<std::list<std::string> >::iterator j = TraceAnalyse::set_conn.begin();	j != TraceAnalyse::set_conn.end(); ++j)
			{
				if ((find(j->begin(), j->end(), i->ip_left) != j->end()) && (find(j->begin(), j->end(), i->ip_right) != j->end()))
				{
					//����ͬһ��ͨ��
					bNew = false;
					break;
				}
			}
			if(bNew)
			{
				cout<<"AnalyseSS:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
				topo_edge_list.push_back(*i);
			}
		}
		else
		{
			bool bNew = true;
			for(EDGE_LIST::iterator j = topo_edge_list.begin();
				j != topo_edge_list.end();
				++j)
			{
				if( (i->ip_left == j->ip_left && i->inf_left == j->inf_left) || 
					(i->ip_right == j->ip_right && i->inf_right == j->inf_right) ||
					(i->ip_left == j->ip_right && i->inf_left == j->inf_right) ||
					(i->ip_right == j->ip_left && i->inf_right == j->inf_left))
				{
					bNew = false;
					break;
				}
			}
			if(bNew)
			{
				cout<<"AnalyseSS:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
				topo_edge_list.push_back(*i);
			}
		}
	}

	//add by wings 2009-11-13
	ofstream output1("test/edge_ssss.txt",ios::out);
	for(EDGE_LIST::const_iterator i = topo_edge_list.begin(); i != topo_edge_list.end(); i++)
	{
		string line = i->ip_left+":"+i->inf_left+"--"+i->ip_right+":"+i->inf_right+";";
		output1 << line << endl;
	}
	output1.close();
	return true;
}

bool topoAnalyse::AnalyseRS_COMP(void)
{
	EDGE_LIST edge_list_rs;
	bool bGoOn = true;
	while(bGoOn)
	{
		list<string> iplist_del;//�������·����ip
		bGoOn = false;
		for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
			i != aft_list.end();
			++i)
		{//��source ip ѭ��
			string ip_src = i->first;
                        for(std::map<string,list<string> >::iterator j = i->second.begin();
				j != i->second.end();
				++j)
			{//��source port ѭ��
				bool bSkip = false;
				for(list<string>::iterator isw = sw_list.begin();
					isw != sw_list.end();
					++isw)
				{
					if(find(j->second.begin(), j->second.end(), *isw) != j->second.end())
					{
						bSkip = true;
						break;
					}
				}
				if(bSkip)
				{//�������ڽ����豸�Ľӿڼ���
					continue;
				}

				string pt_src = j->first; 
				for(list<string>::iterator k = j->second.begin();
					k != j->second.end();
					k++)
				{//��dest ip ѭ��
					if(find(sw_list.begin(), sw_list.end(), *k) != sw_list.end())
					{//����Ϊ�����豸��dest_ip
						continue;
					}
					string ip_dest = *k;
					if(find(rt_list.begin(), rt_list.end(), ip_dest) != rt_list.end())
					{//Ŀ��ipΪrouter
						bGoOn = true;
						string pt_dest = "PX";//Ŀ���豸�Ķ˿�ȱʡΪPX
						if(arp_list.find(ip_dest) != arp_list.end())
						{//dest ip������arp�����б���
                                                        for(std::map<string,list<string> >::iterator j_dest = arp_list[ip_dest].begin();
								j_dest != arp_list[ip_dest].end();
								j_dest++)
							{//Ѱ�ҶԶ��豸�а���source ip�Ķ˿�
								if(find(j_dest->second.begin(), j_dest->second.end(), ip_src) != j_dest->second.end())
								{
									pt_dest = j_dest->first;
									break;
								}
							}
						}
						if(pt_dest != "PX")
						{//���Բ����ҵ�Ŀ�Ķ˿ڵı�
							//��֤һ���˿�ֻ����һ���豸
							bool bNew = true;
							if(bNew)
							{
								EDGE edge_tmp;
								edge_tmp.ip_left  = ip_src;
								edge_tmp.pt_left  = pt_src;
								edge_tmp.inf_left = pt_src;
								edge_tmp.ip_right  = ip_dest;
								edge_tmp.pt_right  = pt_dest;
								edge_tmp.inf_right = pt_dest;
								edge_list_rs.push_back(edge_tmp);
							}
						}
						//�����Ѵ����ip�б�
						if(find(iplist_del.begin(), iplist_del.end(), ip_dest) == iplist_del.end())
						{
							iplist_del.push_back(ip_dest);
						}
					}
				}
			}
		}
		//ɾ��������dest ip
		if(!iplist_del.empty())
		{
			for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
				i != aft_list.end();
				i++)
			{//��source ip ѭ��
				string ip_src = i->first;
                                for(std::map<string,list<string> >::iterator j = i->second.begin();
					j != i->second.end();
					j++)
				{//��source port ѭ��
					for(list<string>::iterator k = j->second.begin();
						k != j->second.end();)
					{
						if(find(iplist_del.begin(), iplist_del.end(), *k) != iplist_del.end())
						{//ִ��ɾ��
							j->second.erase(k++);
						}
						else
						{
							++k;
						}
					}
				}
			}
		}
	}
	for(EDGE_LIST::iterator i = edge_list_rs.begin(); 
		i != edge_list_rs.end(); 
		i++)
	{
		bool bNew = true;
		for(EDGE_LIST::iterator j = topo_edge_list.begin();
			j != topo_edge_list.end();
			j++)
		{
			if( (i->ip_left == j->ip_left && i->ip_right == j->ip_right) || 
				(i->ip_right == j->ip_left && i->ip_left == j->ip_right))
			{
				bNew = false;
				break;
			}
		}
		if(bNew)
		{
			cout<<"AnalyseRS_COMP:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
			topo_edge_list.push_back(*i);
		}
	}

	//�����ܷ�����router ip ȫ������
	for(list<string>::iterator rt = rt_list.begin();
		rt != rt_list.end();
		rt++)
	{
		for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); i++)
		{//��source ip ѭ��
                        for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); j++)
			{//��source port ѭ��
				j->second.remove(*rt);
			}
		}
	}
	return true;
}

// ����������������֮������ӹ�ϵ
bool topoAnalyse::AnalyseSH_COMP(void)
{
	EDGE_LIST edge_list_sh;
	bool bGoOn = true;
	while(bGoOn)
	{
		bGoOn = false;
		list<string> iplist_del;//�������ip
		for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); i++)
		{//��source ip ѭ��
			string ip_src = i->first;
                        for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); j++)
			{//��source port ѭ��
				if(IsLeafPort(j->second, sw_list))
				{//��Ҷ�Ӷ˿�
					string pt_src = j->first; 
					for(list<string>::iterator k = j->second.begin();
						k != j->second.end();
						k++)
					{//��dest ip ѭ��,Ϊÿ��Ŀ��ip���һ����
						if(find(sw_list.begin(), sw_list.end(), *k) != sw_list.end())
						{//����Ϊ�����豸��dest_ip
							continue;
						}
						if(find(rs_list.begin(), rs_list.end(), *k) != rs_list.end())
						{
							continue;
						}
						bGoOn = true;
						string pt_dest = "PX";
						if(arp_list.find(*k) != arp_list.end())
						{//��·�����Ҵ��ڶ�Ӧ��arp��Ŀ,Ѱ�Ҷ�Ӧ�Ķ˿�(�ӿ�)
                                                        for(std::map<string,list<string> >::iterator i_arp = arp_list[*k].begin();
								i_arp != arp_list[*k].end();
								i_arp++
								)
							{
								if(find(i_arp->second.begin(), i_arp->second.end(), ip_src) != i_arp->second.end())
								{
									pt_dest = i_arp->first;
									break;
								}
							}
						}
						////��֤ÿ��Hostֻ����һ��
						//bool bNew = true;
						//for(EDGE_LIST::iterator ie = edge_list_sh.begin();
						//	ie != edge_list_sh.end();
						//	ie++)
						//{
						//	if(*k == ie->ip_right && pt_dest == ie->pt_right)
						//	{
						//		bNew = false;
						//		break;
						//	}
						//}
						//if(bNew)
						{
							EDGE edge_tmp;
							edge_tmp.ip_left  = ip_src;
							edge_tmp.pt_left  = pt_src;
							edge_tmp.inf_left = pt_src;
							edge_tmp.ip_right  = *k;
							edge_tmp.pt_right  = pt_dest;
							edge_tmp.inf_right = pt_dest;
							edge_list_sh.push_back(edge_tmp);

						}
						//����dest ip ��ӵ��Ѿ������ip��ַ����
						if(find(iplist_del.begin(), iplist_del.end(), *k) == iplist_del.end())
						{
							iplist_del.push_back(*k);
						}
					}//��dest ip ѭ��
				}//��Ҷ�Ӷ˿�
			}
		}
		//ɾ��������dest ip
		for(list<string>::iterator ip_del = iplist_del.begin();
			ip_del != iplist_del.end();
			ip_del++)
		{
			for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
				i != aft_list.end();
				i++)
			{//��source ip ѭ��
				string ip_src = i->first;
                                for(std::map<string,list<string> >::iterator j = i->second.begin();
					j != i->second.end();
					j++)
				{//��source port ѭ��
					j->second.remove(*ip_del);
				}
			}
		}
	}	

	for(EDGE_LIST::iterator i = edge_list_sh.begin(); 
		i != edge_list_sh.end(); 
		i++)
	{
		bool bNew = true;
		for(EDGE_LIST::iterator j = topo_edge_list.begin();
			j != topo_edge_list.end();
			j++)
		{
			if (j->ip_right == i->ip_right) 
			{
				bNew = false;
				break;
			}
		}
		if(bNew)
		{
			cout<<"AnalyseSH_COMP:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
			topo_edge_list.push_back(*i);
		}
	}

	//�����ܷ���������ipȫ������
	for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); i++)
	{//��source ip ѭ��
		string ip_src = i->first;
                for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); j++)
		{//��source port ѭ��
			for(list<string>::iterator k = j->second.begin();
				k != j->second.end();
				)
			{//��dest ip ѭ��
				if(find(sw_list.begin(), sw_list.end(), *k) == sw_list.end()
					&& find(sw_list.begin(), sw_list.end(), *k) == sw_list.end())
				{
					j->second.erase(k++);
				}
				else
				{
					++k;
				}
			}
		}
	}

	//printdata(aft_list);

	return true;
}

// ����������֮������ӹ�ϵ
bool topoAnalyse::AnalyseSS_COMP(void)
{
	if(sw_list.size() < 2)
	{
		return true;
	}

	for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
		i != aft_list.end();
		++i)
	{
		bool bEmpty = true;
                for(std::map<string,list<string> >::iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{
			for(std::list<string>::iterator k = j->second.begin();
				k != j->second.end();
				)
			{
				if(aft_list.find(*k) == aft_list.end() && find(sw_list.begin(), sw_list.end(), *k) == sw_list.end())
				{
					j->second.erase(k++);
				}
				else
				{
					++k;
				}
			}
		}
	}

	EDGE_LIST edge_list_ss;
	bool bGoOn = true;
	while(bGoOn)
	{
		bGoOn = false;
		//�ȴ���������
		bool bNormal = true;
		while(bNormal)
		{
			list<string> iplist_del;//������Ľ���ip
			bNormal = false;
			for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
				i != aft_list.end();
				i++)
			{//��source ip ѭ��
				string ip_src = i->first;
                                for(std::map<string,list<string> >::iterator j = i->second.begin();
					j != i->second.end();
					j++)
				{//��source port ѭ��
					if(j->second.empty())
					{
						continue;
					}
					string pt_src = j->first; 
					if(IsSWLeafPort(j->second, sw_list))
					{//�ǽ���Ҷ�Ӷ˿�
						string ip_dest = *(j->second.begin());
						if(aft_list.find(ip_dest) != aft_list.end())
						{
							if(IsSWLeafDevice(ip_src, aft_list[ip_dest]))
							{//�Զ��ǽ���Ҷ��,���һ������-������
								bNormal = true;
								string pt_dest = "PX";//Ŀ���豸�Ķ˿�ȱʡΪPX
                                                                for(std::map<string,list<string> >::iterator j_dest = aft_list[ip_dest].begin();
									j_dest != aft_list[ip_dest].end();
									++j_dest)
								{//Ѱ�ҶԶ��豸�а���source ip�Ķ˿�
									if(find(j_dest->second.begin(), j_dest->second.end(), ip_src) != j_dest->second.end())
									{
										pt_dest = j_dest->first;
										j_dest->second.remove(ip_src);//ɾ���Զ��е�source ip
										break;
									}
								}
								EDGE edge_tmp;
								edge_tmp.ip_left  = ip_src;
								edge_tmp.pt_left  = pt_src;
								edge_tmp.inf_left = pt_src;
								edge_tmp.ip_right  = ip_dest;
								edge_tmp.pt_right  = pt_dest;
								edge_tmp.inf_right = pt_dest;
								edge_list_ss.push_back(edge_tmp);

								if(find(iplist_del.begin(), iplist_del.end(), ip_dest) == iplist_del.end())
								{
									iplist_del.push_back(ip_dest);
								}
								j->second.clear();//���source port����
							}
						}
					}
				}
			}
		
			//ɾ��������dest����ip
			for(list<string>::iterator ip_del = iplist_del.begin();
				ip_del != iplist_del.end();
				++ip_del)
			{
				for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
					i != aft_list.end();
					++i)
				{//��source ip ѭ��
					string ip_src = i->first;
                                        for(std::map<string,list<string> >::iterator j = i->second.begin();
						j != i->second.end();
						++j)
					{//��source port ѭ��
						j->second.remove(*ip_del);
					}
				}
			}
		}

		//�ٴ���·
		//ֻҪ����һ̨�豸,�䲻ͬ�˿��ܹ�����ͬһ�豸,��һ�����ڻ�·
		//�����ڻ�·,���ڴ����������ߺ�,��Ҫ��������������
		bool bCircle = true;
		while(bCircle)
		{
			bCircle = false;
			for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
				i != aft_list.end();
				++i)
			{//��source ip ѭ��
				if(i->second.size() > 1)
				{//���ܴ��ڻ�·
					string ip_src = i->first;//source ip
                                        std::map<string,list<string> >::iterator k_start;
                                        std::map<string,list<string> >::iterator j_end = i->second.end();
					j_end--;
                                        list<pair<string,string> > sameptip_list; //��ͬ�� <port, dest ip>
                                        for(std::map<string,list<string> >::iterator j = i->second.begin();
						j != j_end;
						++j)
					{//��source port ѭ��
						k_start = j;
						k_start++; // = j+1
                                                for(std::map<string,list<string> >::iterator k = k_start;
							k != i->second.end();
							++k)
						{//�Ƚ� j,k �˿ڼ��ϵ�Ԫ��
							for(list<string>::iterator ip_j = j->second.begin();
								ip_j != j->second.end();
								++ip_j)
							{
								for(list<string>::iterator ip_k = k->second.begin();
									ip_k != k->second.end();
									ip_k++)
								{
									if(*ip_k == *ip_j)
									{
										bool bk = true, bj = true;
                                                                                for(list<pair<string,string> >::iterator ptip = sameptip_list.begin();
											ptip != sameptip_list.end();
											++ptip)
										{
											if(ptip->first == j->first && ptip->second == *ip_j)
											{//��ͬ�˿ڵ���ͬ�ɴ��豸
												bj = false;
											}
											if(ptip->first == k->first && ptip->second == *ip_k)
											{//��ͬ�˿ڵ���ͬ�ɴ��豸
												bk = false;
											}
										}
										if(bk)
											sameptip_list.push_back(make_pair(k->first, *ip_k));
										if(bj)
											sameptip_list.push_back(make_pair(j->first, *ip_j));
									}
								}
							}
						}
					}
					if(!sameptip_list.empty())
					{//���ڻ�·
						bGoOn = true; //�������������������
						//����·��ӵ��߼���
                                                for(list<pair<string,string> >::iterator ptip_dest = sameptip_list.begin();
							ptip_dest != sameptip_list.end();
							++ptip_dest)
						{//���ԶԶ�û��source ip ������FRM_AFTARP_LIST
							string pt_src = ptip_dest->first;
							string ip_dest = ptip_dest->second;
							FRM_AFTARP_LIST::iterator ps_peer = aft_list.find(ip_dest);
							if(ps_peer != aft_list.end())
							{//���ڶԶ˽����豸��ת��������
                                                                for(map<string,list<string> >::iterator j_peer = ps_peer->second.begin();
									j_peer != ps_peer->second.end();
									++j_peer)
								{//�Զ˶˿ڼ���
									for(list<string>::iterator ip_peer = j_peer->second.begin();
										ip_peer != j_peer->second.end();
										)
									{//�Զ˶˿ڼ����е�Ԫ��
										if(*ip_peer == ip_src)
										{//����һ����·��
											EDGE edge_tmp;
											edge_tmp.ip_left  = ip_src;
											edge_tmp.pt_left  = pt_src;
											edge_tmp.inf_left = pt_src;
											edge_tmp.ip_right  = ip_dest;
											edge_tmp.pt_right  = j_peer->first;
											edge_tmp.inf_right = j_peer->first;
											edge_list_ss.push_back(edge_tmp);

											//�ڶԶ��豸��ɾ���� ip_src
											j_peer->second.erase(ip_peer++);
										}
										else
										{
											++ip_peer;
										}
									}
								}
							}
						}
						//��source ��ɾ����·�豸
                                                for(std::map<string,list<string> >::iterator j = i->second.begin();
							j != j_end;
							++j)
						{//��source port ѭ��
							for(list<string>::iterator k = j->second.begin();
								k != j->second.end();
								)
							{//��dest ip
								bool bDelete = false;
                                                                for(list<pair<string,string> >::iterator ptip_dest = sameptip_list.begin();
									ptip_dest != sameptip_list.end();
									++ptip_dest)
								{
									if( ptip_dest->second == *k)
									{
										bDelete = true;
										break;
									}
								}
								if(bDelete)
								{
									j->second.erase(k++);
								}
								else
								{
									++k;
								}
							}
						}
					}
				}
			}
		}
	}
	//todo:ʣ��Ľ�������,�������Ӽ���
	for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
		i != aft_list.end();
		++i)
	{//��source ip ѭ��
		string ip_src = i->first;
                for(std::map<string,list<string> >::iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{//��source port ѭ��
			string pt_src = j->first;
			for(list<string>::iterator k = j->second.begin();
				k != j->second.end();
				++k)
			{//��dest ip ѭ��
				FRM_AFTARP_LIST::iterator res_dest = aft_list.find(*k);
				if(res_dest != aft_list.end())
				{//�Զ˴�����aft��
                                        for(std::map<string,list<string> >::iterator m = res_dest->second.begin();
						m != res_dest->second.end();
						++m)
					{
						//if(find(m->second.begin(), m->second.end(), *k) != m->second.end())
						if(find(m->second.begin(), m->second.end(), ip_src) != m->second.end())
						{
							//���һ��ss��
							EDGE edge_tmp;
							edge_tmp.ip_left  = ip_src;
							edge_tmp.pt_left  = pt_src;
							edge_tmp.inf_left = pt_src;
							edge_tmp.ip_right  = *k;
							edge_tmp.pt_right  = m->first;
							edge_tmp.inf_right = m->first;
							edge_list_ss.push_back(edge_tmp);
							//�ӶԶ˶˿ڼ�����ɾ��Դip
							//m->second.remove(*k);
							m->second.remove(ip_src);
						}
					}
				}
			}
		}
	}

	//printdata(aft_list);
        std::list<std::list<std::string> > set_conn;
	for(EDGE_LIST::iterator i = topo_edge_list.begin();
		i != topo_edge_list.end();
		++i)
	{
		int iConnected = 0;
                std::list<std::list<std::string> >::iterator i_left,i_right;
                for(std::list<std::list<std::string> >::iterator j = set_conn.begin();
			j != set_conn.end();
			++j)
		{
			if( find(j->begin(), j->end(), i->ip_left) != j->end())
			{
				i_left = j;
				iConnected += 1;
				break;
			}
		}
                for(std::list<std::list<std::string> >::iterator j = set_conn.begin();
			j != set_conn.end();
			++j)
		{
			if( find(j->begin(), j->end(), i->ip_right) != j->end())
			{
				i_right = j;
				iConnected += 2;
				break;
			}
		}
		if(iConnected == 0)
		{//����ͨ��
			list<string> list_new;
			list_new.push_back(i->ip_left);
			list_new.push_back(i->ip_right);
			for(EDGE_LIST::iterator j = topo_edge_list.begin();
				j != topo_edge_list.end();
				++j)
			{
				if(i->ip_left == j->ip_left || i->ip_right == j->ip_left)
				{
					if(find(list_new.begin(), list_new.end(), j->ip_right) == list_new.end())
					{
						list_new.push_back(j->ip_right);
					}
				}
				if(i->ip_left == j->ip_right || i->ip_right == j->ip_right)
				{
					if(find(list_new.begin(), list_new.end(), j->ip_left) == list_new.end())
					{
						list_new.push_back(j->ip_left);
					}
				}
			}
			set_conn.push_back(list_new);
		}
		else if(iConnected == 1)
		{//����������ͨ��
			i_left->push_back(i->ip_right);
		}
		else if(iConnected == 2)
		{//����������ͨ��
			i_right->push_back(i->ip_left);
		}
		else if(iConnected == 3)
		{
			if(i_left == i_right)
			{//������ͬһ��ͨ�������ڻ�·
			}
			else
			{//�ϲ���ͨ��
				for(list<string>::iterator j = i_right->begin();
					j != i_right->end();
					++j)
				{
					if(find(i_left->begin(), i_left->end(), *j) == i_left->end())
					{
						i_left->push_back(*j);
					}
				}
				set_conn.remove(*i_right);
			}
		}
	}

	for(EDGE_LIST::iterator i = edge_list_ss.begin(); i != edge_list_ss.end(); ++i)
	{
		int iConnected = 0;
                std::list<std::list<std::string> >::iterator i_left,i_right;
                for(std::list<std::list<std::string> >::iterator j = set_conn.begin();
			j != set_conn.end();
			++j)
		{
			if( find(j->begin(), j->end(), i->ip_left) != j->end())
			{
				i_left = j;
				iConnected += 1;
				break;
			}
		}
                for(std::list<std::list<std::string> >::iterator j = set_conn.begin();
			j != set_conn.end();
			++j)
		{
			if( find(j->begin(), j->end(), i->ip_right) != j->end())
			{
				i_right = j;
				iConnected += 2;
				break;
			}
		}
		bool bNew = true;
		if(iConnected == 0)
		{//����ͨ��
			list<string> list_new;
			list_new.push_back(i->ip_left);
			list_new.push_back(i->ip_right);
			set_conn.push_back(list_new);
		}
		else if(iConnected == 1)
		{//����������ͨ��
			i_left->push_back(i->ip_right);
		}
		else if(iConnected == 2)
		{//����������ͨ��
			i_right->push_back(i->ip_left);
		}
		else if(iConnected == 3)
		{
			if(i_left != i_right)
			{//�ϲ���ͨ��
				for(list<string>::iterator j = i_right->begin();
					j != i_right->end();
					++j)
				{
					if(find(i_left->begin(), i_left->end(), *j) == i_left->end())
					{
						i_left->push_back(*j);
					}
				}
				set_conn.remove(*i_right);
			}
			else
			{//����ͬһ��ͨ����
				bNew = false;
			}
		}
		if(bNew)
		{
			cout<<"AnalyseSS_COMP:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
			topo_edge_list.push_back(*i);
		}
	}
	return true;
}

// ����·����֮������ӹ�ϵ
bool topoAnalyse::AnalyseRRByArp(void)
{
	if(rs_list.empty() || arp_list.empty())
	{
		return true;
	}
	EDGE_LIST edge_list_rr;
	bool bGoOn = true;
	while(bGoOn)
	{
		list<string> iplist_del;//�������·����ip
		bGoOn = false;
		for(FRM_AFTARP_LIST::iterator i = arp_list.begin();
			i != arp_list.end();
			++i)
		{//��source ip ѭ��
			if(find(rs_list.begin(), rs_list.end(), i->first) == rs_list.end())
			{//src ip��Ϊrouter
				continue;
			}
			string ip_src = i->first;
                        for(std::map<string,list<string> >::iterator j = i->second.begin();
				j != i->second.end();
				++j)
			{//��source port ѭ��
				//����û��router�Ķ˿ڼ�
				bool bRPort = false;
				for(list<string>::iterator k = rs_list.begin();
					k != rs_list.end();
					++k)
				{
					if(find(j->second.begin(), j->second.end(), *k) != j->second.end())
					{
						bRPort = true;
						break;
					}
				}
				if(!bRPort)	
				{
					continue;
				}
				else
				{//���԰��������豸�Ķ˿�
					bRPort = false;
					for(list<string>::iterator k = sw_list.begin();
						k != sw_list.end();
						++k)
					{
						if(find(j->second.begin(), j->second.end(), *k) != j->second.end())
						{
							bRPort = true;
							break;
						}
					}
					if(bRPort)	continue;
				}

				string pt_src = j->first; 
				for(list<string>::iterator k = j->second.begin();
					k != j->second.end();
					)
				{//��dest ip ѭ��
					if(find(rs_list.begin(), rs_list.end(), *k) == rs_list.end())
					{//���Է�router��dest ip
						++k;
						continue;
					}
					//Ŀ��ipΪrouter
					string ip_dest = *k;
					bGoOn = true;
					string pt_dest = "PX";//Ŀ���豸�Ķ˿�ȱʡΪPX
					if(arp_list.find(ip_dest) != arp_list.end())
					{//dest ip������arp�����б���
                                                for(std::map<string,list<string> >::iterator j_dest = arp_list[ip_dest].begin();
							j_dest != arp_list[ip_dest].end();
							++j_dest)
						{//Ѱ�ҶԶ��豸�а���source ip�Ķ˿�
							list<string>::iterator ip_tmp = find(j_dest->second.begin(), j_dest->second.end(), ip_src);
							if( ip_tmp != j_dest->second.end())
							{//���ֶԶ�Router
								pt_dest = j_dest->first;
								j_dest->second.erase(ip_tmp);//�ӶԶ�ɾ��source ip
								break;
							}
						}
					}
					EDGE edge_tmp;
					edge_tmp.ip_left  = ip_src;
					edge_tmp.pt_left  = pt_src;
					edge_tmp.inf_left = pt_src;
					edge_tmp.ip_right  = ip_dest;
					edge_tmp.pt_right  = pt_dest;
					edge_tmp.inf_right = pt_dest;
					edge_list_rr.push_back(edge_tmp);
					j->second.erase(k++);//��src ps ɾ��dest ip
				}
			}
		}
	}

	for(EDGE_LIST::iterator i = edge_list_rr.begin();
		i != edge_list_rr.end();
		++i)
	{
		if(i->ip_left == i->ip_right)
		{
			continue;
		}
		// added by zhangyan 2008-12-04
		if (i->inf_left == "PX" || i->inf_right == "PX")
		{
			//����ͨ���������Ƿ��������ӹ�ϵ
			bool bNew = true;
			TraceAnalyse::getConnection(topo_edge_list);
                        for(std::list<std::list<std::string> >::iterator j = TraceAnalyse::set_conn.begin();	j != TraceAnalyse::set_conn.end(); ++j)
			{
				if ((find(j->begin(), j->end(), i->ip_left) != j->end()) && (find(j->begin(), j->end(), i->ip_right) != j->end()))
				{
					//����ͬһ��ͨ��
					bNew = false;
					break;
				}
			}
			if(bNew)
			{
				cout<<"AnalyseRRByArp:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
				topo_edge_list.push_back(*i);
			}
		}
		else
		{
			bool bNew = true;
			// remarked by zhangyan 2008-12-04
			/*if( find(rt_list.begin(), rt_list.end(), i->ip_left) != rt_list.end() )
			{
				for(EDGE_LIST::iterator j = topo_edge_list.begin();
					j != topo_edge_list.end();
					++j)
				{
					if((j->ip_left == i->ip_left && j->inf_left == i->inf_left) 
						|| (j->ip_right == i->ip_left && j->inf_right == i->inf_left)
						)
					{
						bNew = false;
						break;
					}
				}
			}*/
			if(bNew)
			{
				for(EDGE_LIST::iterator j = topo_edge_list.begin();
						j != topo_edge_list.end();
						++j)
				{
					/*if((j->ip_left == i->ip_right && j->inf_left == i->inf_right) 
						|| (j->ip_right == i->ip_right && j->inf_right == i->inf_right)
						)*/
					if ((j->ip_left == i->ip_left && j->pt_left == i->pt_left)
						|| (j->ip_right == i->ip_right && j->pt_right == i->pt_right)
						|| (j->ip_left == i->ip_right && j->pt_left == i->pt_right)
						|| (j->ip_right == i->ip_left && j->pt_right == i->pt_left)
						)
					{
						bNew = false;
						break;
					}
				}
				//���˵�Vlan�˿�
				if((IPADDR_map.find(i->ip_left) != IPADDR_map.end()) && (IPADDR_map.find(i->ip_right) != IPADDR_map.end()))
				{
					bNew = false;
				}
				
			}
			if(bNew)
			{
				cout<<"AnalyseRRByArp:"<<i->ip_left<<" "<<i->inf_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
				topo_edge_list.push_back(*i);
			}
		}
	}
	return true;
}

// ����·����֮������ӹ�ϵ
bool topoAnalyse::AnalyseRRByNbr(void)
{
	EDGE_LIST edge_list_rr;
	for(OSPFNBR_LIST::iterator i = nbr_list.begin();
		i != nbr_list.end();
		++i)
	{//��source ip ѭ��
		string ip_src = i->first;
		string ip_src_frm = ip_src;
		vector<string> ip_src_list;
		for(DEVID_LIST::iterator j = device_list.begin();
			j != device_list.end();
			++j)
		{
			if(find(j->second.ips.begin(), j->second.ips.end(), ip_src) != j->second.ips.end())
			{
				ip_src_list = j->second.ips;
				ip_src_frm = j->first;
				break;
			}
		}
		if(ip_src_list.empty())
		{
			continue;
		}

                for(std::map<string,list<string> >::iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{//��source port ѭ��
			string pt_src = j->first; 
			for(list<string>::iterator k = j->second.begin();
				k != j->second.end();
				++k)
			{//��dest ip ѭ��
				//Ŀ��ipΪrouter
				string ip_dst = *k;
				string ip_dst_frm = ip_dst;

				string ptsrc_to_dst = "0";
				string ptdst_to_src = "0";

				vector<string> ip_dst_list;
				for(DEVID_LIST::iterator m = device_list.begin();
					m != device_list.end();
					++m)
				{
					if(find(m->second.ips.begin(), m->second.ips.end(), ip_dst) != m->second.ips.end())
					{
						ip_dst_list = m->second.ips;
						ip_dst_frm = m->first;
						break;
					}
				}
				if(ip_dst_list.empty())
				{
					continue;
				}
				if((device_list[ip_dst_frm].devType != "0" && device_list[ip_dst_frm].devType != "2" ))
				{
					continue;
				}

				string ip_dst_volid = "";//��ЧĿ�Ķ˿�
				for(vector<string>::iterator vi = ip_dst_list.begin();
					vi != ip_dst_list.end();
					++vi)
				{
					if(nbr_list.find(*vi) != nbr_list.end())
					{
						ip_dst_volid = *vi;
						break;
					}
				}
				if(ip_dst_volid.empty())
				{
					continue;
				}

				//dest ip������nbr_list�����б���
				bool bOk = false;
                                for(map<string,list<string> >::iterator j_dst = nbr_list[ip_dst_volid].begin();
					j_dst != nbr_list[ip_dst_volid].end();
					++j_dst)
				{//Ѱ�ҶԶ��豸�а���source ip�Ķ˿�						
					for(list<string>::iterator m = j_dst->second.begin();
						m != j_dst->second.end();
						++m)
					{
						vector<string>::iterator n1,n2;
						for(n1 = ip_src_list.begin(), n2 = device_list[ip_src_frm].infinxs.begin();
							n1 != ip_src_list.end();
							++n1, ++n2)
						{
							if(*n1 == *m)
							{
								ptsrc_to_dst = *n2;
								bOk = true;
								break;
							}
						}
						if(bOk)	break;
					}
					if(bOk)	break;
				}
				bOk = false;
                                for(map<string,list<string> >::iterator j_src = nbr_list[ip_src].begin();
					j_src != nbr_list[ip_src].end();
					++j_src)
				{//Ѱ�ҶԶ��豸�а���source ip�Ķ˿�						
					for(list<string>::iterator m = j_src->second.begin();
						m != j_src->second.end();
						++m)
					{
						vector<string>::iterator n1,n2;
						for(n1 = ip_dst_list.begin(), n2 = device_list[ip_dst_frm].infinxs.begin();
							n1 != ip_dst_list.end();
							++n1, ++n2)
						{
							if(*n1 == *m)
							{
								ptdst_to_src = *n2;
								bOk = true;
								break;
							}
						}
						if(bOk) break;
					}
					if(bOk) break;
				}

				EDGE edge_tmp;
				edge_tmp.ip_left  = ip_src_frm;
				edge_tmp.pt_left  = ptsrc_to_dst;
				edge_tmp.inf_left = ptsrc_to_dst;
				edge_tmp.ip_right  = ip_dst_frm;
				edge_tmp.pt_right  = ptdst_to_src;
				edge_tmp.inf_right = ptdst_to_src;
				edge_list_rr.push_back(edge_tmp);
			}
		}
	}

	for(EDGE_LIST::iterator i = edge_list_rr.begin();
		i != edge_list_rr.end();
		++i)
	{
		if( i->ip_left == i->ip_right || i->inf_left == "0" || i->inf_right == "0" )
		{
			continue;
		}
		bool bNew = true;
		for(EDGE_LIST::iterator j = topo_edge_list.begin();
			j != topo_edge_list.end();
			++j)
		{
			if ((j->ip_left == i->ip_left && j->ip_right == i->ip_right) 
				|| (j->ip_left == i->ip_right && j->ip_right == i->ip_left)
				|| (j->ip_left == i->ip_left && j->pt_left == i->pt_left)
				|| (j->ip_right == i->ip_right && j->pt_right == i->pt_right)
				|| (j->ip_left == i->ip_right && j->pt_left == i->pt_right)  //added by zhangyan 2008-12-05
				|| (j->ip_right == i->ip_left && j->pt_right == i->pt_left)
				)			
			{
				bNew = false;
				break;
			}
		}
		if(bNew)
		{
			cout<<"AnalyseRRByNbr:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
			topo_edge_list.push_back(*i);
		}
	}
	return true;
}

// ����·����֮������ӹ�ϵ
bool topoAnalyse::AnalyseRRByRt(void)
{
	//�淶route table
	ROUTE_LIST_FRM rttbl_list_frm;
	for(ROUTE_LIST::iterator i = rttbl_list.begin();
		i != rttbl_list.end();
		++i)
	{
		string ip_src = i->first;
		for(DEVID_LIST::iterator j = device_list.begin();
			j != device_list.end();
			++j)
		{
			if(find(j->second.ips.begin(), j->second.ips.end(), ip_src) != j->second.ips.end())
			{
				if(j->second.devType == "0"  || j->second.devType == "2")
				{
					ip_src = j->first;
					if(rttbl_list_frm.find(ip_src) == rttbl_list_frm.end())
					{
                                                map<string,list<string> > ml;
                                                for(map<string,list<ROUTEITEM> >::iterator k = i->second.begin();
							k != i->second.end();
							++k)
						{
							list<string> ipdst_list;
							for(list<ROUTEITEM>::iterator dst = k->second.begin();
								dst != k->second.end();
								++dst)
							{
								for(DEVID_LIST::iterator id_dst = device_list.begin();
									id_dst != device_list.end();
									++id_dst)
								{
									if(find(id_dst->second.ips.begin(), id_dst->second.ips.end(), dst->next_hop) != id_dst->second.ips.end())
									{
										if(id_dst->second.devType == "0" || id_dst->second.devType == "2")
										{
											if(find(ipdst_list.begin(), ipdst_list.end(), id_dst->first) == ipdst_list.end())
											{
												ipdst_list.push_back(id_dst->first);
											}
										}
									}
								}
							}
							if(!ipdst_list.empty())
							{
								ml.insert(make_pair(k->first, ipdst_list));
							}
						}
						if(!ml.empty())
						{
							rttbl_list_frm.insert(make_pair(ip_src, ml));
						}
					}
				}
				break;
			}
		}
	}

	EDGE_LIST edge_list_rr;
	for(ROUTE_LIST_FRM::iterator i = rttbl_list_frm.begin();
		i != rttbl_list_frm.end();
		++i)
	{//��source ip ѭ��
		string ip_src = i->first;
                for(std::map<string,list<string> >::iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{//��source port ѭ��
			string pt_src = j->first; 
			for(list<string>::iterator k = j->second.begin();
				k != j->second.end();
				++k)
			{//��dest ip ѭ��
				string ip_dst = *k;
				string pt_dst = "0";
				if(rttbl_list_frm.find(ip_dst) != rttbl_list_frm.end())
				{//dest ip������rttbl_list�����б���
                                        for(std::map<string,list<string> >::iterator j_dst = rttbl_list_frm[ip_dst].begin();
						j_dst != rttbl_list_frm[ip_dst].end();
						++j_dst)
					{//Ѱ�ҶԶ��豸�а���source ip�Ķ˿�
						list<string>::iterator ip_tmp = find(j_dst->second.begin(), j_dst->second.end(), ip_src);
						if( ip_tmp != j_dst->second.end())
						{//���ֶԶ�Router
							pt_dst = j_dst->first;
							//added by zhangyan 2009-01-16
							EDGE edge_tmp;
							edge_tmp.ip_left  = ip_src;
							edge_tmp.pt_left  = pt_src;
							edge_tmp.inf_left = pt_src;
							edge_tmp.ip_right  = ip_dst;
							edge_tmp.pt_right  = pt_dst;
							edge_tmp.inf_right = pt_dst;
							edge_list_rr.push_back(edge_tmp);
							break;
						}
					}
				}
				//remarkded by zhangyan 2009-01-16
				//�����Ҳ����Զ�port�ı�
				/*EDGE edge_tmp;
				edge_tmp.ip_left  = ip_src;
				edge_tmp.pt_left  = pt_src;
				edge_tmp.inf_left = pt_src;
				edge_tmp.ip_right  = ip_dst;
				edge_tmp.pt_right  = pt_dst;
				edge_tmp.inf_right = pt_dst;
				edge_list_rr.push_back(edge_tmp);*/
			}
		}
	}

	for(EDGE_LIST::iterator i = edge_list_rr.begin();
		i != edge_list_rr.end();
		++i)
	{
		if( i->ip_left == i->ip_right || (i->inf_left == "0" && i->inf_right == "0") )
		{
			continue;
		}

		// added by zhangyan 2008-12-04
		if (i->inf_left == "0" || i->inf_right == "0")
		{
			//����ͨ���������Ƿ��������ӹ�ϵ
			bool bNew = true;
			TraceAnalyse::getConnection(topo_edge_list);
                        for(std::list<std::list<std::string> >::iterator j = TraceAnalyse::set_conn.begin();	j != TraceAnalyse::set_conn.end(); ++j)
			{
				if ((find(j->begin(), j->end(), i->ip_left) != j->end()) && (find(j->begin(), j->end(), i->ip_right) != j->end()))
				{
					//����ͬһ��ͨ��
					bNew = false;
					break;
				}
			}
			if(bNew)
			{
				cout<<"AnalyseRRByRt:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
				topo_edge_list.push_back(*i);
			}
		}
		else
		{
			bool bNew = true;
			for(EDGE_LIST::iterator j = topo_edge_list.begin();
				j != topo_edge_list.end();
				++j)
			{
				if ((j->ip_left == i->ip_left && j->ip_right == i->ip_right) 
					|| (j->ip_left == i->ip_right && j->ip_right == i->ip_left)
					|| (j->ip_left == i->ip_left && j->pt_left == i->pt_left)
					|| (j->ip_right == i->ip_right && j->pt_right == i->pt_right)
					|| (j->ip_left == i->ip_right && j->pt_left == i->pt_right)  //added by zhangyan 2008-12-05
					|| (j->ip_right == i->ip_left && j->pt_right == i->pt_left)
					)			
				{
					bNew = false;
					break;
				}
			}
			if(bNew)
			{
				cout<<"AnalyseRRByRt:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
				topo_edge_list.push_back(*i);
			}
		}
	}
	return true;
}

bool topoAnalyse::AnalyseRRByBgp(void)
{
	EDGE_LIST edge_list_rr;
	for(BGP_LIST::iterator i = bgp_list.begin(); i != bgp_list.end(); ++i)
	{
		string ip_src = i->local_ip;
		string ip_dst = i->peer_ip;
		for(DEVID_LIST::iterator j = device_list.begin(); j != device_list.end(); ++j)
		{
			if(find(j->second.ips.begin(), j->second.ips.end(), ip_src) != j->second.ips.end())
			{
				ip_src = j->first;
				break;
			}
		}
		for(DEVID_LIST::iterator j = device_list.begin(); j != device_list.end(); ++j)
		{
			if(find(j->second.ips.begin(), j->second.ips.end(), ip_dst) != j->second.ips.end())
			{
				ip_dst = j->first;
				break;
			}
		}
		EDGE edge_tmp;
		edge_tmp.ip_left  = ip_src;
		edge_tmp.pt_left  = i->local_port;
		edge_tmp.inf_left = i->local_port;
		edge_tmp.ip_right  = ip_dst;
		edge_tmp.pt_right  = i->peer_port;
		edge_tmp.inf_right = i->peer_port;
		edge_list_rr.push_back(edge_tmp);
	}

	for(EDGE_LIST::iterator i = edge_list_rr.begin(); i != edge_list_rr.end();	++i)
	{
		if( i->ip_left == i->ip_right || (i->inf_left == "0" && i->inf_right == "0") )
		{
			continue;
		}
		bool bNew = true;
		for(EDGE_LIST::iterator j = topo_edge_list.begin();	j != topo_edge_list.end();	++j)
		{
			if ((j->ip_left == i->ip_left && j->ip_right == i->ip_right) 
				|| (j->ip_left == i->ip_right && j->ip_right == i->ip_left)
				|| (j->ip_left == i->ip_left && j->pt_left == i->pt_left)
				|| (j->ip_right == i->ip_right && j->pt_right == i->pt_right)
				|| (j->ip_left == i->ip_right && j->pt_left == i->pt_right) // added by zhangyan 2008-12-04
				|| (j->ip_right == i->ip_left && j->pt_right == i->pt_left)
			    )			
			{
				bNew = false;
				break;
			}
		}
		if(bNew)
		{
			cout<<"AnalyseRRByBgp:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
			topo_edge_list.push_back(*i);
		}
	}
	return true;
}

bool topoAnalyse::AnalyseRRBySubnet(void)
{	
	TraceAnalyse::getConnection(topo_edge_list);
	//���ж�rt_list�е�ip�Ƿ����
	//if (TraceAnalyse::getConnection(topo_edge_list) > 1)
	{	
		EDGE_LIST edge_list_rr;

                for(map<string, list<pair<string, string> > >::iterator iter = IPADDR_map.begin(); iter != IPADDR_map.end(); ++iter)
		{
			string routeip = iter->first;			
			bool bConn_i = false;			
                        list<list<string> >::iterator i_left, i_right;
                        for(list<list<string> >::iterator conn_iter = TraceAnalyse::set_conn.begin(); conn_iter != TraceAnalyse::set_conn.end(); ++conn_iter)
			{
				if(find(conn_iter->begin(), conn_iter->end(), routeip) != conn_iter->end())
				{
					i_left = conn_iter;
					bConn_i = true;	
					break;
				}
			}
						
                        map<string, list<pair<string, string> > >::iterator iter_tmp = iter;
                        for(map<string, list<pair<string, string> > >::iterator ii = ++iter_tmp; ii != IPADDR_map.end(); ++ii)
			{
				bool bConn_j = false;
				bool bSame = false;
				string routeip_tmp = ii->first;
                                for(list<list<string> >::iterator conn_iter = TraceAnalyse::set_conn.begin(); conn_iter != TraceAnalyse::set_conn.end(); ++conn_iter)
				{
					if(find(conn_iter->begin(), conn_iter->end(), routeip_tmp) != conn_iter->end())
					{
						i_right = conn_iter;
						bConn_j = true;						
						break;
					}
				}
				
				if (bConn_i && bConn_j)
				{					
					if(i_left != i_right) //������ͬһ����ͨ��
					{						
						//�ж��Ƿ�����ͬ��subnet						
                                                for(list<pair<string, string> >::iterator subnet_iterI = iter->second.begin(); subnet_iterI != iter->second.end(); ++subnet_iterI)
						{
							string subnet = subnet_iterI->first;								
                                                        for(list<pair<string, string> >::iterator subnet_iterJ = ii->second.begin(); subnet_iterJ != ii->second.end(); ++subnet_iterJ)
							{
								if (subnet == subnet_iterJ->first)
								{//�ҵ���ͬ��subnet, ���һ���ߵ�edge_list_rr
									bSame = true;
									EDGE edge_tmp;
									edge_tmp.ip_left  = routeip;
									edge_tmp.pt_left  = subnet_iterI->second;
									edge_tmp.inf_left = subnet_iterI->second;
									edge_tmp.ip_right  = routeip_tmp;
									edge_tmp.pt_right  = subnet_iterJ->second;
									edge_tmp.inf_right = subnet_iterJ->second;
									edge_list_rr.push_back(edge_tmp);

									//�ϲ�����ͨ��
									for(list<std::string>::iterator j = i_right->begin(); j != i_right->end(); ++j)
									{
										//if(find(i_left->begin(), i_left->end(), *j) == i_left->end())
										{
											i_left->push_back(*j);
										}
									}
									TraceAnalyse::set_conn.remove(*i_right);
									break;
								}
							}
							if(bSame)
							{
								break;
							}
						}//end for �ж��Ƿ�����ͬ��subnet					
					}					
				
				}
				else if(bConn_i && !bConn_j)
				{
					//�ж��Ƿ�����ͬ��subnet					
                                        for(list<pair<string, string> >::iterator subnet_iterI = iter->second.begin(); subnet_iterI != iter->second.end(); ++subnet_iterI)
					{
						string subnet = subnet_iterI->first;								
                                                for(list<pair<string, string> >::iterator subnet_iterJ = ii->second.begin(); subnet_iterJ != ii->second.end(); ++subnet_iterJ)
						{
							if (subnet == subnet_iterJ->first)
							{//�ҵ���ͬ��subnet, ���һ���ߵ�edge_list_rr
								bSame = true;
								EDGE edge_tmp;
								edge_tmp.ip_left  = routeip;
								edge_tmp.pt_left  = subnet_iterI->second;
								edge_tmp.inf_left = subnet_iterI->second;
								edge_tmp.ip_right  = routeip_tmp;
								edge_tmp.pt_right  = subnet_iterJ->second;
								edge_tmp.inf_right = subnet_iterJ->second;
								edge_list_rr.push_back(edge_tmp);

								//���ù���·������ӵ�����ͨ��
								i_left->push_back(routeip_tmp);								
								break;
							}
						}
						if(bSame)
						{
							break;
						}
					}					
				}
				else if(!bConn_i && bConn_j)
				{
					//�ж��Ƿ�����ͬ��subnet					
                                        for(list<pair<string, string> >::iterator subnet_iterI = iter->second.begin(); subnet_iterI != iter->second.end(); ++subnet_iterI)
					{
						string subnet = subnet_iterI->first;								
                                                for(list<pair<string, string> >::iterator subnet_iterJ = ii->second.begin(); subnet_iterJ != ii->second.end(); ++subnet_iterJ)
						{
							if (subnet == subnet_iterJ->first)
							{//�ҵ���ͬ��subnet, ���һ���ߵ�edge_list_rr
								bSame = true;
								EDGE edge_tmp;
								edge_tmp.ip_left  = routeip;
								edge_tmp.pt_left  = subnet_iterI->second;
								edge_tmp.inf_left = subnet_iterI->second;
								edge_tmp.ip_right  = routeip_tmp;
								edge_tmp.pt_right  = subnet_iterJ->second;
								edge_tmp.inf_right = subnet_iterJ->second;
								edge_list_rr.push_back(edge_tmp);

								//���ù���·������ӵ�����ͨ��
								i_right->push_back(routeip);								
								break;
							}
						}
						if(bSame)
						{
							break;
						}
					}					
				}
				else //if(!bConn_i && !bConn_j)
				{
					//�ж��Ƿ�����ͬ��subnet					
                                        for(list<pair<string, string> >::iterator subnet_iterI = iter->second.begin(); subnet_iterI != iter->second.end(); ++subnet_iterI)
					{
						string subnet = subnet_iterI->first;								
                                                for(list<pair<string, string> >::iterator subnet_iterJ = ii->second.begin(); subnet_iterJ != ii->second.end(); ++subnet_iterJ)
						{
							if (subnet == subnet_iterJ->first)
							{//�ҵ���ͬ��subnet, ���һ���ߵ�edge_list_rr
								bSame = true;
								EDGE edge_tmp;
								edge_tmp.ip_left  = routeip;
								edge_tmp.pt_left  = subnet_iterI->second;
								edge_tmp.inf_left = subnet_iterI->second;
								edge_tmp.ip_right  = routeip_tmp;
								edge_tmp.pt_right  = subnet_iterJ->second;
								edge_tmp.inf_right = subnet_iterJ->second;
								edge_list_rr.push_back(edge_tmp);

								//���ù���·������ӵ��µ�����ͨ��
								list<string> subconn;
								subconn.push_back(routeip);
								subconn.push_back(routeip_tmp);
								TraceAnalyse::set_conn.push_back(subconn);
								break;
							}
						}
						if(bSame)
						{
							break;
						}
					}				
				}
			} //end for IPADDR_map	2		
			
		}//end for IPADDR_map 1

		//������һ��·�����˿ڱ����·����ռ�õı�
		EDGE_LIST edge_list_rr_tmp;
		EDGE_LIST edge_list_rr_del;
		for(EDGE_LIST::iterator i = edge_list_rr.begin(); i != edge_list_rr.end();	++i)
		{
			bool bNew = true;
			for(EDGE_LIST::iterator j = edge_list_rr_tmp.begin(); j != edge_list_rr_tmp.end();	++j)
			{
				if ((j->ip_left == i->ip_left && j->pt_left == i->pt_left)
					|| (j->ip_right == i->ip_right && j->pt_right == i->pt_right)
					|| (j->ip_left == i->ip_right && j->pt_left == i->pt_right) // added by zhangyan 2008-12-04
					|| (j->ip_right == i->ip_left && j->pt_right == i->pt_left)
					)			
				{
					bNew = false;
					cout<<"AnalyseRRBySubnet(filter):"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
					edge_list_rr_del.push_back(*j);//����ɾ���߷���edge_list_rr_del
					break;
				}
			}
			if(bNew)
			{				
				edge_list_rr_tmp.push_back(*i);
			}
		}
		
		for(EDGE_LIST::iterator i = edge_list_rr_del.begin(); i != edge_list_rr_del.end();	++i)
		{			
			for(EDGE_LIST::iterator j = edge_list_rr_tmp.begin(); j != edge_list_rr_tmp.end();	++j)
			{
				if ((j->ip_left == i->ip_left && j->pt_left == i->pt_left) 
					//&& (j->ip_right == i->ip_right && j->pt_right == i->pt_right)
					)			
				{					
					cout<<"AnalyseRRBySubnet(delete):"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
					edge_list_rr_tmp.erase(j);
					break;
				}
			}
		}

		for(EDGE_LIST::iterator i = edge_list_rr_tmp.begin(); i != edge_list_rr_tmp.end();	++i)
		{
			/*if( i->ip_left == i->ip_right || (i->inf_left == "0" && i->inf_right == "0") )
			{
				continue;
			}*/
			bool bNew = true;
			for(EDGE_LIST::iterator j = topo_edge_list.begin();	j != topo_edge_list.end();	++j)
			{
				if ((j->ip_left == i->ip_left && j->ip_right == i->ip_right) 
					|| (j->ip_left == i->ip_right && j->ip_right == i->ip_left)
					|| (j->ip_left == i->ip_left && j->pt_left == i->pt_left)
					|| (j->ip_right == i->ip_right && j->pt_right == i->pt_right)
					|| (j->ip_left == i->ip_right && j->pt_left == i->pt_right) // added by zhangyan 2008-12-04
					|| (j->ip_right == i->ip_left && j->pt_right == i->pt_left)
					)			
				{
					bNew = false;
					cout<<"AnalyseRRBySubnet(filterbytopo):"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
					break;
				}
			}
			if(bNew)
			{
				cout<<"AnalyseRRBySubnet:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
				topo_edge_list.push_back(*i);
			}
		}
	}//end if
	
	return true;

}

bool topoAnalyse::isSameFactory(const DEVID_LIST& device_list)
{
	string devFacOid = "";
	for(DEVID_LIST::const_iterator i = device_list.begin(); i != device_list.end(); ++i)
	{
		if(i->second.devType == SWITCH || i->second.devType == ROUTE_SWITCH || i->second.devType == ROUTER)
		{
			if(devFacOid == "")
			{
				devFacOid = ::getFacOid(i->second.sysOid);
			}
			else if(devFacOid != ::getFacOid(i->second.sysOid))
			{
				return false;
			}
		}
	}
	return true;
}

bool topoAnalyse::EdgeAnalyse(void)
{
	if(!topo_edge_list.empty())
	{
		topo_edge_list.clear();
	}
	/*if(isSameFactory(device_list))
	{
		return EdgeAnalyseDirect();
	}*/    //by zhangyan 2008-10-29
	AnalyseRS();
	AnalyseSH();
	AnalyseSS();
	
	/*for(EDGE_LIST::iterator i = edge_list_rs_byaft.begin(); i != edge_list_rs_byaft.end(); ++i)
	{
		bool bNew = true;
		for(EDGE_LIST::iterator j = topo_edge_list.begin();
			j != topo_edge_list.end();
			++j)
		{
			if ((j->ip_left == i->ip_left && j->pt_left == i->pt_left)
				|| (j->ip_right == i->ip_right && j->pt_right == i->pt_right)
			    || (j->ip_left == i->ip_right && j->pt_left == i->pt_right)
				|| (j->ip_right == i->ip_left && j->pt_right == i->pt_left)
			    )
			{
				bNew = false;
				break;
			}
		}
		if(bNew)
		{
			cout<<"AnalyseRS:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
			topo_edge_list.push_back(*i);
		}
	}*/

	/*AnalyseRRByArp();
	AnalyseRRByNbr();
	AnalyseRRByRt();
	AnalyseRRByBgp();*/   //by zhangyan 2008-10-21
	
	//added by zhangyan 2009-01-15
	//�Դ�·�ɵĴ��� (������route���)
	AnalyseRRBySubnet();

	AnalyseRRByRt();  //changed by zhang 2009-03-26 ȥ��·�ɱ��ȡ��
						//changed again by wings 2009-11-13 �ָ�·�ɱ�
	AnalyseRRByNbr();
	AnalyseRRByBgp();
	AnalyseRRByArp();
	
	//begin added by tgf 2008-09-23
	EDGE_LIST edge_list_tmp = getDirectEdge(); 
	for(EDGE_LIST::iterator i = edge_list_tmp.begin(); i != edge_list_tmp.end(); ++i)
	{
		bool bNew = true;
		for(EDGE_LIST::iterator j = topo_edge_list.begin();
			j != topo_edge_list.end();
			++j)
		{
			if ((j->ip_left == i->ip_left && j->pt_left == i->pt_left)
				|| (j->ip_right == i->ip_right && j->pt_right == i->pt_right)
			    || (j->ip_left == i->ip_right && j->pt_left == i->pt_right)
				|| (j->ip_right == i->ip_left && j->pt_right == i->pt_left)
			    )
			{
				bNew = false;
				break;
			}
		}
		if(bNew)
		{
			cout<<"EdgeAnalyseDirect:"<<i->ip_left<<" "<<i->pt_left<<" "<<i->ip_right<<" "<<i->pt_right<<endl;
			topo_edge_list.push_back(*i);
		}
	}
	//end added by tgf 2008-09-23

        SvLog::writeLog("m_param.comp_type:"+m_param.comp_type);
	if(m_param.comp_type != "00")
	{
		CompAftWithArp(m_param.comp_type);
//		AnalyseRS_COMP();
		AnalyseSH_COMP();
		AnalyseSS_COMP();
	}

	return true;
}

bool topoAnalyse::EdgeAnalyseDirect(void)
{
	topo_edge_list = getDirectEdge();
	rs_list.insert(rs_list.end(), sw_list.begin(), sw_list.end());
	EDGE_LIST el_tmp = getDirectEdgeByAft();
	topo_edge_list.insert(topo_edge_list.end(), el_tmp.begin(), el_tmp.end());
	el_tmp = getDirectEdgeByArp();
	topo_edge_list.insert(topo_edge_list.end(), el_tmp.begin(), el_tmp.end());
	return true;
}

// ��ȡtopo���б�
EDGE_LIST& topoAnalyse::getEdgeList(void)
{
	return topo_edge_list;
}

void topoAnalyse::printdata(const FRM_AFTARP_LIST& data_list)
{
	for(FRM_AFTARP_LIST::const_iterator i = data_list.begin(); i != data_list.end(); ++i)
	{
                for(std::map<string,list<string> >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if(!j->second.empty())
			{
				cout << i->first << "-" << j->first << ": ";
				for(list<string>::const_iterator k = j->second.begin();
					k != j->second.end();
					++k)
				{
					cout << *k << ",";
				}
				cout << endl;
			}
		}
	}
}

void topoAnalyse::CompAftWithArp(const string& stype)
{
	aft_list.clear();
	//aft_list.insert(arp_list_bak.begin(), arp_list_bak.end());
	aft_list.insert(arp_list.begin(), arp_list.end());// by zhangyan 2008-10-28	
	return;
}

//��������ͷ���
EDGE_LIST topoAnalyse::getDirectEdge()
{
	EDGE_LIST edge_list_cur;
	for(DIRECTDATA_LIST::iterator i = direct_list.begin(); i != direct_list.end(); ++i)
	{
		string left_ip = i->first;
		for(list<DIRECTITEM>::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			string right_ip = j->PeerIP;
			if(left_ip == right_ip)	continue;

			EDGE edge;
			edge.ip_left = left_ip;
			edge.inf_left = j->localPortInx;
			edge.pt_left = j->localPortInx; // added by zhangyan 2008-11-20
			edge.dsc_left = "";
			edge.ip_right = right_ip;
			edge.inf_right = "0";
			edge.pt_right = "0";
			edge.dsc_right = j->PeerPortDsc;

			IFPROP_LIST::iterator ifprop_left  = ifprop_list.find(left_ip);
			if(ifprop_left != ifprop_list.end())
			{//��߽ӿ�����
				for(list<IFREC>::iterator k = ifprop_left->second.second.begin();
					k != ifprop_left->second.second.end();
					++k)
				{
					if(k->ifIndex == j->localPortInx)
					{
						edge.dsc_left = k->ifDesc; //added by zhangyan 2008-11-20
						//string desc_tmp = k->ifDesc;
						//vector<string> descs = tokenize(desc_tmp, ".");
						//if(descs.size() > 0)
						//{
						//	desc_tmp = descs[0];
						//}
						//edge.dsc_left = desc_tmp;
						//for(list<IFREC>::iterator m = ifprop_left->second.second.begin();
						//	m != ifprop_left->second.second.end();
						//	++m)
						//{
						//	if(m->ifDesc == desc_tmp)
						//	{
						//		//edge.pt_left  = m->ifPort;
						//		edge.pt_left  = m->ifIndex;
						//		edge.inf_left = m->ifIndex;
						//		break;
						//	}
						//}
						break;
					}
				}
			}

			IFPROP_LIST::iterator ifprop_right = ifprop_list.find(right_ip);
			if(ifprop_right != ifprop_list.end())
			{//�ұ߽ӿ�����
				string desc_tmp = j->PeerPortDsc;
				//remarked by zhangyan 2008-11-20
				//vector<string> descs = tokenize(desc_tmp, ".");
				//if(descs.size() > 0)
				//{
				//	desc_tmp = descs[0];
				//}
				//edge.dsc_right = desc_tmp;
				for(list<IFREC>::iterator k = ifprop_right->second.second.begin();
					k != ifprop_right->second.second.end();
					++k)
				{
					if(k->ifDesc == desc_tmp)
					{
						//edge.pt_right = k->ifPort;
						edge.pt_right = k->ifIndex;
						edge.inf_right = k->ifIndex;
						break;
					}
				}
			}

			bool bNew = true;
			for(EDGE_LIST::iterator e = edge_list_cur.begin();
				e != edge_list_cur.end();
				++e)
			{
				if(e->ip_left == e->ip_right)
				{
					continue;
				}

				if(    (edge.ip_left == e->ip_left && edge.inf_left == e->inf_left) 
					|| (edge.ip_right == e->ip_right && edge.inf_right == e->inf_right)
					|| (edge.ip_left == e->ip_right && edge.inf_left == e->inf_right)
					|| (edge.ip_right == e->ip_left && edge.inf_right == e->inf_left)
					)
				{
					bNew = false;
					break;
				}
			}
			if(bNew)
			{
				///test by zhangyan 
				/*if (edge.ip_left == "10.48.0.11" || edge.ip_right == "10.48.0.11")
					cout<<"   edge:"<<edge.ip_left<<":"<<edge.pt_left<<" "<<edge.ip_right<<":"<<edge.pt_right<<endl;*/
				edge_list_cur.push_back(edge);
			}
		}
	}
	return edge_list_cur;
}

// ����������������֮������ӹ�ϵ
EDGE_LIST topoAnalyse::getDirectEdgeByAft(void)
{
	list<string> host_list_exist;
	/*for(EDGE_LIST::iterator m = topo_edge_list.begin();	m != topo_edge_list.end(); ++m)
	{
		if(find(host_list_exist.begin(), host_list_exist.end(), m->ip_right) == host_list_exist.end())
		{
			host_list_exist.push_back(m->ip_right);
		}
	}*/	//���ڴ�topo�ߵ�ip_right������Ϊ��������û��Ҫ��ӵ�host_list_exist
	EDGE_LIST edge_list_cur;
	bool bGoOn = true;
	while(bGoOn)
	{
		bGoOn = false;
		list<string> iplist_del;//�������ip
		for(FRM_AFTARP_LIST::iterator i = aft_list.begin();	i != aft_list.end(); ++i)
		{//��source ip ѭ��
			string ip_src = i->first;
                        for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); ++j)
			{//��source port ѭ��
				if(IsLeafPort(j->second, rs_list))
				{//��Ҷ�Ӷ˿�,��ȫ��Ϊ�����豸
					string pt_src = j->first; 
					for(list<string>::iterator k = j->second.begin(); k != j->second.end();	++k)
					{//��dest ip ѭ��,Ϊÿ��Ŀ��ip���һ����
						//if(find(rs_list.begin(), rs_list.end(), *k) != rs_list.end())
						//{//����Ϊ�豸��dest_ip
						//	continue;
						//}
						bGoOn = true;
						//��֤ÿ��Hostֻ����һ��
						if(find(host_list_exist.begin(), host_list_exist.end(), *k) == host_list_exist.end())
						{
							host_list_exist.push_back(*k);
							EDGE edge_tmp;
							edge_tmp.ip_left  = ip_src;
							edge_tmp.pt_left  = pt_src;
							edge_tmp.inf_left = pt_src;
							edge_tmp.ip_right  = *k;
							edge_tmp.pt_right  = "PX";
							edge_tmp.inf_right = "0";
							edge_list_cur.push_back(edge_tmp);
						}
						//����dest ip ��ӵ��Ѿ������ip��ַ����
						if(find(iplist_del.begin(), iplist_del.end(), *k) == iplist_del.end())
						{
							iplist_del.push_back(*k);
						}
					}//��dest ip ѭ��
				}//��Ҷ�Ӷ˿�
			}
		}
		//ɾ��������dest ip
		for(list<string>::iterator ip_del = iplist_del.begin();
			ip_del != iplist_del.end();
			++ip_del)
		{
			for(FRM_AFTARP_LIST::iterator i = aft_list.begin();
				i != aft_list.end();
				++i)
			{//��source ip ѭ��
				string ip_src = i->first;
                                for(std::map<string,list<string> >::iterator j = i->second.begin();
					j != i->second.end();
					++j)
				{//��source port ѭ��
					j->second.remove(*ip_del);
				}
			}
		}
	}
	return edge_list_cur;
}

// ����������������֮������ӹ�ϵ
EDGE_LIST topoAnalyse::getDirectEdgeByArp(void)
{
	list<string> host_list_exist;
	for(EDGE_LIST::iterator m = topo_edge_list.begin();	m != topo_edge_list.end(); ++m)
	{
		if(find(host_list_exist.begin(), host_list_exist.end(), m->ip_right) == host_list_exist.end())
		{
			host_list_exist.push_back(m->ip_right);
		}
	}
	EDGE_LIST edge_list_cur;

	bool bGoOn = true;
	while(bGoOn)
	{
		bGoOn = false;
		list<string> iplist_del;//�������ip
		for(FRM_AFTARP_LIST::iterator i = arp_list.begin();	i != arp_list.end(); ++i)
		{//��source ip ѭ��
			if(find(rs_list.begin(), rs_list.end(), i->first) == rs_list.end()) continue;//���Է������豸
			string ip_src = i->first;
                        for(std::map<string,list<string> >::iterator j = i->second.begin(); j != i->second.end(); ++j)
			{//��source port ѭ��
				if(IsLeafPort(j->second, rs_list))
				{//��Ҷ�Ӷ˿�
					string pt_src = j->first; 
					for(list<string>::iterator k = j->second.begin(); k != j->second.end();	++k)
					{//��dest ip ѭ��,Ϊÿ��Ŀ��ip���һ����
						if(*k == ip_src) continue;
						//if(find(rs_list.begin(), rs_list.end(), *k) != rs_list.end())
						//{//����Ϊ�豸��dest_ip
						//	continue;
						//}
						bGoOn = true;
						//��֤ÿ��Hostֻ����һ��
						if(find(host_list_exist.begin(), host_list_exist.end(), *k) == host_list_exist.end())
						{
							host_list_exist.push_back(*k);
							EDGE edge_tmp;
							edge_tmp.ip_left  = ip_src;
							edge_tmp.pt_left  = pt_src;
							edge_tmp.inf_left = pt_src;
							edge_tmp.ip_right  = *k;
							edge_tmp.pt_right  = "PX";
							edge_tmp.inf_right = "0";
							edge_list_cur.push_back(edge_tmp);
						}
						//����dest ip ��ӵ��Ѿ������ip��ַ����
						if(find(iplist_del.begin(), iplist_del.end(), *k) == iplist_del.end())
						{
							iplist_del.push_back(*k);
						}
					}//��dest ip ѭ��
				}//��Ҷ�Ӷ˿�
			}
		}
		//ɾ��������dest ip
		for(list<string>::iterator ip_del = iplist_del.begin();
			ip_del != iplist_del.end();
			++ip_del)
		{
			for(FRM_AFTARP_LIST::iterator i = arp_list.begin();
				i != arp_list.end();
				++i)
			{//��source ip ѭ��
				string ip_src = i->first;
                                for(std::map<string,list<string> >::iterator j = i->second.begin();
					j != i->second.end();
					++j)
				{//��source port ѭ��
					j->second.remove(*ip_del);
				}
			}
		}
	}
	return edge_list_cur;
}
