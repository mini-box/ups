

#ifndef _HIDDCDCUSB_H_
#define _HIDDCDCUSB_H_

#include "HIDInterface.h"
#include "usbhid.h"

#define MAX_BUF 500

class HIDGraphics;

#define STATUS_OK 0x00
#define STATUS_ERASE 0x01
#define STATUS_WRITE 0x02
#define STATUS_READ 0x03
#define STATUS_ERROR 0xff

#define DCDCUSB_SETTINGS_ADDR_START 0x003FC0
#define DCDCUSB_SETTINGS_ADDR_END 0x004000
#define DCDCUSB_SETTINGS_PACKS	4

#define DCDCUSB_GET_ALL_VALUES 	0x81
#define DCDCUSB_RECV_ALL_VALUES 0x82
#define DCDCUSB_CMD_OUT 		0xB1
#define DCDCUSB_CMD_IN 			0xB2
#define DCDCUSB_MEM_READ_OUT 	0xA1
#define DCDCUSB_MEM_READ_IN 	0xA2
#define DCDCUSB_MEM_WRITE_OUT 	0xA3
#define DCDCUSB_MEM_WRITE_IN 	0xA4
#define DCDCUSB_MEM_ERASE 		0xA5

#define DCDCUSB_INTERNAL_MESG 	0xFF
#define DCDCUSB_INTERNAL_MESG_DISCONNECTED 0x01

#define DCDCUSB_CMD_SET_AUX_WIN 	0x01
#define DCDCUSB_CMD_SET_PW_SWITCH 	0x02
#define DCDCUSB_CMD_SET_OUTPUT 		0x03
#define DCDCUSB_CMD_WRITE_VOUT 		0x06
#define DCDCUSB_CMD_READ_VOUT 		0x07
#define DCDCUSB_CMD_INC_VOUT		0x0C
#define DCDCUSB_CMD_DEC_VOUT 		0x0D
#define DCDCUSB_CMD_LOAD_DEFAULTS 	0x0E
#define DCDCUSB_CMD_SCRIPT_START 	0x10
#define DCDCUSB_CMD_SCRIPT_STOP 	0x11
#define DCDCUSB_CMD_SLEEP 			0x12
#define DCDCUSB_CMD_READ_REGULATOR_STEP 0x13

//For reading out memory
#define DCDCUSB_TYPE_CODE_MEMORY 	0x00
#define DCDCUSB_TYPE_EPROM_EXTERNAL 0x01
#define DCDCUSB_TYPE_EPROM_INTERNAL 0x02
#define DCDCUSB_TYPE_CODE_SPLASH 	0x03

#define DCDCUSB_FLASH_REPORT_ERASE_MEMORY 	0xF2 // AddressLo : AddressHi : AddressUp (anywhere inside the 64 byte-block to be erased)
#define DCDCUSB_FLASH_REPORT_READ_MEMORY 	0xF3 // AddressLo : AddressHi : AddressUp : Data Length (1...32)
#define DCDCUSB_FLASH_REPORT_WRITE_MEMORY 	0xF4 // AddressLo : AddressHi : AddressUp : Data Length (1...32) : Data....
#define DCDCUSB_KEYBD_REPORT_ERASE_MEMORY 	0xB2 // same as F2 but in keyboard mode
#define DCDCUSB_KEYBD_REPORT_READ_MEMORY 	0xB3 // same as F3 but in keyboard mode
#define DCDCUSB_KEYBD_REPORT_WRITE_MEMORY 	0xB4 // same as F4 but in keyboard mode
#define DCDCUSB_KEYBD_REPORT_MEMORY 		0x41 // response to b3,b4

#define IN_REPORT_EXT_EE_DATA 	0x31
#define OUT_REPORT_EXT_EE_READ 	0xA1
#define OUT_REPORT_EXT_EE_WRITE 0xA2

#define IN_REPORT_INT_EE_DATA 	0x32
#define OUT_REPORT_INT_EE_READ 	0xA3
#define OUT_REPORT_INT_EE_WRITE 0xA4

///// MEASUREMENT CONSTANTS
#define DCDCUSB_CT_RW 	(double)75
#define DCDCUSB_CT_R1 	(double)49900
#define DCDCUSB_CT_R2 	(double)1500
#define DCDCUSB_CT_RP 	(double)10000

#define CHECK_CHAR 		(unsigned char)0xAA //used for line/write check

#define DCDCUSB_MAX_MESSAGE_CNT 64

///// 
class HIDDCDCUSB : public HIDInterface
{

public:
	HIDDCDCUSB(USBHID *d);
	virtual ~HIDDCDCUSB();

	void parseMessage(unsigned char *msg);
	void printValues();
	void printConfiguration();

	bool readOneValue(char *str, int nReadMode, double dMultiplier, int len, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4);
	float convertOneValue2Float(unsigned char *buffer, int nLen, int nIndex, int nReadMode, double dMultiplier);
	void convertOneValue2String(char *destination, int nLen, int nIndex, int nReadMode, double dMultiplier);
	bool setVariableData(int mesg_no, char *str);
	ATXMSG *GetMessages();
	double GetConstant(int i);
	unsigned int *GetTermistorConsts();
	int GetMessageIdxByName(const char *name);
	unsigned char getUPSVariableData(unsigned int cnt, char *name, char *value, char *unit, char *comment);
	void restartUPS();
	void restartUPSInBootloaderMode();
	void setVOutVolatile(float vout);
	void incDecVOutVolatile(unsigned char inc);

	void GetStatus();
	void ReadConfigurationMemory();
	void EraseConfigurationMemory();
	void WriteConfigurationMemory();

	double GetVOut(unsigned char data);
	unsigned char GetData(double vout);

private:
	int m_nVoltageCfg;
	float m_fVIn;
	float m_fIgn;
	float m_fVOut;
	float m_fVOutSet;
	bool m_bPowerSwitch;
	bool m_bOutput;
	bool m_bAuxVin;
	bool m_bVersion;

	int m_nScriptPointer;
	unsigned char m_nVerMajor;
	unsigned char m_nVerMinor;
	int m_nMode;
	int m_nState;
	int m_nTimeCfg;

	char m_strFlagsVoltage[256];
	char m_strFlagsStatus1[256];
	char m_strFlagsStatus2[256];
	char m_strFlagsTimer[256];
	char m_strVoltCfg[256];
	char m_strMode[256];
	char m_strTimerWait[256];
	char m_strTimerVout[256];
	char m_strTimerPwSwitch[256];
	char m_strFlashPointer[256];
	char m_strTimerVAux[256];
	char m_strTimerOffDelay[256];
	char m_strTimerHardOff[256];
	char m_editRegulatorSteps[256];
};

#endif
