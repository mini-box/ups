// HIDDevice.cpp: implementation of the HIDDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HIDDevice.h"
#include "HIDDescriptor.h"

#ifndef _WIN32
#include <linux/hiddev.h>
#define INVALID_HANDLE_VALUE -1
#include <unistd.h>
#include <errno.h>
#endif

#ifdef USE_MFC
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
HIDDevice::HIDDevice(HIDDescriptor* hdesc)
{
	m_pInputReport  = NULL;
	m_hidDescriptor = hdesc;
	m_bDisplayTxRx  = true;
	m_bOpened		= false;
	m_bRunning      = false;
	OpenDevice();
}

HIDDevice::~HIDDevice()
{
	CloseDevice();
	delete m_hidDescriptor;
}

bool HIDDevice::isOpened()
{
	return m_bOpened && m_bRunning;
}

#ifndef _WIN32
int HIDDevice::GetLastError()
{
	return errno;
}
#endif

void HIDDevice::OpenDevice()
{
//	printf("Open: %d (%d)\n",this,m_pInputReport);

	TRACE("Opendevice: path=%s\r\n",m_hidDescriptor->GetHIDPath());
	m_pParser = NULL;

#ifdef _WIN32
	m_pInputReport = new unsigned char[m_hidDescriptor->m_Capabilities.InputReportByteLength];
#else
	m_pInputReport = new unsigned char[100];
#endif

#ifdef _WIN32
	m_WriteHandle=CreateFile(
				m_hidDescriptor->GetHIDPath(), 
				GENERIC_WRITE, 
				FILE_SHARE_READ|FILE_SHARE_WRITE, 
				(LPSECURITY_ATTRIBUTES)NULL,
				OPEN_EXISTING, 
				0, 
				NULL);
#else
  	m_WriteHandle = open(m_hidDescriptor->GetHIDPath(), O_WRONLY);//|O_NONBLOCK);//O_RDWR
#endif

#ifdef _WIN32
	if (m_WriteHandle==INVALID_HANDLE_VALUE) 
#else
	if (m_WriteHandle<=INVALID_HANDLE_VALUE) 
#endif
	{
		TRACE("WRITE: Cannot open port %s error=%d\r\n",m_hidDescriptor->GetHIDPath(),GetLastError());
	}
	else
	{
		TRACE("WRITE: Port %s opened\r\n",m_hidDescriptor->GetHIDPath());
	}

#ifndef _OVERLAPPED_
#ifdef _WIN32
	m_ReadHandle=CreateFile(
				m_hidDescriptor->GetHIDPath(), 
				GENERIC_READ|GENERIC_WRITE, 
				FILE_SHARE_READ|FILE_SHARE_WRITE, 
				(LPSECURITY_ATTRIBUTES)NULL,
				OPEN_EXISTING, 
				0, 
				NULL);
#else
	m_ReadHandle = open(m_hidDescriptor->GetHIDPath(), O_RDONLY|O_NONBLOCK);
#endif

#ifdef _WIN32
	if (m_ReadHandle==INVALID_HANDLE_VALUE) 
#else
	if (m_ReadHandle<=INVALID_HANDLE_VALUE) 
#endif
	{
		TRACE("READD: Cannot open port %s error=%d\r\n",m_hidDescriptor->GetHIDPath(),GetLastError());
	}
	else
	{
		TRACE("READD: Port %s opened\r\n",m_hidDescriptor->GetHIDPath());
	}
#else // not overlapped
#ifdef _WIN32
	m_ReadHandle=CreateFile 
		(m_hidDescriptor->GetHIDPath(), 
		GENERIC_READ, 
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES)NULL, 
		OPEN_EXISTING, 
		FILE_FLAG_OVERLAPPED, 
		NULL);
#else
	m_ReadHandle = open(m_hidDescriptor->GetHIDPath(), O_RDONLY|O_NONBLOCK);
#endif

#ifdef _WIN32
	if (m_ReadHandle==INVALID_HANDLE_VALUE) 
#else
	if (m_ReadHandle<=INVALID_HANDLE_VALUE) 
#endif
	{
		TRACE("READO: Cannot open port %s error=%d\r\n",m_hidDescriptor->GetHIDPath(),GetLastError());
	}
	else
	{
		TRACE("READO: Port %s opened\r\n",m_hidDescriptor->GetHIDPath());
#ifdef _WIN32
		m_hEventObject = CreateEvent 
			(NULL, 
			TRUE, 
			TRUE, 
			"");
		TRACE("CreateEvent: %d\n",GetLastError()) ;

		//Set the members of the overlapped structure.
		m_HIDOverlapped.hEvent = m_hEventObject;
		m_HIDOverlapped.Offset = 0;
		m_HIDOverlapped.OffsetHigh = 0;
#endif
	}
#endif

	if (   (m_ReadHandle!=INVALID_HANDLE_VALUE) 
		&& (m_WriteHandle!=INVALID_HANDLE_VALUE) 
		)
		m_bOpened = true;

	m_bRunning = true;

	start();
}

void HIDDevice::CloseDevice()
{
	if (!m_bOpened) return;

	if (m_WriteHandle!=INVALID_HANDLE_VALUE)
	{
#ifdef _WIN32
		CloseHandle(m_WriteHandle);
#else
		close(m_WriteHandle);
#endif
		m_WriteHandle = INVALID_HANDLE_VALUE;
	}

	m_bRunning = false;

#ifdef _WIN32
#ifdef _OVERLAPPED_
	CancelIo(m_ReadHandle);
	SetEvent(m_hEventObject);
#endif
#endif

	if (m_ReadHandle!=INVALID_HANDLE_VALUE)
	{
#ifdef _WIN32
		CloseHandle(m_ReadHandle);
#else
		close(m_ReadHandle);
#endif
		m_ReadHandle = INVALID_HANDLE_VALUE;
	}

	stop();	

#ifdef _WIN32
#ifdef _OVERLAPPED_
	ResetEvent(m_hEventObject);
	CloseHandle(m_hEventObject);
#endif
#endif

	delete m_pInputReport;
	m_pInputReport = NULL;

	m_bOpened = false;
}


void HIDDevice::setParser(HIDInterface* parser)
{
	m_pParser = parser;
}

void HIDDevice::run( void )
{
#ifdef _WIN32
	HArray array(m_hidDescriptor->m_Capabilities.InputReportByteLength);
#else
	HArray array(100);
#endif
	int cnt = 0;
	while (isRunning() && m_bRunning)
	{
		m_nInputReportRead = 0;
		if (Read() && m_nInputReportRead)
		{
			for (unsigned long i=0;i<m_nInputReportRead;i++)
			{
				array.getBuf()[cnt++] = m_pInputReport[i];
				if (cnt == m_hidDescriptor->GetINLength(array.getBuf()[0]) )
				{
					if (m_pParser) 
						m_pParser->Received(&array);
					cnt = 0;
				}
			}
		}
		Sleep(10);
	}
}

bool HIDDevice::Write(HArray* mesg)
{
	
#ifdef _WIN32
	DWORD	BytesWritten = 0;
	ULONG	Result;
#else
	long	BytesWritten = 0;
	long	Result;
#endif

	unsigned short send = m_hidDescriptor->GetOUTLength(mesg->getBuf()[0]);
	//if (mesg->length() == 1) send = 1;

	unsigned char* buf = new unsigned char[send];
	memset(buf, 0, send);
	memcpy(buf, mesg->getBuf(), mesg->length());

#ifdef _DEBUG
	//if (buf[0]!=0x81)
	{
		TRACE("TX[%d]:",send); 
		for (unsigned int i=0;i<send;i++)
			TRACE("%02X ",buf[i]);
		TRACE("\n");
	}
#endif

#ifdef _WIN32
	Result = WriteFile 
		(m_WriteHandle, 
		buf, 
		send, 
		&BytesWritten, 
		NULL);
#else
	Result = write(m_WriteHandle, buf , send);
	BytesWritten = Result;
#endif

	if (Result <= 0)
	{
		unsigned int err = GetLastError();
#ifdef _DEBUG
		TRACE(">>>TX[ERR=%d(%d)]\n",err,send); 
#endif
		if (err == ERROR_DEVICE_NOT_CONNECTED)
		{
			if (m_bOpened)
			{
				m_bOpened = false;
				if (m_pParser) 
					m_pParser->Disconnected();
			}
		}

		delete buf;
		return false;
	}
	else
	{
		delete buf;
		return true;
	}
	return true;
}

bool HIDDevice::Read()
{
	m_nInputReportRead = 0;

#ifndef _OVERLAPPED_
#ifdef _WIN32
	DWORD	Result;
	Result = ReadFile 
		(m_ReadHandle, 
		m_pInputReport, 
		m_hidDescriptor->m_Capabilities.InputReportByteLength, 
		&m_nInputReportRead,
		NULL);
#else
	long	Result;
#ifdef MY_HIDDEV
	Result = read(m_ReadHandle, m_pInputReport , 100);
	m_nInputReportRead = Result;
#else
	Result = read(m_ReadHandle, m_readEvent, sizeof(struct input_event) * 100);
	m_nInputReportRead = Result;
#endif
#endif
	
	if (Result <= 0)
	{
		TRACE("RX: ERROR 1 =%d\n",GetLastError());
		return false;
	}
	else
	{
#ifdef MY_HIDDEV	
		TRACE("RX[%d]:",m_nInputReportRead);
		for (unsigned int i=0;i<m_nInputReportRead;i++)
			TRACE("%02X ",m_pInputReport[i]);
		TRACE("\n");
#else
		for (int yalv = 0; yalv < (int) (m_nInputReportRead / sizeof(struct input_event)); yalv++)
	    {
		    printf("Event[%d]: time %ld.%06ld, type %x, code %x, value %x\n",
			   yalv,
		       m_readEvent[yalv].time.tv_sec, m_readEvent[yalv].time.tv_usec, m_readEvent[yalv].type,
		       m_readEvent[yalv].code, m_readEvent[yalv].value);

	    }
#endif
		return true;
	}

#else//overlapped////////////////////////

#ifdef _WIN32
	DWORD	Result;
	Result = ReadFile 
		(m_ReadHandle, 
		m_pInputReport, 
		m_hidDescriptor->m_Capabilities.InputReportByteLength, 
		&m_nInputReportRead,
		(LPOVERLAPPED) &m_HIDOverlapped); 
#else
	long	Result;
#ifdef MY_HIDDEV
	Result = read(m_ReadHandle, m_pInputReport , 100);
	m_nInputReportRead = Result;
#else
	Result = read(m_ReadHandle, m_readEvent, sizeof(struct input_event) * 100);
	m_nInputReportRead = Result;
#endif	
#endif
/////////////////////
/*	if (m_nInputReportRead)
	{
		printf("Rx[%d]:",m_nInputReportRead);
		for (unsigned int i=0;i<m_nInputReportRead;i++)
			printf("%02X ",m_pInputReport[i]);
		printf("\n");
	}

	if (!Result && GetLastError() == ERROR_IO_PENDING) 
	{
		if (WaitForSingleObject( m_hEventObject, 10) == WAIT_OBJECT_0)
		{
			GetOverlappedResult(m_ReadHandle, &m_HIDOverlapped, &m_nInputReportRead, TRUE);
		}
	}

	if (m_nInputReportRead)
	{
		printf("RX[%d]:",m_nInputReportRead);
		for (unsigned int i=0;i<m_nInputReportRead;i++)
			printf("%02X ",m_pInputReport[i]);
		printf("\n");
	}


	ResetEvent(m_hEventObject);*/
/////////////////////
#ifdef _WIN32
	if (!Result) 
	{ 
		DWORD err = GetLastError();
		if (err == ERROR_IO_PENDING)
		{
			GetOverlappedResult(m_ReadHandle, &m_HIDOverlapped, &m_nInputReportRead, TRUE) ; 
			Sleep(10);
		}
		else if (err == ERROR_DEVICE_NOT_CONNECTED)
		{
			if (m_bOpened)
			{
				m_bOpened = false;
				if (m_pParser) 
					m_pParser->Disconnected();
			}
		}
	}

	if ((m_nInputReportRead) && (m_nInputReportRead < 100) && m_bRunning && m_bDisplayTxRx)
	{
#ifdef _DEBUG
		//if (m_pInputReport[0]!=0x82)
		{
			TRACE("RX[%d]:",m_nInputReportRead);
			for (unsigned int i=0;i<m_nInputReportRead;i++)
				TRACE("%02X ",m_pInputReport[i]);
			TRACE("\n");
		}
#endif
	}
	else m_nInputReportRead = 0;
#else //linux

#ifdef MY_HIDDEV
	if ((m_nInputReportRead) && (m_nInputReportRead < 100) && m_bRunning && m_bDisplayTxRx)
	{
		TRACE("RX[%d]:",m_nInputReportRead);
		for (unsigned int i=0;i<m_nInputReportRead;i++)
			TRACE("%02X ",m_pInputReport[i]);
		TRACE("\n");
	}
	else m_nInputReportRead = 0;
#else
	if ((m_nInputReportRead) && (m_nInputReportRead < 100) && m_bRunning)
	{
		printf("read=%lu\n",m_nInputReportRead);
		for (int yalv = 0; yalv < (int) (m_nInputReportRead / sizeof(struct input_event)); yalv++)
	    {
		    printf("Event[%d]: time %ld.%06ld, type %x, code %x, value %x\n",
			   yalv,
		       m_readEvent[yalv].time.tv_sec, m_readEvent[yalv].time.tv_usec, m_readEvent[yalv].type,
		       m_readEvent[yalv].code, m_readEvent[yalv].value);

	    }
	}else m_nInputReportRead = 0;
#endif
#endif//linux

#ifdef _WIN32   
	ResetEvent(m_hEventObject);
#endif

 	return true;
 ////////////////
	/*
//	if (Result == 0)
//		printf("ReadFile: %d\n",GetLastError()) ;
//	do{
		Result = WaitForSingleObject(m_hEventObject, 10);
 
		switch (Result)
		{
		case WAIT_OBJECT_0:
			{
				GetOverlappedResult(m_ReadHandle, &m_HIDOverlapped, &m_nInputReportRead, FALSE);

				printf("RX[%d]:",m_nInputReportRead);
				for (unsigned int i=0;i<m_nInputReportRead;i++)
					printf("%02X ",m_pInputReport[i]);
				printf("\n");
//				ResetEvent(m_hEventObject);
			}break;
		case WAIT_TIMEOUT:
			{
				Sleep(10);
			}break;
		default:
			{
				printf("Undefined error=%d(%d)\n",Result, GetLastError());
				Sleep(10);
			}break;
		}
//	}while (Result == WAIT_TIMEOUT);

	ResetEvent(m_hEventObject);*/
#endif
}
