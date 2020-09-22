// HIDDescriptor.h: interface for the HIDDescriptor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HIDDESCRIPTOR_H__675580CB_BA5F_4F69_B226_9674F46A7F48__INCLUDED_)
#define AFX_HIDDESCRIPTOR_H__675580CB_BA5F_4F69_B226_9674F46A7F48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define HID_STR_LEN	256

#include "stdafx.h"

#ifdef _WIN32
extern "C" {
#include "hidsdi.h"
#include <setupapi.h>
}
#else
#include <asm/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/input.h>
#include <linux/hiddev.h>
#endif

#define MY_HIDDEV

/*
//just for testing
struct hiddev_devinfo
{
	unsigned short vendor;
	unsigned short product;
	unsigned short version;
	int num_applications;
    int busnum;
	int devnum;
	int ifnum;
};

struct hiddev_string_descriptor
{
	char value[HID_STR_LEN];
};*/

class HIDDescriptor  
{
public:
	HIDDescriptor();
	virtual ~HIDDescriptor();

	const char* GetHIDPath();
	const int GetINLength(unsigned char mesg);
	const int GetOUTLength(unsigned char mesg);
	
#ifndef _WIN32
	void Reset();
#endif

	char								m_sManufacturer[HID_STR_LEN];
	char								m_sProduct[HID_STR_LEN];
	unsigned short						m_nVendor;
	unsigned short						m_nPID;

#ifdef _WIN32
	PSP_DEVICE_INTERFACE_DETAIL_DATA    m_interfaceDetailData;
	DWORD                               m_interfaceDetailDataSize;
	HIDP_CAPS							m_Capabilities;
#else
	char								m_sPath[1024];

#ifdef MY_HIDDEV
	struct hiddev_devinfo				m_deviceInfo;
	
	unsigned char 						m_inputLenghts[256];
	unsigned char						m_outputLengths[256];		
#else
	struct input_id		 				m_deviceInfo;
#endif
#endif
};

#endif // !defined(AFX_HIDDESCRIPTOR_H__675580CB_BA5F_4F69_B226_9674F46A7F48__INCLUDED_)
