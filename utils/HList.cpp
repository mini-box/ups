#include "util.h"
#include "HList.h"

HList::HList()
{
	m_ptmp = m_pend = m_phead = NULL;
	m_elements = 0;
}

HList::~HList()
{
	removeall();
}

void HList::removeall()
{
	m_lock.lock();

	ITEM* ptmp = getHead();
	ITEM* ptemp;
	while( ptmp != NULL )
	{
		ptemp = ptmp;
		ptmp = ptmp->pnext;
		delete ptemp;
	}

	m_ptmp = m_pend = m_phead = NULL;
	m_elements = 0;

	m_lock.unlock();
}

bool HList::add( void* elem )
{
	ITEM* ptemp = new ITEM();
	
	ptemp->pdata = elem;
	ptemp->pnext = NULL;
	
	if( !m_lock.lock() )
	{
		delete ptemp;
		return false;
	}
	if( m_phead == NULL )
	{
		m_pend = m_phead = ptemp;
	}
	else
	{
		m_pend->pnext = ptemp;
		m_pend = ptemp;
	}
	m_elements++;
	m_lock.unlock();
	return true;
}

bool HList::remove( void* elem )
{
	if( !m_lock.lock() )
	{
		return false;
	}
	bool b = removeWithNoLocking( elem );
	m_lock.unlock();
	return b;
}

bool HList::removeWithNoLocking( void* elem )
{
	bool bFound = true;
	ITEM* ptemp = m_phead;
	ITEM* ptmp = NULL;
	while( ptemp!=NULL )
	{
		if( ptemp->pdata == elem )
		{
			break;
		}
		ptmp = ptemp;
		ptemp = ptemp->pnext;
	}
	if( ptemp == NULL )					//No element was found found 
	{
		bFound = false;
	}
	else if( m_pend == m_phead )		//List containing just the founded element
	{
		m_pend = m_phead = NULL;
	}
	else if( ptemp == m_phead )			//Founded element is the first element
	{
		m_phead = ptemp->pnext;
	}
	else if( ptemp == m_pend )			//Founded element is the last element
	{
		ptmp->pnext = NULL;
		m_pend = ptmp;
	}
	else								//Founded element is in the middle of the list
	{
		ptmp->pnext = ptemp->pnext;
	}
	if( bFound )
	{
		delete ptemp;
		m_elements--;
	}
	return bFound;
}

void HList::removeElement( ITEM* t, ITEM* t1 )
{
	//head of the list
	if (t1==NULL) return;

	m_elements--;

	if( t == NULL )
	{
		m_phead = t1->pnext;
		//only one element in list
		if ((m_phead!=NULL)&&( m_phead->pnext == NULL ))
			m_pend = m_phead;
		delete t1;
		return;
	}

	t->pnext = t1->pnext;

	//end of the list
	if( t->pnext == NULL )
	{
		m_pend = t;
	}
	delete t1;
}

bool HList::exist( void* data )
{
	if( !m_lock.lock() )
	{
		return false;
	}
	bool bFound = false;
	ITEM* ptemp = m_phead;
	while( ptemp != NULL )
	{
		if( data == ptemp->pdata )
		{
			bFound = true;
			break;
		}
		ptemp = ptemp->pnext;
	}
	m_lock.unlock();
	return bFound;
}

bool HList::lock( void )
{
	return m_lock.lock();
}

void HList::unlock( void )
{
	m_lock.unlock();
}

ITEM* HList::getHead( void )
{
	return m_phead;
}

void HList::moveToHead( void )
{
	m_ptmp = m_phead;
}

void* HList::nextElement( void )
{
	void* pdata = NULL;
	if( m_ptmp != NULL )
	{
		pdata = m_ptmp->pdata;
		m_ptmp = m_ptmp->pnext;
	}
	return pdata;
}

void* HList::getData( void )
{
	void* pdata = NULL;
	if( m_ptmp != NULL )
	{
		pdata = m_ptmp->pdata;
		m_ptmp = m_ptmp->pnext;
	}
	return pdata;
}

void* HList::nextElement( ITEM** t)
{
	if (*t == getHead()) moveToHead();
	
	void* data = nextElement();

	*t = m_ptmp;

	return data;
}
