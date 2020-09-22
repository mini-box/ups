// HIDDevices.h: interface for the HIDDevices class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HIDDEVICES_H__91EBA3B7_8961_423F_95F0_25F998DD9F17__INCLUDED_)
#define AFX_HIDDEVICES_H__91EBA3B7_8961_423F_95F0_25F998DD9F17__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HIDDevice.h"
#include "HList.h"
#include "HArray.h"
class HIDUPS12V;
class HIDGraphics;

#define ALL_HID		0
#define CHOOSE_HID	1
#define MY_HID		2

#define EXTENDED_LOGGING

class HIDDevices
{
public:
	HIDDevices();
	virtual ~HIDDevices();

	void enumerateHIDDevices();

#ifndef _WIN32
	void setRootPath(const char* path);
#endif
	
	HIDDescriptor* chooseHIDDevice(const char* manufact, const char* prod_lcd, const char* prod_flash, const unsigned short vendor, const unsigned short PID, int devno, int nReadLen, int nWriteLen);
	HIDDevice* openDevice(HIDDescriptor* hdesc);
//	HIDAcme* openAcme(HIDDescriptor* hdesc, HIDGraphics* gp);

private:
#ifdef _WIN32
	bool GetDeviceProperty(HDEVINFO IntDevInfo2, GUID* guid, DWORD Index , int type , HList* path_list, const char* smanufact, const char* sprod_lcd, const char* sprod_flash, const unsigned short nvendor, const unsigned short nPID, int nReadLen, int nWriteLen);
#else
	bool GetDeviceProperty(int type , HList* path_list = NULL, const char* smanufact = NULL, const char* sproduct = NULL, const unsigned short nvendor=-1, const unsigned short nPID=-1);

	char m_rootPath[1024];
#endif
};

#endif // !defined(AFX_HIDDEVICES_H__91EBA3B7_8961_423F_95F0_25F998DD9F17__INCLUDED_)
