#include "monitor.h"
#include "Device.h"
#include "ScanUtils.h"
#include "DeviceFactory.h"
#include "DeviceRegister.h"
//#include "plugins/ScanPlugin/Device.h"
//#include "plugins/ScanPlugin/ScanUtils.h"
//#include "plugins/ScanPlugin/DeviceFactory.h"
#include <string>
#include <QtDebug>
#include <list>
#include <QStringList>
#include <QString>

using namespace std;


monitor::monitor(QString ip, bool bGetPortFlow, string port, QObject *parent) : QThread(parent)
{
    m_ip = q2s(ip);
    Wizard *wizard = new Wizard();
    loadConfig(wizard);
    m_bGetPortFlow = bGetPortFlow;
    m_port = port;

    m_bStop = false;

    m_iCurrentPortFlow = 0;
    m_iLastPortFlow = 0;
    m_iLastTickCount = 0;
    m_iCurrentTickCount = 0;
}

monitor::~monitor()
{
}

void monitor::Start()
{
    start(HighestPriority);
}

void monitor::stopMonitor()
{
    m_bStop = true;
}

void monitor::run()
{
    if (m_bGetPortFlow)
    {
        Sleep(10000);
        while(!m_bStop)
        {
            getPortFlow(true);
            Sleep(2000);
            getPortFlow(false);
            Sleep(60000);
        }
    }
    else
    {
        IFRecList infprops;
        IFFlowList infFlow;
        getCiscoInfProp(infprops, infFlow);
        emit SendIfrecInfo(infprops, infFlow);
    }

}

void monitor::loadConfig(Wizard *wizard)
{
    spr.ip = m_ip;
    int iTryTimes = wizard->scanPerformanceConfigWizard->Retry;
    if(iTryTimes <= 0)
    {
        spr.retry = 1;
    }
    else
    {
        spr.retry = iTryTimes;
    }

    int iTimeOut = wizard->scanPerformanceConfigWizard->TimeOut;
    if(iTimeOut < 100)
    {
        spr.timeout = 200;//==2000ms 低于100ms的超时设置启用默认值200 *10ms
    }
    else
    {
        spr.timeout = iTimeOut/10;
    }

    string defaultCommunity = q2s(wizard->communityOfRangeConfigWizard->DefaultGetCommunity);
    foreach(QString communityRangeList, wizard->communityOfRangeConfigWizard->CommunityOfRangeList)
    {
        QStringList community = communityRangeList.split("-", QString::SkipEmptyParts);
        string startip = q2s(community.at(0));
        string endip = q2s(community.at(1));
        unsigned long startnum = ntohl(inet_addr(startip.c_str()));
        unsigned long endnum   = ntohl(inet_addr(endip.c_str()));

        unsigned long ip = ntohl(inet_addr(m_ip.c_str()));
        if (ip >= startnum && ip <= endnum)
        {
            spr.community = q2s(community.at(2));
        }
    }
}



IFPROP_LIST monitor::getInterfaceInfo()
{

    string test = getFacOid("1.3.6.1.4.1.9.1.563");
    qDebug() << "test :" << test.c_str();
    //Device* device = DeviceFactory::Instance()->CreateDevice(getFacOid("1.3.6.1.4.1.9.1.563"));

    IFPROP_LIST inflist_cur;

    SnmpDG snmp;
    SnmpPara spr;
    spr.ip = "192.168.0.248";
    spr.retry = 3;
    spr.timeout = 200;
    spr.community = "dragon";

    map<string, string> oidIndexList;
    //spr.snmpver = ""
    //inflist_cur = device->getInfProp(snmp, spr, oidIndexList, false);

    IFPROP_LIST::iterator it = inflist_cur.begin();
    for (it; it != inflist_cur.end(); it++)
    {
        //    if (!inflist_cur.empty())
        //    {

        // qDebug() << "infolist not empty";
    }
    //    }


    return inflist_cur;
}


// 获取特定设备的接口信息
IFRecList monitor::getUnixInfProp(SnmpDG& snmp, const SnmpPara& spr, map<string, string> oidIndexList,bool bRouter)
{
    IFPROP_LIST ifprop_list;
    ifprop_list.clear();
    std::list<IFREC> infprops;//(ifindex,ifType,ifDescr,ifMac,ifPort,ifSpeed)
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
            //	SvLog::writeLog("read inf");
            //	for(list<pair<string, string>>::iterator item = infDescs.begin(); item != infDescs.end(); ++item)
            //	{
            //		if(item->second.compare(0,3,"eth") == 0)
            //		{
            //			string port = item->second.substr(3);
            //			//SvLog::writeLog("port:"+port+"infInde:"+infIndex);
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
                        std::transform(inf_tmp.ifMac.begin(), inf_tmp.ifMac.end(), inf_tmp.ifMac.begin(), (int(*)(int))toupper);
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
    return infprops;
}

void monitor::getPortFlow(bool bFirstGetPortFlow)
{
    string strTickCount = snmp.GetMibObject(spr,"1.3.6.1.2.1.1.3");
    string strPortInOctetsMib;
    string strPortOutOctetsMib;
    strPortInOctetsMib = "1.3.6.1.2.1.2.2.1.10." + m_port;
    strPortOutOctetsMib = "1.3.6.1.2.1.2.2.1.16." + m_port;
    QString infInOctets;
    QString infOutOctets;
    infInOctets = s2q(snmp.GetMibObject(spr,strPortInOctetsMib,true));
    infOutOctets = s2q(snmp.GetMibObject(spr,strPortOutOctetsMib,true));

    uint iInOctets = 0;
    uint iOutOctets = 0;
    if (infInOctets != "")
    {
        iInOctets = infInOctets.toUInt();
    }
    if (infOutOctets != "")
    {
        iOutOctets = infOutOctets.toUInt();
    }

    long int iTemp = iInOctets + iOutOctets;

    QString strTemp = strTickCount.c_str();

    if (strTemp != "")
    {
        QStringList strTempList = strTemp.split(",",QString::SkipEmptyParts);
        QString strTimeExact = strTempList.at(1);
        QStringList strTimeExactList = strTimeExact.split(".",QString::SkipEmptyParts);
        QString strTime = strTimeExactList.at(0);
        QStringList strTimeList = strTime.split(":",QString::SkipEmptyParts);
        QString strHour = strTimeList.at(0);
        QString strMin = strTimeList.at(1);
        QString strSec = strTimeList.at(2);
        int iHour = strHour.toInt();
        int iMin = strMin.toInt();
        int iSec = strSec.toInt();


        if (!bFirstGetPortFlow)
        {
            m_iCurrentTickCount = iHour*3600 + iMin*60 + iSec;
            m_iCurrentPortFlow = iTemp;
            int iActualFlow = m_iCurrentPortFlow - m_iLastPortFlow;
            int iActualTime = m_iCurrentTickCount - m_iLastTickCount;
            //        qDebug() << "port : " << m_port.c_str() << " inmib : " << strPortInOctetsMib.c_str() << " ip : " << spr.ip.c_str() << " iCurr : " << m_iCurrentPortFlow << " iLast : " << m_iLastPortFlow;

            if (iActualFlow > 0 && iActualTime > 0)
            {
                int iSpeed = iActualFlow / iActualTime * 8 / 1024;
                emit SendPortFlow(iSpeed);
            }
            else
            {
                emit SendPortFlow(0);
            }

        }
        else
        {
            m_iLastPortFlow = iTemp;
            m_iLastTickCount = iHour*3600 + iMin*60 + iSec;
            bFirstGetPortFlow = false;
        }
    }


}


void monitor::getCiscoInfProp(IFRecList &infprops, IFFlowList &infFlow, bool bRouter)
{
    IFPROP_LIST ifprop_list;
    list<string> list_cmty;
    //        list<IFREC> infprops;//(ifindex,ifType,ifDescr,ifMac,ifPort,ifSpeed)
    //        list<IFFLOW> infFlow;
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
            //getLogicEntity(snmp, spr);
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
                        qDebug() << "community_tmp : " << community_tmp.c_str();
                        list_cmty.push_back(community_tmp);
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

        list<pair<string,string> > infAdminStatus = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.7");

        list<pair<string,string> > infStatus = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.8");

        //        list<pair<string,string> > infInOctets = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.10");

        //        list<pair<string,string> > infInUcastPkgs = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.11");

        //        list<pair<string,string> > infOutOctets = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.16");

        //        list<pair<string,string> > infOutUcastPkgs = snmp.GetMibTable(spr,"1.3.6.1.2.1.2.2.1.17");

        string strTickCount = snmp.GetMibObject(spr,"1.3.6.1.2.1.1.3");
        qDebug() << "get tick count : " << strTickCount.c_str();



        //std::list<IFREC> infprops;//(ifindex,ifType,ifDescr,ifMac,ifPort,ifSpeed)
        for(list<pair<string,string> >::iterator itype = infTypes.begin(); itype != infTypes.end(); ++itype)
        {
            IFREC inf_tmp;
            IFFLOW inf_flowTemp;
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

            ;
            for(list<pair<string,string> >::iterator iAdminStatus = infAdminStatus.begin(); iAdminStatus != infAdminStatus.end(); iAdminStatus++)
            {
                if(iAdminStatus->first.substr(20) == inf_tmp.ifIndex)
                {
                    inf_flowTemp.ifAdminStatus = iAdminStatus->second;
                    break;
                }
            }



            for(list<pair<string,string> >::iterator iStatus = infStatus.begin(); iStatus != infStatus.end(); iStatus++)
            {
                if(iStatus->first.substr(20) == inf_tmp.ifIndex)
                {
                    inf_flowTemp.ifStatus = iStatus->second;
                    break;
                }
            }

            //            for(list<pair<string,string> >::iterator iInOctets = infInOctets.begin(); iInOctets != infInOctets.end(); iInOctets++)
            //            {
            //                string temp = iInOctets->first.substr(21);
            //                //qDebug() << "temp : " << temp.c_str() << " :index : " << inf_tmp.ifIndex.c_str() << " old :" << iInOctets->first.c_str();
            //                if(iInOctets->first.substr(21) == inf_tmp.ifIndex)
            //                {
            //                    inf_flowTemp.ifInOctets = iInOctets->second;
            //                    break;
            //                }
            //            }

            //            for(list<pair<string,string> >::iterator iInUcastPkgs = infInUcastPkgs.begin(); iInUcastPkgs != infInUcastPkgs.end(); iInUcastPkgs++)
            //            {
            //                if(iInUcastPkgs->first.substr(21) == inf_tmp.ifIndex)
            //                {
            //                    inf_flowTemp.ifInUcastPkgs = iInUcastPkgs->second;
            //                    break;
            //                }
            //            }

            //            for(list<pair<string,string> >::iterator iOutOctets = infOutOctets.begin(); iOutOctets != infOutOctets.end(); iOutOctets++)
            //            {
            //                if(iOutOctets->first.substr(21) == inf_tmp.ifIndex)
            //                {
            //                    inf_flowTemp.ifOutOctets = iOutOctets->second;
            //                    break;
            //                }
            //            }

            //            for(list<pair<string,string> >::iterator iOutUcastPkgs = infOutUcastPkgs.begin(); iOutUcastPkgs != infOutUcastPkgs.end(); iOutUcastPkgs++)
            //            {
            //                if(iOutUcastPkgs->first.substr(21) == inf_tmp.ifIndex)
            //                {
            //                    inf_flowTemp.ifOutUcastPkgs = iOutUcastPkgs->second;
            //                    break;
            //                }
            //            }

            //            inf_flowTemp.ifIndex = inf_tmp.ifIndex;

            //            QString strTemp = strTickCount.c_str();
            //            QStringList strTempList = strTemp.split(",",QString::SkipEmptyParts);
            //            QString strTimeExact = strTempList.at(1);
            //            QStringList strTimeExactList = strTimeExact.split(".",QString::SkipEmptyParts);
            //            QString strTime = strTimeExactList.at(0);
            //            QStringList strTimeList = strTime.split(":",QString::SkipEmptyParts);
            //            QString strHour = strTimeList.at(0);
            //            QString strMin = strTimeList.at(1);
            //            QString strSec = strTimeList.at(2);
            //            int iHour = strHour.toInt();
            //            int iMin = strMin.toInt();
            //            int iSec = strSec.toInt();
            //            inf_flowTemp.iTickCount = iHour*3600 + iMin*60 + iSec;
            //            qDebug() << "calc itickcout : " << inf_flowTemp.iTickCount;

            infprops.push_back(inf_tmp);
            infFlow.push_back(inf_flowTemp);
        }
        if(!infprops.empty())
        {
            ifprop_list.insert(make_pair(spr.ip,make_pair(infAmount,infprops)));
        }
    }
    //return infprops;
}


