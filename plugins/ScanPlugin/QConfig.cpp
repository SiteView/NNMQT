#include "QConfig.h"

using std::pair;
using std::make_pair;

QConfig::QConfig()
{
    m_strDefaultCommunityGet = "public1";
    m_strDefaultCommunitySet = "";
    m_iDepth = 1;
    m_iTryTimes = 3;
    m_iMaxThreadCount = 30;
    m_iTimeOut = 200;
    //m_listScanScales.push_back(make_pair("192.168.9.1", "192.168.9.254"));

   // m_listScanScales.push_back(make_pair("192.168.0.1", "192.168.0.254"));

    Community community;
//    community.StartIp = "192.168.0.248";
//    community.EndIp = "192.168.0.248";
//    community.Get = "dragon";
//    communityList.push_back(community);

//    community.StartIp = "192.168.0.251";
//    community.EndIp = "192.168.0.251";
//    community.Get = "public";
//    communityList.push_back(community);

    community.StartIp = "192.168.0.253";
    community.EndIp = "192.168.0.253";
    community.Get = "public";
    communityList.push_back(community);

//    community.StartIp = "192.168.9.254";
//    community.EndIp = "192.168.9.254";
//    community.Get = "public1";
//    communityList.push_back(community);

//    seeds.push_back("192.168.0.248");
//    seeds.push_back("192.168.0.251");
    seeds.push_back("192.168.0.253");
//    seeds.push_back("192.168.9.254");

}

QConfig::~QConfig()
{

}


