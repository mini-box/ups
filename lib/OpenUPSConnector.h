#pragma once
#include "HThread.h"
#include "HLock.h"
#include "HArray.h"
#include "TxFile.h"
#include "HIDOpenUPS2.h"
#include "DeviceDetector.h"

#define SETTINGS_PACKS 64
//state machin states
#define SM_BASIC	1
#define SM_LOADING	2
#define SM_SAVING	3

#define DBG_LEN		25 //4 + 17 + 4

class OpenUPSConnector :
	public HThread, private DeviceDetector
{
public:
	OpenUPSConnector(unsigned int timer, int counter);
	virtual ~OpenUPSConnector(void);

	bool OpenDevice();
	void CloseDevice();

	virtual void run( void );

	HIDOpenUPS2*	m_pUPS;

	bool m_bConnected;

	int m_nDevCounter;

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

	unsigned char m_nState;//offline,usb,batpowered,vinpowered

	unsigned char m_bStateUPS;
	unsigned char m_bStateCHG;
	unsigned char m_bStateDBG;
	unsigned char m_bCapacity;
	unsigned char m_bBatOn[3];
	unsigned int  m_nRTE;
	unsigned char m_bUPSMode;

	unsigned char m_bVOutPot;
	unsigned char m_bConfigSwitch;

	unsigned char m_bDBG[DBG_LEN];//4 byte + 8 byte + 4 byte
	unsigned char m_bDBG2[30];
	
	unsigned int  m_nShutdownType;//from FW 1.4

	unsigned char m_chStateMachine;
	unsigned char m_nLoadCfgStatus;
	unsigned char m_nSaveCfgStatus;
	
	void readIniFromDevice(bool tofile, bool compare_with_old);
	void writeIniToDevice(bool fromfile);

	unsigned char setVariableData(int mesg_no, char* str);
	float convertOneValue2Float(unsigned char* buffer, int nLen, int nIndex, int nReadMode, double dMultiplier);
	void  convertOneValue2String(char* destination, int nLen, int nIndex, int nReadMode, double dMultiplier);

	void sendCommand(unsigned char command, unsigned char value);//send UPS command
	void sendCommand(unsigned char command, unsigned char value1, unsigned char value2);//send UPS command

	void sendMessage(unsigned char mesg, unsigned char value1, unsigned char value2);//send UPS message/generic

	bool readOneValue(char* str, int nReadMode, double dMultiplier, int len, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4);

private:
	int m_nThreadCounter;
	unsigned char m_chPackages[SETTINGS_PACKS*16];

	bool  m_bSettingsCompareWithOld;//compare the old settings with the newly arriwed one
	FILE* m_fileSettingsFromDevice;
	bool  m_bSettingsToDeviceFile;
	TxFile m_txfSettingsToDevice;
	unsigned long m_ulSettingsAddr;

	unsigned int m_nTimerMax;
	unsigned int m_nTimer1_4;//1/4
	unsigned int m_nTimer2_4;//2/4
	unsigned int m_nTimer3_4;//3/4

	virtual void deviceChanged();

protected:
	
	void resetValues();

	void parseMessage(HArray* parray);
	HLock m_lock;
};
