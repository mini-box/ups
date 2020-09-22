
#ifndef __H_LIST_H__
#define __H_LIST_H__

#include "HLock.h"

#ifdef HIDAPI_EXPORTS
struct HIDAPI_API Item
#else
struct Item
#endif
{
	void* pdata;
	struct Item* pnext;
};

typedef struct Item ITEM;

#ifdef HIDAPI_EXPORTS
class HIDAPI_API HList
#else
class HList
#endif
{
public:
	HList();
	virtual ~HList();
	inline int& elements( void ){return m_elements;}
	bool add( void* elem );
	bool remove( void* elem );
	bool removeWithNoLocking( void* elem );
	bool lock( void );
	void unlock( void );
	void moveToHead( void );
	ITEM* getHead( void );
	bool exist( void* data );
	void* nextElement( void );
	void* nextElement( ITEM** t);
	void removeElement( ITEM* parent, ITEM* t );
	void* getData( void );

	void removeall();

private:

	ITEM* m_phead;
	ITEM* m_pend;
	ITEM* m_ptmp;
	HLock m_lock;
	int m_elements;
};
#endif //__H_LIST_H__
