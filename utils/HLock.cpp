#include "util.h"
#include "HLock.h"

HLock::HLock()
{
#ifdef _WIN32
	InitializeCriticalSection( &m_mutex );
#else
	m_bUnderDestroy = false;
	pthread_mutexattr_t mutexAttr;
	pthread_mutexattr_init( &mutexAttr );
	pthread_mutexattr_settype( &mutexAttr, PTHREAD_MUTEX_ERRORCHECK_NP );
	pthread_mutex_init( &m_mutex, &mutexAttr );
	pthread_mutexattr_destroy( &mutexAttr );
#endif
}

HLock::~HLock()
{
	destroy();
}

bool HLock::lock( void )
{
#ifdef _WIN32
	__try
	{
		EnterCriticalSection( &m_mutex );
	}
	__except( 1 )
	{
		return false;
	}
#else
	pthread_mutex_lock( &m_mutex );
	if (m_bUnderDestroy)
	{
		pthread_mutex_unlock( &m_mutex );
		return false;
	}
#endif
	return true;
}

void HLock::unlock( void )
{
#ifdef _WIN32
	__try
	{
		LeaveCriticalSection( &m_mutex );
	}
	__except( 1 )
	{
	}
#else
	if( !m_bUnderDestroy )
	{
		pthread_mutex_unlock( &m_mutex );
	}
#endif
}

void HLock::destroy( void )
{
#ifdef _WIN32
	__try
	{
		DeleteCriticalSection( &m_mutex );
	}
	__except( 1 )
	{
	}
#else
	m_bUnderDestroy = true;
	pthread_mutex_unlock( &m_mutex );
#endif
}
