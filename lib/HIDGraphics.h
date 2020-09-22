// HIDGraphics.h: interface for the HIDGraphics class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HIDGRAPHICS_H__54654349_7A5C_41B7_AA83_B10CF7F6BE3E__INCLUDED_)
#define AFX_HIDGRAPHICS_H__54654349_7A5C_41B7_AA83_B10CF7F6BE3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class HIDGraphics
{
public:
	HIDGraphics();
	virtual ~HIDGraphics();

	virtual void UpdateStatus(bool fromsocket, int type, int n1, int n2)=0;
};

#endif // !defined(AFX_HIDGRAPHICS_H__54654349_7A5C_41B7_AA83_B10CF7F6BE3E__INCLUDED_)
