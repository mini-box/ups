// HArray.h: interface for the HArray class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HARRAY_H__AE52A320_3A09_4E75_8279_5BDD166D2C27__INCLUDED_)
#define AFX_HARRAY_H__AE52A320_3A09_4E75_8279_5BDD166D2C27__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef HIDAPI_EXPORTS
class HIDAPI_API HArray  
#else
class HArray
#endif
{
public:
	HArray();
	HArray(char* array);
	//HArray(unsigned short* array);
	HArray(unsigned char* array, int len);
	HArray(int len);
	HArray(HArray* array);

	virtual ~HArray();

	unsigned char getAt(int i);
	unsigned char* getBuf(){return m_pArray;};
	const unsigned int length(){return m_nLen;};
	bool equals(HArray* array);
	bool equals(HArray* array, unsigned int to);
	bool equalsSpecial(HArray* array, unsigned int to);
	
	void erase();

	void setValue(HArray* array);
	void setValue(char* array);
	//void setValue(unsigned short* array);
	void setValue(unsigned char* array, int len);

	void setHIDPkt(unsigned char rid, unsigned int rlen, unsigned int len, ...);

private:
	unsigned char* m_pArray;
	unsigned int   m_nLen;
};

#endif // !defined(AFX_HARRAY_H__AE52A320_3A09_4E75_8279_5BDD166D2C27__INCLUDED_)
