// HIDOpenUPS2.cpp: implementation of the HIDOpenUPS2 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HIDOpenUPS2.h"
#include "HIDDevice.h"
#include "HIDDescriptor.h"
#include "HIDGraphics.h"
#include <math.h>

#ifdef USE_MFC
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif

#define A_SLEEP			5      
#define A_TIMEOUT		3000
#define BUF_LINE_LEN	47
#define STR_LINE_LEN	57

#define PKG_LEN_LCD		64
#define PKG_LEN_FLASH	37

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

double g_memConstants[]=
{
	0.001, //Vin				//0
	0.001, //Vout				//1
	0.001, //Vbat+chg		    //2
	0.001, //A chg Hi range	    //3
	0.001, //A chg Low range	//4
	0.001, //A discharge		//5
	0.001  //battery
};

ATXMSG g_memMessages[MAX_MESSAGE_CNT] = 
{
	{67, 1, _T("OPENUPSMODE")						, true,17, 1		, _T("OPENUPS mode\r\n0-Autorestart when input is present=yes\t(UPS)\r\n1-Autorestart when input is present=no\t(LAPTOP)\r\n2-Energy Pack mode\t\t\t(ON/OFF)"), _T("[-]") },
	{28, 4, _T("CAPACITY")							, true,16, 1		, _T("Battery Capacity.\r\nDefault 1500mAh"), _T("[mAh]") },
	{80, 2, _T("MOB_ONOFF_TOUT")					, true, 1, 10		, _T("The powerswitch on the motherboard(if connected) will be shorted  this period to turn the Motherboard ON or OFF\r\nDefault is 500ms"), _T("[mS]") },
	{66, 1, _T("UPS_BUTTON_ON_TOUT")				, true, 1, 10		, _T("The pushbutton connected on the P10 header must be shorted this period to be considered as pressed.\r\nDefault 100ms."), _T("[mS]") },

	{ 0, 1, _T("UPS_CONFIG")						, true,14, 1		, _T("Configuration register. Used for enabling disabling modules.0-disabled;1-enabled\r\nbit7:reserved\r\nbit6:reserved\r\nbit5:reserved\r\nbit4:Enable No Battery Mode (FW 1.4+)\r\nbit3:reserved\r\nbit2:Enable temperature compensated charge algorithm\r\nbit1:Enable shutdown from battery if system was shut down from OS sensed by USB\r\nbit0:Enable output voltage settings directly from OUT_VOLTAGE parameter. If disabled output voltage is set by configuration switch on the back side of the PCBA."), _T("[bit7..bit0]") },
	{100,1, _T("UPS_CONFIG2")						, true,14, 1		, _T("Configuration register.Used for enabling disabling modules.0-disabled;1-enabled\r\nbit7:reserved\r\nbit6:reserved\r\nbit5:reserved\r\nbit4:reserved\r\nbit3:reserved\r\nbit2:reserved\r\nbit1:Enable Ship mode, consumption is less than 1uA. Unit can be waked up only by input power presence.\r\nbit0:Enable Sleep mode, consumption is 530uA. Unit can be waked up by button or input power presence.\r\nThe combination of bit1 ,bit0 sets the configurable low power modes\r\n01: Sleep mode CPU OFF (530uA)\r\n10: Ship mode CPU OFF(1uA)\r\n11 or 00: Standby mode CPU ON(15mA) ==> Ship mode CPU OFF(1uA)"), _T("[bit7..bit0]") },

//	{ 1, 1, _T("UPS_INIT_DELAY_TOUT")				, true, 1, 1		, _T("Initial delay before starting the UPS\r\nDefault is 1 sec."), _T("[s]") },
//	{ 2, 2, _T("UPS_VIN_MAX_SHUTDOWN")				, true,10, 0		, _T("Max allowed input voltage. In case input voltage exceeds predefined value shutdown will be initiated.\r\nDefault is 25V."), _T("[V]") },
	{ 4, 2, _T("UPS_VIN_MIN_START")					, true,10, 0		, _T("If input voltage is above this threshold the UPS will start and take power from the input.\r\nDefault is 11V."), _T("[V]") },
//  { 6, 2, _T("UPS_VIN_MIN_STOP")					, true,10, 0		, _T("If input voltage is below this threshold the UPS will try to run on battery.\r\nDefault is 6V."), _T("[V]") },
	{ 8, 2, _T("UPS_VCELL_MIN_START")				, true,10, 6		, _T("If Vin is not present and all the battery cells are above this threshold the UPS can start and runs on battery\r\nDefault is 3V."), _T("[V]") },
	{10, 2, _T("UPS_VCELL_MIN_STOP")				, true,10, 6		, _T("If VIN is not present and one of the battery cells is less than this threshold during the time period specified by UPS_VBAT_UVP_OFF_TOUT parameter the UPS will initiate shut down procedure.\r\nDefault is 2.8V"), _T("[V]") },
/**/{16, 2, _T("UPS_VCELL_MIN_HARD_STOP")			, true,10, 6		, _T("If VIN is not present and one of the battery cells is less than this threshold the UPS will instantly shut down to preserve battery.\r\nDefault is 2V "), _T("[V]") },
	{12, 2, _T("UPS_VBAT_UVP_OFF_TOUT")				, true, 1, 1		, _T("During this period battery cells are checked against UPS_VCELL_MIN_STOP. If cell voltage exceeds UPS_VCELL_MIN_STOP the timer is rearmed so it can filter unwanted noise in the system.\r\nDefault is 5 sec."), _T("[s]") },
	{14, 2, _T("UPS_HARDOFF_TOUT")					, true, 1, 1		, _T("After UPS_VBAT_UVP_OFF_TOUT passed motherboard is signaled to shut down. The UPS will wait this period so that the Motherboard can shut down gracefully, than cut power and enter deep sleep. This period should not be too long to prevent battery drain.\r\nDefault is 60 sec."), _T("[s]") },

	{101,2, _T("UPS_VBAT_SLEEP_TOUT")				, true, 3, 1		, _T("Deep sleep timeout..If system is running on battery and this period has elapsed with no input power applied, then the UPS will initiate shut down procedure. \"Never\" is allowed as value (to disable this feature).\r\nDefault is \"Never\"."), _T("[HH:MM:SS]") },
	
//moved up	{16, 2, _T("UPS_VCELL_MIN_HARD_STOP")			, true,10, 6		, _T("If VIN is not present and one of the battery cells is less than this threshold the UPS will instantly shut down to preserve battery.\r\nDefault is 2V "), _T("[V]") },
//	{18, 2, _T("UPS_SWITCHOVER_VBAT_TOUT")			, true, 1, 10		, _T("After UPS switches to battery, can switch back to Vin only after this time ellapses\r\nDefault is 1000 ms"), _T("[ms]") },
//	{20, 2, _T("UPS_SWITCHOVER_VIN_TOUT")			, true, 1, 1		, _T("After UPS switches to input, can switch back to battery only after this time ellapses\r\nDefault is 1 ms"), _T("[ms]") },
	{22, 2, _T("DCHG_IMAX")							, true,10, 5		, _T("Max allowed discharge current. In case discharge current exceeds this threshold shutdown will be initiated.\r\nDefault is 10 A."), _T("[A]") },
	{24, 1, _T("AFE_OLV")				            , true,40, 1		, _T("Advanced parameter.\r\nAFE Overload Voltage treshold configuration register.\r\nDefault is 00."), _T("[hex]") },
	{25, 1, _T("AFE_OLD")				            , true,40, 1		, _T("Advanced parameter.\r\nAFE Overload Delay time configuration register.\r\nDefault is 00."), _T("[hex]") },
	{26, 1, _T("AFE_SCC")				            , true,40, 1		, _T("Advanced parameter.\r\nAFE Short Circuit In Charge configuration register.\r\nDefault is 50."), _T("[hex]") },
	{27, 1, _T("AFE_SCD")				            , true,40, 1		, _T("Advanced parameter.\r\nAFE Short Circuit In Discharge configuration register.\r\nDefault is 50."), _T("[hex]") },
// moved up	{28, 4, _T("CAPACITY")							, true,16, 1		, _T("Battery Capacity.\r\nDefault 3400mAh"), _T("[mAh]") },
 
//moved down	{32, 2, _T("BAL_ENABLE_TOUT")					, true,1, 5		, _T("Balancing enable timeout.\r\nDefault is 2 min."), _T("[min]") },
//moved down	{34, 2, _T("BAL_DISABLE_TOUT")					, true,1, 5		, _T("Balancing disable timeout.\r\nDefault is 2 min."), _T("[min]") },
	{36, 2, _T("CHG_VCOND")							, true,10, 6		, _T("Conditioning/Pre-charge voltage. Charge current is limited until cell voltage exceeds this value and for at least CHG_TCOND time.\r\nDefault is 3V."), _T("[V]") },
	{38, 2, _T("CHG_TCOND")							, true, 1, 1		, _T("Conditioning/Precharge time. Charge current is limited until cell voltage exceeds CHG_VCOND value and it is applied during this time.\r\nDefault is 30 sec."), _T("[s]") },
//	{40, 2, _T("CHG_IBULK")							, true,10, 3		, _T("Fast charge current limit (constant current mode)\r\nDefault is 1750mA."), _T("[mA]") },
	{42, 2, _T("CHG_BULK_STOP_VOLTAGE")				, true,10, 6		, _T("Maximum allowed bulk charge voltage/cell during constant current/constant voltage charging\r\nDefault is 3.5[V/cell]"), _T("[V/cell]") },
	{71, 1, _T("CHG_HYSTERESIS")					, true,10, 6		, _T("An over-voltage value (CHG_BULK_STOP_VOLTAGE+CHG_HYSTERESIS ) that it is allowed when charging.\r\nIf one of the cells exceeds this value charging is immediately stopped.\r\nDefault is 100mV/cell."), _T("[V/cell]") },
//moved down	{44, 2, _T("OUT_FOLLOW_OFFSET")					, true,30, 10		, _T("-"), _T("[V]") },
//	{46, 2, _T("CHG_IMIN")							, true,10, 3		, _T("If charge current in CV mode is less than this value\r\nPbSO4: enter maintanence mode and apply the float charge voltage(CHG_START_VOLTAGE)\r\nLiFePO4: cut off the charge voltage\r\nDefault is 290mA"), _T("[mA]") },

	{96, 2, _T("CHG_GLOBAL_TOUT")					, true, 1, 0.5		, _T("Global charge timeout.\r\nDefault is 180 min."), _T("[min]") },
//	{98, 2, _T("CHG_IFLOAT")						, true,10, 3		, _T("Charge current limit in float charge mode for PbSO4 batteries (Firmware v1.3+)"), _T("[mA]") },
	{94, 2, _T("CHG_TOPPING_TIMER")					, true, 1, 1		, _T("For Lithium based batteries after an overvoltage condition is detected for a cell a resting period is set by this timer before applying a small topping charge in case other cells are still not charged.\r\nDefault is 18000s."), _T("[s]") },
	
	{48, 2, _T("CHG_START_VOLTAGE")					, true,10, 6		, _T("If cell voltage is below this value charging can be started.\r\nDefault is 3.32V/cell"), _T("[V/cell]") },
	{50, 1, _T("CHG_BAT_TYPE")						, true, 1, 1		, _T("The battery chemistry to be charged.\r\n1-LiFePO4"), _T("") },
	{51, 2, _T("CHG_TEMP_COLD")						, true,15, 1		, _T("COLD temperature treshold for temperature compensated charge current regulation\r\nDefault 5°C"), _T("[°C]") },
	{53, 2, _T("CHG_TEMP_COOL")						, true,15, 1		, _T("COOL temperature treshold for temperature compensated charge current regulation\r\nDefault 10°C"), _T("[°C]") },
	{55, 2, _T("CHG_TEMP_WARM")						, true,15, 1		, _T("WARM temperature treshold for temperature compensated charge current regulation\r\nDefault 50°C"), _T("[°C]") },
	{57, 2, _T("CHG_TEMP_HOT")						, true,15, 1		, _T("HOT temperature treshold for temperature compensated charge current regulation\r\nDefault 55°C"), _T("[°C]") },
//moved down {59, 1, _T("OUT_STARTVOLTAGE_OFFSET")			, true, 1, 1		, _T("Soft start output voltage offset"), _T("[nr]") },

	{60, 2, _T("BAL_VCELL_MIN")						, true,10, 6		, _T("Balancing is allowed  if cell voltages are above this value.\r\nDefault is 3.32V"), _T("[V]") },
	{62, 1, _T("BAL_VCELL_DIFF_START")				, true,10, 6		, _T("If the voltage difference between cells exceeds this value start balancing the cells.\r\nDefault is 70mV."), _T("[V]") },
	{63, 1, _T("BAL_VCELL_DIFF_STOP")				, true,10, 6		, _T("If the voltage difference between cells is less than this value stop balancing the cells.\r\nDefault is 40mV."), _T("[V]") },
/**/{32, 2, _T("BAL_ENABLE_TOUT")					, true,1,  5		, _T("Balancing enable timeout (FW 1.5+)\r\nDefault 72000 min."), _T("[min]") },
/**/{34, 2, _T("BAL_DISABLE_TOUT")					, true,1,  5		, _T("Balancing disable timeout (FW 1.5+)\r\nDefault 1440 min."), _T("[min]") },
	
	{64, 2, _T("OUT_VOLTAGE")						, true,/*7*/10, 1	, _T("The output voltage. Bit0 in UPS_CONFIG parameter should be set to “1” in order this parameter has effect.\r\nDefault is 12V."), _T("[V]") },
/**/{59, 1, _T("OUT_STARTVOLTAGE_OFFSET")			, true, 1, 1		, _T("Soft start output voltage offset"), _T("[nr]") },
//moved up	{66, 1, _T("UPS_BUTTON_ON_TOUT")				, true, 1, 10		, _T(""), _T("[]") },
//moved up	{67, 1, _T("OPENUPSMODE")						, true,17, 1		, _T("OpenUPS Mode\r\n0-Auto restart when Input Power is Present=YES\r\n1-Auto restart when Input Power is Present=NO"), _T("[-]") },
//	{68, 1, _T("OUT_REGULATOR_OFFSET")				, true, 1, 1		, _T("Regulator offset"), _T("[step]") },
	{69, 1, _T("OUT_MAX_REGULATOR_STEP")			, true, 1, 1		, _T("Maximum allowed regulation step number for output module (0-255)\r\nDefault 255."), _T("[step]") },
//removed	{70, 1, _T("CELLS")								, true, 1, 1		, _T("Number of configured cells to charge/balance\r\nDefault is 4."), _T("[pcs]") },
//moved up	{71, 1, _T("CHG_HYSTERESIS")					, true,10, 2		, _T("An overvoltage value (CHG_BULK_STOP_VOLTAGE +CHG_HYSTERESIS ) that it is allowed when charging.If one of the cells exceeds this value charging is imediately stopped.\r\nDefault is 200mV/cell."), _T("[V]") },
//removed	{72, 4, _T("POUT_LO")							, true,16, 0.000001	, _T("Output power low treshold for motherboard alive sensing.If output power is lower than this treshold shut down impulse is NOT sent to the motherboard.\r\nDefault is 2W."), _T("[W]") },
//removed	{76, 4, _T("POUT_HI")							, true,16, 0.000001	, _T("Output power high treshold for motherboard alive sensing. If output power is higher than this treshold shut down impulse is sent to the motherboard.\r\nDefault is 6W."), _T("[W]") },
//removed   {44, 2, _T("OUT_FOLLOW_OFFSET")					, true,30, 10		, _T("Offset for adaptive output regulation.\r\nDefault is 1V."), _T("[V]") },
	
//moved up	{80, 2, _T("MOB_ONOFF_TOUT")					, true, 1, 10		, _T("The powerswitch on the motherboard(if connected) will be shorted  this period to turn the Motherboard ON or OFF\r\nDefault is 500ms"), _T("[mS]") },
	{82, 2, _T("OCV_SOC0")							, true,10, 6		, _T("Open Circuit Voltage State Of Charge detection for initial 0% fuel gauge estimation.\r\nDefault is 3.1V"), _T("[V]") },
	{84, 2, _T("OCV_SOC10")							, true,10, 6		, _T("Open Circuit Voltage State Of Charge detection for initial 10% fuel gauge estimation.\r\nDefault is 3.2V"), _T("[V]") },
	{86, 2, _T("OCV_SOC25")							, true,10, 6		, _T("Open Circuit Voltage State Of Charge detection for initial 25% fuel gauge estimation.\r\nDefault is 3.25V"), _T("[V]") },
	{88, 2, _T("OCV_SOC50")							, true,10, 6		, _T("Open Circuit Voltage State Of Charge detection for initial 50% fuel gauge estimation.\r\nDefault is 3.30V"), _T("[V]") },
	{90, 2, _T("OCV_SOC75")							, true,10, 6		, _T("Open Circuit Voltage State Of Charge detection for initial 75% fuel gauge estimation.\r\nDefault is 3.32V"), _T("[V]") },
	{92, 2, _T("OCV_SOC100")						, true,10, 6		, _T("Open Circuit Voltage State Of Charge detection for initial 100% fuel gauge estimation.\r\nDefault is 3.35V"), _T("[V]") },
//moved up	{94, 2, _T("CHG_TOPPING_TIMER")					, true, 1, 1		, _T("For Lithium based batteries after an overvoltage condition is detected for a cell a resting period is set by this timer before applying a small topping charge in case other cells are still not charged.\r\nDefault is 1800 sec."), _T("[s]") },

//moved up	{96, 2, _T("CHG_GLOBAL_TOUT")					, true, 1, 0.5		, _T("Global charge timeout (Firmware v1.3+)"), _T("[min]") },
//deleted	{98, 2, _T("CHG_IFLOAT")						, true,10, 3		, _T("Charge current limit in float charge mode (Firmware v1.3+)"), _T("[mA]") },
//moved up  {100,1, _T("UPS_CONFIG2")						, true,14, 1		, _T("Configuration register (Firmware V1.4+). Used for enabling disabling modules. LSB bit is b0.\r\nbit0: if bit is set and running on battery and measured output power<POUT_LO than go to deep sleep"), _T("[bits]") },
//moved up	{101,2, _T("UPS_VBAT_SLEEP_TOUT")				, true, 3, 1		, _T("Deep sleep timeout (Firmware V1.4+).  If system is running on battery and this period has elapsed with no input power applied then the UPS will initiate shut down procedure.\r\n\"Never\" is allowed as value (to disable this feature)"), _T("[HH:MM:SS]") },

	{1020, 2, _T("WRITE COUNT")						, true, 1, 1		, _T("Number of times the flash memory has been written. Parameter is Read only."), _T("[count]") },
};

unsigned int g_memTerm[]={	 (unsigned int)0x31,(unsigned int)0x40,(unsigned int)0x53,(unsigned int)0x68,(unsigned int)0x82
							,(unsigned int)0xA0,(unsigned int)0xC3,(unsigned int)0xE9,(unsigned int)0x113,(unsigned int)0x13F
							,(unsigned int)0x16E,(unsigned int)0x19F,(unsigned int)0x1CF,(unsigned int)0x200,(unsigned int)0x22F
							,(unsigned int)0x25C,(unsigned int)0x286,(unsigned int)0x2AE,(unsigned int)0x2D3,(unsigned int)0x2F4
							,(unsigned int)0x312,(unsigned int)0x32D,(unsigned int)0x345,(unsigned int)0x35A,(unsigned int)0x36D
							,(unsigned int)0x37E,(unsigned int)0x38C,(unsigned int)0x399,(unsigned int)0x3A5,(unsigned int)0x3AF
							,(unsigned int)0x3B7,(unsigned int)0x3BF,(unsigned int)0x3C6,(unsigned int)0x3CC};

ATXMSG* HIDOpenUPS2::GetMessages()
{
	return g_memMessages;
}

double HIDOpenUPS2::GetConstant(int i)
{
	return g_memConstants[i];
}

int HIDOpenUPS2::GetMessageNoByName(const char* name)
{
	for (int i=0;i<MAX_MESSAGE_CNT;i++)
	{
		if (g_memMessages[i].nLen!=0)
			if (strcmp(g_memMessages[i].strName, name) == 0) 
				return i;
	}
	
	return -1;
}

unsigned int* HIDOpenUPS2::GetTermistorConsts()
{
	return g_memTerm;
}

/*
double HIDOpenUPS2::GetVoltageVOut(unsigned char data)
{
	double rpot = ((double)data) * CT_RP / (double)257 + CT_RW;
	double voltage = (double)80 * ( (double)1 + CT_R1_VOUT/(rpot+CT_R2_VOUT));
	voltage = floor(voltage);
	return voltage/100;
}

unsigned char HIDOpenUPS2::GetDataVOut(double vout)
{
	if (vout < 0.9) return (unsigned char)0xff;//low limit

	double rpot = CT_V_FEEDBACK_OUT * CT_R1_VOUT / (vout - CT_V_FEEDBACK_OUT) - CT_R2_VOUT;
	double result = (257 * (rpot-CT_RW) / CT_RP);

	if (result<0) result = 0;
	if (result>255) result = 255;

	return (unsigned char)result;
}
*/

HIDOpenUPS2::HIDOpenUPS2(HIDDevice* hdev, int timeout):HIDInterface(hdev)
{
	if (hdev)
		TRACE( "HID device %s %s opened\n"
		, hdev->GetDescriptor()->m_sManufacturer
		, hdev->GetDescriptor()->m_sProduct);
	TRACE( "HID device could not be opened\n");
		
	m_nError = -1;
	m_pGraphics = NULL;

	m_nTimeout = timeout;
}

HIDOpenUPS2::~HIDOpenUPS2()
{
	m_nError = 0;

	m_list.lock();
	m_list.moveToHead();
	HArray* a = (HArray*)m_list.nextElement();
	while (a)
	{
		delete a;
		a = (HArray*)m_list.nextElement();
	}
	m_list.removeall();
	m_list.unlock();
}

HArray* HIDOpenUPS2::getReceivedMessage()
{
	m_list.lock();
	m_list.moveToHead();
	HArray* a = (HArray*)m_list.nextElement();
	if (a)
		m_list.removeWithNoLocking(a);
	m_list.unlock();

	return a;
}

bool HIDOpenUPS2::waitForAnswer()
{
	int tmout = 0;
	while (m_nError == -1)
	{
		Sleep(A_SLEEP);
		tmout++;
		if (tmout > A_TIMEOUT) return false;
	}
	return true;
}

bool HIDOpenUPS2::sendMessage(unsigned char cType, unsigned int buflen, unsigned char* buffer, unsigned int len, ...)
{
	HArray hArray(buflen+ len + 1);
	hArray.getBuf()[0] = cType;

	va_list args;
	va_start(args, len);
	unsigned int cnt = 0;
#ifdef _WIN32	
	unsigned char i = va_arg(args, unsigned char);
#else
	unsigned char i = va_arg(args, unsigned int);
#endif
	while (cnt<len)
	{
		hArray.getBuf()[1+cnt] = i;
		cnt++;
		if (cnt < len)
#ifdef _WIN32			
		 	i = va_arg(args, unsigned char);
#else
			i = va_arg(args, unsigned int);
#endif
	}
	
	va_end(args);
	
	memcpy(hArray.getBuf()+1+len, buffer, buflen);

	m_nError = -1;
	if (Write(&hArray))
	{
		if ((cType == UPS12V_MEM_READ_OUT)||(cType == UPS12V_MEM_ERASE)||(cType == UPS12V_MEM_WRITE_OUT))
		{
			int tmout = 0;
			while (m_nError == -1)
			{
				Sleep(A_SLEEP);
				tmout++;
				if (tmout > A_TIMEOUT) return false;
			}
		}
		return true;
	}
	else return false;
}

bool HIDOpenUPS2::sendMessage(unsigned char cType, unsigned int len, ...)
{
	HArray hArray(len + 1);
	hArray.getBuf()[0] = cType;

	va_list args;
	va_start(args, len);
	unsigned int cnt = 0;
#ifdef _WIN32	
	unsigned char i = va_arg(args, unsigned char);
#else
	unsigned char i = va_arg(args, unsigned int);
#endif
	while (cnt<len)
	{
		hArray.getBuf()[1+cnt] = i;
		cnt++;
		if (cnt < len)
#ifdef _WIN32			
		 	i = va_arg(args, unsigned char);
#else
			i = va_arg(args, unsigned int);
#endif
	}
	
	va_end(args);

	m_nError = -1;
	if (Write(&hArray))
	{
		if ((cType == UPS12V_MEM_READ_OUT)||(cType == UPS12V_MEM_ERASE)||(cType == UPS12V_MEM_WRITE_OUT))
		{
			int tmout = 0;
			while (m_nError == -1)
			{
				Sleep(A_SLEEP);
				tmout++;
				if (tmout > A_TIMEOUT) return false;
			}
		}
		return true;
	}
	else return false;
}

void HIDOpenUPS2::Received(HArray* array)
{
	m_list.add(new HArray(array));

	m_nError = 0;
}

void HIDOpenUPS2::Disconnected()
{
	unsigned char ch[24] = {INTERNAL_MESG, INTERNAL_MESG_DISCONNECTED,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	m_list.add(new HArray(ch, 24));
}

void HIDOpenUPS2::connectGraph(HIDGraphics* hg)
{
	m_lockCommand.lock();
	m_pGraphics = hg;
	m_lockCommand.unlock();
}

bool HIDOpenUPS2::Write(HArray* mesg)
{
	m_lockWrite.lock();
	bool b = m_pParent->Write(mesg);
	m_lockWrite.unlock();

	return b;
}


//////////////// READ STUFF ////////////

int HIDOpenUPS2::_getReadMsg(int type)
{
	switch (type)
	{
	case TYPE_CODE_MEMORY:		return FLASH_REPORT_READ_MEMORY;
	case TYPE_EPROM_EXTERNAL:	return OUT_REPORT_EXT_EE_READ;
	case TYPE_EPROM_INTERNAL:	return OUT_REPORT_INT_EE_READ;
	case TYPE_CODE_SPLASH:		return KEYBD_REPORT_READ_MEMORY;
	}
	return 0;
}

