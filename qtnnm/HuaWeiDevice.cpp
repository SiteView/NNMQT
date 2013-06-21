#include "StdAfx.h"
#include "HuaWeiDevice.h"

HuaWeiDevice::HuaWeiDevice(void)
{
}

HuaWeiDevice::~HuaWeiDevice(void)
{
}

DIRECTDATA_LIST HuaWeiDevice::getDirectData(SnmpDG& snmp, const SnmpPara& spr)
{	
	//hgmp
	directdata_list.clear();	
	//PeerMac:1.3.6.1.4.1.2011.6.7.5.6.1.1(localportindex,peermac)
        list<pair<string,string> > peerMacs = snmp.GetMibTable(spr, "1.3.6.1.4.1.2011.6.7.5.6.1.1");
	if(!peerMacs.empty())
	{
		//PeerPort:1.3.6.1.4.1.2011.6.7.5.6.1.2	(localportindex,peerport)
                list<pair<string,string> > peerPorts = snmp.GetMibTable(spr, "1.3.6.1.4.1.2011.6.7.5.6.1.2");
		if(!peerPorts.empty())
		{
			std::list<DIRECTITEM> item_list; //[localportindex,peerid,peerip,peerportdesc]			
                        for(list<pair<string,string> >::iterator imac = peerMacs.begin(); imac != peerMacs.end(); ++imac)
			{
				string imac_tmp = imac->first.substr(29);
				vector<string> local_indexs = tokenize(imac_tmp, ".", true);
				/*if(local_indexs.size() < 8)
				{
					continue;
				}*/				
                                for(list<pair<string,string> >::iterator iport = peerPorts.begin(); iport != peerPorts.end(); ++iport)
				{
					string iport_tmp = iport->first.substr(29);
					vector<string> pt_indexs = tokenize(iport_tmp, ".", true);
					/*if(pt_indexs.size() < 8)
					{
						continue;
					}*/
					//reworked by zhangyan 2008-10-08
					//if(local_indexs[0] == pt_indexs[0])
					if (imac_tmp == iport_tmp)
					{
						string mac_str = replaceAll(imac->second, " ", "").substr(0, 12);
						if (mac_str.length() > 0)
						{
                                                        std::transform(mac_str.begin(), mac_str.end(), mac_str.begin(), (int(*)(int))toupper);
							DIRECTITEM item_tmp;
							item_tmp.localPortInx = local_indexs[0];
							item_tmp.localPortDsc = "";
							item_tmp.PeerID = "";
							item_tmp.PeerIP = mac_str;//mac
							item_tmp.PeerPortDsc = iport->second;
							item_tmp.PeerPortInx = "";
							item_list.push_back(item_tmp);								
						}
						break;
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
