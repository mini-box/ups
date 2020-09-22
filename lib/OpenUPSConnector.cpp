#include "stdafx.h"
#include "OpenUPSConnector.h"
#include "HIDDevices.h"
#include <math.h>
#include <errno.h>

#define SLEEP_PERIOD 5

OpenUPSConnector::OpenUPSConnector(unsigned int timer, int counter)
:m_txfSettingsToDevice("settings.ini")
{
//	_CrtSetBreakAlloc(97);

	if (timer < 15) timer = 15;

	m_nTimerMax  = timer / SLEEP_PERIOD;
	if (m_nTimerMax < 4) m_nTimerMax = 4;
	m_nTimer1_4 = m_nTimerMax / 4;
	m_nTimer2_4 = m_nTimerMax / 4 * 2;
	m_nTimer3_4 = m_nTimerMax / 4 * 3;

	m_pUPS = NULL;
	m_bConnected = false;

	m_fileSettingsFromDevice = NULL;

	m_nDevCounter = counter;

	resetValues();
}

OpenUPSConnector::~OpenUPSConnector(void)
{
	CloseDevice();
}

void OpenUPSConnector::resetValues()
{
	int i;

	m_fVIN			= 0;
	m_fVBat			= 0;
	m_fVOut			= 0;
	m_fCCharge		= 0;
	m_fCDischarge	= 0;
	m_fVDuty        = 0;
	for (i=0;i<3;i++)
	{
		m_fTemperature[i] = 0;
		m_fVCell[i]	= 0;
		m_bBatOn[i] = 0;
	}

	for (i=0;i<8;i++)
	{
		m_bCheck23[i] = 0;
		m_bCheck24[i] = 0;
		m_bCheck25[i] = 0;
	}

	for (i=0;i<DBG_LEN;i++)
		m_bDBG[i] = 0;

	for (i=0;i<30;i++)
		m_bDBG2[i] = 0;

	m_nVerMajor		= 0;
	m_nVerMinor		= 0;

	m_nState		= 0;

	m_bStateUPS		= 0;
	m_bStateCHG		= 0;
	m_bStateDBG		= 0;
	m_bCapacity		= 0;
	m_nRTE			= 0;
	m_bUPSMode		= 0;
	m_nShutdownType = 0;

	m_bVOutPot		= 0;
	m_bConfigSwitch = 0;

	m_chStateMachine = SM_BASIC; 
	m_nSaveCfgStatus = 0xF0;
	m_nLoadCfgStatus = 0xF0;
	memset(m_chPackages,0,SETTINGS_PACKS*16);
	m_bSettingsCompareWithOld = false;//compare the old settings with the newly arriwed one
	if (m_fileSettingsFromDevice != NULL) fclose(m_fileSettingsFromDevice);
	m_fileSettingsFromDevice = NULL;
	m_bSettingsToDeviceFile = false;
}

void OpenUPSConnector::run()
{
	m_nThreadCounter = 0;
	while (isRunning())
	{
		switch (m_chStateMachine)
		{
		case SM_BASIC://normal read operation
			if (m_nThreadCounter==0)
			{
				m_lock.lock();
				if (m_pUPS) m_pUPS->sendMessage(UPS12V_GET_ALL_VALUES, 0);
				m_lock.unlock();
			}
			else if (m_nThreadCounter==m_nTimer1_4)
			{
				m_lock.lock();
				if (m_pUPS) m_pUPS->sendMessage(UPS12V_GET_ALL_VALUES_2, 0);
				m_lock.unlock();
			}
			else if (m_nThreadCounter==m_nTimer2_4)
			{
				m_lock.lock();
				if (m_pUPS) m_pUPS->sendMessage(UPS12V_CLOCK_OUT, 0);
				m_lock.unlock();
			}
			else if (m_nThreadCounter==m_nTimer3_4)
			{
				m_lock.lock();
				if (m_pUPS) m_pUPS->sendMessage(UPS12V_GET_ALL_VALUES_3, 0);
				m_lock.unlock();
			}
			m_nThreadCounter = (m_nThreadCounter+1)%m_nTimerMax;
			break;
		case SM_LOADING://load settings
			if (m_nThreadCounter > 100)//0.5 sec timeout
			{//timeout
				m_nThreadCounter = 0;
				m_chStateMachine = 0;
				m_nLoadCfgStatus = 0xF5;//timeout
			}
			else m_nThreadCounter++;
			break;
		case SM_SAVING://save settings
			
			break;
		default://error
			m_chStateMachine = SM_BASIC;
		}

		m_lock.lock();
		HArray* parray = m_pUPS->getReceivedMessage();
		while (parray)
		{
			parseMessage(parray);
			delete parray;
			parray = m_pUPS->getReceivedMessage();
		}
		m_lock.unlock();

		Sleep(SLEEP_PERIOD);
	}
}

void OpenUPSConnector::deviceChanged()
{
	m_lock.lock();
	if (m_pUPS && m_pUPS->getParent()->isOpened())
	{
		m_lock.unlock();
		return;
	}

	OpenDevice();

	m_lock.unlock();
}

bool OpenUPSConnector::OpenDevice()
{
	stop();

	m_lock.lock();

	m_bConnected = false;
	if (m_pUPS)
	{
		HIDDevice* dev = m_pUPS->getParent();
		if (dev) dev->CloseDevice();
		delete m_pUPS;
		if (dev) delete dev;
		m_pUPS = NULL;

		resetValues();
	}

	HIDDevices hdevs;
	HIDDescriptor* hdesc = hdevs.chooseHIDDevice(MANUFACTURER, PROD_NORMAL, NULL, UPS12V_VID, UPS12V_PID, m_nDevCounter, UPS12V_MSG_READ, UPS12V_MSG_WRITE);

	if (hdesc)
	{
		HIDDevice* dev = hdevs.openDevice(hdesc);

		m_pUPS = new HIDOpenUPS2(dev, 5000);

		m_bConnected = true;

		m_lock.unlock();

		start();

		return true;
	}
	
	m_lock.unlock();
	return false;
}

void OpenUPSConnector::CloseDevice()
{
	bool stopped = stop();

	HIDDevice* dev = NULL;

	m_lock.lock();
	m_bConnected = false;
	if (m_pUPS)
	{
		dev = m_pUPS->getParent();
		if (dev) dev->CloseDevice();
		delete m_pUPS;
	}
	m_pUPS = NULL;
	m_lock.unlock();

	if (dev) delete dev;
}

void OpenUPSConnector::readIniFromDevice(bool tofile, bool compare_with_old)
{
	m_lock.lock();

	m_bSettingsCompareWithOld = compare_with_old;
	m_nLoadCfgStatus = 0;//start loading

	if (!m_bConnected)
	{
		m_chStateMachine = SM_BASIC;
		m_nLoadCfgStatus = 0xF2;//not connected
		m_lock.unlock();
		return;
	}

	if (tofile)
	{
		m_fileSettingsFromDevice = fopen("settings.ini", "wt");
		if (!m_fileSettingsFromDevice)
		{
			m_chStateMachine = SM_BASIC;
			m_nLoadCfgStatus = 0xF3;//file not found
			m_lock.unlock();
			return;
		}

		fprintf(m_fileSettingsFromDevice, "#This file is a parameter and script file for Open-UPS2\r\n");
		fprintf(m_fileSettingsFromDevice, "#Can be created from the Open-UPS2 software importing from device\r\n");
		fprintf(m_fileSettingsFromDevice, "#DO NOT MODIFY IT MANUALLY! WRONG VALUES CAN BURN DOWN YOUR DEVICE!\r\n");
		fprintf(m_fileSettingsFromDevice, "#version:\r\n");
		fprintf(m_fileSettingsFromDevice, "00\r\n");
		fprintf(m_fileSettingsFromDevice, "#-----------------------------------\r\n");
	}
	else m_fileSettingsFromDevice = NULL;

	m_ulSettingsAddr = SETTINGS_ADDR_START;
	m_nThreadCounter = 0;
	m_chStateMachine = SM_LOADING;//load settings

	m_pUPS->sendMessage(UPS12V_MEM_READ_OUT, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);

	m_lock.unlock();
}

void OpenUPSConnector::writeIniToDevice(bool fromfile)
{
	if ((fromfile == false) && (m_nLoadCfgStatus != 100))
	{
		m_chStateMachine = SM_BASIC;
		m_nSaveCfgStatus = 0xF8;
		printf("Load first ...");
		return;
	}

	m_lock.lock();

	m_nSaveCfgStatus = 0;//start loading

	if (!m_bConnected)
	{
		m_chStateMachine = SM_BASIC;
		m_nSaveCfgStatus = 0xF2;//not connected
		m_lock.unlock();
		return;
	}

	m_bSettingsToDeviceFile = fromfile;
	if (m_bSettingsToDeviceFile)
	{
		if (m_txfSettingsToDevice.Open() != 0)
		{
			m_chStateMachine = SM_BASIC;
			m_nSaveCfgStatus = 0xF3;//file not found
			m_lock.unlock();
			return;
		}
	}

	m_ulSettingsAddr = SETTINGS_ADDR_START;
	m_nThreadCounter = 0;
	m_chStateMachine = SM_SAVING;//load settings

	m_pUPS->sendMessage(UPS12V_MEM_ERASE, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x40);
	m_pUPS->waitForAnswer();//todo - check error?

	if (m_bSettingsToDeviceFile == false)
		 m_pUPS->sendMessage(UPS12V_MEM_WRITE_OUT, 16, m_chPackages+(m_ulSettingsAddr-SETTINGS_ADDR_START) , 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
	else
	{
		HArray line;
	
		m_txfSettingsToDevice.NextArray(&line);
		if (line.length()<=0)
		{
			m_chStateMachine = SM_BASIC;
			m_nSaveCfgStatus = 0xF5;//file empty
			m_lock.unlock();
			return;
		}
		unsigned char version = line.getAt(0);

		m_txfSettingsToDevice.NextArray(&line);
		if (line.length()<=0)
		{
			m_chStateMachine = SM_BASIC;
			m_nSaveCfgStatus = 0xF6;//file invalid
			m_lock.unlock();
			return;
		}

		unsigned long addr_read = 0;
		addr_read = line.getAt(1);
		addr_read = (addr_read << 8);
		addr_read = addr_read | line.getAt(0);
		
		if (addr_read != m_ulSettingsAddr)
		{
			m_chStateMachine = SM_BASIC;
			m_nSaveCfgStatus = 0xF7;//file invalid
			m_lock.unlock();
			return;
		}

		m_pUPS->sendMessage(UPS12V_MEM_WRITE_OUT, 16, line.getBuf()+2 , 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
		memcpy(m_chPackages+(m_ulSettingsAddr-SETTINGS_ADDR_START), line.getBuf()+2, 16);//copy to buffer for later doublecheck
	}

	m_lock.unlock();
}

void OpenUPSConnector::sendCommand(unsigned char command, unsigned char value)
{
	m_lock.lock();

	if (!m_bConnected)
	{
		m_lock.unlock();
		return;
	}

	m_pUPS->sendMessage(UPS12V_CMD_OUT, 3, command, value, 0);

	m_lock.unlock();
}

void OpenUPSConnector::sendCommand(unsigned char command, unsigned char value1, unsigned char value2)
{
	m_lock.lock();

	if (!m_bConnected)
	{
		m_lock.unlock();
		return;
	}

	m_pUPS->sendMessage(UPS12V_CMD_OUT, 3, command, value1, value2);

	m_lock.unlock();
}

void OpenUPSConnector::sendMessage(unsigned char mesg, unsigned char value1, unsigned char value2)
{
	m_lock.lock();

	if (!m_bConnected)
	{
		m_lock.unlock();
		return;
	}

	m_pUPS->sendMessage(mesg, 2, value1, value2);

	m_lock.unlock();
}

void OpenUPSConnector::parseMessage(HArray* parray)
{
	if (parray && parray->length()>=24)
	{
		switch (parray->getAt(0))
		{
		case INTERNAL_MESG:
			{
				switch (parray->getAt(1))
				{
				case INTERNAL_MESG_DISCONNECTED:
					{
						m_bConnected = false;
						resetValues();
					}break;
				}
			}break;
		case UPS12V_RECV_ALL_VALUES:
			{
				int i;

				m_nVerMajor			= (parray->getAt(31) >> 4) & 0x0F;
				m_nVerMinor			= parray->getAt(31) & 0x0F;

				m_fVIN  = convertOneValue2Float(parray->getBuf(), 2, 1, 20, HIDOpenUPS2::GetConstant(0));
				m_fVOut = convertOneValue2Float(parray->getBuf(), 2, 3, 20, HIDOpenUPS2::GetConstant(1));
				m_fVBat = convertOneValue2Float(parray->getBuf(), 2, 5, 20, HIDOpenUPS2::GetConstant(6));

				for (i=0;i<3;i++)
					m_fVCell[i] = convertOneValue2Float(parray->getBuf(), 2, 7+2*i, 20, HIDOpenUPS2::GetConstant(6));

				for (i=0;i<8;i++)
				{
					m_bCheck23[i] = ((parray->getAt(23) >> i)&1);
					m_bCheck24[i] = ((parray->getAt(24) >> i)&1);
					m_bCheck25[i] = ((parray->getAt(25) >> i)&1);
				}

				m_fCCharge	= convertOneValue2Float(parray->getBuf(), 2, 19, 2, (double)0.001);
				m_fCDischarge = convertOneValue2Float(parray->getBuf(), 2, 21, 2, (double)0.001);
				
				if (m_bCheck24[6])		m_nState = 1;
				else if (m_bCheck24[5])	m_nState = 2;
				else					m_nState = 3;//only usb

				m_fTemperature[0] = convertOneValue2Float(parray->getBuf(), 2, 13, 15, 1);
				m_fTemperature[1] = convertOneValue2Float(parray->getBuf(), 2, 15, 15, 1);
				m_fTemperature[2] = convertOneValue2Float(parray->getBuf(), 2, 17, 15, 1);

				m_fVDuty = convertOneValue2Float(parray->getBuf(), 2, 26, 20, (double)0.0009765625);

				m_bVOutPot = 255 - parray->getAt(28);
				m_bConfigSwitch = parray->getAt(29);
			}break;
		case UPS12V_CLOCK_IN:
			{
				int i;

				if ((m_nVerMajor >= 1) && (m_nVerMinor >= 4))
				{
					m_nShutdownType = parray->getAt(7);
					m_nShutdownType = (m_nShutdownType << 8) | parray->getAt(6);
				}
				else m_nShutdownType = 0;

				m_bStateUPS = parray->getAt(9);
				m_bStateCHG = parray->getAt(10);
				m_bStateDBG = parray->getAt(11);
				m_bCapacity = parray->getAt(12);

				for (i=0;i<3;i++)
					m_bBatOn[i] = ((parray->getAt(15) >> i)&1);

				m_nRTE = parray->getAt(17);
				m_nRTE = (m_nRTE << 8) | parray->getAt(16);

				m_bUPSMode = parray->getAt(25);

				for (i=0;i<4;i++)
					m_bDBG[i] = parray->getAt(28+i);
			}break;
		case UPS12V_RECV_ALL_VALUES_2:
			{
				int i;

				//m_fOutputPower = convertOneValue2Float(parray->getBuf(), 4, 1, 16, 0.000001);
				for (i=0;i<21;i++)
					m_bDBG[4+i] = parray->getAt(5+i);
			}break;
		case UPS12V_RECV_ALL_VALUES_3:
			{
				for (int i=0;i<30;i++)
					m_bDBG2[i] = parray->getAt(1+i);
			}break;
		case UPS12V_MEM_READ_IN:
			{
				if (m_chStateMachine == SM_LOADING)
				{
					if (m_fileSettingsFromDevice)
					{
						fprintf(m_fileSettingsFromDevice, "%02lx %02lx", m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF);
						for (int len=0;len<16;len++)
							fprintf(m_fileSettingsFromDevice, " %02x", parray->getAt(5+len));
						fprintf(m_fileSettingsFromDevice, "\r\n");
					}
					if (m_bSettingsCompareWithOld)
					{
						if (   (m_ulSettingsAddr < (SETTINGS_ADDR_END-16)) 
							&& ((m_ulSettingsAddr < SETTINGS_ADDR_CALIBR_START)||(m_ulSettingsAddr >= SETTINGS_ADDR_CALIBR_END))
							)
						{
							for (int len=0;len<16;len++)
								if ((m_chPackages+(m_ulSettingsAddr-SETTINGS_ADDR_START))[len] != parray->getAt(5+len))
								{
									m_nThreadCounter = 0;
									m_chStateMachine = 0;
									m_nLoadCfgStatus = 0xF4;//comparition mismatch

									if (m_fileSettingsFromDevice)
									{
										fclose(m_fileSettingsFromDevice);
										m_fileSettingsFromDevice = NULL;
									}
									return;
								}
						}
					}
					memcpy(m_chPackages+(m_ulSettingsAddr-SETTINGS_ADDR_START), parray->getBuf()+5, 16);

					m_ulSettingsAddr += 16;

					if (m_ulSettingsAddr < SETTINGS_ADDR_END)
					{
						m_pUPS->sendMessage(UPS12V_MEM_READ_OUT, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
						m_nLoadCfgStatus++;
						m_nThreadCounter = 0;
					}
					else
					{//load succesfull
						m_nLoadCfgStatus = 100;//success 
						m_nThreadCounter = 0;
						m_chStateMachine = 0;
						if (m_fileSettingsFromDevice)
						{
							fclose(m_fileSettingsFromDevice);
							m_fileSettingsFromDevice = NULL;
						}
					}
				}
			}break;
		case UPS12V_MEM_WRITE_IN:
			{
				if (m_chStateMachine == SM_SAVING)
				{
					if (   (m_ulSettingsAddr < (SETTINGS_ADDR_END-16)) 
							&& ((m_ulSettingsAddr < SETTINGS_ADDR_CALIBR_START)||(m_ulSettingsAddr >= SETTINGS_ADDR_CALIBR_END))
							)
					{// don't check the last line because that contains data filled up by the MCU (internal data)
						for (int i=0;i<16;i++)
							if (parray->getAt(5+i)!=(m_chPackages+(m_ulSettingsAddr-SETTINGS_ADDR_START))[i]) 
							{
								TRACE("%02x!=%02X\n",parray->getAt(5+i),(m_chPackages+(m_ulSettingsAddr-SETTINGS_ADDR_START))[i]);
								m_nThreadCounter = 0;
								m_chStateMachine = 0;
								m_nSaveCfgStatus = 0xF4;//comparition mismatch
								return;
							}
					}

					m_ulSettingsAddr += 16; 

					if (m_ulSettingsAddr < SETTINGS_ADDR_END)
					{
						if (m_bSettingsToDeviceFile == false)
							 m_pUPS->sendMessage(UPS12V_MEM_WRITE_OUT, 16, m_chPackages+(m_ulSettingsAddr-SETTINGS_ADDR_START) , 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
						else
						{
							HArray line;
							m_txfSettingsToDevice.NextArray(&line);
							if (line.length()<=0)
							{
								m_chStateMachine = SM_BASIC;
								m_nSaveCfgStatus = 0xF6;//file invalid
								m_lock.unlock();
								return;
							}

							unsigned long addr_read = 0;
							addr_read = line.getAt(1);
							addr_read = (addr_read << 8);
							addr_read = addr_read | line.getAt(0);
							
							if (addr_read != m_ulSettingsAddr)
							{
								m_chStateMachine = SM_BASIC;
								m_nSaveCfgStatus = 0xF7;//file invalid
								m_lock.unlock();
								return;
							}

							m_pUPS->sendMessage(UPS12V_MEM_WRITE_OUT, 16, line.getBuf()+2 , 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
							memcpy(m_chPackages+(m_ulSettingsAddr-SETTINGS_ADDR_START), line.getBuf()+2, 16);//copy to buffer for later doublecheck
						}
						m_nSaveCfgStatus++;
						m_nThreadCounter = 0;
					}
					else
					{//load succesfull
						m_nSaveCfgStatus = 100;//success 
						m_nThreadCounter = 0;
						m_chStateMachine = 0;
					}
				}
			}break;
		}
	}
}

float OpenUPSConnector::convertOneValue2Float(unsigned char* buffer, int nLen, int nIndex, int nReadMode, double dMultiplier)
{
	double value = -1;

	if (nLen == 1)
		value = (double)buffer[nIndex] * dMultiplier;
	else if (nLen == 2)
		value = (double)( (((int)buffer[nIndex+1] ) << 8)|buffer[nIndex]) * dMultiplier;
	else if (nLen == 4)
		value = (double)( (((int)buffer[nIndex+3] ) << 24) | (((int)buffer[nIndex+2] ) << 16) | (((int)buffer[nIndex+1] ) << 8) | buffer[nIndex]) * dMultiplier;

	switch (nReadMode)
	{
		case 10:
			{
				if (nLen == 1)
					value = (double)buffer[nIndex] * HIDOpenUPS2::GetConstant((int)dMultiplier);
				else
					value = (double)( (((int)buffer[nIndex+1] ) << 8)|buffer[nIndex]) * HIDOpenUPS2::GetConstant((int)dMultiplier);
			}break;
		case 30:
			{
				if (nLen == 1)
					value = (double)buffer[nIndex] * HIDOpenUPS2::GetConstant(6);
				else
					value = (double)( (((int)buffer[nIndex+1] ) << 8)|buffer[nIndex]) * HIDOpenUPS2::GetConstant(6);
			}break;
		case 15:
			{
				unsigned int termistor = 0;
				termistor = buffer[nIndex+1];
				termistor = (termistor << 8) | buffer[nIndex];

				if (termistor <= HIDOpenUPS2::GetTermistorConsts()[0])
					value = -40;
				else if (termistor >= HIDOpenUPS2::GetTermistorConsts()[TERMISTOR_CONSTS-1])
					value = 125;
				else
				{
					int pos = -1;
					for (int i=TERMISTOR_CONSTS-1;i>=0;i--)
					{
						if (termistor >= HIDOpenUPS2::GetTermistorConsts()[i])
						{
							pos = i;
							break;
						}
					}

					//if (termistor != HIDOpenUPS2::GetTermistorConsts()[i]) pos--;//latest value which is smaller than my one

					if (termistor == HIDOpenUPS2::GetTermistorConsts()[pos])
						value = pos*5-40;
					else
					{
						int t1 = pos*5-40;
						int t2 = (pos+1)*5-40;

						unsigned int d1 = HIDOpenUPS2::GetTermistorConsts()[pos];
						unsigned int d2 = HIDOpenUPS2::GetTermistorConsts()[pos+1];

						float dtemp = ((float)termistor - (float)d1)*((float)t2-(float)t1)/((float)d2-(float)d1);
						
						int temp = (int)ceil(dtemp) + t1;

						value = temp;
					}
				}
			}break;
	}

	//DBGOutput("convertOneValue2Float %.3f\n",value);

	return (float)value;
}

void OpenUPSConnector::convertOneValue2String(char* destination, int nLen, int nIndex, int nReadMode, double dMultiplier)
{
	destination[0] = 0; //empty string
	double value = 0;

	if (nLen == 1)
		value = (double)m_chPackages[nIndex] * dMultiplier;
	else if (nLen == 2)
		value = (double)( (((int)m_chPackages[nIndex+1] ) << 8)|m_chPackages[nIndex]) * dMultiplier;
	else if (nLen == 4)
		value = (double)( (((int)m_chPackages[nIndex+3] ) << 24) | (((int)m_chPackages[nIndex+2] ) << 16) | (((int)m_chPackages[nIndex+1] ) << 8) | m_chPackages[nIndex]) * dMultiplier;

	switch (nReadMode)
	{
		case 2:   
		case 9:
			sprintf(destination, "%.3f",(float)value);
			break;
		case 20:  sprintf(destination, "%.2f",(float)value);break;
		case 10:
			{
				if (nLen == 1)
					value = (double)m_chPackages[nIndex] * HIDOpenUPS2::GetConstant((int)dMultiplier);
				else if (nLen == 2)
					value = (double)( (((int)m_chPackages[nIndex+1] ) << 8)|m_chPackages[nIndex]) * HIDOpenUPS2::GetConstant((int)dMultiplier);
				else if (nLen == 4)
					value = (double)( (((int)m_chPackages[nIndex+3] ) << 24) | (((int)m_chPackages[nIndex+2] ) << 16) | (((int)m_chPackages[nIndex+1] ) << 8) | m_chPackages[nIndex]) * HIDOpenUPS2::GetConstant((int)dMultiplier);
				sprintf(destination, "%.3f",(float)value);
			}break;
		case 30:
			{
				if (nLen == 1)
					value = (double)m_chPackages[nIndex] * HIDOpenUPS2::GetConstant(6);
				else if (nLen == 2)
					value = (double)( (((int)m_chPackages[nIndex+1] ) << 8)|m_chPackages[nIndex]) * HIDOpenUPS2::GetConstant(6);
				else if (nLen == 4)
					value = (double)( (((int)m_chPackages[nIndex+3] ) << 24) | (((int)m_chPackages[nIndex+2] ) << 16) | (((int)m_chPackages[nIndex+1] ) << 8) | m_chPackages[nIndex]) * HIDOpenUPS2::GetConstant(6);
				if (value > dMultiplier) value = dMultiplier;
				sprintf(destination, "%.3f",(float)value);
			}break;
		case 12:
		case 13:
			sprintf(destination, "%d", m_chPackages[nIndex]);
			break;
		case 14:
			char2bin(destination, m_chPackages[nIndex]);
			break;
		case 15:
			{
				unsigned int termistor = 0;
				termistor = m_chPackages[nIndex+1];
				termistor = (termistor << 8) | m_chPackages[nIndex];

				if (termistor <= HIDOpenUPS2::GetTermistorConsts()[0])
					strcpy(destination, "-40");
				else if (termistor >= HIDOpenUPS2::GetTermistorConsts()[TERMISTOR_CONSTS-1])
					strcpy(destination, "125");
				else
				{
					int pos = -1;
					for (int i=TERMISTOR_CONSTS-1;i>=0;i--)
					{
						if (termistor >= HIDOpenUPS2::GetTermistorConsts()[i])
						{
							pos = i;
							break;
						}
					}

					//if (termistor != HIDOpenUPS2::GetTermistorConsts()[i]) pos--;//latest value which is smaller than my one

					if (termistor == HIDOpenUPS2::GetTermistorConsts()[pos])
						sprintf(destination, "%d",pos*5-40);
					else
					{
						int t1 = pos*5-40;
						int t2 = (pos+1)*5-40;

						unsigned int d1 = HIDOpenUPS2::GetTermistorConsts()[pos];
						unsigned int d2 = HIDOpenUPS2::GetTermistorConsts()[pos+1];

						float dtemp = ((float)termistor - (float)d1)*((float)t2-(float)t1)/((float)d2-(float)d1);
						
						int temp = (int)ceil(dtemp) + t1;

						sprintf(destination, "%d",temp);
					}
				}
			}break;
		case 16:
			{//4 byte value
				if (dMultiplier == 1)
					 sprintf(destination,"%.0f", (float)value);
				else sprintf(destination,"%.2f", (float)value);
			}break;
		case 17:
			{
				sprintf(destination, "%d", (int)value);
			}break;
		case 3:
			{
				unsigned int ivalue = (unsigned int)value;
				if (ivalue == 0xFFFF)
					strcpy(destination, "Never");
				else sprintf(destination, "%02d:%02d:%02d"
						, ivalue/3600
						, (ivalue/60)%60
						, ivalue % 60
						);
			}break;
		case 4:
			{
				double dvalue = (m_chPackages[nIndex] & 0x7f) * dMultiplier;
				if ((m_chPackages[nIndex] & 0x80)!=0)
					 sprintf(destination,"-%.3f", (float)value);
				else sprintf(destination,"%.3f", (float)value);
			}break;
		case 5:
			{
				sprintf(destination, "%d", (int)value);
			}break;
		case 6:
			{
				strcpy(destination, "-");
			}break;
		case 7:
			{//no multiplier - voltage value calcualted with formula
				//! sprintf(destination, "%.3f", HIDOpenUPS2::GetVoltageVOut(m_chPackages[nIndex]));
			}break;
		case 8:
			{
				if (nLen == 1)
				{
					if (m_chPackages[nIndex] == 0xFF)
						 strcpy(destination, "Never");
					else sprintf(destination, "%d", (int)value);
				}
				else
				{
					if ((m_chPackages[nIndex] == 0xFF) && (m_chPackages[nIndex+1] == 0xFF))
						 strcpy(destination, "Never");
					else sprintf(destination, "%d", (int)value);
				}
			}break;
		case 40://hex
			if (nLen == 1)
				sprintf(destination, "%02x",(int)value);
			else if (nLen == 2)
				sprintf(destination, "%04x",(int)value);
			else if (nLen == 4) 
				sprintf(destination, "%08x",(int)value);
			break;
		case 1:
		default:
			{
				sprintf(destination, "%d", (int)value);
			}
	}

	//DBGOutput("convertOneValue2String %d %s %f [%x][%x]\n",nIndex,destination,(float)value,(int)m_chPackages[nIndex],(int)m_chPackages[nIndex+1]);
}

unsigned char OpenUPSConnector::setVariableData(int mesg_no, char* str)
{
	int len = HIDOpenUPS2::GetMessages()[mesg_no].nLen;

	if (len!=0)
	{
		unsigned char c1 = 0;
		unsigned char c2 = 0;
		unsigned char c3 = 0;
		unsigned char c4 = 0;

		if (readOneValue(str
						,HIDOpenUPS2::GetMessages()[mesg_no].nReadMode
						,HIDOpenUPS2::GetMessages()[mesg_no].dMultiplier
						,len
						,c1, c2, c3, c4))
		{
			switch (len)
			{
			case 1:
				m_chPackages[HIDOpenUPS2::GetMessages()[mesg_no].nIndex]   = c2;
				break;
			case 2:
				m_chPackages[HIDOpenUPS2::GetMessages()[mesg_no].nIndex]   = c1;
				m_chPackages[HIDOpenUPS2::GetMessages()[mesg_no].nIndex+1] = c2;
				break;
			case 4:
				m_chPackages[HIDOpenUPS2::GetMessages()[mesg_no].nIndex]   = c1;
				m_chPackages[HIDOpenUPS2::GetMessages()[mesg_no].nIndex+1] = c2;
				m_chPackages[HIDOpenUPS2::GetMessages()[mesg_no].nIndex+2] = c3;
				m_chPackages[HIDOpenUPS2::GetMessages()[mesg_no].nIndex+3] = c4;
				break;
			}
			return 1;
		}
		else return 0;
	}
	return 1;
}

bool OpenUPSConnector::readOneValue(char* str, int nReadMode, double dMultiplier, int len, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4) 
{
	bool ok = false;
	c1 = 0;
	c2 = 0;

	double val = 0;
	
	switch (nReadMode)
	{
	case 3:
		{//hh:mm:ss
			if (_strcmpi(str, "Never")==0)
			{
				val = 0xFFFF;
				ok = true;
			}
			else if ((strlen(str)==8)&&(str[2]==':')&&(str[5]==':'))
			{
				_TCHAR* end = NULL;
				char temps[10];
				
				str_left(temps, str, 2);
				int hh  = _tcstol(temps, &end, 10);
				if (((errno != ERANGE)&&(errno != EINVAL))&&(hh>=0)&&(hh<18))
				{
					str_mid(temps, str, 3, 2);
					int mm  = _tcstol(temps, &end, 10);
					if (((errno != ERANGE)&&(errno != EINVAL))&&(mm>=0)&&(mm<60))
					{
						str_right(temps, str, 2);
						int ss  = _tcstol(temps, &end, 10);

						if (((errno != ERANGE)&&(errno != EINVAL))&&(ss>=0)&&(ss<60))
						{
							val = hh*3600+mm*60+ss;
							ok = true;
						}
					}
				}
			}
		}break;
	case 4:
		{//"signed" float
			if (strlen(str)!=0)
			{
				bool negative = false;
				if (str[0] == '-')
				{
					str = str+1;
					negative = true;
				}

				_TCHAR* end = NULL;
				val = _tcstod(str, &end);

				if ((errno != ERANGE)&&(errno != EINVAL))
				{
					ok = true;

					val /= dMultiplier;
					if (val < 0) val = 0;
					double rem = val - (int) val;
					unsigned long uval = (unsigned long)val;
					if (rem >= 0.5) uval += 1;

					if (uval > 0x7F) uval = 0x7F;

					c1 = 0;
					c2 = (unsigned char)uval;
					if (negative) c2 |= 0x80;
				}
			}
		}break;
	case 6:
		{
			val = 0xFF;
			ok = true;
		}break;
	case 7:
		{
			/*_TCHAR* end = NULL;
			val = _tcstod(str, &end);

			c2 = 0;
			c1 = HIDOpenUPS2::GetDataVOut(val);
			ok = true;*/
		}break;
	case 8:
		{
			if (_strcmpi(str, "Never")==0)
			{
				val = (double)0xFFFF * dMultiplier;
				ok = true;
			}
			else
			{
				TCHAR* end = NULL;
				val = _tcstod(str, &end);
				if ((errno != ERANGE)&&(errno != EINVAL)) ok = true;
			}
		}break;
	case 12:
	case 13:
		{
			c1 = 0;
			c2 = atoi(str);
			ok = true;
		}break;
	case 14:
		{
			c1 = 0;
			c2 = bin2char(str, &ok);
		}break;
	case 15:
		{
			unsigned int termistor = 0;
			
			int temp = atoi(str);
			if (temp <= -40) termistor = HIDOpenUPS2::GetTermistorConsts()[0];
			else if (temp >= 125) termistor = HIDOpenUPS2::GetTermistorConsts()[TERMISTOR_CONSTS-1];
			else 
			{
				int pos = (temp + 40) / 5;
				int temp_dif = (temp + 40) % 5;

				if (temp_dif == 0)
					termistor = HIDOpenUPS2::GetTermistorConsts()[pos];
				else
				{
					unsigned int d1 = HIDOpenUPS2::GetTermistorConsts()[pos];
					unsigned int d2 = HIDOpenUPS2::GetTermistorConsts()[pos+1];

					termistor = d1 + temp_dif * (d2-d1) / 5;
				}
			}

			c1 = (unsigned char)(((unsigned int)termistor) & 0xFF);
			c2 = (unsigned char)((((unsigned int)termistor)>>8) & 0xFF);
			ok = true;
		}break;
	case 16:
		{
			TCHAR* end = NULL;
			val = _tcstod(str, &end);

			if ((val != 0)&&(errno != ERANGE)&&(errno != EINVAL))
			{
				val = ceil(val / dMultiplier);
				unsigned int ival = (unsigned int) val;

				c1 = (unsigned char)( ival & 0xFF);
				c2 = (unsigned char)((ival>>8 ) & 0xFF);
				c3 = (unsigned char)((ival>>16) & 0xFF);
				c4 = (unsigned char)((ival>>24) & 0xFF);
				ok = true;
			}
		}break;
	case 17:
		{
			_TCHAR* end = NULL;
			val = _tcstod(str, &end);
			if ((errno != ERANGE)&&(errno != EINVAL)) ok = true;
			if ((val < 0) || (val > 255)) ok = false;
		}break;
	case 40:
		{
			_TCHAR* end = NULL;
			val  = _tcstol(str, &end, 16);
			ok = true;
		}break;//hex
	case 1:
		//normal read - integer
	case 2:
	case 9:
	case 5:
	default:
		{//normal read - float
			_TCHAR* end = NULL;
			val = _tcstod(str, &end);

			if ((errno != ERANGE)&&(errno != EINVAL)) ok = true;
		}
	}

	if (ok)
	{
		if ((nReadMode != 4)&&(nReadMode != 7)&&(nReadMode != 9)&&(nReadMode != 11)&&(nReadMode != 12)&&(nReadMode != 13)&&(nReadMode != 14)&&(nReadMode != 15)&&(nReadMode != 16))
		{
			switch (nReadMode)
			{
				case 10: val /= HIDOpenUPS2::GetConstant((int)dMultiplier); break;
				case 30: val /= HIDOpenUPS2::GetConstant(6); break;
				default:val /= dMultiplier;
			}

			if (nReadMode!=5)
			{//unsigned
				if (val < 0) val = 0;
				double rem = val - (int) val;
				unsigned long uval = (unsigned long)val;
				if (rem >= 0.5) uval += 1;

				if (len == 1)
				{
					if (uval > 0xFF) uval = 0xFF;
					
					c1 = 0;
					c2 = (unsigned char)uval;
				}
				else
				{
					if (uval > 0xFFFF) uval = 0xFFFF;

					c1 = (unsigned char)(((unsigned int)uval) & 0xFF);
					c2 = (unsigned char)((((unsigned int)uval)>>8) & 0xFF);
				}

				//DBGOutput("      uval1=%d %x\n",uval,uval);
			}
			else
			{
				double rem = -val + (int) val;
				long uval = (long)val;
				if (rem >= 0.5) uval += 1;

				if (len == 1)
				{
					if (uval > 127) uval = 127;
					if (uval <-128) uval = -128;
					
					c1 = 0;
					c2 = (char)uval;
				}
				else
				{
					if (uval > 32767) uval = 32767;
					if (uval <-32768) uval = -32768;

					c1 = (char)(((int)uval) & 0xFF);
					c2 = (char)((((int)uval)>>8) & 0xFF);
				}
				//DBGOutput("      uval2=%d %x\n",uval,uval);
			}
		}
	}

	//DBGOutput("readOneValue %f [%02x][%02x]\n",(float)val,(unsigned int)c2,(unsigned int)c1);

	return ok;
}