// HIDDevices.cpp: implementation of the HIDDevices class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HIDDevices.h"

#ifdef USE_MFC
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HIDDevices::HIDDevices()
{
#ifndef _WIN32
#ifdef MY_HIDDEV
	strcpy(m_rootPath, "/dev/hiddev");
#else
	strcpy(m_rootPath, "/dev/input/event");
#endif
#endif
}

HIDDevices::~HIDDevices()
{

}

#ifdef _WIN32
bool HIDDevices::GetDeviceProperty(HDEVINFO IntDevInfo2, GUID* guid, DWORD Index, int type, HList* path_list, const char* smanufact, const char* sprod_lcd, const char* sprod_flash, const unsigned short nvendor, const unsigned short nPID, int nReadLen, int nWriteLen)
{
	HDEVINFO IntDevInfo = SetupDiGetClassDevs (
                 (LPGUID)guid,
                 NULL,                                   // Enumerator
                 NULL,                                   // Parent Window
                 (DIGCF_PRESENT | DIGCF_INTERFACEDEVICE  // Only Devices present & Interface class
                 ));

	if( IntDevInfo == INVALID_HANDLE_VALUE ) 
	{
        printf( "SetupDiGetClassDevs failed with error: %d\n", GetLastError() );
        return false;
    }

	if (type == ALL_HID)
		printf( "DEVPROP: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n"
		, ((unsigned char*)guid)[3]
		, ((unsigned char*)guid)[2]
		, ((unsigned char*)guid)[1]
		, ((unsigned char*)guid)[0]
		, ((unsigned char*)guid)[5]
		, ((unsigned char*)guid)[4]
		, ((unsigned char*)guid)[7]
		, ((unsigned char*)guid)[6]
		, ((unsigned char*)guid)[8]
		, ((unsigned char*)guid)[9]
		, ((unsigned char*)guid)[10]
		, ((unsigned char*)guid)[11]
		, ((unsigned char*)guid)[12]
		, ((unsigned char*)guid)[13]
		, ((unsigned char*)guid)[14]
		, ((unsigned char*)guid)[15]
		);

    SP_DEVICE_INTERFACE_DATA            interfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    interfaceDetailData = NULL;
	HIDP_CAPS							Capabilities;
    
    HANDLE                              hDevice;
    BOOL                                status = TRUE;
    ULONG                               length = 0,
                                        returned = 0;
    DWORD                               interfaceDetailDataSize,
                                        reqSize,
                                        errorCode, 
                                        i;


    interfaceData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);
	DWORD idx = 0;

	while (idx < 1000)
	{
		status = SetupDiEnumDeviceInterfaces ( 
					IntDevInfo,             // Interface Device Info handle
					0,                      // Device Info data
					(LPGUID)guid,//(LPGUID)&DiskClassGuid, // Interface registered by driver
					idx, //Index,                  // Member
					&interfaceData          // Device Interface Data
					);

		idx ++;

		if ( status == FALSE ) {
			errorCode = GetLastError();
			if ( errorCode == ERROR_NO_MORE_ITEMS ) 
			{
				printf( "No more interfaces %d\n", idx);
				SetupDiDestroyDeviceInfoList(IntDevInfo);
				return false;
			}
			else 
			{
				printf( "SetupDiEnumDeviceInterfaces failed with error: %d\n", errorCode  );
			}

			continue;
		}
        
		//
		// Find out required buffer size, so pass NULL 
		//

		status = SetupDiGetDeviceInterfaceDetail (
					IntDevInfo,         // Interface Device info handle
					&interfaceData,     // Interface data for the event class
					NULL,               // Checking for buffer size
					0,                  // Checking for buffer size
					&reqSize,           // Buffer size required to get the detail data
					NULL                // Checking for buffer size
					);

		//
		// This call returns ERROR_INSUFFICIENT_BUFFER with reqSize 
		// set to the required buffer size. Ignore the above error and
		// pass a bigger buffer to get the detail data
		//

		if ( status == FALSE ) {
			errorCode = GetLastError();
			if ( errorCode != ERROR_INSUFFICIENT_BUFFER ) 
			{
				printf( "SetupDiGetDeviceInterfaceDetail failed with error: %d\n", errorCode   );
				continue;
			}
		}

		//
		// Allocate memory to get the interface detail data
		// This contains the devicepath we need to open the device
		//

		interfaceDetailDataSize = reqSize;
		interfaceDetailData = (_SP_DEVICE_INTERFACE_DETAIL_DATA_A *)malloc (interfaceDetailDataSize);
		if ( interfaceDetailData == NULL ) 
		{
			printf( "Unable to allocate memory to get the interface detail data.\n" );
			continue;
		}
		interfaceDetailData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);

		status = SetupDiGetDeviceInterfaceDetail (
					  IntDevInfo,               // Interface Device info handle
					  &interfaceData,           // Interface data for the event class
					  interfaceDetailData,      // Interface detail data
					  interfaceDetailDataSize,  // Interface detail data size
					  &reqSize,                 // Buffer size required to get the detail data
					  NULL);                    // Interface device info

		if ( status == FALSE ) 
		{
			printf( "Error in SetupDiGetDeviceInterfaceDetail failed with error: %d\n", GetLastError() );

			free (interfaceDetailData);
			continue;
		}

		//
		// Now we have the device path. Open the device interface
		// to send Pass Through command

		hDevice = CreateFile(
					interfaceDetailData->DevicePath,    // device interface name
					GENERIC_READ | GENERIC_WRITE,       // dwDesiredAccess
					FILE_SHARE_READ | FILE_SHARE_WRITE, // dwShareMode
					NULL,                               // lpSecurityAttributes
					OPEN_EXISTING,                      // dwCreationDistribution
					0,                                  // dwFlagsAndAttributes
					NULL                                // hTemplateFile
					);
            
		//
		// We have the handle to talk to the device. 
		// So we can release the interfaceDetailData buffer
		//

		printf("[%d]\n", idx);
		
		if (hDevice == INVALID_HANDLE_VALUE) 
		{
#ifndef EXTENDED_LOGGING
			if (type == MY_HID) 
#endif
				printf("   CreateFile failed with error: %d (%s)\n", GetLastError(), interfaceDetailData->DevicePath);
		}
		else
		{
#ifdef EXTENDED_LOGGING
			
			printf("   CreateFile: %d (%s)\n", GetLastError(), interfaceDetailData->DevicePath);
#endif
			
			HIDD_ATTRIBUTES Attributes;
			Attributes.Size = sizeof(Attributes);
			status = HidD_GetAttributes(hDevice, &Attributes);

			if ( status == FALSE ) 
			{
				printf( "   Error in HidD_GetAttributes failed with error: %d\n", GetLastError() );
			}
			else
			{
#ifndef EXTENDED_LOGGING
				if (type != MY_HID)	
#endif
				{
				    printf("   Vendor:%x PID=%x Vers=%u\n",Attributes.VendorID,Attributes.ProductID,Attributes.VersionNumber);
					printf("   path=%s\n",interfaceDetailData->DevicePath);
				}

				unsigned char buffer[HID_STR_LEN*2];
				char manuf[HID_STR_LEN];
				if (HidD_GetManufacturerString(hDevice, buffer, sizeof(buffer))) 
				{
					i = 0; memset(manuf,0,sizeof(manuf));
					while ((buffer[i*2] != 0)&&(i<HID_STR_LEN)) { manuf[i] = char(buffer[i*2]); i ++; }
				}else strcpy(manuf, "- not present -"); 

				char product[HID_STR_LEN];
				if (HidD_GetProductString(hDevice, buffer, sizeof(buffer))) 
				{
					i = 0; memset(product,0,sizeof(product));
					while ((buffer[i*2] != 0)&&(i<HID_STR_LEN)) { product[i] = char(buffer[i*2]); i ++; }
				} else strcpy(product, "- not present -");
#ifndef EXTENDED_LOGGING
				if (type != MY_HID)
#endif
				{
					printf("   Manufact: %s   ",manuf);
					printf("   Product: %s\n",product);
				}

				//Get the Capabilities structure for the device.
				PHIDP_PREPARSED_DATA	PreparsedData;

				if (HidD_GetPreparsedData(hDevice,&PreparsedData))
				{
					HidP_GetCaps 
						(PreparsedData, 
						&Capabilities);
					
					if (type == ALL_HID) 
					{
						printf("Capabilities:\n");

					//Display the capabilities

						printf("%s%X\n", "Usage Page: ", Capabilities.UsagePage);
						printf("%s%d\n", "Input Report Byte Length: ", Capabilities.InputReportByteLength);
						printf("%s%d\n", "Output Report Byte Length: ", Capabilities.OutputReportByteLength);
						printf("%s%d\n", "Feature Report Byte Length: ", Capabilities.FeatureReportByteLength);
						printf("%s%d\n", "Number of Link Collection Nodes: ", Capabilities.NumberLinkCollectionNodes);
						printf("%s%d\n", "Number of Input Button Caps: ", Capabilities.NumberInputButtonCaps);
						printf("%s%d\n", "Number of InputValue Caps: ", Capabilities.NumberInputValueCaps);
						printf("%s%d\n", "Number of InputData Indices: ", Capabilities.NumberInputDataIndices);
						printf("%s%d\n", "Number of Output Button Caps: ", Capabilities.NumberOutputButtonCaps);
						printf("%s%d\n", "Number of Output Value Caps: ", Capabilities.NumberOutputValueCaps);
						printf("%s%d\n", "Number of Output Data Indices: ", Capabilities.NumberOutputDataIndices);
						printf("%s%d\n", "Number of Feature Button Caps: ", Capabilities.NumberFeatureButtonCaps);
						printf("%s%d\n", "Number of Feature Value Caps: ", Capabilities.NumberFeatureValueCaps);
						printf("%s%d\n", "Number of Feature Data Indices: ", Capabilities.NumberFeatureDataIndices);
					}
					else 
#ifndef EXTENDED_LOGGING
						if (type != MY_HID)
#endif
					{
						printf("%s%d ", "    Input Report Length: ", Capabilities.InputReportByteLength);
						printf("%s%d\n", "    Output Report Length: ", Capabilities.OutputReportByteLength);
					}


					HidD_FreePreparsedData(PreparsedData);
				}

				if (path_list)
				{
					bool add = true;
					if (type == MY_HID)
					{
						if (Attributes.VendorID != nvendor) add = false;
						else
						{
							if (Attributes.ProductID != nPID) add = false;
							else if (strcmp(manuf, smanufact) != 0) add = false;
//								else if (strcmp(product, sprod_lcd) != 0) add = false;
							else if (Capabilities.InputReportByteLength != nReadLen) add = false;
							else if (Capabilities.OutputReportByteLength != nWriteLen) add = false;
						}
					}

#ifdef EXTENDED_LOGGING
					if (add)
						printf("    Added to list\n");
					else printf("    NOT Added to list\n");
#endif

					if (add)
					{
						HIDDescriptor* hdesc = new HIDDescriptor();

						strcpy(hdesc->m_sManufacturer, manuf);
						strcpy(hdesc->m_sProduct, product);

						hdesc->m_nVendor = Attributes.VendorID;
						hdesc->m_nPID = Attributes.ProductID;

						hdesc->m_interfaceDetailDataSize = interfaceDetailDataSize;
						hdesc->m_interfaceDetailData = (_SP_DEVICE_INTERFACE_DETAIL_DATA_A *)malloc(interfaceDetailDataSize);
						
						memcpy(hdesc->m_interfaceDetailData, interfaceDetailData, interfaceDetailDataSize);
						memcpy(&hdesc->m_Capabilities, &Capabilities, sizeof(Capabilities));

						path_list->add(hdesc);
					}
				}
			}
		}
		free (interfaceDetailData);
		CloseHandle(hDevice);
	}
	SetupDiDestroyDeviceInfoList(IntDevInfo);
    return true;
}
#else
bool HIDDevices::GetDeviceProperty(int type, HList* path_list, const char* smanufact, const char* sproduct, const unsigned short nvendor, const unsigned short nPID)
{
	int dev = 0;
#ifdef MY_HIDDEV	
	struct hiddev_string_descriptor dev_descriptor;
#else

#endif
	HIDDescriptor hdesc;
  	int fd = -1;
	
	for (dev=0;dev<20;dev++)
	{
		sprintf(hdesc.m_sPath, "%s%d",m_rootPath, dev);
	
		if ((fd = open(hdesc.m_sPath, O_RDONLY)) >= 0) 
		{
			hdesc.Reset();
			
			if (type == ALL_HID)
	  			printf("Path:%s\n",hdesc.m_sPath);
	  					
  			strcpy(hdesc.m_sProduct, "Error");
#ifdef MY_HIDDEV  			
    		ioctl(fd, HIDIOCGNAME(1024), &hdesc.m_sProduct);
#else
			ioctl(fd, EVIOCGNAME(1024), &hdesc.m_sProduct);
#endif
    		if (type == ALL_HID)
	  			printf("Name:%s\n",hdesc.m_sProduct);
	
#ifdef MY_HIDDEV
  			ioctl(fd, HIDIOCGDEVINFO, &hdesc.m_deviceInfo);
#else
			ioctl(fd, EVIOCGID, &hdesc.m_deviceInfo);
#endif
  			
  			if (type == ALL_HID)
  			{
	  			printf("vendor 0x%04hx product 0x%04hx version 0x%04hx ",
          			hdesc.m_deviceInfo.vendor, hdesc.m_deviceInfo.product, hdesc.m_deviceInfo.version);
#ifdef MY_HIDDEV          			
  				printf("has %i application%s ", hdesc.m_deviceInfo.num_applications,
         			(hdesc.m_deviceInfo.num_applications==1?"":"s"));
  				printf("and is on bus: %d devnum: %d ifnum: %d\n",
         			hdesc.m_deviceInfo.busnum, hdesc.m_deviceInfo.devnum, hdesc.m_deviceInfo.ifnum);
#endif
  				printf("Parameters:\n");
  			}
  			
  			hdesc.m_nVendor = hdesc.m_deviceInfo.vendor;
  			hdesc.m_nPID    = hdesc.m_deviceInfo.product;
  			
#ifdef MY_HIDDEV  			
  			int i = 0;
  			for (i=0;i<5;i++)
  			{
				dev_descriptor.index = i;
  				ioctl(fd, HIDIOCGSTRING, &dev_descriptor);
  				
  				if (type == ALL_HID)
					printf(">%02d> %s\n", i, dev_descriptor.value);
  			}

  			ioctl(fd, HIDIOCINITREPORT, 0);	
  			
 			struct hiddev_report_info rinfo;
 			struct hiddev_field_info finfo;
 			struct hiddev_usage_ref uref;
 			int rtype, j;
 			const char *rtype_str;
 
 			for (rtype = HID_REPORT_TYPE_MIN; rtype <= HID_REPORT_TYPE_MAX;rtype++) 
 			{
 	  			switch (rtype) 
 	  			{
 	  			case HID_REPORT_TYPE_INPUT: rtype_str = "Input"; break;
 	  			case HID_REPORT_TYPE_OUTPUT: rtype_str = "Output"; break;
 	  			case HID_REPORT_TYPE_FEATURE: rtype_str = "Feature"; break;
 	  			default: rtype_str = "Unknown"; break;
 	  			}
 	  			if (type == ALL_HID) printf("Reports of type %s (%d):\n", rtype_str, rtype);
 	  			rinfo.report_type = rtype;
 	  			rinfo.report_id = HID_REPORT_ID_FIRST;
 	  			while (ioctl(fd, HIDIOCGREPORTINFO, &rinfo) >= 0) 
 	  			{
 					if (type == ALL_HID) printf("  Report id: %hx (%d fields)\n",
 										rinfo.report_id, rinfo.num_fields);
 					for (i = 0; i < rinfo.num_fields; i++) 
 					{ 
 		  				memset(&finfo, 0, sizeof(finfo));
 		  				finfo.report_type = rinfo.report_type;
 		  				finfo.report_id = rinfo.report_id;
 		  				finfo.field_index = i;
 		  				ioctl(fd, HIDIOCGFIELDINFO, &finfo);
 		  				
 		  				if (i==0)
 		  				{
 		  					switch (rtype)
 		  					{
 		  					case HID_REPORT_TYPE_INPUT:  hdesc.m_inputLenghts[rinfo.report_id]  = finfo.maxusage;break;
 		  					case HID_REPORT_TYPE_OUTPUT: hdesc.m_outputLengths[rinfo.report_id] = finfo.maxusage;break;
 		  					}
 		  				}
 		  				
 		  				if (type == ALL_HID)
 		  					printf("    Field: %d: app: %04x phys %04x "
 				  					"flags %x (%d usages) unit %x exp %d\n", 
 				  					i, finfo.application, finfo.physical, finfo.flags,
 				  					finfo.maxusage, finfo.unit, finfo.unit_exponent);
 		  				/*memset(&uref, 0, sizeof(uref));
 		  				for (j = 0; j < finfo.maxusage; j++) 
 		  				{
 							uref.report_type = finfo.report_type;
 							uref.report_id = finfo.report_id;
 							uref.field_index = i;
 							uref.usage_index = j;
 							ioctl(fd, HIDIOCGUCODE, &uref);
 							ioctl(fd, HIDIOCGUSAGE, &uref);
 							printf("      Usage: %04x val %d\n", 
 									uref.usage_code, uref.value);
 		  				}*/	
 					}
 					rinfo.report_id |= HID_REPORT_ID_NEXT;
 				}//while
 	  		}//for
#else
			
#endif 	  		
 	  		
 	  		{
 				if (path_list)
				{
					bool add = true;
					if (type == MY_HID)
					{
						if (hdesc.m_nVendor != nvendor) add = false;
						else
						{
							//if (nPID != 65535) 
							//{
								//if (Attributes.ProductID != nPID) add = false;
								//else if (strcmp(manuf, smanufact) != 0) add = false;
								//else if (strcmp(product, sproduct) != 0) add = false;
								//else if (Capabilities.InputReportByteLength != 24) add = false;
								//else if (Capabilities.OutputReportByteLength != 24) add = false;
							//}
							//else
							//{
								if (hdesc.m_nPID == 0x02)//normal mode
								{
									if (strcmp(hdesc.m_sProduct, "ACME Labs ACME LCD keypad") != 0) add = false;
								}
								else if (hdesc.m_nPID == 0x01)//flash mode
								{
									if (strcmp(hdesc.m_sProduct, "ACME Labs ACME Firmware flasher") != 0) add = false;
								}
								else add = false;
							//}
						}
					}

					if (add)
					{
						HIDDescriptor* phdesc = new HIDDescriptor();

						memcpy(phdesc, &hdesc, sizeof(HIDDescriptor));
						
						//printf("\n%s=%s\n", phdesc->GetHIDPath(), hdesc.GetHIDPath());
						
						path_list->add(phdesc);
					}
				}
			}
  			
  			close(fd);
  		}
	}

	return true;
}
#endif

void HIDDevices::enumerateHIDDevices()
{
#ifdef _WIN32
	GUID			HidGuid;
	HidD_GetHidGuid(&HidGuid);

	GetDeviceProperty(NULL, &HidGuid, -1, ALL_HID, NULL, NULL, NULL, NULL, 0, 0, 0, 0 );
#else //linux
	GetDeviceProperty(ALL_HID);
#endif
}

HIDDescriptor* HIDDevices::chooseHIDDevice(const char* manufact, const char* prod_lcd, const char* prod_flash, const unsigned short vendor, const unsigned short PID, int devno, int nReadLen, int nWriteLen)
{
	HIDDescriptor* pchosen = NULL;

	HList hlist;

#ifdef _WIN32
	GUID			HidGuid;
	HidD_GetHidGuid(&HidGuid);
#endif

	if (manufact == NULL)
	{
		printf("Choose one device:\n");

#ifdef _WIN32
		GetDeviceProperty(NULL, &HidGuid, -1, CHOOSE_HID, &hlist, NULL, NULL, NULL, 0, 0, 0, 0);
#else
		GetDeviceProperty(CHOOSE_HID, &hlist);
#endif
		
		int dev = 0;
		printf(" DEVICE:"); 
		if (devno == -1)
			 scanf("%d", &dev);
		else 
		{
			dev = devno;
			printf("%d\n", dev);
		}

		int cnt = 0;

		hlist.moveToHead();
		HIDDescriptor* a = (HIDDescriptor*)hlist.nextElement();
		while (a)
		{
			if (cnt == dev)
				 pchosen = a;
			else delete a;

			cnt++;
			a = (HIDDescriptor*)hlist.nextElement();
		}
	}
	else
	{
#ifdef _WIN32
		GetDeviceProperty(NULL, &HidGuid, -1, MY_HID, &hlist, manufact, prod_lcd, prod_flash, vendor, PID, nReadLen, nWriteLen);
#else
// bool HIDDevices::GetDeviceProperty(int type, HList* path_list, const char* smanufact, const char* sproduct, const unsigned short nvendor, const unsigned short nPID)

		GetDeviceProperty(MY_HID, &hlist, manufact, prod_lcd, vendor, PID);
#endif
		if (hlist.elements() > 1)
		{
			if (devno != -1)
			{
				hlist.moveToHead();
				for (int i=0;i<devno;i++) 
					pchosen = (HIDDescriptor*)hlist.nextElement();
			}
			else
			{//in case of more devices simply choose the 1st one if devno is not set
				hlist.moveToHead();
				pchosen = (HIDDescriptor*)hlist.nextElement();

				//delete the rest
				HIDDescriptor* a = (HIDDescriptor*)hlist.nextElement();
				while (a)
				{
					delete a;
					a = (HIDDescriptor*)hlist.nextElement();
				}
			}
		}
		else if (hlist.elements() == 0)
		{
			printf("No device found - set the -manuf, -product, -vendor, -pid propperly:\n");
		}
		else
		{
			hlist.moveToHead();
			pchosen = (HIDDescriptor*)hlist.nextElement();

			//delete the rest
			HIDDescriptor* a = (HIDDescriptor*)hlist.nextElement();
			while (a)
			{
				delete a;
				a = (HIDDescriptor*)hlist.nextElement();
			}
		}
	}

	return pchosen;
}

HIDDevice* HIDDevices::openDevice(HIDDescriptor* hdesc)
{
	HIDDevice* dev = new HIDDevice(hdesc);

	return dev;
}

#ifndef _WIN32
void HIDDevices::setRootPath(const char* path)
{
	if (path) 
		strcpy(m_rootPath, path);	
}
#endif
/*
HIDAcme* HIDDevices::openAcme(HIDDescriptor* hdesc, HIDGraphics* gp)
{
	HIDDevice* dev = openDevice(hdesc);
	HIDAcme* pAcme = new HIDAcme(dev);
	pAcme->connectGraph(gp);

	return pAcme;
}*/
