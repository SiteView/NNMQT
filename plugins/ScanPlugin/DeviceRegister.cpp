#include "StdAfx.h"
#include "DeviceRegister.h"

namespace
{
	Device* CreateUnivDevice()
	{
		return new UnivDevice;
	}
	Device* CreateCiscoDevice()
	{
		return new CiscoDevice;
	}
	//add by zhangyan 2008-10-6
	Device* CreateHuaWeiDevice()
	{
		return new HuaWeiDevice;
	}
	Device* CreateH3CDevice()
	{
		return new H3CDevice;
	}

	//Add new device constructor at here.....

}

DeviceRegister::DeviceRegister(void)
{
}

DeviceRegister::~DeviceRegister(void)
{
}

bool DeviceRegister::registerDevice()
{
	DeviceFactory::Instance()->RegisterDevice(DEVICE_UNIV, CreateUnivDevice);
	DeviceFactory::Instance()->RegisterDevice(DEVICE_CISCO, CreateCiscoDevice);
	////add by zhangyan 2008-10-6
	//DeviceFactory::Instance()->RegisterDevice(DEVICE_HUAWEI, CreateHuaWeiDevice);
	//DeviceFactory::Instance()->RegisterDevice(DEVICE_H3C, CreateH3CDevice);

	//Add new device register at here.....

	return true;
}
