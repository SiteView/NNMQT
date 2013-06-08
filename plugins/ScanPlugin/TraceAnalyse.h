#pragma once
#include "commondef.h"

using namespace std;

class TraceAnalyse
{
public:	
        TraceAnalyse(const DEVID_LIST& devlist, const ROUTEPATH_LIST& routepath_list, const list<pair<pair<string, string>, string> >& RouteDESTIPPairList, const list<string>& unManagedDevices, const SCAN_PARAM& param);
        TraceAnalyse(const DEVID_LIST& devlist, const ROUTEPATH_LIST& routepath_list, const list<pair<pair<string, string>, string> >& RouteDESTIPPairList, const list<string>& unManagedDevices);
	~TraceAnalyse(void){};	
	static int getConnection(const EDGE_LIST& topo_edge_list);
	//bool AnalyseTraceEdge(EDGE_LIST& topo_edge_list, string Type);
	bool AnalyseRRByRtPath(EDGE_LIST& topo_edge_list);
	bool AnalyseRRByRtPath_Direct(EDGE_LIST& topo_edge_list);

private:	
	SCAN_PARAM scanParam;	
	DEVID_LIST device_list;
	ROUTEPATH_LIST rtpath_list;
	//list[<<leftip,leftport>,rightip>]
	//leftip--作为trace目的ip, rightip--作为不可网管设备
        list<pair<pair<string, string>, string> > RouteIPPairList;
	list<string> unManagedIPList;
public:	
        static list<list<string> > set_conn;
};
