#include "StdAfx.h"
#include "TraceAnalyse.h"
#include "TraceReader.h"
#include "StreamData.h"
#include "svLog.h"
#include <algorithm>

list<list<string> > TraceAnalyse::set_conn;

TraceAnalyse::TraceAnalyse(const DEVID_LIST& devlist, const ROUTEPATH_LIST& routepath_list, 
                                                   const list<pair<pair<string, string>, string> >& RouteDESTIPPairList,
						   const list<string>& unManagedDevices, const SCAN_PARAM& param)
{
	// add by zhangyan 2008-09-5	
	//Hops = tHops;
	scanParam.timeout = param.timeout;
	scanParam.retrytimes = param.retrytimes;

	device_list = devlist;
	rtpath_list = routepath_list;	
	RouteIPPairList = RouteDESTIPPairList;
	unManagedIPList = unManagedDevices;
}

TraceAnalyse::TraceAnalyse(const DEVID_LIST& devlist, const ROUTEPATH_LIST& routepath_list, 
                                                   const list<pair<pair<string, string>, string> >& RouteDESTIPPairList,
						   const list<string>& unManagedDevices)
{	
	device_list = devlist;
	rtpath_list = routepath_list;	
	RouteIPPairList = RouteDESTIPPairList;
	unManagedIPList = unManagedDevices;
}

//bool TraceAnalyse::AnalyseTraceEdge(EDGE_LIST& topo_edge_list, string Type)
//{
//	getConnection(topo_edge_list);
//	if (set_conn.size() > 1)
//	{//利用trace补充topo边
//		if (Type == "1")
//		{//从文件扫描时
//			AnalyseRRByRtPath_Direct(topo_edge_list);
//		}
//		else
//		{
//			AnalyseRRByRtPath(topo_edge_list);
//		}		
//	}
//	return true;
//}

int TraceAnalyse::getConnection(const EDGE_LIST& topo_edge_list)
{
	//填充连通集set_conn
	//begin	
	set_conn.clear();
	for(EDGE_LIST::const_iterator i = topo_edge_list.begin();
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
		{//新连通集
			list<std::string> list_new;
			list_new.push_back(i->ip_left);
			list_new.push_back(i->ip_right);
			for(EDGE_LIST::const_iterator j = topo_edge_list.begin();
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
		{//边属于左连通集
			i_left->push_back(i->ip_right);
		}
		else if(iConnected == 2)
		{//边属于右连通集
			i_right->push_back(i->ip_left);
		}
		else if(iConnected == 3)
		{
			if(i_left == i_right)
			{//边属于同一连通集，存在环路
			}
			else
			{//合并连通集
				for(list<std::string>::iterator j = i_right->begin();
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
	}//end 填充连通集set_conn
	return set_conn.size();
}

bool TraceAnalyse::AnalyseRRByRtPath_Direct(EDGE_LIST& topo_edge_list)
{
	//1.存在路径
	//2.路径中有一段走了不可网管设备(如电信设备)
	EDGE_LIST edge_list_rr;			
        SvLog::writeLog("TraceAnalyse direct.");
	for(ROUTEPATH_LIST::iterator rp_iter = rtpath_list.begin(); rp_iter != rtpath_list.end(); ++rp_iter)
	{
		int num = 0;
		list<string>::iterator ipleft_iter, ipright_iter;
		for (list<string>::iterator ip_iter = rp_iter->begin(); ip_iter != rp_iter->end(); ++ip_iter)
		{
			if (find(unManagedIPList.begin(), unManagedIPList.end(), *ip_iter) != unManagedIPList.end())
			{				
				if (num == 0)
				{
					ipleft_iter = ip_iter;
				}
				else
				{
					ipright_iter = ip_iter;
				}
				num++;
			}			
		}
		if ((num < 2) || (*ipleft_iter == *ipright_iter))
		{//忽略不能正常分析的trace路径			
			continue;			
		}
		
		//规范化*ipleft_iter前的ips
		list<string> ipleft_list;
		for (list<string>::iterator iter = rp_iter->begin(); iter != ipleft_iter; ++iter)
		{			
			for(DEVID_LIST::iterator j = device_list.begin(); j != device_list.end(); ++j)
			{
				if(find(j->second.ips.begin(), j->second.ips.end(), *iter) != j->second.ips.end())
				{
					ipleft_list.push_back(j->first);					
					break;
				}
			}
		}
		int numL = 0, numR = 0;
		string ip_left, port_left, ip_right, port_right;
                list<pair<pair<string, string>, string> >::iterator l_iter, r_iter;
                for (list<pair<pair<string, string>, string> >::iterator iter = RouteIPPairList.begin(); iter != RouteIPPairList.end(); ++iter)
		{
			if ((*iter).second == *ipleft_iter)
			{
				numL++;
				l_iter = iter;
				if (find(ipleft_list.begin(), ipleft_list.end(), (*iter).first.first) != ipleft_list.end())//add by zhangyan 2008-09-02
				{
					ip_left = (*iter).first.first;						
					port_left = (*iter).first.second;				
				}
			}
			else if ((*iter).second == *ipright_iter)
			{
				numR++;
				r_iter = iter;
				if (find(ipright_iter, rp_iter->end(), (*iter).first.first) != rp_iter->end())//add by zhangyan 2008-09-02
				{
					ip_right = (*iter).first.first;	//ip_right不需规范化					
					port_right = (*iter).first.second;
				}
			}
		}	
		if (ip_left.empty() && (numL == 1))
		{
			ip_left = (*l_iter).first.first;
			port_left = (*l_iter).first.second;
		}		
		if (ip_right.empty() && (numR == 1))
		{
			ip_right = (*r_iter).first.first;
			port_right = (*r_iter).first.second;
		}

		if (!ip_left.empty() && !ip_right.empty())
		{
			EDGE edge_tmp;
			edge_tmp.ip_left  = ip_left;
			edge_tmp.pt_left  = port_left;
			edge_tmp.inf_left = port_left;
			edge_tmp.ip_right  = ip_right;
			edge_tmp.pt_right  = port_right;
			edge_tmp.inf_right = port_right;
			edge_list_rr.push_back(edge_tmp);	
			cout<<"edge(TR): "<<ip_left<<"::"<<port_left<<"-"<<ip_right<<"::"<<port_right<<endl;		
		}		
	}

	cout << "router-router(by Tracert)" << endl;
	for (EDGE_LIST::iterator i = edge_list_rr.begin(); i != edge_list_rr.end(); ++i)
	{
		int iConnected = 0;
                std::list<std::list<std::string> >::iterator i_left,i_right;
                for (std::list<std::list<std::string> >::iterator j = set_conn.begin();
			j != set_conn.end();
			++j)
		{			
			if (find(j->begin(), j->end(), i->ip_left) != j->end())
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
		
		if (iConnected == 3)
		{
			if (i_left != i_right)
			{//不属于同一连通集时
				topo_edge_list.push_back(*i);
				cout <<"topo edge(TR): "<<i->ip_left << "-" << i->inf_left << "," << i->ip_right << "-" << i->inf_right << endl;
				
				//合并连通集				
				for(list<std::string>::iterator j = i_right->begin();
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
	
	return true;
}

bool TraceAnalyse::AnalyseRRByRtPath(EDGE_LIST& topo_edge_list)
{
	//1.存在路径
	//2.路径中有一段走了不可网管设备(如电信设备)
	EDGE_LIST edge_list_rr;			
	list<string> retracertIp_list;
	for(ROUTEPATH_LIST::iterator rp_iter = rtpath_list.begin(); rp_iter != rtpath_list.end(); ++rp_iter)
	{
		int num = 0;
		list<string>::iterator ipleft_iter, ipright_iter;
		for (list<string>::iterator ip_iter = rp_iter->begin(); ip_iter != rp_iter->end(); ++ip_iter)
		{
			if (find(unManagedIPList.begin(), unManagedIPList.end(), *ip_iter) != unManagedIPList.end())
			{
				if (num == 0)
				{
					ipleft_iter = ip_iter;
				}
				else
				{
					ipright_iter = ip_iter;
				}
				num++;
			}			
		}		
		if ((num < 2) || (*ipleft_iter == *ipright_iter))
		{
			list<string>::iterator iplast_iter = rp_iter->end();
			iplast_iter--;
			if (find(rp_iter->begin(), rp_iter->end(), "*") != rp_iter->end() && (*iplast_iter != "*"))
			{							
				retracertIp_list.push_back(*iplast_iter);
			}
			continue;			
		}
		
		//规范化*ipleft_iter前的ips
		list<string> ipleft_list;
		for (list<string>::iterator iter = rp_iter->begin(); iter != ipleft_iter; ++iter)
		{			
			for(DEVID_LIST::iterator j = device_list.begin(); j != device_list.end(); ++j)
			{
				if(find(j->second.ips.begin(), j->second.ips.end(), *iter) != j->second.ips.end())
				{
					ipleft_list.push_back(j->first);					
					break;
				}
			}
		}
		int numL = 0, numR = 0;
		string ip_left, port_left, ip_right, port_right;
                list<pair<pair<string, string>, string> >::iterator l_iter, r_iter;
                for (list<pair<pair<string, string>, string> >::iterator iter = RouteIPPairList.begin(); iter != RouteIPPairList.end(); ++iter)
		{
			if ((*iter).second == *ipleft_iter)
			{
				numL++;
				l_iter = iter;
				if (find(ipleft_list.begin(), ipleft_list.end(), (*iter).first.first) != ipleft_list.end())//add by zhangyan 2008-09-02
				{
					ip_left = (*iter).first.first;						
					port_left = (*iter).first.second;				
				}
			}
			else if ((*iter).second == *ipright_iter)
			{
				numR++;
				r_iter = iter;
				if (find(ipright_iter, rp_iter->end(), (*iter).first.first) != rp_iter->end())//add by zhangyan 2008-09-02
				{
					ip_right = (*iter).first.first;	//ip_right不需规范化					
					port_right = (*iter).first.second;
				}
			}
		}	
		if (ip_left.empty() && (numL == 1))
		{
			ip_left = (*l_iter).first.first;
			port_left = (*l_iter).first.second;
		}		
		if (ip_right.empty() && (numR == 1))
		{
			ip_right = (*r_iter).first.first;
			port_right = (*r_iter).first.second;
		}
		
		if (!ip_left.empty() && !ip_right.empty())
		{
			EDGE edge_tmp;
			edge_tmp.ip_left  = ip_left;
			edge_tmp.pt_left  = port_left;
			edge_tmp.inf_left = port_left;
			edge_tmp.ip_right  = ip_right;
			edge_tmp.pt_right  = port_right;
			edge_tmp.inf_right = port_right;
			edge_list_rr.push_back(edge_tmp);	
			cout<<"edge(TR): "<<ip_left<<"::"<<port_left<<"-"<<ip_right<<"::"<<port_right<<endl;		
		}
		else 
		{
			list<string>::iterator iplast_iter = rp_iter->end();
			iplast_iter--;
			if (*iplast_iter != "*")
			{
				retracertIp_list.push_back(*iplast_iter);
			}
		}		
	}

	cout << "router-router(by Tracert)" << endl;
	for (EDGE_LIST::iterator i = edge_list_rr.begin(); i != edge_list_rr.end(); ++i)
	{
		int iConnected = 0;
                std::list<std::list<std::string> >::iterator i_left,i_right;
                for (std::list<std::list<std::string> >::iterator j = set_conn.begin();
			j != set_conn.end();
			++j)
		{
			if (find(j->begin(), j->end(), i->ip_left) != j->end())
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

		if (iConnected == 3)
		{
			if (i_left != i_right)
			{//不属于同一连通集时
				topo_edge_list.push_back(*i);
				cout <<"topo edge(TR): "<<i->ip_left << "-" << i->inf_left << "," << i->ip_right << "-" << i->inf_right << endl;
				
				//合并连通集				
				for(list<std::string>::iterator j = i_right->begin();
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

	if (set_conn.size() > 1 && !retracertIp_list.empty())
	{//重新进行路由跟踪
		TraceReader *myTR = new TraceReader(scanParam.retrytimes, scanParam.timeout, 30);//Hops暂时先固定为30		
		for (list<string>::iterator iter = retracertIp_list.begin(); iter != retracertIp_list.end(); ++iter)
		{
			myTR->RouteDESTIPPairList.push_back(make_pair(make_pair(*iter, ""), ""));
		}
		rtpath_list = myTR->getTraceRouteByIPs();//得到新的路由跟踪路径表		
		if (!rtpath_list.empty())
		{
			//保存新得到的路径表
			StreamData myfile;
			myfile.saveRenewTracertList(rtpath_list);
			//对新得到的路径表进行分析
			AnalyseRRByRtPath_Direct(topo_edge_list);
		}			
	}
		
	return true;
}

