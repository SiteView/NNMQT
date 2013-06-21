//SnmpDG.h

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

#pragma once

#ifndef SNMPDG_H
#define SNMPDG_H

#include "SnmpPara.h"
#include "iostream"
#include "snmp_pp/snmp_pp.h"
#include "list"
#include "assert.h"

using namespace std;

class SnmpDG
{
public:
	SnmpDG(void);
	~SnmpDG(void);

	const int GetLastError() const;
	const string & GetLastErrorMsg() const;

	const string & GetMibObject(const SnmpPara& spr, const string oid) const;
        const list<pair<string,string> > & GetMibTable(const SnmpPara& spr, const string oid) const;

protected:
	void SetLastError(const int error_code) const;

private:
	mutable UdpAddress m_address;
	mutable CTarget m_target;

	bool m_inited_success;
	int non_reps;                 // non repeaters default is 0
	int max_reps;                 // maximum repetitions default is 10
	mutable int m_error_code;
	mutable string m_error_msg;

	mutable string m_mib_object;
	string m_empty_object;
        mutable list<pair<string, string> > m_mib_table;
        list<pair<string, string> > m_empty_table;

        const list<pair<string,string> >& getMibTableByBulk(const snmp_version version, const string oid) const;

public:  // by zhangyan 2009-01-05
	const string & GetMibObject(const snmp_version version, const SnmpPara& spr, const string oid) const;
        const list<pair<string,string> > & GetMibTable(const snmp_version version, const SnmpPara& spr, const string oid) const;

};

#endif
