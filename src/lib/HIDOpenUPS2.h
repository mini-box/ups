#ifndef _HIDOPENUPS2_H_
#define _HIDOPENUPS2_H_

#include "HIDInterface.h"
#include "usbhid.h"

#include "HArray.h"
#include "HLock.h"
#include "HList.h"

#define OPENUPS2_MSG_READ  32
#define OPENUPS2_MSG_WRITE 32

#define MAX_BUF			500

#define ETA_CHG (double)0.90

#define STATUS_OK						0x00
#define STATUS_ERASE					0x01
#define STATUS_WRITE					0x02
#define STATUS_READ						0x03
#define STATUS_ERROR					0xff

#define OPENUPS2_GET_ALL_VALUES			0x81
#define OPENUPS2_RECV_ALL_VALUES		0x82
#define OPENUPS2_CLOCK_OUT				0x83
#define OPENUPS2_CLOCK_IN				0x84
#define OPENUPS2_GET_ALL_VALUES_2		0x85
#define OPENUPS2_RECV_ALL_VALUES_2		0x86
#define OPENUPS2_GET_ALL_VALUES_3		0x87
#define OPENUPS2_RECV_ALL_VALUES_3		0x88
#define OPENUPS2_CMD_OUT				0xB1
#define OPENUPS2_CMD_IN					0xB2
#define OPENUPS2_MEM_READ_OUT			0xA1
#define OPENUPS2_MEM_READ_IN			0xA2
#define OPENUPS2_MEM_WRITE_OUT			0xA3
#define OPENUPS2_MEM_WRITE_IN			0xA4
#define OPENUPS2_MEM_ERASE				0xA5

#define OPENUPS2_ENTER_BOOTLOADER_OUT	0xA9
#define OPENUPS2_ENTER_BOOTLOADER_IN	0xAA

#define INTERNAL_MESG					0xFF
#define INTERNAL_MESG_DISCONNECTED		0x01

//commands
#define OPENUPS2_DCMD_FORCE_PRECHARGE			0x01
#define OPENUPS2_DCMD_ENABLE_CHARGE				0x02
#define OPENUPS2_DCMD_PSW						0x03
#define OPENUPS2_DCMD_RELAY						0x04
#define OPENUPS2_DCMD_ENABLE_OUTPUT				0x05
#define OPENUPS2_DCMD_LDO_EN						0x07

#define OPENUPS2_DCMD_CELL1_DISCHARGE			0x11
#define OPENUPS2_DCMD_CELL2_DISCHARGE			0x12
#define OPENUPS2_DCMD_CELL3_DISCHARGE			0x13
#define OPENUPS2_DCMD_CELL4_DISCHARGE			0x14
#define OPENUPS2_DCMD_CELL5_DISCHARGE			0x15
#define OPENUPS2_DCMD_CELL6_DISCHARGE			0x16

#define OPENUPS2_DCMD_SPI_CS_AD					0x21
#define OPENUPS2_DCMD_SPI_CS_VBAT				0x22
#define OPENUPS2_DCMD_SPI_CS_TEMP				0x23
#define OPENUPS2_DCMD_SPI_CS_VOUT				0x24
#define OPENUPS2_DCMD_SPI_CLK					0x25
#define OPENUPS2_DCMD_SPI_DATAOUT				0x26
	
#define OPENUPS2_DCMD_SPI_WRITE_DATA_VOL_VOUT	0x30
#define OPENUPS2_DCMD_SPI_READ_DATA_VOL_VOUT		0x31
#define OPENUPS2_DCMD_SPI_INC_VOL_WIPER_VOUT		0x32
#define OPENUPS2_DCMD_SPI_DEC_VOL_WIPER_VOUT		0x33
#define OPENUPS2_DCMD_SPI_VOUT_STAT				0x34
#define OPENUPS2_DCMD_SPI_VOUT_TCON				0x35

#define OPENUPS2_DCMD_SPI_READ_AD_CHANNEL		0x36

#define OPENUPS2_DCMD_SPI_WRITE_DATA_VOL_VCHG	0x40
#define OPENUPS2_DCMD_SPI_READ_DATA_VOL_VCHG		0x41
//#define OPENUPS2_DCMD_SPI_INC_VOL_WIPER_VCHG		0x42
//#define OPENUPS2_DCMD_SPI_DEC_VOL_WIPER_VCHG		0x43
#define OPENUPS2_DCMD_I2C_INC_VOL_WIPER_VCHG_ROUGH 0x45
#define OPENUPS2_DCMD_I2C_DEC_VOL_WIPER_VCHG_ROUGH 0x46
#define OPENUPS2_DCMD_I2C_INC_VOL_WIPER_VCHG_FINE  0x47
#define OPENUPS2_DCMD_I2C_DEC_VOL_WIPER_VCHG_FINE  0x48

#define OPENUPS2_DCMD_LED						0x50
#define OPENUPS2_DCMD_READ_AFE					0x51
#define OPENUPS2_DCMD_WRITE_AFE					0x52

#define OPENUPS2_DCMD_SET_UVP_LOW				0x70
#define OPENUPS2_DCMD_SET_UVP_HIGH				0x71

#define OPENUPS2_DCMD_SET_OUTPUT_FREQUENCY		0x72
#define OPENUPS2_DCMD_SET_CHARGE_FREQUENCY		0x73

#define OPENUPS2_DCMD_RESTART_UPS				0xAA

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
#define OPENUPS2_CT_RW		(double)75
#define OPENUPS2_CT_RP		(double)5000
#define OPENUPS2_CT_R1_VOUT	(double)56200
#define OPENUPS2_CT_R2_VOUT	(double)2740
#define OPENUPS2_CT_V_FEEDBACK_OUT (double)1.2

#define CHECK_CHAR (unsigned char)0xAA //used for line/write check 

#define MAX_MESSAGE_CNT 256

#define TERMISTOR_CONSTS 34

#define OPENUPS2_SETTINGS_ADDR_START			0x007800
#define OPENUPS2_SETTINGS_ADDR_END				0x007C00
#define OPENUPS2_SETTINGS_ADDR_CALIBR_START		0x0078D0
#define OPENUPS2_SETTINGS_ADDR_CALIBR_END		0x007900
#define OPENUPS2_SETTINGS_PACKS					64
#define OPENUPS2_SCRIPTS_PACKS					4
#define OPENUPS2_SCRIPT_START_ADDR				0x3C00//3F80
#define OPENUPS2_SCRIPT_END_ADDR				0x3BF0


class HIDOpenUPS2 : public HIDInterface
{
public:
	HIDOpenUPS2(USBHID *d);
	virtual ~HIDOpenUPS2();
	
	void parseMessage(unsigned char *msg);
	void printValues();
	void printConfiguration();

	bool readOneValue(char *str, int nReadMode, double dMultiplier, int len, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4);
	float convertOneValue2Float(unsigned char *buffer, int nLen, int nIndex, int nReadMode, double dMultiplier);
	void convertOneValue2String(char *destination, int nLen, int nIndex, int nReadMode, double dMultiplier);
	bool setVariableData(int mesg_no, char *str);
	ATXMSG* GetMessages();
	double GetConstant(int i);
	unsigned int* GetTermistorConsts();
	int GetMessageIdxByName(const char* name);
	unsigned char getUPSVariableData(unsigned int cnt, char *name, char *value, char *unit, char *comment);
	void restartUPS();
	void restartUPSInBootloaderMode();
	void setVOutVolatile(float vout);
	void incDecVOutVolatile(unsigned char inc);

	void GetStatus();
	void ReadConfigurationMemory();
	void EraseConfigurationMemory();
	void WriteConfigurationMemory();


private:
	float m_fVIN;
	float m_fVBat;
	float m_fVOut;
	float m_fCCharge;
	float m_fCDischarge;
	float m_fTemperature[3];
	float m_fVCell[3];
	float m_fVDuty;
	unsigned char m_bCheck23[8];
	unsigned char m_bCheck24[8];
	unsigned char m_bCheck25[8];

	unsigned char m_nVerMajor;
	unsigned char m_nVerMinor;

	unsigned char m_nState; //offline,usb,batpowered,vinpowered

	unsigned char m_bStateUPS;
	unsigned char m_bStateCHG;
	unsigned char m_bStateDBG;
	unsigned char m_bCapacity;
	unsigned char m_bBatOn[3];
	unsigned int m_nRTE;
	unsigned char m_bUPSMode;

	unsigned char m_bVOutPot;
	unsigned char m_bConfigSwitch;

	unsigned char m_bDBG[DBG_LEN]; //4 byte + 8 byte + 4 byte
	unsigned char m_bDBG2[30];

	unsigned int m_nShutdownType; //from FW 1.4

	unsigned char m_chStateMachine;
	unsigned char m_nLoadCfgStatus;
	unsigned char m_nSaveCfgStatus;
};

#endif
