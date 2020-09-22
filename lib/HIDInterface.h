// HIDInterface.h: interface for the HIDInterface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HIDINTERFACE_H__27FBB2E6_FCE2_4860_9AFB_C27E5C6DA74E__INCLUDED_)
#define AFX_HIDINTERFACE_H__27FBB2E6_FCE2_4860_9AFB_C27E5C6DA74E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HArray.h"
class HIDDevice;

class HIDInterface
{
public:
	HIDInterface(HIDDevice* hdev);
	virtual ~HIDInterface();

	virtual void Received(HArray* array)=0;
	virtual void Disconnected()=0;

	void OpenDevice();
	void CloseDevice();

protected:
	HIDDevice* m_pParent;
};

#endif // !defined(AFX_HIDINTERFACE_H__27FBB2E6_FCE2_4860_9AFB_C27E5C6DA74E__INCLUDED_)
