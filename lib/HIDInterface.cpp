// HIDInterface.cpp: implementation of the HIDInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HIDInterface.h"
#include "HIDDevice.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HIDInterface::HIDInterface(HIDDevice* hdev)
{
	m_pParent = hdev;
	if (m_pParent) m_pParent->setParser(this);
}

HIDInterface::~HIDInterface()
{
	if (m_pParent) m_pParent->setParser(NULL);
}

void HIDInterface::OpenDevice()
{
	if (m_pParent) m_pParent->OpenDevice();
}

void HIDInterface::CloseDevice()
{
	if (m_pParent) m_pParent->CloseDevice();
}
