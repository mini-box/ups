#include "stdafx.h"

#include "HThread.h"
#include <limits.h>

#ifndef _WIN32
#define	INVALID_HANDLE_VALUE	0
#endif

const unsigned int HThread::INFINIT_WAIT = UINT_MAX;

#ifdef _WIN32
DWORD WINAPI runStub( LPVOID mthread )
#else
void* runStub( void* mthread)
#endif
{
	HThread* pThread = static_cast<HThread* >(mthread);
	pThread->m_bIsRunning = true;
	pThread->m_bExited = false;
	pThread->run();
	pThread->m_bIsRunning = false;
	pThread->m_bExited = true;
#ifdef _WIN32
	return 0;
#else
	return NULL;
#endif
}

HThread::HThread( )
{
	m_bIsRunning = false;
	m_bExited = true;
	m_thread = INVALID_HANDLE_VALUE;
}

HThread::~HThread()
{
	stop();
}

bool HThread::start( void )
{
	if( m_bExited )
	{
		m_bIsRunning = true;
		m_bExited = false;

#ifdef _WIN32
		DWORD dw;
		if( (m_thread = CreateThread( NULL, 4096, runStub, this, 0, &dw )) == INVALID_HANDLE_VALUE )
		{
			m_bIsRunning = false;
			m_bExited = true;
			return false;
		}
#else
		pthread_attr_t attr;
		pthread_attr_init( &attr );
		pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
		pthread_attr_setschedpolicy( &attr, SCHED_OTHER );
		pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
		if( pthread_create( &m_thread, &attr, runStub, this ) != 0 )
		{
			m_thread = INVALID_HANDLE_VALUE;
			m_bIsRunning = false;
			m_bExited = true;
			return false;
		}
#endif
	}
	return true;
}

bool HThread::stop( unsigned int timeout )
{
	m_bIsRunning = false;
	if( !m_bExited )
	{
		for( unsigned int i = 0; (i <= timeout/10) || (timeout == INFINIT_WAIT); i++)
		{
			if( m_bExited )
			{
				break;
			}
			Sleep( 10 );
		}
	}
#ifdef _WIN32
	if( m_thread != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_thread );
		m_thread = INVALID_HANDLE_VALUE;
	}
#else
	m_thread = INVALID_HANDLE_VALUE;
#endif
	return m_bExited;
}

void HThread::run( void )
{
}

