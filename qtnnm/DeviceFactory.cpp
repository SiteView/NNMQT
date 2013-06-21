#include "stdafx.h"
#include "DeviceFactory.h"

std::auto_ptr<DeviceFactory> DeviceFactory::m_pInstance(NULL);

// Returns 'true' if registration was successful
bool DeviceFactory::RegisterDevice(const string& sysOid, CreateDeviceCallback CreateFn)
{
	return callbacks_.insert(CallbackMap::value_type(sysOid, CreateFn)).second;
}

bool DeviceFactory::UnregisterDevice(const string& sysOid)
{
	return callbacks_.erase(sysOid) == 1;
}

Device* DeviceFactory::CreateDevice(const string& SysOid)
{
	if (callbacks_[SysOid] != NULL)
	{
		return ((callbacks_[SysOid])());
	}	
	return ((callbacks_["0.0.0.0.0.0.0"])());
}
