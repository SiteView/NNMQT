#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H
#pragma once

#include "stdafx.h"

#include "Device.h"


using namespace std;

class DeviceFactory
{
private:
	DeviceFactory()
	{
		cout<<"object is ["<<this<<"] Do< Construction"<<endl;
	}

public:
	virtual ~DeviceFactory()
	{
		cout<<"object is ["<<this<<"] Do Destruction>"<<endl;
	}

public:
	typedef Device* (*CreateDeviceCallback)();
private:
	typedef std::map<string, CreateDeviceCallback> CallbackMap;

public:
	static DeviceFactory* Instance()
	{
		if(!m_pInstance.get())
		{
			m_pInstance = std::auto_ptr<DeviceFactory>(new DeviceFactory());
		}
		return m_pInstance.get();
	}

	// Returns 'true' if registration was successful
	bool RegisterDevice(const string& sysOid, CreateDeviceCallback CreateFn);
	// Returns 'true' if the ShapeId was registered before
	bool UnregisterDevice(const string& sysOid);
	Device* CreateDevice(const string& sysOid);

private:
	static std::auto_ptr<DeviceFactory> m_pInstance;
	CallbackMap callbacks_;
};
#endif