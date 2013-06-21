#ifndef TELNETREADER_H
#define TELNETREADER_H
#pragma once

//#include "SvLog.h"
#include "stdafx.h"
#include "scanutils.h"




using namespace std;

//telnet������������ݽṹ
typedef struct {
    char valueStr1[256];//port,infindex
    char valueStr2[256];//mac
    char valueStr3[256];//ip  (��AFT��Ч)
    char valueStr4[256];
}AftData;

#define TELNET_REC_COUNT 512

class TelnetReader
{
public:
	TelnetReader();
public:
	~TelnetReader(void);

public:
        map<string, list<string> > getAftData(const string & ips);
        map<string, list<pair<string,string> > > getArpData(const string & ips);
	
	//added by zhangyan 2009-01-13
	list<string> telnetIPList_Arp;
	list<string> telnetIPList_Aft;

private:
	// ��ȡע����ֵ
	string getRegValue();
	string path; //added by zhangyan 2009-01-13
	void GetTelnetIPS();//added by zhangyan 2009-01-13

	HMODULE DeviceDll;
	
	//ip��ַ, ���鳤��(in-����ĳ���,out-ʵ�ʵļ�¼��), ���ݽ��
	//return - success:��¼��, fail:-1
	typedef int (GetAftByTelnet)(const char *, int &, AftData*);
	//ip��ַ, ���鳤��(in-����ĳ���,out-ʵ�ʵļ�¼��), ���ݽ��
	//return - success:��¼��, fail:-1
	typedef int (GetArpByTelnet)(const char *, int &, AftData*);
	//sysoid, factory, model, typeName, shortType(�豸����)
	//�豸���ͣ�0-���㽻����1-���㽻����2-·������3-����ǽ��4-��������5-PC��6-����
	//return - success: >=0, fail:< 0
	typedef int (GetTypeByOid)(const char * , char *, char *, char *, char *);

	GetAftByTelnet* getAftByTelnet;
	GetArpByTelnet* getArpByTelnet;
	//GetTypeByOid* getTypeByOid;
};
#endif