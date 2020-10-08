#ifndef _HIDOPENUPS_H_
#define _HIDOPENUPS_H_

#include "HIDInterface.h"
#include "usbhid.h"

#include "HArray.h"

#define ETA_CHG (double)0.90

#define STATUS_OK						0x00
#define STATUS_ERASE					0x01
#define STATUS_WRITE					0x02
#define STATUS_READ						0x03
#define STATUS_ERROR					0xff

#define OPENUPS_GET_ALL_VALUES			0x81
#define OPENUPS_RECV_ALL_VALUES			0x82
#define OPENUPS_CLOCK_OUT				0x83
#define OPENUPS_CLOCK_IN					0x84
#define OPENUPS_GET_ALL_VALUES_2			0x85
#define OPENUPS_RECV_ALL_VALUES_2		0x86
#define OPENUPS_CMD_OUT					0xB1
#define OPENUPS_CMD_IN					0xB2
#define OPENUPS_MEM_READ_OUT				0xA1
#define OPENUPS_MEM_READ_IN				0xA2
#define OPENUPS_MEM_WRITE_OUT			0xA3
#define OPENUPS_MEM_WRITE_IN				0xA4
#define OPENUPS_MEM_ERASE				0xA5

#define OPENUPS_ENTER_CALIBRATION_OUT	0xA9
#define OPENUPS_ENTER_CALIBRATION_IN		0xAA

#define INTERNAL_MESG					0xFF
#define INTERNAL_MESG_DISCONNECTED		0x01

//commands
#define OPENUPS_DCMD_3V3LP						0x01
#define OPENUPS_DCMD_ENABLE_VINPATH				0x02
#define OPENUPS_DCMD_ENABLE_BATTERYPATH			0x03
#define OPENUPS_DCMD_ENABLE_OUTPUT				0x04
#define OPENUPS_DCMD_PWM_OUT					0x05
#define OPENUPS_DCMD_PSW						0x06
#define OPENUPS_DCMD_ENABLE_CHARGER				0x07
#define OPENUPS_DCMD_PWM_CHARGER				0x08
#define OPENUPS_DCMD_CHG_FB						0x09
#define OPENUPS_DCMD_BULK_CHARGE				0x0A

#define OPENUPS_DCMD_CELL1_DISCHARGE			0x11
#define OPENUPS_DCMD_CELL2_DISCHARGE			0x12
#define OPENUPS_DCMD_CELL3_DISCHARGE			0x13
#define OPENUPS_DCMD_CELL4_DISCHARGE			0x14
#define OPENUPS_DCMD_CELL5_DISCHARGE			0x15
#define OPENUPS_DCMD_CELL6_DISCHARGE			0x16

#define OPENUPS_DCMD_SPI_CS_AD					0x21
#define OPENUPS_DCMD_SPI_CS_VBAT				0x22
#define OPENUPS_DCMD_SPI_CS_TEMP				0x23
#define OPENUPS_DCMD_SPI_CS_VOUT				0x24
#define OPENUPS_DCMD_SPI_CLK					0x25
#define OPENUPS_DCMD_SPI_DATAOUT				0x26
	
#define OPENUPS_DCMD_SPI_WRITE_DATA_VOL_VOUT	0x30
#define OPENUPS_DCMD_SPI_READ_DATA_VOL_VOUT		0x31
#define OPENUPS_DCMD_SPI_INC_VOL_WIPER_VOUT		0x32
#define OPENUPS_DCMD_SPI_DEC_VOL_WIPER_VOUT		0x33
#define OPENUPS_DCMD_SPI_VOUT_STAT				0x34
#define OPENUPS_DCMD_SPI_VOUT_TCON				0x35

#define OPENUPS_DCMD_SPI_READ_AD_CHANNEL		0x36

#define OPENUPS_DCMD_SPI_WRITE_DATA_VOL_VCHG	0x40
#define OPENUPS_DCMD_SPI_READ_DATA_VOL_VCHG		0x41
//#define OPENUPS_DCMD_SPI_INC_VOL_WIPER_VCHG		0x42
//#define OPENUPS_DCMD_SPI_DEC_VOL_WIPER_VCHG		0x43
#define OPENUPS_DCMD_I2C_INC_VOL_WIPER_VCHG_ROUGH 0x45
#define OPENUPS_DCMD_I2C_DEC_VOL_WIPER_VCHG_ROUGH 0x46
#define OPENUPS_DCMD_I2C_INC_VOL_WIPER_VCHG_FINE  0x47
#define OPENUPS_DCMD_I2C_DEC_VOL_WIPER_VCHG_FINE  0x48

#define OPENUPS_DCMD_LED						0x50	

#define OPENUPS_DCMD_SET_UVP_LOW				0x70
#define OPENUPS_DCMD_SET_UVP_HIGH				0x71

#define OPENUPS_DCMD_SET_OUTPUT_FREQUENCY		0x72
#define OPENUPS_DCMD_SET_CHARGE_FREQUENCY		0x73

#define OPENUPS_DCMD_RESTART_UPS				0xAA

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
#define CT_RW		(double)75
#define CT_RP		(double)5000
#define CT_R1_VOUT	(double)27000//24900
#define CT_R2_VOUT	(double)750

#define CT_R1_VBAT	(double)150000
#define CT_R2_VBAT	(double)4990
#define CT_RP1_VBAT	(double)50000
#define CT_RP2_VBAT	(double)5000
#define CT_LSB_RP1	(double)(CT_RP1_VBAT/129)
#define CT_LSB_RP2	(double)(CT_RP2_VBAT/257)

#define CHECK_CHAR (unsigned char)0xAA //used for line/write check 

#define MAX_MESSAGE_CNT 256

#define TERMISTOR_CONSTS 34

#define SCRIPTS_PACKS 4
#define SCRIPT_START_ADDR			0x3C00//3F80
#define SCRIPT_END_ADDR				0x3FB0

class HIDOpenUPS : public HIDInterface
{
public:
	HIDOpenUPS(USBHID *d);
	virtual ~HIDOpenUPS();

	void GetStatus();
	void ReadConfigurationMemory();
	void EraseConfigurationMemory();
	void WriteConfigurationMemory();
	void parseMessage(unsigned char *msg);
	void printValues();
	float convertOneValue2Float(unsigned char *buffer, int nLen, int nIndex, int nReadMode, double dMultiplier);
	bool readOneValue(char *str, int nReadMode, double dMultiplier, int len, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4);
	void convertOneValue2String(char *destination, int nLen, int nIndex, int nReadMode, double dMultiplier);
	bool setVariableData(int mesg_no, char *str);
	UVP* GetUVPs();
	ATXMSG* GetMessages();
	double GetConstant(int i);
	unsigned int* GetTermistorConsts();
	unsigned char getUPSVariableData(unsigned int cnt, char *name, char *value, char *unit, char *comment);
	void restartUPS();
	void restartUPSInBootloaderMode();

	double GetVoltageVOut(unsigned char data);
	unsigned char GetDataVOut(double vout);
	double GetVoltageVBat(unsigned char rp1, unsigned rp2);
	unsigned short GetDataVBat(double vout);

private:
	float m_fVIN;
	float m_fVBat;
	float m_fVOut;
	float m_fCCharge;
	float m_fCDischarge;
	float m_fCIn;
	float m_fTemperature;
	float m_fVCell[6];
	bool  m_bCheck23[8];
	bool  m_bCheck24[8];
	bool  m_bCheck25[8];

	unsigned char m_nVerMajor;
	unsigned char m_nVerMinor;

	unsigned char m_nState;

	unsigned int m_nYear;
	unsigned int m_nMonth;
	unsigned int m_nDay;

	unsigned int m_nHour;
	unsigned int m_nMin;
	unsigned int m_nSec;

	unsigned int m_nCapacity;
	unsigned int m_nRTE;

	float m_fOutputPower;

	unsigned char m_nOtherState[7];
};

#endif
