#ifndef _HIDNUCUPS_H_
#define _HIDINUCUPS_H_

#include "usbhid.h"
#include "util.h"
#include "HIDInterface.h"

#define NUC_OUT_REPORT_IO_DATA			0x81
#define NUC_IN_REPORT_IO_DATA			0x82
#define NUC_OUT_REPORT_IO_DATA2			0x83
#define NUC_IN_REPORT_IO_DATA2			0x84
#define NUC_OUT_REPORT_IO_DATA3			0x85
#define NUC_IN_REPORT_IO_DATA3			0x86
#define NUC_OUT_REPORT_IO_DATA4			0x87
#define NUC_IN_REPORT_IO_DATA4			0x88
#define NUC_OUT_REPORT_IO_DATA5			0x89
#define NUC_IN_REPORT_IO_DATA5			0x8A

#define NUC_CMD_OUT						0xB1
#define NUC_CMD_IN						0xB2
#define NUC_MEM_READ_OUT				0xA1
#define NUC_MEM_READ_IN					0xA2
#define NUC_MEM_WRITE_OUT				0xA3
#define NUC_MEM_WRITE_IN				0xA4
#define NUC_MEM_ERASE					0xA5

#define NUC_ENTER_BOOTLOADER_OUT		0xA9
#define NUC_ENTER_BOOTLOADER_IN			0xAA

#define INTERNAL_MESG					0xFF
#define INTERNAL_MESG_DISCONNECTED		0x01

//commands
#define DCMD_RESTART_NUC				0xAA

//For reading out memory
#define TYPE_CODE_MEMORY				0x00
#define TYPE_EPROM_EXTERNAL				0x01
#define TYPE_EPROM_INTERNAL				0x02
#define TYPE_CODE_SPLASH				0x03

#define FLASH_REPORT_ERASE_MEMORY		0xF2				// AddressLo : AddressHi : AddressUp (anywhere inside the 64 byte-block to be erased)
#define FLASH_REPORT_READ_MEMORY		0xF3				// AddressLo : AddressHi : AddressUp : Data Length (1...32)
#define FLASH_REPORT_WRITE_MEMORY		0xF4				// AddressLo : AddressHi : AddressUp : Data Length (1...32) : Data....
#define KEYBD_REPORT_ERASE_MEMORY		0xB2				// same as F2 but in keyboard mode
#define KEYBD_REPORT_READ_MEMORY		0xB3				// same as F3 but in keyboard mode
#define KEYBD_REPORT_WRITE_MEMORY		0xB4				// same as F4 but in keyboard mode
#define KEYBD_REPORT_MEMORY				0x41				// response to b3,b4

#define IN_REPORT_EXT_EE_DATA		0x31
#define OUT_REPORT_EXT_EE_READ		0xA1
#define OUT_REPORT_EXT_EE_WRITE		0xA2

#define IN_REPORT_INT_EE_DATA		0x32
#define OUT_REPORT_INT_EE_READ		0xA3
#define OUT_REPORT_INT_EE_WRITE		0xA4

///// MEASUREMENT CONSTANTS
#define NUC_CT_RW		(double)75
#define NUC_CT_RP		(double)5000
#define NUC_CT_R1_VOUT	(double)56200
#define NUC_CT_R2_VOUT	(double)2740
#define NUC_CT_V_FEEDBACK_OUT (double)1.2

#define CHECK_CHAR (unsigned char)0xAA //used for line/write check 

#define MAX_MESSAGE_CNT 256

#define TERMISTOR_CONSTS 34

#define NUC_SETTINGS_ADDR_START			0x003000
#define NUC_SETTINGS_ADDR_END			0x003100
#define NUC_SETTINGS_PACKS 16


class HIDNUCUPS: public HIDInterface {
	public:
		HIDNUCUPS(USBHID *d);
		~HIDNUCUPS();

		void GetStatus();
		void ReadConfigurationMemory();
		void EraseConfigurationMemory();
		void WriteConfigurationMemory();
		void parseMessage(unsigned char *msg);
		void printValues();
		float convertOneValue2Float(unsigned char *buffer, int nLen, int nIndex, int nReadMode, double dMultiplier);
		bool readOneValue(char *str, int nReadMode, double dMultiplier, int len, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4);
		bool readOneValue(int mesg_no, char* str, int nReadMode, double dMultiplier, int len, double dMin, double dMax, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4);

		void convertOneValue2String(char *destination, int nLen, int nIndex, int nReadMode, double dMultiplier);
		bool setVariableData(int mesg_no, char *str);
		ATXMSG* GetMessages();
		double GetConstant(int i);
		unsigned int* GetTermistorConsts();
		unsigned int GetSpecialConstants(unsigned int type, unsigned int cnt);
		unsigned char getUPSVariableData(unsigned int cnt, char *name, char *value, char *unit, char *comment);
		void restartUPS();
		void restartUPSInBootloaderMode();

private:
		unsigned int m_nInput;
		unsigned int m_nOutput;
		unsigned int m_nChargerFlags;
		unsigned int m_nStateFlags;
		unsigned int m_nShutdownFlags;

		float m_fVIn;
		float m_fIOut;
		float m_fVOut;
		float m_fVIgnition;
		float m_fPOut;
		float m_fTemperature[4];
		float m_fBatVoltage[4];
		unsigned char m_fCellBalanceOn[4];
		unsigned char m_fCellDetected[4];
		float m_fBatOverallVoltage;
		float m_fBatPack;
		float m_fIChgDchg;

		unsigned char m_nVerMajor;
		unsigned char m_nVerMinor;

		unsigned char m_bDbgOn;//debug messages should be requested

		unsigned char m_nState;//offline,usb,batpowered,vinpowered
		unsigned char m_bDBG[32];
		unsigned char m_bDBG2[32];
		unsigned char m_bDBG3[32];

		unsigned int m_nTimer[10];
		unsigned int m_nStateMachine[5];
		unsigned int m_nChgTimer[5];

		unsigned char m_chChargeEndedCondition;
		unsigned char m_chBatteryLevel;
		
		unsigned char m_chMode;

		unsigned int m_nCPUUsage;
		
		const char *SM_UPS[16] = { "INIT",
					   "LPWR",
					   "OFF",
					   "WAIT_IGNITION_ON_TO_OUTPUT_ON",
					   "OUTPUT_ON",
					   "WAIT_OUTPUT_ON_TO_MOBPULSE_ON",
					   "MOBPULSE_ON",
					   "ON",
					   "WAIT_IGNITION_OFF_TO_MOBPULSE_OFF",
					   "WAIT_HARDOF",
					   "OUTPUT_OFF",
					   "BUCKBOOST_ON",
					   "MOBPULSE_OFF",
					   "ENTER_LOWPOWER",
					   "UNKNOWN",
					   "UNKNOWN"};
	

		const char *SM_AFE[8] = {  "IDLE",
					   "INIT",
					   "GAINSREAD",
					   "UPDATE",
					   "NORMAL",
					   "SHIP01",
					   "SHIP10",
					   "SHIPMODE"};
	


		const char *SM_CHG[16] = { "IDLE",
					   "INIT",
					   "CONDITION",
					   "START",
					   "PRECHARGE_CC",
					   "WAIT_BULK_CC",
					   "BULK_CC",
					   "BULK_CV",
					   "BULK_END",
					   "FLOAT",
					   "UNKNOWN",
					   "UNKNOWN",
					   "UNKNOWN",
					   "UNKNOWN",
					   "UNKNOWN",
					   "END"};
	
	
		const char *SM_DTCT[16] = {"IDLE",
					   "STOP_BALANCE",
					   "START",
					   "WMEASCYCLCOMPL",
					   "START_AGAIN",
					   "WMEASCYCLCOMPL_AGAIN",
					   "CALC",
					   "UNKNOWN",
					   "UNKNOWN",
					   "UNKNOWN",
					   "UNKNOWN",
					   "UNKNOWN",
					   "UNKNOWN",
					   "UNKNOWN",
					   "UNKNOWN",
					   "END"};
	



		const char *SM_LOWP[4] = { "OFF",
					   "ON",
					   "STANDBY",
					   "SHIP"};
	


	
	
};

#endif // _HIDNUCUPS_H_