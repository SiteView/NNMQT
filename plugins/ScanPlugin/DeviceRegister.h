#pragma once

#include <string>
#include "DeviceFactory.h"

#include "UnivDevice.h"
#include "CiscoDevice.h"
#include "HuaWeiDevice.h"
#include "H3CDevice.h"

using namespace std;

const string DEVICE_UNIV = "0.0.0.0.0.0.0";
const string DEVICE_CISCO = "1.3.6.1.4.1.9";
const string DEVICE_HUAWEI = "1.3.6.1.4.1.2011";
const string DEVICE_H3C = "1.3.6.1.4.1.25506";

//Add new device factory oid at here.....

class DeviceRegister
{
public:
	DeviceRegister(void);
public:
	~DeviceRegister(void);
public:
	static bool registerDevice();
};
