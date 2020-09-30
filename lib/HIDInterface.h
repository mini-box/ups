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
		int sendMessageWithBuffer(unsigned char cType, unsigned int buflen, unsigned char* buffer, unsigned int len, ...);
		int sendMessage(unsigned char cType, unsigned int len, ...);		
		int sendCommand(unsigned char command, unsigned char value);
		int sendCommandEx(unsigned char command, unsigned char value1, unsigned char value2);
		int recvMessage(unsigned char *buffer);
		int GetMessageIdxByName(const char* name);
		int varsToFile(const char *filename, bool withComments);
		int fileToVars(const char *filename);

		virtual void parseMessage(unsigned char *msg){};
		virtual void printValues(){};
		virtual float convertOneValue2Float(unsigned char *buffer, int nLen, int nIndex, int nReadMode, double dMultiplier){};
		virtual bool readOneValue(char *str, int nReadMode, double dMultiplier, int len, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4){};
		virtual void convertOneValue2String(char *destination, int nLen, int nIndex, int nReadMode, double dMultiplier){};
		virtual bool setVariableData(int mesg_no, char *str){};
		virtual ATXMSG* GetMessages(){};
		virtual double GetConstant(int i){};
		virtual unsigned int* GetTermistorConsts(){};
		virtual unsigned char getUPSVariableData(unsigned int cnt, char *name, char *value, char *unit, char *comment){};
		virtual void restartUPS(){};
		virtual void restartUPSInBootloaderMode(){};		

		unsigned char m_chPackages[SETTINGS_PACKS * 16];

		USBHID *d;
};

#endif // _HIDINTERFACE_H_