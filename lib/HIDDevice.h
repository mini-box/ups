// HIDDevice.h: interface for the HIDDevice class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HIDDEVICE_H__81424E04_F03B_4B48_AAE0_83D4E41C5375__INCLUDED_)
#define AFX_HIDDEVICE_H__81424E04_F03B_4B48_AAE0_83D4E41C5375__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HIDDescriptor.h"
#include "HIDInterface.h"
#include "HArray.h"
#include "HThread.h"

#define _OVERLAPPED_

#define ERROR_DEVICE_NOT_CONNECTED -1

class HIDDevice : protected HThread
{
public:
	HIDDevice(HIDDescriptor* hdesc);
	virtual ~HIDDevice();

	void setParser(HIDInterface* parser);

	bool Write(HArray* mesg);
	virtual void run( void );

	void toggleDisplayTxRx(){m_bDisplayTxRx = !m_bDisplayTxRx;};

	HIDDescriptor* GetDescriptor(){return m_hidDescriptor;};

	friend class HIDInterface;

	bool isOpened();
	void CloseDevice();

protected:
#ifndef _WIN32
	int GetLastError();
#endif
	bool Read();

	void OpenDevice();
	
	HIDDescriptor*	m_hidDescriptor;

	//members for read
#ifdef _WIN32
	HANDLE			m_WriteHandle;
	HANDLE			m_ReadHandle;
#ifdef _OVERLAPPED_
	HANDLE			m_hEventObject;
	OVERLAPPED		m_HIDOverlapped;
#endif
#else //linux
	int			m_WriteHandle;
	int			m_ReadHandle;
#endif
	
	unsigned char*		m_pInputReport;
#ifdef _WIN32
	DWORD			m_nInputReportRead;
#else
	unsigned long 		m_nInputReportRead;
#ifndef MY_HIDDEV
	struct input_event	m_readEvent[100];
	struct input_event	m_writeEvent[100];
#endif
#endif

	HIDInterface*		m_pParser;

	bool				m_bRunning;

	bool				m_bDisplayTxRx;

	bool				m_bOpened;
};

#endif // !defined(AFX_HIDDEVICE_H__81424E04_F03B_4B48_AAE0_83D4E41C5375__INCLUDED_)
