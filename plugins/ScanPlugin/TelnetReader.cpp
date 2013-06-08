#include "StdAfx.h"
#include "TelnetReader.h"
#include "tchar.h"
#include "svlog.h"
#include <QtDebug>

//added by zhangyan 2009-01-13
#define MAX_ALLSECTIONS 2048  //全部的段名
#define MAX_SECTION 20  //一个段名长度

TelnetReader::TelnetReader()
{
	getAftByTelnet = NULL;
	getArpByTelnet = NULL;
	DeviceDll = NULL;
	string PathFileName = getRegValue();
	if(PathFileName != "")
	{
		char lpDllFullPathName[MAX_PATH];
		WCHAR pszLibFileName[MAX_PATH]={0};
                strncpy(lpDllFullPathName, PathFileName.c_str(), MAX_PATH);
		MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS,
			lpDllFullPathName, strlen(lpDllFullPathName),
			pszLibFileName, MAX_PATH);
		DeviceDll = LoadLibrary( pszLibFileName);
		if (DeviceDll) 
		{
			getAftByTelnet = reinterpret_cast<GetAftByTelnet *>(GetProcAddress(DeviceDll, "ReadAftByTelnet"));
			getArpByTelnet = reinterpret_cast<GetArpByTelnet *>(GetProcAddress(DeviceDll, "ReadArpByTelnet"));
			if (getAftByTelnet == NULL || getArpByTelnet == NULL)
			{
				FreeLibrary(DeviceDll);
				getAftByTelnet = NULL;
				getArpByTelnet = NULL;
				DeviceDll = NULL;
			}
		}
	}
	else //added by zhangyan 2009-01-13
	{
		path = "";
	}

	GetTelnetIPS();
}

TelnetReader::~TelnetReader(void)
{
	if(DeviceDll != NULL)
	{
		if (getAftByTelnet != NULL)
		{
			getAftByTelnet = NULL;
		}
		if (getArpByTelnet != NULL) 
		{
			getArpByTelnet = NULL;
		}
		FreeLibrary(DeviceDll);
		DeviceDll = NULL;
	}
}

// 获取注册表的值
string TelnetReader::getRegValue()
{
	string regValue("");
	HKEY hkResult = 0;
	LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                //_T("Software\\Siteview"),// 键路径
                (const WCHAR *)"Software\\Siteview",// 键路径
		0,
		KEY_READ,
		&hkResult);
	if( ERROR_SUCCESS == lRet)
	{// 打开键成功
		std::wstring strValue;
		DWORD dwType=REG_NONE;
		DWORD dwCount=0;
		//先查询键值的长度。
		LONG lResult = RegQueryValueEx(hkResult, 
                        //_T("COMMONDIR"),//键名
                        (const WCHAR *)"COMMONDIR",
			NULL,
			&dwType,
			NULL, 
			&dwCount);
		if (lResult == ERROR_SUCCESS)
		{         
			strValue.resize(dwCount);
                        lResult = RegQueryValueEx(hkResult,(const WCHAR*)"COMMONDIR", NULL, &dwType,
				(LPBYTE)strValue.data(), &dwCount);
			if(lResult == ERROR_SUCCESS)
			{
				path = ws2s(strValue); //added by zhangyan 2009-01-13
				regValue = path + "\\ReadDeviceInfo.dll";
			}
		}		
		::RegCloseKey( hkResult );
	}
	return regValue;
}

void TelnetReader::GetTelnetIPS()
{	
	telnetIPList_Aft.clear();
	telnetIPList_Arp.clear();

	if (path.empty())
	{
		return;
	}

	int i;  
	int iPos = 0;  
	int iMaxCount;
	char chSectionNames[MAX_ALLSECTIONS] = {0}; //存放所有提出来的字符串
	char chSection[MAX_SECTION] = {0}; //存放一个段名
	string filename = path + "\\telnet.ini";

	DWORD dw = GetPrivateProfileSectionNamesA(chSectionNames, MAX_ALLSECTIONS, filename.c_str());
	if (dw == 0)
	{
		return;
	}

	//以下循环，截断到两个连续的0
	for (i = 0; i < MAX_ALLSECTIONS; i++)
	{
		if (chSectionNames[i] == 0)
			if (0 == chSectionNames[i+1])
				break;
	}

	iMaxCount = i + 1; 

	//得到telnet.ini中所有的section
	list<string> sectionList;
	for (i = 0; i < iMaxCount; i++)
	{
		chSection[iPos++] = chSectionNames[i];
		if (chSectionNames[i] == 0)
		{   
			sectionList.push_back((string)chSection);
			memset(chSection, 0, MAX_SECTION);
			iPos=0;
		}
	}

	char IniBuff[50];
	for (list<string>::iterator iter = sectionList.begin(); iter != sectionList.end(); ++iter)
	{
		ZeroMemory(IniBuff, 50);
		GetPrivateProfileStringA((*iter).c_str(), "HostIp", "", IniBuff, 50, filename.c_str());
		string hostip = IniBuff;
		
		ZeroMemory(IniBuff, 50);
		GetPrivateProfileStringA((*iter).c_str(), "indicatorIndex", "", IniBuff, 50, filename.c_str());
		string indicatorIndex = IniBuff;
		if (indicatorIndex == "20000")
		{
			telnetIPList_Aft.push_back(hostip);
		}
		else if(indicatorIndex == "20001")
		{
			telnetIPList_Arp.push_back(hostip);
		}
	}

	////测试telnetIPList_Aft及telnetIPList_Arp
	//for (list<string>::iterator iter = telnetIPList_Aft.begin(); iter != telnetIPList_Aft.end(); ++iter)
	//{
	//	cout<<"telnet ip (by aft): "<<*iter<<endl;
	//}
	//for (list<string>::iterator iter = telnetIPList_Arp.begin(); iter != telnetIPList_Arp.end(); ++iter)
	//{
	//	cout<<"telnet ip (by arp): "<<*iter<<endl;
	//}
}


//使用Telenet 读取一台设备的AFT数据
map<string, list<string> > TelnetReader::getAftData(const string & ips)
{
        map<string, list<string> > port_macs; //{port,[mac]}

	if (getAftByTelnet)
	{
                SvLog::writeLog("Start read the aft table of " + ips + " by telnet");
		int nn = TELNET_REC_COUNT;
		AftData* aftdata = new AftData[nn];
		int res = getAftByTelnet(ips.c_str(), nn, aftdata);
		if(res == -1 && nn != -1)
		{
			delete[] aftdata;
			nn += 100; //留出余量
			aftdata = new AftData[nn];
			res = getAftByTelnet(ips.c_str(), nn, aftdata);
		}
		if(res != -1)
		{
                        std::map<std::string, std::list<std::string> >::iterator i_pm;
			for(int i = 0; i < res && i < nn; i++)
			{
				string port = aftdata[i].valueStr1;
				string mac  = aftdata[i].valueStr2;
                                std::transform(mac.begin(), mac.end(), mac.begin(), (int(*)(int))toupper);
				i_pm = port_macs.find(port);
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
					port_macs.insert(make_pair(port, maclist));
                                        qDebug() << "telnet source ip : " << ips.c_str() << "port : " << port.c_str();
				}
			}
		}
		if(aftdata)
		{
			delete[] aftdata;
			aftdata = NULL;
		}
	}
	return port_macs;
}

map<string, list<pair<string, string> > > TelnetReader::getArpData(const string& ips)
{
        map<string, list<pair<string, string> > > port_ipmacs; //{infindex,[(ip,mac)]}
	if (getArpByTelnet) 
	{
                SvLog::writeLog("Start read the arp table of " + ips + " by telnet");

		int nn = TELNET_REC_COUNT;
		AftData* arpdata = new AftData[nn];
		int res = getArpByTelnet(ips.c_str(), nn, arpdata);
		if(res == -1 && nn != -1)
		{//
			delete[] arpdata;
			nn += 100; //留出余量
			arpdata = new AftData[nn];
			res = getArpByTelnet(ips.c_str(), nn, arpdata);
		}
		if(res != -1)
		{
                        std::map<std::string, std::list<pair<std::string,std::string> > >::iterator i_pm;
			for(int i = 0; i < res && i < nn; i++)
			{
				string port_tmp = arpdata[i].valueStr1;
				string mac_tmp  = arpdata[i].valueStr2;
				string ip_tmp   = arpdata[i].valueStr3;
				i_pm = port_ipmacs.find(port_tmp);
				if(i_pm != port_ipmacs.end())
				{
					bool bExisted = false;
                                        for(std::list<pair<std::string,std::string> >::iterator j = i_pm->second.begin();
						j != i_pm->second.end();
						++j)
					{
						if( j->first == ip_tmp && j->second == mac_tmp)
						{
							bExisted = true;
							break;
						}
					}
					if(!bExisted)
					{
						i_pm->second.push_back(make_pair(ip_tmp,mac_tmp));
					}
				}
				else
				{
                                        std::list<pair<std::string,std::string> > ipmaclist;
					ipmaclist.push_back(make_pair(ip_tmp,mac_tmp));
					port_ipmacs.insert(make_pair(port_tmp, ipmaclist));
				}
			}
		}
		if(arpdata)
		{
			delete[] arpdata;
			arpdata = NULL;
		}
	}
	return port_ipmacs;
}

