#ifndef _HIDINTERFACE_H_
#define _HIDINTERFACE_H_

#include "usbhid.h"

#define DBG_LEN 25 //4 + 17 + 4
#define SETTINGS_PACKS 64
#define MAX_MESSAGE_CNT 256

struct ATXMSG
{
	unsigned int nIndex;
	unsigned int nLen;
	const char* strName;
	bool bEnabled; // enabled to write or just to read
	int nReadMode; // 1=normal, 2=float, 3=hh:mm:ss
	double	 dMultiplier;
	const char*  strText; // description/comment
	const char*  strUnit; // measurement unit
};
typedef ATXMSG _ATXMSG;


class HIDInterface {
	public:
		HIDInterface(USBHID *d);
		virtual ~HIDInterface();

		virtual void GetStatus() = 0;
		virtual void ReadConfigurationMemory() = 0;
		virtual void EraseConfigurationMemory() = 0;
		virtual void WriteConfigurationMemory() = 0;
		
		int sendMessageWithBuffer(unsigned char cType, unsigned int buflen, unsigned char* buffer, unsigned int len, ...);
		int sendMessage(unsigned char cType, unsigned int len, ...);		
		int sendCommand(unsigned char command, unsigned char value);
		int sendCommandEx(unsigned char command, unsigned char value1, unsigned char value2);
		int recvMessage(unsigned char *buffer);
		int GetMessageIdxByName(const char* name);
		int varsToFile(const char *filename, bool withComments);
		int fileToVars(const char *filename);

		virtual void parseMessage(unsigned char *msg) = 0;
		virtual void printValues() = 0;
		void printConfiguration(bool withComments);
		virtual float convertOneValue2Float(unsigned char *buffer, int nLen, int nIndex, int nReadMode, double dMultiplier) = 0;
		virtual bool readOneValue(char *str, int nReadMode, double dMultiplier, int len, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4) = 0;
		virtual void convertOneValue2String(char *destination, int nLen, int nIndex, int nReadMode, double dMultiplier) = 0;
		virtual bool setVariableData(int mesg_no, char *str) = 0;
		virtual ATXMSG* GetMessages() = 0;
		virtual double GetConstant(int i) = 0;
		virtual unsigned int* GetTermistorConsts() = 0;
		virtual unsigned char getUPSVariableData(unsigned int cnt, char *name, char *value, char *unit, char *comment) = 0;
		virtual void restartUPS() = 0;
		virtual void restartUPSInBootloaderMode() = 0;

		unsigned char m_chPackages[SETTINGS_PACKS * 16];

		USBHID *d;
};

#endif // _HIDINTERFACE_H_