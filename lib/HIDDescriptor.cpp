// HIDDescriptor.cpp: implementation of the HIDDescriptor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HIDDescriptor.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HIDDescriptor::HIDDescriptor()
{
	m_sManufacturer[0] = 0;
	m_sProduct[0] = 0;
	m_nVendor	= -1;
	m_nPID		= -1;
#ifdef _WIN32
	m_interfaceDetailData = NULL;
#else
	m_sPath[0] = 0;
	
#ifdef MY_HIDDEV	
	int i;
	for (i = 0;i<256;i++)
	{
		m_inputLenghts[i] = 0xff;
		m_outputLengths[i]= 0xff;		
	}
#endif

#endif
}

HIDDescriptor::~HIDDescriptor()
{
#ifdef _WIN32
	if (m_interfaceDetailData) free(m_interfaceDetailData);
#endif
}

#ifndef _WIN32
void HIDDescriptor::Reset()
{
#ifdef MY_HIDDEV
	int i;
	for (i = 0;i<256;i++)
	{
		m_inputLenghts[i] = 0xff;
		m_outputLengths[i]= 0xff;		
	}	
#endif	
}
#endif

const char* HIDDescriptor::GetHIDPath()
{
#ifdef _WIN32
	return m_interfaceDetailData->DevicePath;
#else
	return m_sPath;
#endif
}

const int HIDDescriptor::GetINLength(unsigned char mesg)
{
#ifdef _WIN32
	return m_Capabilities.InputReportByteLength;
#else
#ifdef MY_HIDDEV
	return m_inputLenghts[mesg]+1;
#else
	return 1;
#endif
#endif
}

const int HIDDescriptor::GetOUTLength(unsigned char mesg)
{
#ifdef _WIN32
	return m_Capabilities.OutputReportByteLength;
#else
#ifdef MY_HIDDEV
	return m_outputLengths[mesg]+1;
#else
	return 1;
#endif
#endif
}

