// OpenUPS2Lib.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "OpenUPS2Lib.h"
#include "OpenUPSConnector.h"
#include "..\HIDDevice.h"
#include "..\HIDDescriptor.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

OpenUPSConnector* g_pUPSConn = NULL;

OPENUPS2LIB_API unsigned char upsOpenDeviceHandler(unsigned int timer)
{
	if (!g_pUPSConn)
		g_pUPSConn = new OpenUPSConnector(timer, -1);

	g_pUPSConn->m_nDevCounter = -1;
	
	if (g_pUPSConn->OpenDevice()) 
		 return 1;
	else return 0;
}

OPENUPS2LIB_API unsigned char upsOpenDeviceHandlerByCounter(unsigned int timer, int counter)
{
	if (!g_pUPSConn)
		g_pUPSConn = new OpenUPSConnector(timer, counter);
	
	g_pUPSConn->m_nDevCounter = counter;

	if (g_pUPSConn->OpenDevice()) 
		 return 1;
	else return 0;
}

OPENUPS2LIB_API void upsCloseDeviceHandler()
{
	if (g_pUPSConn)
	{
		g_pUPSConn->CloseDevice();
		delete g_pUPSConn;
		g_pUPSConn = NULL;
	}
}

OPENUPS2LIB_API void getUPSDevicePath(char* path)
{
	path[0] = 0;

	if (g_pUPSConn && g_pUPSConn->m_bConnected)
	{
		if (g_pUPSConn->m_pUPS->getParent())
		{
			if (g_pUPSConn->m_pUPS->getParent()->GetDescriptor())
				strcpy(path, g_pUPSConn->m_pUPS->getParent()->GetDescriptor()->GetHIDPath());
		}
	}
}

OPENUPS2LIB_API unsigned char isUPSConnected()
{
	if (!g_pUPSConn) return 0;
	if (g_pUPSConn->m_bConnected)
		return g_pUPSConn->m_chStateMachine;
	else return 0;
}

OPENUPS2LIB_API float getUPSVIN()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_fVIN;
}

OPENUPS2LIB_API float getUPSVBat()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_fVBat;
}

OPENUPS2LIB_API float getUPSVOut()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_fVOut;
}

OPENUPS2LIB_API float getUPSCCharge()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_fCCharge;
}

OPENUPS2LIB_API float getUPSCDischarge()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_fCDischarge;
}

OPENUPS2LIB_API float getUPSVCell(int i)
{
	if (!g_pUPSConn) return 0;
	if ((i>=0)&&(i<3))
		return g_pUPSConn->m_fVCell[i];
	else return 0;
}

OPENUPS2LIB_API float getUPSVDuty()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_fVDuty;
}

OPENUPS2LIB_API float  getUPSTemperature(int i)
{
	if (!g_pUPSConn) return 0;
	if ((i>=0)&&(i<3))
		 return g_pUPSConn->m_fTemperature[i];
	else return 0;
}

OPENUPS2LIB_API unsigned char getUPSVerMajor()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_nVerMajor;
}

OPENUPS2LIB_API unsigned char getUPSVerMinor()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_nVerMinor;
}

OPENUPS2LIB_API unsigned char getUPSState()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_nState;
}

OPENUPS2LIB_API unsigned char getUPSRemainingCapacity()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_bCapacity;
}

OPENUPS2LIB_API unsigned char getUPSVOutPot()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_bVOutPot;
}

OPENUPS2LIB_API unsigned char getUPSConfigSwitch()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_bConfigSwitch;
}

OPENUPS2LIB_API unsigned char getUPSStateByteUPS()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_bStateUPS;
}

OPENUPS2LIB_API unsigned char getUPSStateByteCHG()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_bStateCHG;
}

OPENUPS2LIB_API unsigned char getUPSStateByteDBG()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_bStateDBG;
}

OPENUPS2LIB_API unsigned char getUPSBatteryOn(int i)
{
	if (!g_pUPSConn) return 0;
	if ((i>=0)&&(i<3))
		return g_pUPSConn->m_bBatOn[i];
	else return 0;
}

OPENUPS2LIB_API unsigned int  getUPSRTE()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_nRTE;
}

OPENUPS2LIB_API unsigned char getUPSMode()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_bUPSMode;
}

OPENUPS2LIB_API unsigned char getUPSDbgByte(int i)
{
	if (!g_pUPSConn) return 0;
	if ((i>=0)&&(i<DBG_LEN))
		return g_pUPSConn->m_bDBG[i];
	else return 0;
}

OPENUPS2LIB_API unsigned char getUPSDbg2Byte(int i)
{
	if (!g_pUPSConn) return 0;
	if ((i>=0)&&(i<30))
		return g_pUPSConn->m_bDBG2[i];
	else return 0;
}

OPENUPS2LIB_API unsigned int getUPSShutdownType()
{
	if (!g_pUPSConn) return 0;
	return g_pUPSConn->m_nShutdownType;
}

OPENUPS2LIB_API unsigned char getUPSCheck23(int i)
{
	if (!g_pUPSConn) return 0;
	if ((i>=0)&&(i<8))
		return g_pUPSConn->m_bCheck23[i];
	else return 0;
}

OPENUPS2LIB_API unsigned char getUPSCheck24(int i)
{
	if (!g_pUPSConn) return 0;
	if ((i>=0)&&(i<8))
		return g_pUPSConn->m_bCheck24[i];
	else return 0;
}

OPENUPS2LIB_API unsigned char getUPSCheck25(int i)
{
	if (!g_pUPSConn) return 0;
	if ((i>=0)&&(i<8))
		return g_pUPSConn->m_bCheck25[i];
	else return 0;
}
//////////////////////////////////////////////////////////////////////////////////
OPENUPS2LIB_API void restartUPS()
{
	setUPSCommand1Byte(DCMD_RESTART_UPS, 1);
}

OPENUPS2LIB_API void restartUPSInBootloaderMode()
{
	if (!g_pUPSConn) return;
	g_pUPSConn->sendMessage(0xA9, 0xFA, 0xCD);
}

OPENUPS2LIB_API void setUPSCommand1Byte(unsigned char command, unsigned char value)
{
	if (!g_pUPSConn) return;
	g_pUPSConn->sendCommand(command, value);
}

OPENUPS2LIB_API void setUPSCommand2Byte(unsigned char command, unsigned int  value)
{
	if (!g_pUPSConn) return;
	g_pUPSConn->sendCommand(command, (value >> 8) & 0xFF, value & 0xFF);
}

OPENUPS2LIB_API void setVOutVolatile(float vout)
{
	if (!g_pUPSConn) return;
//	unsigned char val = HIDOpenUPS2::GetDataVOut(vout);
//	g_pUPSConn->sendCommand(DCMD_SPI_WRITE_DATA_VOL_VOUT, val, 0);

	int voltage = (int)(vout * 1000.0);//mV
	g_pUPSConn->sendCommand(DCMD_SPI_WRITE_DATA_VOL_VOUT, (voltage >> 8) & 0xFF, voltage & 0xFF);
}

OPENUPS2LIB_API void incDecVOutVolatile(unsigned char inc)
{
	if (inc)//wiper is opposite to vout
		 setUPSCommand1Byte(DCMD_SPI_DEC_VOL_WIPER_VOUT, 0);
	else setUPSCommand1Byte(DCMD_SPI_INC_VOL_WIPER_VOUT, 0);
}

//////////////////////////////////////////////////////////////////////////////////
OPENUPS2LIB_API unsigned int getUPSMaxVariableCnt()
{
	return MAX_MESSAGE_CNT;
}

OPENUPS2LIB_API unsigned char getUPSVariableData(unsigned int cnt, char* name, char* value, char* unit, char* comment)
{
	if (!g_pUPSConn) return 0;
	if ((cnt < 0)||(cnt>=MAX_MESSAGE_CNT)) return 0;
	if (HIDOpenUPS2::GetMessages()[cnt].nLen == 0) return 0;

	if (name) strcpy(name, HIDOpenUPS2::GetMessages()[cnt].strName);
	if (unit) strcpy(unit, HIDOpenUPS2::GetMessages()[cnt].strUnit);
	if (comment) strcpy(comment, HIDOpenUPS2::GetMessages()[cnt].strText);

	if (value)
	{
		//if (g_pUPSConn->m_nLoadCfgStatus != 100) return 0;//todo back

		g_pUPSConn->convertOneValue2String(value
					, HIDOpenUPS2::GetMessages()[cnt].nLen
					, HIDOpenUPS2::GetMessages()[cnt].nIndex
					, HIDOpenUPS2::GetMessages()[cnt].nReadMode
					, HIDOpenUPS2::GetMessages()[cnt].dMultiplier);
	}

	return HIDOpenUPS2::GetMessages()[cnt].nReadMode;
}

OPENUPS2LIB_API unsigned char setUPSVariableData(unsigned int cnt, char* value)
{
	if (!g_pUPSConn) return false;
	if (g_pUPSConn->m_nLoadCfgStatus != 100) return false;
	if ((cnt < 0)||(cnt>=MAX_MESSAGE_CNT)) return false;
	if (HIDOpenUPS2::GetMessages()[cnt].nLen == 0) return false;

	return g_pUPSConn->setVariableData(cnt, value);
}

OPENUPS2LIB_API void startUPSLoadingSettings(unsigned char save_to_file, unsigned char compare_with_old)
{
	if (!g_pUPSConn) return;
	g_pUPSConn->readIniFromDevice(save_to_file != 0, compare_with_old != 0);
}

OPENUPS2LIB_API unsigned char getUPSLoadingSettingsState()
{//0-64 - steps, 100=success, 0xF1-0xFF=failure
	if (!g_pUPSConn) return 0xFF;
	return g_pUPSConn->m_nLoadCfgStatus;
}

OPENUPS2LIB_API void startUPSSaveSettings(unsigned char from_file)
{
	if (!g_pUPSConn) return;
	g_pUPSConn->writeIniToDevice(from_file != 0);
}

OPENUPS2LIB_API unsigned char getUPSSaveSettingsState()
{
	if (!g_pUPSConn) return 0xFF;
	return g_pUPSConn->m_nSaveCfgStatus;
}