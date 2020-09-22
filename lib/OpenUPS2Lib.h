// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the OPENUPS2LIB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// OPENUPS2LIB_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef OPENUPS2LIB_EXPORTS
#define OPENUPS2LIB_API __declspec(dllexport)
#else
#define OPENUPS2LIB_API __declspec(dllimport)
#endif

#define DCMD_SPI_WRITE_DATA_VOL_VOUT	0x30
#define DCMD_SPI_INC_VOL_WIPER_VOUT		0x32
#define DCMD_SPI_DEC_VOL_WIPER_VOUT		0x33

extern "C" OPENUPS2LIB_API unsigned char upsOpenDeviceHandler(unsigned int timer);
extern "C" OPENUPS2LIB_API unsigned char upsOpenDeviceHandlerByCounter(unsigned int timer, int counter);
extern "C" OPENUPS2LIB_API void upsCloseDeviceHandler();
/** Get opened device path
@param path - recommended length 1024, will return empty string if no device opened*/
extern "C" OPENUPS2LIB_API void getUPSDevicePath(char* path);

extern "C" OPENUPS2LIB_API unsigned char isUPSConnected();//0=not connected, 1=normal state,2=loading settings from device,3=saving settings from pc,4=saving settings from file
extern "C" OPENUPS2LIB_API float getUPSVIN();
extern "C" OPENUPS2LIB_API float getUPSVBat();
extern "C" OPENUPS2LIB_API float getUPSVOut();
extern "C" OPENUPS2LIB_API float getUPSCCharge();
extern "C" OPENUPS2LIB_API float getUPSCDischarge();
extern "C" OPENUPS2LIB_API float getUPSVDuty();
extern "C" OPENUPS2LIB_API float getUPSVCell(int i);
extern "C" OPENUPS2LIB_API float getUPSTemperature(int i);
extern "C" OPENUPS2LIB_API unsigned char getUPSVerMajor();
extern "C" OPENUPS2LIB_API unsigned char getUPSVerMinor();
extern "C" OPENUPS2LIB_API unsigned char getUPSState();

extern "C" OPENUPS2LIB_API unsigned char getUPSRemainingCapacity();

extern "C" OPENUPS2LIB_API unsigned char getUPSVOutPot();
extern "C" OPENUPS2LIB_API unsigned char getUPSConfigSwitch();
extern "C" OPENUPS2LIB_API unsigned char getUPSStateByteUPS();
extern "C" OPENUPS2LIB_API unsigned char getUPSStateByteCHG();
extern "C" OPENUPS2LIB_API unsigned char getUPSStateByteDBG();
extern "C" OPENUPS2LIB_API unsigned char getUPSBatteryOn(int i);
extern "C" OPENUPS2LIB_API unsigned int  getUPSRTE();
extern "C" OPENUPS2LIB_API unsigned char getUPSMode();
extern "C" OPENUPS2LIB_API unsigned char getUPSDbgByte(int i);
extern "C" OPENUPS2LIB_API unsigned char getUPSDbg2Byte(int i);

extern "C" OPENUPS2LIB_API unsigned int getUPSShutdownType();//works only from Firmware 1.4

extern "C" OPENUPS2LIB_API unsigned char getUPSCheck23(int i);
extern "C" OPENUPS2LIB_API unsigned char getUPSCheck24(int i);
extern "C" OPENUPS2LIB_API unsigned char getUPSCheck25(int i);

extern "C" OPENUPS2LIB_API void restartUPS();
extern "C" OPENUPS2LIB_API void restartUPSInBootloaderMode();

extern "C" OPENUPS2LIB_API void setUPSCommand1Byte(unsigned char command, unsigned char value);
extern "C" OPENUPS2LIB_API void setUPSCommand2Byte(unsigned char command, unsigned int  value);

extern "C" OPENUPS2LIB_API void incDecVOutVolatile(unsigned char inc);
extern "C" OPENUPS2LIB_API void setVOutVolatile(float vout);

///////////////////////////////////////////////////////////////
extern "C" OPENUPS2LIB_API unsigned int getUPSMaxVariableCnt();
extern "C" OPENUPS2LIB_API unsigned char getUPSVariableData(unsigned int cnt, char* name, char* value, char* unit, char* comment);
extern "C" OPENUPS2LIB_API void startUPSLoadingSettings(unsigned char to_file, unsigned char compare_with_old);//from device
extern "C" OPENUPS2LIB_API unsigned char getUPSLoadingSettingsState();//0-64 - steps, 100=success, 0xF1-0xFF=failure
extern "C" OPENUPS2LIB_API unsigned char setUPSVariableData(unsigned int cnt, char* value);
extern "C" OPENUPS2LIB_API void startUPSSaveSettings(unsigned char from_file);//to device
extern "C" OPENUPS2LIB_API unsigned char getUPSSaveSettingsState();//0-64 - steps, 100=success, 0xF1-0xFF=failure

