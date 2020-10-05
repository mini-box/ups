
#ifndef __H_LOCK_H__
#define __H_LOCK_H__

#ifndef _WIN32

#include <pthread.h>
typedef pthread_mutex_t CRITICAL_SECTION;
#endif

#ifdef HIDAPI_EXPORTS
class HIDAPI_API HLock
#else
class HLock
#endif
{
public:
	HLock();
	virtual ~HLock();
	bool lock( void );
	void unlock( void );
	void destroy( void );

private:
	CRITICAL_SECTION m_mutex;
#ifndef _WIN32
	bool m_bUnderDestroy;
#endif
};

#endif //__H_LOCK_H__
