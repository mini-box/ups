/*
 *    Author: Nicu Pavel <npavel@mini-box.com>
 *    Copyright (c) 2020 Mini-Box.com
 *    Licensed under the terms of the GNU Lesser General Public License v2.1
 *  	http://www.opensource.org/licenses/lgpl-2.1.php 
 */

#include "util.h"
#include "HIDOpenUPS.h"
#include <unistd.h>

#include <math.h>

double g_OpenUPS_memConstants[]=
{
	0.03545,//Vin				//0
	0.02571,//Vout				//1
	0.00857,//Vbat+chg+cell		//2
	12.411, //A chg Hi range	//3
	0.8274, //A chg Low range	//4
	16.113	//A discharge		//5
};

ATXMSG g_OpenUPS_memMessages[MAX_MESSAGE_CNT] = 
{
	{67, 1, _T("OPENUPSMODE")						, true,17, 1, 0, 0	, _T("OpenUPS Mode\r\n0-Auto restart when Input Power is Present=YES\r\n1-Auto restart when Input Power is Present=NO"), _T("[-]") },
	{28, 4, _T("CAPACITY")							, true,16, 1, 0, 0		, _T("Battery Capacity.\r\nDefault 7000mAh"), _T("[mAh]") },
	{70, 1, _T("CELLS")								, true, 1, 1, 0, 0		, _T("Number of configured cells to charge/balance\r\nDefault is 1."), _T("[pcs]") },
	{80, 2, _T("MOB_ONOFF_TOUT")					, true, 1, 10, 0, 0		, _T("The powerswitch on the motherboard(if connected) will be shorted  this period to turn the Motherboard ON or OFF\r\nDefault is 500ms"), _T("[mS]") },

	{ 0, 1, _T("UPS_CONFIG")						, true,14, 1, 0, 0		, _T("Configuration register. Used for enabling disabling modules. LSB bit is b0.\r\nb6- Stop impulse\t\tb5- Start impulse\r\nb4- Coulomb counter\tb3- A/D Low pass Filter module\r\nb2- Balance module\t\tb1- Charge module\r\nb0- Output module\r\nDefault is all enabled"), _T("[bits]") },
	{100,1, _T("UPS_CONFIG2")						, true,14, 1, 0, 0		, _T("Configuration register (Firmware V1.4+). Used for enabling disabling modules. LSB bit is b0.\r\nbit0: if bit is set and running on battery and measured output power<POUT_LO than go to deep sleep"), _T("[bits]") },

	{ 1, 1, _T("UPS_INIT_DELAY_TOUT")				, true, 1, 1, 0, 0		, _T("Initial delay before starting the UPS\r\nDefault is 1 sec."), _T("[s]") },
	{ 2, 2, _T("UPS_VIN_MAX_SHUTDOWN")				, true,10, 0, 0, 0		, _T("Max allowed input voltage. In case input voltage exceeds predefined value shutdown will be initiated.\r\nDefault is 35V."), _T("[V]") },
	{ 4, 2, _T("UPS_VIN_MIN_START")					, true,10, 0, 0, 0		, _T("If input voltage is above this threshold the UPS will start and take power from the input.\r\nDefault is 11V."), _T("[V]") },
/***/{ 6, 2, _T("UPS_VIN_MIN_STOP")					, true,11, 0, 0, 0		, _T("If input voltage is below this threshold the UPS will try to run on battery.\r\nDefault is 6V."), _T("[V]") },
	{ 8, 2, _T("UPS_VCELL_MIN_START")				, true,10, 2, 0, 0		, _T("If Vin is not present and all the battery cells are above this threshold the UPS can start and runs on battery\r\nDefault is 11.7V."), _T("[V]") },
	{10, 2, _T("UPS_VCELL_MIN_STOP")				, true,10, 2, 0, 0		, _T("If VIN is not present and one of the battery cells is less than this threshold during the time period specified by  UPS_VBAT_UVP_OFF_TOUT parameter the UPS will initiate shut down procedure.\r\nDefault is 11.4V."), _T("[V]") },
	{12, 2, _T("UPS_VBAT_UVP_OFF_TOUT")				, true, 1, 1, 0, 0		, _T("During this period battery cells are checked against UPS_VCELL_MIN_STOP. If cell voltage exceeds UPS_VCELL_MIN_STOP the timer is rearmed so it can filter unwanted noise in the system.\r\nDefault is 5 sec."), _T("[s]") },
	{14, 2, _T("UPS_HARDOFF_TOUT")					, true, 1, 1, 0, 0		, _T("After UPS_VBAT_UVP_OFF_TOUT passed motherboard is signaled to shut down. The UPS will wait this period so that the Motherboard can shut down gracefully, than cut power and enter deep sleep.This period should not be too long to prevent battery drain.\r\nDefault is 60 sec."), _T("[s]") },\

	{101,2, _T("UPS_VBAT_SLEEP_TOUT")				, true, 3, 1, 0, 0		, _T("Deep sleep timeout (Firmware V1.4+).  If system is running on battery and this period has elapsed with no input power applied then the UPS will initiate shut down procedure.\r\n\"Never\" is allowed as value (to disable this feature)"), _T("[HH:MM:SS]") },
	
//	{16, 2, _T("UPS_VBAT_PRESENT")					, true,10, 2, 0, 0		, _T("If VBAT< VBAT_PRESENT the battery is considered not present(used for powering the device only from USB for configuration,so the device won't enter deep sleep etc."), _T("[V]") },
	{18, 2, _T("UPS_SWITCHOVER_VBAT_TOUT")			, true, 1, 10, 0, 0		, _T("After UPS switches to battery, can switch back to Vin only after this time ellapses\r\nDefault is 1000 ms"), _T("[ms]") },
	{20, 2, _T("UPS_SWITCHOVER_VIN_TOUT")			, true, 1, 1, 0, 0		, _T("After UPS switches to input, can switch back to battery only after this time ellapses\r\nDefault is 1 ms"), _T("[ms]") },
	{22, 2, _T("DCHG_IMAX")							, true,10, 5, 0, 0		, _T("Max allowed discharge current. In case discharge current exceeds this threshold shutdown will be initiated.\r\nDefault is 10000 mA."), _T("[mA]") },
//	{24, 4, _T("CAPACITY")							, true,16, 4, 0, 0		, _T("Battery capacity"), _T("[mAh]") }, //old style with constant
// moved up	{28, 4, _T("CAPACITY")							, true,16, 1, 0, 0		, _T("Battery Capacity.\r\nDefault 3400mAh"), _T("[mAh]") },
 
//	{32, 2, _T("CHG_COND_VBAT_MIN")					, true,10, 2, 0, 0		, _T("Charge qualification is enabled if Battery voltage is less than this value"), _T("[V]") },
	{34, 2, _T("CHG_ICOND")							, true,10, 3, 0, 0		, _T("Conditioning/Precharge current. Charge current is limited to this value until cell voltage exceeds CHG_VCOND value and for at least CHG_TCOND time.\r\nDefault is 100mA."), _T("[mA]") },
	{36, 2, _T("CHG_VCOND")							, true,10, 2, 0, 0		, _T("Conditioning/Precharge voltage. Charge current is limited to CHG_ICOND until cell voltage exceeds this value and for at least CHG_TCOND time.\r\nDefault is 11.2V."), _T("[V]") },
	{38, 2, _T("CHG_TCOND")							, true, 1, 1, 0, 0		, _T("Conditioning/Precharge time. Charge current is limited to  CHG_ICOND value until cell voltage exceeds CHG_VCOND value and it is applied during this time.\r\nDefault is 30 sec."), _T("[s]") },
	{40, 2, _T("CHG_IBULK")							, true,10, 3, 0, 0		, _T("Fast charge current limit (constant current mode)\r\nDefault is 1750mA."), _T("[mA]") },
	{42, 2, _T("CHG_BULK_STOP_VOLTAGE")				, true,10, 2, 0, 0		, _T("Maximum allowed bulk charge voltage/cell during constant current/constant voltage charging\r\nDefault is 14.1V"), _T("[V/cell]") },
	{71, 1, _T("CHG_HYSTERESIS")					, true,10, 2, 0, 0		, _T("An overvoltage value (CHG_BULK_STOP_VOLTAGE +CHG_HYSTERESIS ) that it is allowed when charging.If one of the cells exceeds this value charging is imediately stopped.\r\nDefault is 100mV/cell."), _T("[V/cell]") },
//	{44, 2, _T("CHG_TBULK")							, true, 1, 1, 0, 0		, _T("Terminate bulk charge if charging for more than this period"), _T("[s]") },
	{46, 2, _T("CHG_IMIN")							, true,10, 3, 0, 0		, _T("If charge current in CV mode is less than this value\r\nPbSO4: enter maintanence mode and apply the float charge voltage(CHG_START_VOLTAGE)\r\nLiFePO4: cut off the charge voltage\r\nDefault is 290mA"), _T("[mA]") },

	{96, 2, _T("CHG_GLOBAL_TOUT")					, true, 1, 0.5, 0, 0		, _T("Global charge timeout (Firmware v1.3+)"), _T("[min]") },
	{98, 2, _T("CHG_IFLOAT")						, true,10, 3, 0, 0		, _T("Charge current limit in float charge mode for PbSO4 batteries (Firmware v1.3+)"), _T("[mA]") },
	{94, 2, _T("CHG_TOPPING_TIMER")					, true, 1, 1, 0, 0		, _T("For Lithium based batteries after an overvoltage condition is detected for a cell a resting period is set by this timer before applying a small topping charge in case other cells are still not charged.\r\nDefault is 1800 sec."), _T("[s]") },
	
	{48, 2, _T("CHG_START_VOLTAGE")					, true,10, 2, 0, 0		, _T("If cell voltage is below this value charging can be started. For SLA battery this also sets the floating charge voltage.\r\nDefault is 13.5V/cell"), _T("[V/cell]") },
	{50, 1, _T("CHG_BAT_TYPE")						, true, 1, 1, 0, 0		, _T("The battery chemistry to be charged.\r\n0- PbSO4 (float charge is applied)\r\n1- LiFePO4 (float charge is not applied)\r\n2- LiPO (float charge is not applied)\r\nOther (float charge is not applied)\r\nDefault is 0."), _T("") },
	{51, 2, _T("CHG_TEMP_PCB")						, true,15, 1, 0, 0		, _T("Charge current is limited by CHG_ILIMIT_TEMP_PCB amount every 30s,if board temperature exceeds this value. Can limit the charge current till CHG_IBULK/4 value.\r\nDefault is 55�C."), _T("[�C]") },
/***/{103,2, _T("CHG_TEMP_PCB_LOW")					, true,15, 1, 0, 0		, _T("Charge current is limited by CHG_ILIMIT_TEMP_PCB amount every 30s,if board temperature is lower than this value for SLA, turns off charging for LiIon and LiFePo4. Can limit the charge current till CHG_IBULK/4 value(Firmware V1.6+)\r\nDefault is 5�C."), _T("[�C]") },
	{53, 2, _T("CHG_ILIMIT_TEMP_PCB")				, true,10, 3, 0, 0		, _T("Limit charge current with this amount if board temperature exceeds CHG_TEMP_PCB.\r\nDefault is 50mA."), _T("[mA]") },
//not implemented yet	{55, 1, _T("CHG_TEMP_CELL_LIMIT")			    , true, 1, 1, 0, 0		, _T("Charge current is limited by CHG_ILIMIT_TEMP_CELL amount if cell temperature exceeds this value"), _T("[�C]") },
//not implemented yet	{56, 2, _T("CHG_ILIMIT_TEMP_CELL")				, true,10, 3, 0, 0		, _T("Limit charge current with this amount if cell  temperature exceeds CHG_TEMP_CELL_LIMIT"), _T("[mA]") },
//not implemented yet	{58, 1, _T("CHG_TEMP_CELL_CUTOFF")			    , true, 1, 1, 0, 0		, _T("Charge current is terminated if if cell temperature exceeds this value"), _T("[�C]") },
/***/{59, 1, _T("CHG_FREQUENCY")					, true,12, 1, 0, 0		, _T("The working frequency of the charger buck-boost converter\r\nDefault is 333kHz."), _T("[kHz]") },

	{60, 2, _T("BAL_VCELL_MIN")						, true,10, 2, 0, 0		, _T("Balancing is allowed  if cell voltages are above this value.\r\nDefault is 3V"), _T("[V]") },
	{62, 1, _T("BAL_VCELL_DIFF_START")				, true,10, 2, 0, 0		, _T("If the voltage difference between cells exceeds this value start balancing the cells.\r\nDefault is 70mV."), _T("[V]") },
	{63, 1, _T("BAL_VCELL_DIFF_STOP")				, true,10, 2, 0, 0		, _T("If the voltage difference between cells is less than this value stop balancing the cells.\r\nDefault is 40mV."), _T("[V]") },
	
	{64, 2, _T("OUT_VOLTAGE")						, true,7 , 1, 0, 0		, _T("The output voltage.\r\nDefault is 12V."), _T("[V]") },
/***/{66, 1, _T("OUT_FREQUENCY")					, true,13, 1, 0, 0		, _T("The working frequency of the output buck-boost converter.\r\nDefault 300kHz"), _T("[kHz]") },
//moved up	{67, 1, _T("OPENUPSMODE")						, true,17, 1, 0, 0		, _T("OpenUPS Mode\r\n0-Auto restart when Input Power is Present=YES\r\n1-Auto restart when Input Power is Present=NO"), _T("[-]") },
//	{68, 1, _T("OUT_REGULATOR_OFFSET")				, true, 1, 1, 0, 0		, _T("Regulator offset"), _T("[step]") },
	{69, 1, _T("OUT_MAX_REGULATOR_STEP")			, true, 1, 1, 0, 0		, _T("Maximum allowed regulation step number for output module (0-255)\r\nDefault 100."), _T("[step]") },
//moved up	{70, 1, _T("CELLS")								, true, 1, 1, 0, 0		, _T("Number of configured cells to charge/balance\r\nDefault is 4."), _T("[pcs]") },
//moved up	{71, 1, _T("CHG_HYSTERESIS")					, true,10, 2, 0, 0		, _T("An overvoltage value (CHG_BULK_STOP_VOLTAGE +CHG_HYSTERESIS ) that it is allowed when charging.If one of the cells exceeds this value charging is imediately stopped.\r\nDefault is 200mV/cell."), _T("[V]") },
	{72, 4, _T("POUT_LO")							, true,16, 0.000001, 0, 0	, _T("Output power low threshold for motherboard alive sensing.If output power is lower than this threshold shut down impulse is NOT sent to the motherboard.\r\nDefault is 2W."), _T("[W]") },
	{76, 4, _T("POUT_HI")							, true,16, 0.000001, 0, 0	, _T("Output power high threshold for motherboard alive sensing. If output power is higher than this threshold shut down impulse is sent to the motherboard.\r\nDefault is 6W."), _T("[W]") },
	
//moved up	{80, 2, _T("MOB_ONOFF_TOUT")					, true, 1, 10, 0, 0		, _T("The powerswitch on the motherboard(if connected) will be shorted  this period to turn the Motherboard ON or OFF\r\nDefault is 500ms"), _T("[mS]") },

	{82, 2, _T("OCV_SOC0")							, true,10, 2, 0, 0		, _T("Open Circuit Voltage State Of Charge detection for initial 0% fuel gauge estimation.\r\nDefault is 11.8V"), _T("[V]") },
	{84, 2, _T("OCV_SOC10")							, true,10, 2, 0, 0		, _T("Open Circuit Voltage State Of Charge detection for initial 10% fuel gauge estimation.\r\nDefault is 11.9V"), _T("[V]") },
	{86, 2, _T("OCV_SOC25")							, true,10, 2, 0, 0		, _T("Open Circuit Voltage State Of Charge detection for initial 25% fuel gauge estimation.\r\nDefault is 12V"), _T("[V]") },
	{88, 2, _T("OCV_SOC50")							, true,10, 2, 0, 0		, _T("Open Circuit Voltage State Of Charge detection for initial 50% fuel gauge estimation.\r\nDefault is 12.3V"), _T("[V]") },
	{90, 2, _T("OCV_SOC75")							, true,10, 2, 0, 0		, _T("Open Circuit Voltage State Of Charge detection for initial 75% fuel gauge estimation.\r\nDefault is 12.6V"), _T("[V]") },
	{92, 2, _T("OCV_SOC100")						, true,10, 2, 0, 0		, _T("Open Circuit Voltage State Of Charge detection for initial 100% fuel gauge estimation.\r\nDefault is 12.8V"), _T("[V]") },

//moved up	{94, 2, _T("CHG_TOPPING_TIMER")					, true, 1, 1, 0, 0		, _T("For Lithium based batteries after an overvoltage condition is detected for a cell a resting period is set by this timer before applying a small topping charge in case other cells are still not charged.\r\nDefault is 1800 sec."), _T("[s]") },
//moved up	{96, 2, _T("CHG_GLOBAL_TOUT")					, true, 1, 0.5, 0, 0		, _T("Global charge timeout (Firmware v1.3+)"), _T("[min]") },
//moved up	{98, 2, _T("CHG_IFLOAT")						, true,10, 3, 0, 0		, _T("Charge current limit in float charge mode (Firmware v1.3+)"), _T("[mA]") },

//moved up  {100,1, _T("UPS_CONFIG2")						, true,14, 1, 0, 0		, _T("Configuration register (Firmware V1.4+). Used for enabling disabling modules. LSB bit is b0.\r\nbit0: if bit is set and running on battery and measured output power<POUT_LO than go to deep sleep"), _T("[bits]") },
//moved up	{101,2, _T("UPS_VBAT_SLEEP_TOUT")				, true, 3, 1, 0, 0		, _T("Deep sleep timeout (Firmware V1.4+).  If system is running on battery and this period has elapsed with no input power applied then the UPS will initiate shut down procedure.\r\n\"Never\" is allowed as value (to disable this feature)"), _T("[HH:MM:SS]") },
//moved up	{103,2, _T("CHG_TEMP_PCB_LOW")					, true,15, 1, 0, 0		, _T("Charge current is limited by CHG_ILIMIT_TEMP_PCB amount every 30s,if board temperature is lower than this value for SLA, turns off charging for LiIon and LiFePo4. Can limit the charge current till CHG_IBULK/4 value.\r\nDefaults is 0�C."), _T("[�C]") },

	{1020, 2, _T("WRITE COUNT")						, true, 1, 1, 0, 0		, _T("Number of times the flash memory has been written. Parameter is Read only."), _T("[count]") },
};

UVP g_OpenUPS_memUVP[32] = 
{
	{0, 0, 9.08},
	{0, 1, 10.21},
	{0, 2, 11.34},
	{0, 3, 12.48},
	{0, 4, 13.61},
	{0, 5, 14.75},
	{0, 6, 15.88},
	{0, 7, 17.02},
	{0, 8, 18.15},
	{0, 9, 19.28},
	{0, 10, 20.42},
	{0, 11, 21.55},
	{0, 12, 22.69},
	{0, 13, 23.82},
	{0, 14, 24.96},
	{0, 15, 26.09},
	{1, 0, 0},
	{1, 1, 1.51},
	{1, 2, 3.03},
	{1, 3, 4.54},
	{1, 4, 6.05},
	{1, 5, 7.56},
	{1, 6, 9.08},
	{1, 7, 10.59},
	{1, 8, 12.1},
	{1, 9, 13.61},
	{1, 10, 15.13},
	{1, 11, 16.64},
	{1, 12, 18.15},
	{1, 13, 19.66},
	{1, 14, 21.18},
	{1, 15, 22.69}
};

unsigned int g_OpenUPS_memTerm[]={	 (unsigned int)0x31,(unsigned int)0x40,(unsigned int)0x53,(unsigned int)0x68,(unsigned int)0x82
							,(unsigned int)0xA0,(unsigned int)0xC3,(unsigned int)0xE9,(unsigned int)0x113,(unsigned int)0x13F
							,(unsigned int)0x16E,(unsigned int)0x19F,(unsigned int)0x1CF,(unsigned int)0x200,(unsigned int)0x22F
							,(unsigned int)0x25C,(unsigned int)0x286,(unsigned int)0x2AE,(unsigned int)0x2D3,(unsigned int)0x2F4
							,(unsigned int)0x312,(unsigned int)0x32D,(unsigned int)0x345,(unsigned int)0x35A,(unsigned int)0x36D
							,(unsigned int)0x37E,(unsigned int)0x38C,(unsigned int)0x399,(unsigned int)0x3A5,(unsigned int)0x3AF
							,(unsigned int)0x3B7,(unsigned int)0x3BF,(unsigned int)0x3C6,(unsigned int)0x3CC};



HIDOpenUPS::HIDOpenUPS(USBHID *d): HIDInterface(d)
{
	m_ulSettingsAddr = OPENUPS_SETTINGS_ADDR_START;	
}

HIDOpenUPS::~HIDOpenUPS()
{
}

ATXMSG* HIDOpenUPS::GetMessages()
{
	return g_OpenUPS_memMessages;
}

double HIDOpenUPS::GetConstant(int i)
{
	return g_OpenUPS_memConstants[i];
}

unsigned int* HIDOpenUPS::GetTermistorConsts()
{
	return g_OpenUPS_memTerm;
}

UVP* HIDOpenUPS::GetUVPs()
{
	return g_OpenUPS_memUVP;
}

void HIDOpenUPS::printValues()
{

	fprintf(stdout, "VIN: %f\n VBat: %f\n VOut: %f \n CCharge: %f\n", m_fVIN, m_fVBat, m_fVOut, m_fCCharge);
	fprintf(stdout, "Temperature: %f\n ", m_fTemperature);
	fprintf(stdout, "VCell1: %f\n VCell2: %f\n VCell3: %f\n", m_fVCell[0], m_fVCell[1], m_fVCell[2]);
	fprintf(stdout, "CDischarge: %f\n ", m_fCDischarge);
	fprintf(stdout, "VerMajor: %d\n VerMinor: %d\n", m_nVerMajor, m_nVerMinor);
	fprintf(stdout, "State: %d\n ", m_nState);
	fprintf(stdout, "Capacity: %d\n", m_nCapacity);
	fprintf(stdout, "RTE: %d\n", m_nRTE);
}

void HIDOpenUPS::GetStatus() 
{
	unsigned char recv[32];
	int ret;

	sendMessage(OPENUPS_GET_ALL_VALUES, 0);
	usleep(1000);
	recvMessage(recv);
	parseMessage(recv);
	usleep(1000);

	ret = sendMessage(OPENUPS_GET_ALL_VALUES_2, 0);
	usleep(1000);
	recvMessage(recv);
	parseMessage(recv);
	usleep(1000);

	//TODO This breaks further communication with device. Needs more investigation. 
	/*
	ret = sendMessage(OPENUPS_CLOCK_OUT, 0);
	usleep(1000);
	recvMessage(recv);
	parseMessage(recv);
	*/
}

void HIDOpenUPS::ReadConfigurationMemory() 
{
	unsigned char recv[32];

	m_ulSettingsAddr = OPENUPS_SETTINGS_ADDR_START;
	memset(m_chPackages, 0, SETTINGS_PACKS * 16);

	while (m_ulSettingsAddr < OPENUPS_SETTINGS_ADDR_END)
	{
		sendMessage(OPENUPS_MEM_READ_OUT, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
		recvMessage(recv);
		parseMessage(recv);
		m_ulSettingsAddr += 16;
	}
}

void HIDOpenUPS::EraseConfigurationMemory() 
{
	unsigned char recv[32];
	int ret;

	m_ulSettingsAddr = OPENUPS_SETTINGS_ADDR_START;
	sendMessage(OPENUPS_MEM_ERASE, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x40);

	int retries = 5;
	while ((ret = recvMessage(recv) <= 0) && retries > 0) {
		retries--;
		usleep(500);
		fprintf(stderr, "Erase 0x%02x retry %d/5\n", recv[0], retries);
	}

	if (retries <= 0) {
		fprintf(stderr, "Error waiting for erase operation to finish\n");
	} else {
		fprintf(stderr, "Successfully erased (0x%02x) configuration memory\n", recv[0]);
	}
}

void HIDOpenUPS::WriteConfigurationMemory()
{
	unsigned char recv[32];
	int ret;

	m_ulSettingsAddr = OPENUPS_SETTINGS_ADDR_START;
	while (m_ulSettingsAddr < OPENUPS_SETTINGS_ADDR_END)
	{
		sendMessageWithBuffer(OPENUPS_MEM_WRITE_OUT, 16, m_chPackages+(m_ulSettingsAddr-OPENUPS_SETTINGS_ADDR_START), 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
		ret = recvMessage(recv);
		
		if (ret <= 0 || recv[0] != OPENUPS_MEM_WRITE_IN) {
			fprintf(stderr, "Error (%d, 0x%02x) writing configuration variables , aborting ...\n", ret, recv[0]);
			break;
		} else {
			fprintf(stderr, "Wrote page 0x%lx of configuration\n", m_ulSettingsAddr);
		}		
		m_ulSettingsAddr += 16;
	}
}

void HIDOpenUPS::parseMessage(unsigned char *msg)
{
	if (!msg)
		return;

	switch (msg[0])
	{
		case OPENUPS_RECV_ALL_VALUES:
			{
				m_nVerMajor			= (msg[31] >> 4) & 0x0F;
				m_nVerMinor			= msg[31] & 0x0F;

				m_fVIN  = convertOneValue2Float(msg, 2, 1, 20, GetConstant(0));
				m_fVOut = convertOneValue2Float(msg, 2, 3, 20, GetConstant(1));
				m_fVBat = convertOneValue2Float(msg, 2, 5, 2, GetConstant(2));

				for (int i=0;i<6;i++)
					m_fVCell[i] = convertOneValue2Float(msg, 2, 7+2*i, 2, GetConstant(2));

				for (int i=0;i<8;i++)
				{
					m_bCheck23[i] = (((msg[23] >> i)&1) == 1);
					m_bCheck24[i] = (((msg[24] >> i)&1) == 1);
					m_bCheck25[i] = (((msg[25] >> i)&1) == 1);
				}
				m_nOtherState[4] = msg[23];
				m_nOtherState[5] = msg[24];
				m_nOtherState[6] = msg[25];

				m_fCCharge	= convertOneValue2Float(msg, 2, 19, 2, GetConstant(4)/(double)1000);
				
				if (m_bCheck24[6])
				{
					m_nState = 1;
					
					m_fCDischarge = convertOneValue2Float(msg, 2, 21, 2, GetConstant(5)/(double)1000);
					m_fCIn		  = 0;
				}
				else if (m_bCheck24[5])
				{
					m_nState = 2;

					double chg_current = convertOneValue2Float(msg, 2, 19, 2, GetConstant(4)/(double)1000);
					double vbat = convertOneValue2Float(msg, 2, 5, 2, GetConstant(2));
					double vin  = convertOneValue2Float(msg, 2, 1, 20, GetConstant(0));
					double outbuckboost_input_current = convertOneValue2Float(msg, 2, 21, 2, GetConstant(5)/(double)1000);

					m_fCIn = chg_current * vbat / (vin * ETA_CHG) + outbuckboost_input_current;
					m_fCDischarge = 0;
				}
				else
				{//only usb
					m_nState = 3;

					m_fCDischarge = 0;
					m_fCIn		  = 0;
				}

				m_fTemperature = convertOneValue2Float(msg, 2, 26, 15, 1);
			}break;
		case OPENUPS_CLOCK_IN:
			{
				m_nYear  = 2000+bcd2hex(msg[2]);
				m_nMonth = bcd2hex(msg[3]);
				m_nDay   = bcd2hex(msg[4]);

				m_nHour  = bcd2hex(msg[6]);
				m_nMin   = bcd2hex(msg[7]);
				m_nSec   = bcd2hex(msg[8]);

				m_nCapacity = msg[12];
				m_nRTE   = msg[17];
				m_nRTE	 = (m_nRTE << 8) | msg[16];

				m_nOtherState[0] = msg[9]; //upsstate
				m_nOtherState[1] = msg[10];//charger state
				m_nOtherState[2] = msg[11];//out state
				m_nOtherState[3] = msg[15];//battery on
			}break;
		case OPENUPS_RECV_ALL_VALUES_2:
			{
				m_fOutputPower = convertOneValue2Float(msg, 4, 1, 16, 0.000001);
			}break;

		case OPENUPS_MEM_READ_IN:
			{
				memcpy(m_chPackages + (m_ulSettingsAddr - OPENUPS_SETTINGS_ADDR_START), msg + 5, 16);
				/*
				fprintf(stderr, "%02lx %02lx", m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF);
				for (int len = 0; len < 16; len++)
					fprintf(stderr, " %02x", msg[5 + len]);
				fprintf(stderr, "\n");				
				*/
			}
			break;
		}
}

float HIDOpenUPS::convertOneValue2Float(unsigned char *buffer, int nLen, int nIndex, int nReadMode, double dMultiplier)
{
	double value = -1;

	if (nLen == 1)
		value = (double)buffer[nIndex] * dMultiplier;
	else if (nLen == 2)
		value = (double)( (((int)buffer[nIndex+1] ) << 8)|buffer[nIndex]) * dMultiplier;
	else if (nLen == 4)
		value = (double)( (((int)buffer[nIndex+3] ) << 24) | (((int)buffer[nIndex+2] ) << 16) | (((int)buffer[nIndex+1] ) << 8) | buffer[nIndex]) * dMultiplier;

	switch (nReadMode)
	{
		case 10:
			{
				if (nLen == 1)
					value = (double)buffer[nIndex] * GetConstant(dMultiplier);
				else
					value = (double)( (((int)buffer[nIndex+1] ) << 8)|buffer[nIndex]) * GetConstant(dMultiplier);
			}break;
		case 15:
			{
				unsigned int termistor = 0;
				termistor = buffer[nIndex+1];
				termistor = (termistor << 8) | buffer[nIndex];

				if (termistor <= GetTermistorConsts()[0])
					value = -40;
				else if (termistor >= GetTermistorConsts()[TERMISTOR_CONSTS-1])
					value = 125;
				else
				{
					int pos = -1;
					for (int i=TERMISTOR_CONSTS-1;i>=0;i--)
					{
						if (termistor >= GetTermistorConsts()[i])
						{
							pos = i;
							break;
						}
					}

					if (termistor == GetTermistorConsts()[pos])
						value = pos*5-40;
					else
					{
						int t1 = pos*5-40;
						int t2 = (pos+1)*5-40;

						unsigned int d1 = GetTermistorConsts()[pos];
						unsigned int d2 = GetTermistorConsts()[pos+1];

						float dtemp = ((float)termistor - (float)d1)*((float)t2-(float)t1)/((float)d2-(float)d1);
						
						int temp = ceil(dtemp) + t1;

						value = temp;
					}
				}
			}break;
	}

	return value;
}

bool HIDOpenUPS::readOneValue(char *str, int nReadMode, double dMultiplier, int len, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4)
{
	bool ok = false;
	c1 = 0;
	c2 = 0;

	double val = 0;
	
	switch (nReadMode)
	{
	case 3:
		{//hh:mm:ss
			if (strcmp(str, "Never") == 0)
			{
				val = 0xFFFF;
				ok = true;
			}
			else if ((strlen(str) == 8) && (str[2]==':') && (str[5]==':'))
			{
				char* end = NULL;
				char temp[10];
				str_left(temp, str, 2);
				int hh  = _tcstol(temp, &end, 10);
				if ((end != temp ) && (hh>=0) && (hh<18))
				{
					str_mid(temp, str, 3,2);
					int mm  = _tcstol(temp, &end, 10);
					if ((end != temp) && (mm>=0) && (mm<60))
					{
						str_right(temp, str, 2);
						int ss  = _tcstol(temp, &end, 10);

						if ((end != temp) && (ss>=0) && (ss<60))
						{
							val = hh*3600+mm*60+ss;
							ok = true;
						}
					}
				}
			}
		}break;
	case 4:
		{//"signed" float
			if (strlen(str) != 0)
			{
				bool negative = false;
				char* end = NULL;
				char temp[10];

				if (str[0] == '-')
				{
					str_right(temp, str, strlen(str) - 1);
					str = temp;
					negative = true;
				}
				str_left(temp, str, 8);				
				val = _tcstod(temp, &end);

				if (end != temp)
				{
					ok = true;

					val /= dMultiplier;
					if (val < 0) val = 0;
					double rem = val - (int) val;
					unsigned long uval = (unsigned long)val;
					if (rem >= 0.5) uval += 1;

					if (uval > 0x7F) uval = 0x7F;

					c1 = 0;
					c2 = (unsigned char)uval;
					if (negative) c2 |= 0x80;
				}
			}
		}break;
	case 6:
		{
			val = 0xFF;
			ok = true;
		}break;
	case 7:
		{
			char* end = NULL;
			char temp[10];
			str_left(temp, str, 8);
			val = _tcstod(temp, &end);

			c2 = 0;
			c1 = GetDataVOut(val);
			ok = true;
		}break;
	case 8:
		{
			if (strcmp(str, "Never") == 0)
			{
				val = (double)0xFFFF * dMultiplier;
				ok = true;
			}
			else
			{
				char* end = NULL;
				char temp[10];

				str_left(temp, str, 8);
				val = _tcstod(temp, &end);
				if (end != temp) ok = true;
			}
		}break;
	case 9:
		{
			char* end = NULL;
			char temp[10];
			str_left(temp, str, 8);
			val = _tcstod(temp, &end);

			c1 = 0;
			c2 = GetDataVBat(val);
			ok = true;
		}break;
	case 11:
		{
			int cnt = atoi(str);
			if ((cnt>=0) && (cnt<32))
			{
				c1 = GetUVPs()[cnt].nCVR_2;
				c2 = GetUVPs()[cnt].nCVR_1;
				ok = true;
			}
		}break;
	case 12:
	case 13:
		{
			c1 = 0;
			c2 = atoi(str);
			ok = true;
		}break;
	case 14:
		{
			c1 = 0;
			c2 = bin2char(str, &ok);
		}break;
	case 15:
		{
			unsigned int termistor = 0;
			
			int temp = atoi(str);
			if (temp <= -40) termistor = GetTermistorConsts()[0];
			else if (temp >= 125) termistor = GetTermistorConsts()[TERMISTOR_CONSTS-1];
			else 
			{
				int pos = (temp + 40) / 5;
				int temp_dif = (temp + 40) % 5;

				if (temp_dif == 0)
					termistor = GetTermistorConsts()[pos];
				else
				{
					unsigned int d1 = GetTermistorConsts()[pos];
					unsigned int d2 = GetTermistorConsts()[pos+1];

					termistor = d1 + temp_dif * (d2-d1) / 5;
				}
			}

			c1 = (unsigned char)(((unsigned int)termistor) & 0xFF);
			c2 = (unsigned char)((((unsigned int)termistor)>>8) & 0xFF);
			ok = true;
		}break;
	case 16:
		{
			TCHAR* end = NULL;

			val = _tcstod(str, &end);

			if ((val != 0) && (end != str))
			{
				val = ceil(val / dMultiplier);
				unsigned int ival = (unsigned int) val;

				c1 = (unsigned char)( ival & 0xFF);
				c2 = (unsigned char)((ival>>8 ) & 0xFF);
				c3 = (unsigned char)((ival>>16) & 0xFF);
				c4 = (unsigned char)((ival>>24) & 0xFF);
				ok = true;
			}
		}break;
	case 17:
		{
			char* end = NULL;
			char temp[10];
			str_left(temp, str, 8);
			val = _tcstod(temp, &end);
			if (end != temp) ok = true;
			if ((val != 0)&&(val != 1)) ok = false;

		}break;
	case 1:
		//normal read - integer
	case 2:
	case 5:
	default:
		{//normal read - float
			char* end = NULL;
			char temp[10];
			str_left(temp, str, 8);
			val = _tcstod(temp, &end);
			if (end != temp) ok = true;
		}
	}

	if (ok)
	{
		if ((nReadMode != 4)&&(nReadMode != 7)&&(nReadMode != 9)&&(nReadMode != 11)&&(nReadMode != 12)&&(nReadMode != 13)&&(nReadMode != 14)&&(nReadMode != 15)&&(nReadMode != 16))
		{
			if (nReadMode == 10)
				 val /= GetConstant(dMultiplier);
			else 
				val /= dMultiplier;

			if (nReadMode!=5)
			{//unsigned
				if (val < 0) val = 0;
				double rem = val - (int) val;
				unsigned long uval = (unsigned long)val;
				if (rem >= 0.5) uval += 1;

				if (len == 1)
				{
					if (uval > 0xFF) uval = 0xFF;
					
					c1 = 0;
					c2 = (unsigned char)uval;
				}
				else
				{
					if (uval > 0xFFFF) uval = 0xFFFF;

					c1 = (unsigned char)(((unsigned int)uval) & 0xFF);
					c2 = (unsigned char)((((unsigned int)uval)>>8) & 0xFF);
				}
			}
			else
			{
				double rem = -val + (int) val;
				long uval = (long)val;
				if (rem >= 0.5) uval += 1;

				if (len == 1)
				{
					if (uval > 127) uval = 127;
					if (uval <-128) uval = -128;
					
					c1 = 0;
					c2 = (char)uval;
				}
				else
				{
					if (uval > 32767) uval = 32767;
					if (uval <-32768) uval = -32768;

					c1 = (char)(((int)uval) & 0xFF);
					c2 = (char)((((int)uval)>>8) & 0xFF);
				}
			}
		}
	}

	return ok;
}

void HIDOpenUPS::convertOneValue2String(char *destination, int nLen, int nIndex, int nReadMode, double dMultiplier)
{
	destination[0] = 0; //empty string
	double value = 0;
	
	if (nLen == 1)
		value = (double)m_chPackages[nIndex] * dMultiplier;
	else if (nLen == 2)
		value = (double)( (((int)m_chPackages[nIndex+1] ) << 8)|m_chPackages[nIndex]) * dMultiplier;
	else if (nLen == 4)
		value = (double)( (((int)m_chPackages[nIndex+3] ) << 24) | (((int)m_chPackages[nIndex+2] ) << 16) | (((int)m_chPackages[nIndex+1] ) << 8) | m_chPackages[nIndex]) * dMultiplier;

	switch (nReadMode)
	{
		case 2:   sprintf(destination, "%f", (float)value); break;
		case 20:  sprintf(destination, "%.2f", (float)value); break;
		case 10:
			{
				if (nLen == 1)
					value = (double)m_chPackages[nIndex] * GetConstant(dMultiplier);
				else if (nLen == 2)
					value = (double)( (((int)m_chPackages[nIndex+1] ) << 8)|m_chPackages[nIndex]) * GetConstant(dMultiplier);
				else if (nLen == 4)
					value = (double)( (((int)m_chPackages[nIndex+3] ) << 24) | (((int)m_chPackages[nIndex+2] ) << 16) | (((int)m_chPackages[nIndex+1] ) << 8) | m_chPackages[nIndex]) * GetConstant(dMultiplier);
				sprintf(destination, "%f", (float)value);
			}break;
		case 11:
			{
				int pos = -1;
				for (int i=0;i<32;i++)
					if (  (GetUVPs()[i].nCVR_1 == m_chPackages[nIndex+1])
						&&(GetUVPs()[i].nCVR_2 == m_chPackages[nIndex]))
						pos = i;

				sprintf(destination, "%d", pos);
			}break;
		case 12:
		case 13:
			sprintf(destination, "%d", m_chPackages[nIndex]);
			break;
		case 14:
			char2bin(destination, m_chPackages[nIndex]);
			break;
		case 15:
			{
				unsigned int termistor = 0;
				termistor = m_chPackages[nIndex+1];
				termistor = (termistor << 8) | m_chPackages[nIndex];

				if (termistor <= GetTermistorConsts()[0])
					sprintf(destination, "%s", "-40");
				else if (termistor >= GetTermistorConsts()[TERMISTOR_CONSTS-1])
					sprintf(destination, "%s", "125");
				else
				{
					int pos = -1;
					for (int i=TERMISTOR_CONSTS-1;i>=0;i--)
					{
						if (termistor >= GetTermistorConsts()[i])
						{
							pos = i;
							break;
						}
					}

					if (termistor == GetTermistorConsts()[pos])
						sprintf(destination, "%d", pos * 5-40);
					else
					{
						int t1 = pos*5-40;
						int t2 = (pos+1)*5-40;

						unsigned int d1 = GetTermistorConsts()[pos];
						unsigned int d2 = GetTermistorConsts()[pos+1];

						float dtemp = ((float)termistor - (float)d1)*((float)t2-(float)t1)/((float)d2-(float)d1);
						
						int temp = ceil(dtemp) + t1;

						sprintf(destination, "%d", temp);
					}
				}
			}break;
		case 16:
			{//4 byte value
				if (nLen == 1)
					value = (double)m_chPackages[nIndex];
				else if (nLen == 2)
					value = (double)( (((int)m_chPackages[nIndex+1] ) << 8)|m_chPackages[nIndex]);
				else if (nLen == 4)
					value = (double)( (((int)m_chPackages[nIndex+3] ) << 24) | (((int)m_chPackages[nIndex+2] ) << 16) | (((int)m_chPackages[nIndex+1] ) << 8) | m_chPackages[nIndex]);

				value = value * dMultiplier;

				if (dMultiplier == 1)
					 sprintf(destination, "%.0f", (float)value);
				else 
					sprintf(destination, "%.2f", (float)value);
			}break;
		case 17:
			{
				sprintf(destination, "%d", (int)value);
			}break;
		case 3:
			{
				unsigned int ivalue = (unsigned int)value;
				if (ivalue == 0xFFFF)
					sprintf(destination, "%s", "Never");
				else 
					sprintf(destination, "%02d:%02d:%02d"
						, ivalue/3600
						, (ivalue/60)%60
						, ivalue % 60
						);
			}break;
		case 4:
			{
				double dvalue = (m_chPackages[nIndex] & 0x7f) * dMultiplier;
				if ((m_chPackages[nIndex] & 0x80)!=0)
					dvalue *= -1;

				sprintf(destination, "%f", (float)dvalue);				
			}break;
		case 5:
			{
				if (nLen == 1)
					value = (double)((char)m_chPackages[nIndex]) * dMultiplier;
				else
					value = (double)( (((int)((char)m_chPackages[nIndex+1]) ) << 8)|m_chPackages[nIndex])
						* dMultiplier;
				sprintf(destination, "%d", (int)value);
			}break;
		case 6:
			{
				sprintf(destination, "%s", "-");
			}break;
		case 7:
			{//no multiplier - voltage value calcualted with formula
				sprintf(destination, "%f", GetVoltageVOut(m_chPackages[nIndex]));
			}break;
		case 8:
			{
				if (nLen == 1)
				{
					if (m_chPackages[nIndex] == 0xFF)
						 sprintf(destination, "%s", "Never");
					else 
						sprintf(destination, "%d", (int)value);
				}
				else
				{
					if ((m_chPackages[nIndex] == 0xFF) && (m_chPackages[nIndex+1] == 0xFF))
						 sprintf(destination, "%s", "Never");
					else 
						sprintf(destination, "%d", (int)value);
				}
			}break;
		case 9:
			{//no multiplier - voltage value calcualted with formula
				sprintf(destination, "%f", GetVoltageVBat(m_chPackages[nIndex], m_chPackages[nIndex+1]));
			}break;
		case 1:
		default:
			{
				sprintf(destination, "%d", (int)value);
			}
	}

}

bool HIDOpenUPS::setVariableData(int mesg_no, char *str)
{
	int len = GetMessages()[mesg_no].nLen;

	if (len !=0)
	{
		unsigned char c1 = 0;
		unsigned char c2 = 0;
		unsigned char c3 = 0;
		unsigned char c4 = 0;

		if (readOneValue(str
						,GetMessages()[mesg_no].nReadMode
						,GetMessages()[mesg_no].dMultiplier
						,len
						,c1, c2, c3, c4))
		{
			switch (len)
			{
			case 1:
				m_chPackages[GetMessages()[mesg_no].nIndex]   = c2;
				break;
			case 2:
				m_chPackages[GetMessages()[mesg_no].nIndex]   = c1;
				m_chPackages[GetMessages()[mesg_no].nIndex+1] = c2;
				break;
			case 4:
				m_chPackages[GetMessages()[mesg_no].nIndex]   = c1;
				m_chPackages[GetMessages()[mesg_no].nIndex+1] = c2;
				m_chPackages[GetMessages()[mesg_no].nIndex+2] = c3;
				m_chPackages[GetMessages()[mesg_no].nIndex+3] = c4;
				break;
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

unsigned char HIDOpenUPS::getUPSVariableData(unsigned int cnt, char *name, char *value, char *unit, char *comment)
{
	if ((cnt < 0) || (cnt >= MAX_MESSAGE_CNT))
		return 0;

	if (GetMessages()[cnt].nLen == 0)
		return 0;

	if (name)
		strcpy(name, GetMessages()[cnt].strName);
	if (unit)
		strcpy(unit, GetMessages()[cnt].strUnit);
	if (comment)
		strcpy(comment, GetMessages()[cnt].strText);

	if (value)
	{
		convertOneValue2String(value, GetMessages()[cnt].nLen,
							   GetMessages()[cnt].nIndex,
							   GetMessages()[cnt].nReadMode,
							   GetMessages()[cnt].dMultiplier);
	}

	return GetMessages()[cnt].nReadMode;
}

void HIDOpenUPS::restartUPS()
{
	sendCommand(OPENUPS_DCMD_RESTART_UPS, 1);
}

void HIDOpenUPS::restartUPSInBootloaderMode()
{

}

double HIDOpenUPS::GetVoltageVOut(unsigned char data)
{
	double rpot = ((double)data) * CT_RP / (double)257 + CT_RW;
	double voltage = (double)80 * ( (double)1 + CT_R1_VOUT/(rpot+CT_R2_VOUT));
	voltage = floor(voltage);
	return voltage/100;
}

unsigned char HIDOpenUPS::GetDataVOut(double vout)
{
	if (vout < 0.9) return (unsigned char)0xff;//low limit

	double rpot = (double)0.8 * CT_R1_VOUT / (vout - (double)0.8) - CT_R2_VOUT;
	double result = (257 * (rpot-CT_RW) / CT_RP);

	if (result<0) result = 0;
	if (result>255) result = 255;

	return (unsigned char)result;
}

double HIDOpenUPS::GetVoltageVBat(unsigned char rp1, unsigned rp2)
{
	double voltage = ( (double)1 + CT_R1_VBAT / ((double)(rp1*CT_LSB_RP1)+(double)(rp2*CT_LSB_RP2)+CT_R2_VBAT)) * (double)123;
	voltage = floor(voltage);
	return voltage/100;
}

unsigned short HIDOpenUPS::GetDataVBat(double vout)
{
	if (vout < 1.24) return 0x7fff;//low limit

	double rpot = ((double)1.23 * (CT_R1_VBAT + CT_R2_VBAT) - vout * CT_R2_VBAT) / (vout - (double)1.23);
	
	int r1 = (int) (rpot / CT_LSB_RP1);
	r1 -= 6;//leave something to R2
	if (r1<0) r1 = 0;
	else if (r1>127) r1 = 127;
	
	double remainder = rpot - (double)r1*CT_LSB_RP1;
	int r2 = (int) (remainder / CT_LSB_RP2);
	if (r2<0) r2 = 0;
	else if (r2>255) r2 = 255;

	unsigned short result = (r1 << 8)|r2;
	return (unsigned short)result;
}
