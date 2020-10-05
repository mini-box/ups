
#ifndef __H_THREAD_H__
#define __H_THREAD_H__


#ifndef _WIN32
#include <pthread.h>
typedef pthread_t HANDLE;
#define Sleep(x)
#endif

#ifdef HIDAPI_EXPORTS
class HIDAPI_API HThread 
#else
class HThread
#endif
{
public:
	HThread( );
	virtual ~HThread();
	virtual bool start( void );
	virtual bool stop( unsigned int timeout = 5000 );
	inline bool& isRunning( void )
	{
		return m_bIsRunning;
	}

protected:
	virtual void run( void );

private:
#ifdef _WIN32
	friend DWORD WINAPI runStub( LPVOID mthread );
#else
	friend void* runStub( void* mthread );
#endif

public:
	static const unsigned int INFINIT_WAIT;

private:
	bool m_bIsRunning, m_bExited;
	HANDLE m_thread;
};

#endif //__H_THREAD_H__
