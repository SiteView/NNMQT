//SnmpDG.cpp

/*
 *
 * Copyright (c) 2007-2008 DragonFlow
 *
 * Created date: 2008-01-10
 *
 * Autor: Tang guofu 
 * 
 * last modified 2008-01-10 by Tang guofu 
 *
 */

//#include "StdAfx.h"
#include "stdafx.h"
#include "SnmpDG.h"

SnmpDG::SnmpDG() : m_inited_success(true),
                   m_error_code(SNMP_CLASS_SUCCESS),
			       non_reps(0), 
				   max_reps(100)
{
	m_inited_success = true;
}

SnmpDG::~SnmpDG(void)
{
}

const int SnmpDG::GetLastError() const
{
	return m_error_code;
}

const string & SnmpDG::GetLastErrorMsg() const
{
	return m_error_msg;
}

void SnmpDG::SetLastError(const int error_code) const
{
	assert(m_inited_success);
	m_error_code = error_code;
//	m_error_msg = m_snmp->error_msg(m_error_code);
}


//update by wings 09-11-13
/*const string & SnmpDG::GetMibObject(const SnmpPara& spr, const string oid) const
{
	string myoid = oid + ".0";
	//myoid += ".0";
	m_mib_object = GetMibObject(version2c, spr, myoid);
	if(m_mib_object.empty())
	{
		m_mib_object = GetMibObject(version1, spr, myoid);
	}
	/*if(m_mib_object.empty())
	{
		myoid += ".0";
		m_mib_object = GetMibObject(version2c, spr, myoid);
		if(m_mib_object.empty())
		{
			m_mib_object = GetMibObject(version1, spr, myoid);
		}
	}*/
/*	return m_mib_object;
}

const list<pair<string,string>> & SnmpDG::GetMibTable(const SnmpPara& spr, const string oid) const
{
	list<pair<string, string>> m_table_tmp;
	m_table_tmp = GetMibTable(version2c, spr, oid);
	cout<<"GetMibTable : "<<oid.c_str()<<endl;
	if(m_table_tmp.empty())
	{
		m_table_tmp = GetMibTable(version1, spr, oid);
	}
	m_mib_table = m_table_tmp;
	return m_mib_table;
}*/
//update by wings 09-11-13
//不能默认为版本自适应只有当版本参数不为1，2时，才是自适应。
const string & SnmpDG::GetMibObject(const SnmpPara& spr, const string oid) const
{
	string myoid = oid + ".0";
	//myoid += ".0";
	if(spr.snmpver=="2")
	{
		m_mib_object = GetMibObject(version2c, spr, myoid);
	}
	else if(spr.snmpver=="1")
	{
		m_mib_object = GetMibObject(version1, spr, myoid);
	}
	else
	{
		m_mib_object = GetMibObject(version2c, spr, myoid);
		if(m_mib_object.empty())
		{
			m_mib_object = GetMibObject(version1, spr, myoid);
		}
	}
	return m_mib_object;
}

const list<pair<string,string> > & SnmpDG::GetMibTable(const SnmpPara& spr, const string oid) const
{
        list<pair<string, string> > m_table_tmp;
	if(spr.snmpver=="2")
	{
		m_table_tmp = GetMibTable(version2c, spr, oid);
	}
	else if(spr.snmpver=="1")
	{
		m_table_tmp = GetMibTable(version1, spr, oid);
	}
	else
	{
		m_table_tmp = GetMibTable(version2c, spr, oid);
		if(m_table_tmp.empty())
		{
			m_table_tmp = GetMibTable(version1, spr, oid);
		}
	}
	m_mib_table = m_table_tmp;
        cout << "GetMibTable : " << oid.c_str() << endl;
	return m_mib_table;
}


const string & SnmpDG::GetMibObject(const snmp_version  version, const SnmpPara& spr,  const string oid) const
{
	if(!m_inited_success)
	{
		return m_empty_object;
	}
	Snmp::socket_startup();

	UdpAddress address(spr.ip.c_str());
	//string myoid = oid;

	Pdu pdu;
	Vb vb;
	vb.set_oid(oid.c_str());//(myoid.c_str());
	pdu += vb;

	CTarget ctarget(address); 
	ctarget.set_version( version );
	ctarget.set_retry(spr.retry);           
	ctarget.set_timeout(spr.timeout); 
	ctarget.set_readcommunity(spr.community.c_str());
	SnmpTarget *target;
	target = &ctarget;

	int status;
	Snmp snmp(status, 0, false);
	if (status == SNMP_CLASS_SUCCESS)
	{
		if ((status = snmp.get( pdu, *target)) == SNMP_CLASS_SUCCESS)
		{
			pdu.get_vb( vb,0);
			//string oid_tmp = vb.get_printable_oid();
			m_mib_object = vb.get_printable_value();
		}
		else
		{
			m_mib_object = string("");
			SetLastError(status);
		}
	}
	else
	{
		m_mib_object = string("");
		SetLastError(status);
	}
	Snmp::socket_cleanup();  // Shut down socket subsystem
	return m_mib_object;
}

const list<pair<string,string> >& SnmpDG::getMibTableByBulk(const snmp_version  version, const string oid) const
{
	m_mib_table.clear();
	//if(!m_inited_success)
	//{
	//	return m_empty_table;
	//}
	//Snmp::socket_startup();  // Initialize socket subsystem
	//Pdu pdu;                           // construct a Pdu object
	//Vb vb;                             // construct a Vb object
	//vb.set_oid(oid.c_str());
	//pdu += vb;

	//CTarget ctarget(m_address); 
	//ctarget.set_version(version);
	//ctarget.set_retry(m_retry);           
	//ctarget.set_timeout(m_timeout); 
	//ctarget.set_readcommunity(m_community.c_str());
	//
	//SnmpTarget *target;
	//target = &ctarget;
	//int status;
	//Snmp snmp(status, 0, false);
	//if (status == SNMP_CLASS_SUCCESS)
	//{
	//	if(( status = snmp.get_bulk(pdu, *target, non_reps, max_reps))== SNMP_CLASS_SUCCESS) 
	//	{
	//		for(int z = 0; z < pdu.get_vb_count(); z++) 
	//		{
	//			pdu.get_vb( vb,z);
	//			string oid_tmp  = vb.get_printable_oid();
	//			if(oid_tmp.substr(0, oid.length()) == oid) 
	//			{
	//				string value_tmp = vb.get_printable_value();
	//				m_mib_table.push_back(make_pair(oid_tmp, value_tmp));
	//			}
	//			if ( vb.get_syntax() == sNMP_SYNTAX_ENDOFMIBVIEW) 
	//			{
	//				cout << "End of MIB view.\n\n";
	//			}
	//		}
	//	}
	//	else
	//	{
	//		cout << "SNMP++ GetBulk Error, " << snmp.error_msg( status) << "\n";
	//		SetLastError(status);
	//	}
	//}
	//else
	//{
	//	cout << "SNMP++ Session Create Fail, " << snmp.error_msg(status) << "\n";
	//	SetLastError(status);
	//}
	//Snmp::socket_cleanup();  // Shut down socket subsystem
	return m_mib_table;
}

const list<pair<string,string> > & SnmpDG::GetMibTable(const snmp_version  version, const SnmpPara& spr, const string oid) const
{
	m_mib_table.clear();
	if(!m_inited_success)
	{
		return m_mib_table;
	}
	Snmp::socket_startup();  // Initialize socket subsystem

	UdpAddress address( spr.ip.c_str());
	Oid myoid(oid.c_str());      // default is sysDescr
	if( !myoid.valid())
	{// check validity of user oid
		return m_mib_table;
	}
	Pdu pdu;                           // construct a Pdu object
	Vb vb;                             // construct a Vb object(SNMP++ Variable Binding)
	vb.set_oid(myoid);//oid.c_str());
	pdu += vb;

	CTarget ctarget(address); 
	ctarget.set_version( version );
	ctarget.set_retry(spr.retry);           
	ctarget.set_timeout(spr.timeout); 
	ctarget.set_readcommunity(spr.community.c_str());
	SnmpTarget *target;
	target = &ctarget;
	int status;
	Snmp snmp(status, 0, false);
	if (status == SNMP_CLASS_SUCCESS)
	{
		DWORD dwStartTime_ttl = GetTickCount();
	   	DWORD dwStartTime = 0, dwEndTime = 0;
		while ( (status = snmp.get_next(pdu, *target)) == SNMP_CLASS_SUCCESS) //get_next命令是按列遍历oid
		{
			pdu.get_vb(vb, 0);
			Oid oid_tmp = vb.get_oid();//.get_printable_oid();该表项的oid
			string str_oid_tmp = oid_tmp.get_printable();
			if(oid_tmp.nCompare(myoid.len(), myoid) != 0)//判断是否已越界，如果是则结束循环
			{
				break;
			}

			bool bNew = true;
			string value_tmp = vb.get_printable_value();
                        for(std::list<pair<string,string> >::iterator item = m_mib_table.begin();
				item != m_mib_table.end();
				item++)
			{
				if(item->first == str_oid_tmp)//value_tmp)//去掉重复的oid
				{
					bNew = false;
					cout << "ip:" + spr.ip + ",repeat oid:" + str_oid_tmp;
					break;
				}
			}
			if(bNew)
			{
				dwStartTime = GetTickCount();
				m_mib_table.push_back(make_pair(str_oid_tmp,value_tmp));
				vb.set_oid(oid_tmp);
				vb.set_null();
				pdu.set_vb(vb,0);
			}
			else
			{
 			//	if ( GetTickCount() - dwStartTime > 10000 ) //10s
				//{
				//	cout << "Timeout because read repeat data " << endl;
					break;
				//}
			}
			if ( GetTickCount() - dwStartTime_ttl > 300000 )  //5min
			{
				cout << "Timeout because read operation " << endl;
				break;
			}
		}
	}
	else
	{
		cout << "SNMP++ Session Create Fail, " << snmp.error_msg(status) << "\n";
		SetLastError(status);
	}
	Snmp::socket_cleanup();  // Shut down socket subsystem
	return m_mib_table;
}
