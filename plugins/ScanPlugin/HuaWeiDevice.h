#pragma once
#include "UnivDevice.h"

class HuaWeiDevice : virtual public UnivDevice
{
public:
	HuaWeiDevice(void);
	~HuaWeiDevice(void);

	virtual DIRECTDATA_LIST getDirectData(SnmpDG& snmp, const SnmpPara& spr);

};