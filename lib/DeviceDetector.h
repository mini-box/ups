#ifndef _DEVICE_DETECTOR_H_
#define _DEVICE_DETECTOR_H_

class DeviceDetector
{
public:
	DeviceDetector();
	virtual ~DeviceDetector();

	virtual void deviceChanged()=0; //called when something changed in the usb device list

#ifdef _WIN32
	BOOL DoRegisterDeviceInterface(HWND hWnd,GUID InterfaceClassGuid,HDEVNOTIFY *hDevNotify);

	HANDLE m_hHDWThread;
	DWORD  m_dwHDWThreadId;
	HWND   m_hHDWDeviceDetector;
	HDEVNOTIFY m_hHDWDevNotify;
#endif
};

#endif //_DEVICE_DETECTOR_H_
