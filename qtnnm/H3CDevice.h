#ifndef H3CDEVICE_H
#define H3CDEVICE_H
#pragma once
#include "HuaWeiDevice.h"

class H3CDevice : virtual public HuaWeiDevice
{
public:
	H3CDevice(void);
	~H3CDevice(void);
};

#endif