#include "util.h"
#ifndef _WIN32
#include <stdarg.h>
#endif
#include "HArray.h"


HArray::HArray()
{
	m_pArray = NULL;m_nLen = 0;
}

HArray::HArray(char* array)
{
	m_pArray = NULL;m_nLen = 0;
	setValue(array);
}

HArray::HArray(unsigned char* array, int len)
{
	m_pArray = NULL;m_nLen = 0;
	setValue(array, len);
}

HArray::HArray(HArray* array)
{
	m_pArray = NULL;m_nLen = 0;
	setValue(array->getBuf(), array->length());
}

/*HArray::HArray(unsigned short* array)
{
	m_pArray = NULL;m_nLen = 0;
	setValue(array);
}*/

HArray::HArray(int len)
{
	m_nLen = len;
	m_pArray = new unsigned char[len];
	memset(m_pArray, 0, len);
}

HArray::~HArray()
{
	if (m_pArray) delete m_pArray;
}

void HArray::erase()
{
	if (m_pArray) delete m_pArray;
	m_pArray = NULL;m_nLen = 0;
}

bool HArray::equals(HArray* array)
{
	if (this->m_nLen != array->length()) return false;

	for (unsigned int i = 0;i<this->m_nLen;i++)
		if (this->m_pArray[i] != array->getBuf()[i]) return false;

	return true;
}

bool HArray::equals(HArray* array, unsigned int to)
{
	if (this->m_nLen < to) return false;
	if (array->length() < to) return false;

	for (unsigned int i = 0;i<to;i++)
		if (this->m_pArray[i] != array->getBuf()[i]) return false;

	return true;
}

bool HArray::equalsSpecial(HArray* array, unsigned int to)
{
	if (this->m_nLen < to) return false;
	if (array->length() < to) return false;

	for (unsigned int i = 1;i<to;i++)
		if (this->m_pArray[i] != array->getBuf()[i]) return false;

	return true;
}

void HArray::setValue(HArray* array)
{
	setValue(array->getBuf(), array->length());
}

void HArray::setValue(char* array)
{
	if (m_pArray) delete m_pArray;

	m_nLen = strlen(array)+1;
	m_pArray = new unsigned char[m_nLen];
	strcpy((char*)m_pArray, array);
}

/*void HArray::setValue(unsigned short* array)
{
	if (m_pArray) delete m_pArray;

	m_nLen = 2*wcslen(array)+2;
	m_pArray = new unsigned char[m_nLen];
	wcscpy((unsigned short*)m_pArray, array);
}*/

void HArray::setValue(unsigned char* array, int len)
{
	if (m_pArray) delete m_pArray;

	m_nLen = len;
	if (len>0)
	{
		m_pArray = new unsigned char[len];
		memcpy(m_pArray, array, len);
	}
	else m_pArray = NULL;
}

void HArray::setHIDPkt(unsigned char rid, unsigned int rlen, unsigned int len, ...)
{
	if (m_pArray) delete m_pArray;
	m_nLen = rlen + 1;
	m_pArray = new unsigned char[rlen+1];
	m_pArray[0] = rid;
	va_list args;
	va_start(args, len);
	unsigned int cnt = 0;
#ifdef _WIN32	
	unsigned char i = va_arg(args, unsigned char);
#else
	unsigned char i = va_arg(args, unsigned int);
#endif
	while (cnt<len)
	{
		m_pArray[1+cnt] = i;
		cnt++;
		if (cnt < len)
#ifdef _WIN32			
		 	i = va_arg(args, unsigned char);
#else
			i = va_arg(args, unsigned int);
#endif
	}
	
	va_end(args);
}

unsigned char HArray::getAt(int i)
{
	if ((m_nLen <= i)||(!m_pArray)) return -1;

	return m_pArray[i];
}
