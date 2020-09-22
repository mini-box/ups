#include "DeviceDetector.h"
#include <stdio.h>

static DeviceDetector* m_pDevDetector = NULL;

DeviceDetector::DeviceDetector() {}

DeviceDetector::~DeviceDetector() {}

#ifdef _WIN32
#define STR_HIDDEN TEXT("hidden_ups12v")

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
    	{
		case WM_CREATE	:
				return 0;
		case WM_DESTROY :
				PostQuitMessage(0);
				return 0;
		case WM_DEVICECHANGE:
			{
				if (m_pDevDetector)
					m_pDevDetector->deviceChanged();
            
			    return 0;
			}
     }
     return DefWindowProc (hwnd, message, wParam, lParam) ;
}

DWORD _RegisterDeviceDetectorClass(HINSTANCE hInstance)
{
	WNDCLASS   wndclass;
    
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 16;
	wndclass.cbWndExtra    = 16;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = STR_HIDDEN;
	if (RegisterClass (&wndclass) == 0) 
		return FALSE;

	return TRUE;	
}

DWORD WINAPI _HDWWorker(PVOID pData)
{
#ifdef _OUTLOG
    OutLog(true, "DeviceDetector Thread started\n");
#endif

	MSG 	msg;	
	HWND	hWnd;

	// Create tool window with no caption so that it does not show up in the task mgr or in the task bar 
	hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, STR_HIDDEN, NULL, 
				    WS_POPUP, 
				    0, 0, 0, 0, NULL, 
				    NULL, GetModuleHandle(NULL), NULL);
    	
	if (hWnd == NULL)
	{
#ifdef _OUTLOG
		OutLog(true, "DeviceDetector Thread failed\n");
#endif
		ExitThread(0);
	}
#ifdef _OUTLOG
	OutLog(true, "DeviceDetector Thread ok %d\n",(unsigned long)hWnd);
#endif

	// Pass the hWnd back to the creator
	((DeviceDetector *)pData)->m_hHDWDeviceDetector = hWnd;

#ifdef _OUTLOG
	OutLog(true, "  window=%d\n",(unsigned long)hWnd);
#endif

	GUID oGUID_HID;
    HidD_GetHidGuid(&oGUID_HID);

	if(!((DeviceDetector *)pData)->DoRegisterDeviceInterface(hWnd,oGUID_HID, &((DeviceDetector *)pData)->m_hHDWDevNotify))
	{
#ifdef _OUTLOG
		OutLog(true, "DoRegisterDeviceInterface failed\n");
#endif
	}
	else
	{
#ifdef _OUTLOG
		OutLog(true, "DoRegisterDeviceInterface succeeded\n");
#endif
	}
	
	// Go into a message loop ....
	while (GetMessage (&msg, NULL, 0, 0))
    {
      	TranslateMessage (&msg);
        DispatchMessage (&msg);		  
    }

#ifdef _OUTLOG
    OutLog(true, "DeviceDetector Thread exited\n");
#endif

    return 0;
}

///////////////// class
DeviceDetector::DeviceDetector()
{
#ifdef _OUTLOG
	OutLog(true, "Registering hidden window\n");
#endif

	m_pDevDetector = this;

	_RegisterDeviceDetectorClass(GetModuleHandle(NULL));

	m_hHDWThread = CreateThread(NULL, 0, _HDWWorker, this, 0, &m_dwHDWThreadId);
	if (m_hHDWThread == NULL)
	{
#ifdef _OUTLOG
		OutLog(true, "  window reg failed\n",(unsigned long)GetLastError());
#endif
	}
}

DeviceDetector::~DeviceDetector()
{
#ifdef _OUTLOG
	OutLog(true, "Unregistering device notification\n");
#endif
	UnregisterDeviceNotification(&m_hHDWDevNotify);

#ifdef _OUTLOG
	OutLog(true, "Unregistering hidden window\n");
#endif
	if (m_hHDWDeviceDetector != NULL)
		SendMessage(m_hHDWDeviceDetector, WM_QUIT, 0, 0);

	m_pDevDetector = NULL;
}

BOOL DeviceDetector::DoRegisterDeviceInterface( 
    HWND hWnd,
    GUID InterfaceClassGuid, 
    HDEVNOTIFY *hDevNotify 
)
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = 
        sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = InterfaceClassGuid;

    *hDevNotify = RegisterDeviceNotification( hWnd, 
        &NotificationFilter,
        DEVICE_NOTIFY_WINDOW_HANDLE
    );

    if(!*hDevNotify) 
    {
#ifdef _OUTLOG
        OutLog(true, "RegisterDeviceNotification failed: %d\n", GetLastError());
#endif
        return FALSE;
    }

    return TRUE;
}
#endif//_WIN32