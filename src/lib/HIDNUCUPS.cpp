/*
 *    Author: Nicu Pavel <npavel@mini-box.com>
 *    Copyright (c) 2020 Mini-Box.com
 *    Licensed under the terms of the GNU Lesser General Public License v2.1
 *  	http://www.opensource.org/licenses/lgpl-2.1.php 
 */

#include <math.h>
#include <unistd.h>

#include "HIDNUCUPS.h"

double g_NUCUPS_memConstants[]=
{
	0.001, 
};

ATXMSG g_NUCUPS_memMessages[MAX_MESSAGE_CNT] = 
{
	{0x00, 2, _T("NUC_UPS_MODE")					, true, 1, 1, 0, 0      , _T("0-Dumb mode	                           (DUMB)\r\n1-Automotive mode	   	     (AUTOMOTIVE)"), _T("[-]")  },
	{0x02, 2, _T("UPS_ON_BATTERY_TOUT")			    , true, 3, 1, 0, 0, _T("Deep sleep timeout. If system is running on battery and this period has elapsed with no input power applied, the UPS will initiate shut down procedure. \"Never\" is allowed as value (to disable this feature). Default is \"Never\"."), _T("[hh:mm:ss]") },
	{0x04, 2, _T("INIT_TOUT")						, true, 1,10, 0, 0, _T("When all power supply start-up conditions are met the PSU will wait this time before continuing with the start-up sequence.\r\nDefault is 1000ms."), _T("[ms]") },
	{0x06, 2, _T("VIN_MIN_STARTUP")					, true, 1, 1, 6000, 50000, _T("If the input voltage is beyond this threshold and all other start-up conditions are met, the PSU can start.\r\nDefault is 11V."), _T("[mV]") },
	{0x08, 2, _T("VIN_MIN_RUNNING")					, true, 1, 1, 6000, 50000, _T("If input voltage is below this threshold, the output is powered from battery.\r\nDefault is 8V."), _T("[mV]") },
	//{0x0A, 2, _T("VIN_MAX_SHUTDOWN")				, true, 1, 1, 0, 0, _T("?"), _T("[-]") },
	{0x0C, 2, _T("VIN_COUNT")						, true, 1,10, 0, 0, _T("Input voltage filtering.\r\nDefault is 100ms."), _T("[ms]") },
	{0x0E, 2, _T("IGN_COUNT")						, true, 1,10, 0, 0, _T("Ignition voltage filtering.\r\nDefault is 100ms."), _T("[ms]") },
	
	{0x10, 2, _T("IGN_HIGH_THRESHOLD")				, true, 1, 1, 0, 0, _T("If ignition voltage is beyond this threshold, ignition is considered to be ON.\r\nDefault is 6V."), _T("[mV]") },
	{0x12, 2, _T("IGN_LOW_THRESHOLD")				, true, 1, 1, 0, 0, _T("If ignition voltage is below this threshold, ignition is considered to be OFF.\r\nDefault is 5V."), _T("[mV]") },
	{0x14, 2, _T("IGN_ON_TO_OUTPUT_ON_TOUT")		, true, 1,10, 0, 0, _T("After ignition is considered ON, the PSU will wait this time before the output is turned ON.\r\nDefault is 2000ms."), _T("[ms]") },
	{0x16, 2, _T("IGN_CANCEL_TOUT")					, true, 1, 1, 0, 0, _T("After the motherboard boots up, the ignition voltage sensing will be disabled for this period.\r\nDefault is 60s."), _T("[s]") },
	{0x18, 2, _T("OUTPUT_ON_TO_MOB_PULSE_ON_TOUT")	, true, 1,10, 0, 0, _T("After the output was turned on, the UPS will wait this period before sending the ON pulse to the motherboard.\r\nDefault is 20ms"), _T("[ms]") },
	{0x1A, 2, _T("IGN_OFF_TO_MOB_PULSE_OFF_TOUT")	, true, 1, 1, 0, 0, _T("In automotive mode, after the ignition is considered OFF, this UPS will wait this period before sending the OFF pulse to the motherboard.\r\nDefault is 5s"), _T("[s]") },
	{0x1C, 2, _T("IOUT_OFFSET")						, true, 1, 1, 0, 0, _T("This value will be summed with the measured input current, this is a calibration parameter.\r\nDefault is 0mA"), _T("[mA]") },
	{0x1E, 2, _T("MOB_PULSEWIDTH")					, true, 1,10, 0, 0, _T("The length of the start-up/shutdown pulse sent to the motherboard.\r\nDefault is 500ms."), _T("[ms]") },
	
	{0x20, 2, _T("HARD_OFF_TOUT")					, true, 1, 1, 0, 0, _T("After the shutdown pulse is sent to the motherboard, the UPS will wait this period before the output is turned OFF. This time-out allows the operating system to perform a clean shutdown.\r\nDefault is 60s."), _T("[s]") },
	//{0x22, 2, _T("IOUT_LIMIT")						, true, 1, 1, 0, 0, _T("?"), _T("[-]") },
#ifdef MBOX_DEBUG
	{0x24, 2, _T("PWM_SPREAD_MODULATING_PERIOD")	, true, 1, 1,	 1,1000.0	, _T("Modulation period parameter of the Random Spread Frequency Modulation module (used for EMI reduction purposes)\r\n1-1000ms"), _T("[ms]") },//1-1000 Hz 
	{0x26, 2, _T("PWM_SPREAD_PERCENT")				, true, 1, 1,	 0,	 20.0	, _T("Frequency deviation parameter of the Random Spread Frequency Modulation module (used for EMI reduction purposes)\r\n0-20%"), _T("[%]") },
	{0x28, 2, _T("PWM_FREQ")						, true,18, 1,200.0,	700.0	, _T("Operating frequency of the Switched Mode Power Supply\r\n200-700kHz"), _T("[kHz]") },//1428-5000 ns
#endif
	{0x2A, 2, _T("CONFIG1")							, true,14, 1, 0, 0, _T("Configuration register [b7..b0]. Used for enabling/disabling modules. 0 - disabled, 1 - enabled\r\nb7 - Unused\r\nb6 - Unused\r\nb5 - Unused\r\nb4 - Unused\r\nb3 - If set, the USB sense is enabled, +5V USB is used to detect the Motherboard alive status\r\nb2 - If set, the Power measurement (Vout x Iout) is used to detect the Motherboard alive status\r\nb1 - If set, shutdown pulse is enabled through the PWRSW connector.\r\nb0 - If set, startup pulse is enabled through the PWRSW connector."), _T("[-]") },
	{0x2C, 2, _T("CONFIG2")							, true,14, 1, 0, 0, _T("Configuration register. Used for enabling/disabling modules. 0 - disabled, 1 - enabled\r\nb7..b0 - Reserved/Unused "), _T("[-]") },
	//{0x2E, 2, _T("WRITE_COUNT")						, true, 1, 1, 0, 0, _T("?"), _T("[-]") },

//moved	{0x30, 2, _T("BAL_ENABLE_TOUT")					, true, 1, 1, 0, 0, _T("?"), _T("[-]") },
	{0x32, 2, _T("CAPACITY")						, true, 1, 1, 0, 0, _T("Battery Capacity.\r\nDefault is 1000mAh."), _T("[mAh]") },
	{0x34, 2, _T("UPS_BUTTON_ON_TOUT")				, true, 1,10, 0, 0, _T("Filtering time for button pressing.\r\nDefault is 500ms."), _T("[ms]") },
	{0x36, 2, _T("UPS_VCELL_MIN_START")				, true, 1, 1, 0, 0, _T("If Vin is not present and all the battery cells are above this threshold, the UPS can start and will run on battery.\r\nDefault is 3V."), _T("[mV]") },
	{0x38, 2, _T("UPS_VCELL_MIN_STOP")				, true, 1, 1, 3000, 4300, _T("If VIN is not present and one of the battery cells is less than this threshold , the UPS will initiate a shutdown procedure.\r\nDefault is 2.8V."), _T("[mV]") },
//moved	{0x3A, 2, _T("BAL_DISABLE_TOUT")			, true, 1, 1, 0, 0, _T(""), _T("[ms]") },
	{0x3C, 2, _T("UPS_VCELL_MIN_HARD_STOP")			, true, 1, 1, 3000, 4300, _T("If VIN is not present and one of the battery cells is less than this threshold, the UPS will instantly shut down to preserve battery.\r\nDefault is 3V."), _T("[mV]") },
	{0x3E, 2, _T("UPS_OVERLOAD")					, true, 1, 1, 0, 0, _T("Max allowed discharge current. In case the discharge current exceeds this threshold, shutdown will be initiated\r\nDefault is 6000mA"), _T("[mA]") },

	{0x40, 2, _T("DCHG_TEMP_COLD")					, true,15, 1, 0, 0, _T("COLD temperature threshold for discharge.In case exceeded UPS will shutdown when on battery.\r\nDefault -20�C(FW ver 1.1)"), _T("[�C]") },
/**/{0x4E, 2, _T("DCHG_TEMP_HOT")					, true,15, 1, 0, 0, _T("HOT temperature threshold for discharge.In case exceeded UPS will shutdown when on battery.\r\nDefault 60�C(FW ver 1.1)"), _T("[�C]") },
	{0x42, 2, _T("CHG_COND_TOUT")					, true, 1, 1, 0, 0, _T("Conditioning/Precharge time. Charge current is limited until cell voltage exceeds CHG_VCOND value and it is applied during this time.\r\nDefault is 30 sec."), _T("[s]") },
	{0x44, 2, _T("CHG_BULK_STOP_VOLTAGE")			, true, 1, 1, 0, 0, _T("Maximum allowed bulk charge voltage/cell during constant current/constant voltage charging.\r\nDefault is 4.1 [V/cell]."), _T("[mV/cell]") },
	{0x46, 2, _T("CHG_HYSTERESIS")					, true, 1, 1, 0, 0, _T("An overvoltage value ( CHG_BULK_STOP_VOLTAGE + CHG_HYSTERESIS ) that is allowed when charging. If one of the cells exceeds this value, charging is immediately stopped.\r\nDefault is 100mV/cell"), _T("[mV/cell]") },
	{0x48, 2, _T("CHG_START_VOLTAGE")				, true, 1, 1, 0, 0, _T("If cell voltage is below this value, charging can be started.\r\nDefault is 3.85V/cell."), _T("[mV/cell]") },
	{0x4A, 2, _T("CHG_GLOBAL_TOUT")					, true, 1, 1, 0, 0, _T("Global charge timeout.\r\nDefault is 240 min."), _T("[min]") },
	{0x4C, 2, _T("CHG_TOPPING_TOUT")				, true, 1, 1, 0, 0, _T("After an overvoltage condition is detected for a cell (for Lithium based batteries), a resting period is set by this timer before applying a small topping charge - in case other cells are still not charged.\r\nDefault is 60s."), _T("[s]") },
//moved	{0x4E, 2, _T("DCHG_TEMP_HOT")					, true,15, 1, 0, 0, _T("HOT temperature threshold for discharge.In case exceeded UPS will shutdown when on battery.\r\Default 60�C(FW ver 1.1)"), _T("[�C]") },

	{0x50, 2, _T("CHG_TEMP_COLD")					, true,15, 1, 0, 0, _T("COLD temperature threshold for temperature compensated charge current regulation.\r\nDefault 5�C(FW ver 1.0)"), _T("[�C]") },
	{0x52, 2, _T("CHG_TEMP_COOL")					, true,15, 1, 0, 0, _T("COOL temperature threshold for temperature compensated charge current regulation.\r\nDefault 10�C(FW ver 1.0)"), _T("[�C]") },
	{0x54, 2, _T("CHG_TEMP_WARM")					, true,15, 1, 0, 0, _T("WARM temperature threshold for temperature compensated charge current regulation.\r\nDefault 50�C(FW ver 1.0)"), _T("[�C]") },
	{0x56, 2, _T("CHG_TEMP_HOT")					, true,15, 1, 0, 0, _T("HOT temperature threshold for temperature compensated charge current regulation.\r\nDefault 55�C(FW ver 1.0)"), _T("[�C]") },
	{0x58, 2, _T("UPS_VCELL_ADC_OFFSET")			, true, 1, 1, 0, 0, _T("This value will be summed with the measured cell voltages, this is a calibration parameter.\r\nDefault is 0mV."), _T("[mV]") },
	{0x5A, 2, _T("CHG_CELL_VCOND")					, true, 1, 1, 0, 0, _T("Conditioning/Pre-charge voltage. Charge current is limited until cell voltage exceeds this value and for at least CHG_TCOND time.\r\nDefault is 3.3V"), _T("[mV]") },
	{0x5C, 2, _T("BAL_VCELL_MIN")					, true, 1, 1, 0, 0, _T("Balancing is allowed if cell voltages are above this value.\r\nDefault is 3.4V"), _T("[mV]") },
	{0x5E, 2, _T("BAL_VCELL_DIFF_START")			, true, 1, 1, 0, 0, _T("If the voltage difference between cells exceeds this value, cell balancing will start.\r\nDefault is 80mV."), _T("[mV]") },

	{0x60, 2, _T("BAL_VCELL_DIFF_STOP")				, true, 1, 1, 0, 0, _T("If the voltage difference between cells is less than this value, cell balancing will stop.\r\nDefault is 40mV."), _T("[mV]") },
/**/{0x30, 2, _T("BAL_ENABLE_TOUT")					, true, 1, 5, 0, 0,_T("Balancing enable timeout (FW ver 1.3)"), _T("[min]") },
/**/{0x3A, 2, _T("BAL_DISABLE_TOUT")				, true, 1, 5, 0, 0,_T("Balancing disable timeout (FW ver 1.3)"), _T("[min]") },
	{0x62, 2, _T("OCV_SOC0")						, true, 1, 1, 0, 0, _T("Open Circuit Voltage State Of Charge detection for initial 0% fuel gauge estimation.\r\nDefault is 3.3V."), _T("[mV]") },
	{0x64, 2, _T("OCV_SOC10")						, true, 1, 1, 0, 0, _T("Open Circuit Voltage State Of Charge detection for initial 10% fuel gauge estimation.\r\nDefault is 3.68V."), _T("[mV]") },
	{0x66, 2, _T("OCV_SOC25")						, true, 1, 1, 0, 0, _T("Open Circuit Voltage State Of Charge detection for initial 25% fuel gauge estimation.\r\nDefault is 3.76V."), _T("[mV]") },
	{0x68, 2, _T("OCV_SOC50")						, true, 1, 1, 0, 0, _T("Open Circuit Voltage State Of Charge detection for initial 50% fuel gauge estimation.\r\nDefault is 3.82V."), _T("[mV]") },
	{0x6A, 2, _T("OCV_SOC75")						, true, 1, 1, 0, 0, _T("Open Circuit Voltage State Of Charge detection for initial 75% fuel gauge estimation.\r\nDefault is 3.97V."), _T("[mV]") },
	{0x6C, 2, _T("OCV_SOC100")						, true, 1, 1, 0, 0, _T("Open Circuit Voltage State Of Charge detection for initial 100% fuel gauge estimation.\r\nDefault is 4.17V."), _T("[mV]") },

	{0x78, 4, _T("POUT_HIGH_THRESHOLD")				, true, 1, 1, 0, 0, _T("If output power measured is higher than this threshold the connected motherboard is considered to be ON. Together with POUT_LOW_THRESHOLD parameter sets a hysteresis for motherboard status.\r\nDefault is 3W."), _T("[mW]") },
	{0x7C, 4, _T("POUT_LOW_THRESHOLD")				, true, 1, 1, 0, 0, _T("If output power measured is lower than this threshold the connected motherboard is considered to be OFF. Together with POUT_HIGH_THRESHOLD parameter sets a hysteresis for motherboard status.\r\nDefault is 1W."), _T("[mW]") },

	{0xFE, 2, _T("WRITE_COUNT (READ ONLY)")			, false, 1, 1, 0, 0, _T("Write count"),  _T("[-]") },

#ifdef MBOX_DEBUG
	{0x6E, 2, _T("PROTECT1")						, true,14, 1, 0, 0, _T("?"),_T("[-]") },
	 {0x6E, 2, _T("SCD_TRESHOLD")					, true,101, 1, 0, 0, __T("?"), T("[mA]")  },
	 {0x6E, 2, _T("SCD_DELAY")						, true,102, 1, 0, 0,  _T("?"), _T("[usec]")},
	{0x70, 2, _T("PROTECT2")						, true,14, 1, 0, 0, _T("?"),_T("[-]") },
	 {0x70, 2, _T("OCD_TRESHOLD")					, true,103, 1, 0, 0, _T("?"),_T("[mA]") },
	 {0x70, 2, _T("OCD_DELAY")						, true,104, 1, 0, 0, _T("?"),_T("[msec]") },
	{0x72, 2, _T("PROTECT3")						, true,14, 1, 0, 0, _T("?"),_T("[-]") },
	 {0x72, 2, _T("UV_DELAY")						, true,105, 1, 0, 0, _T("?"),_T("[sec]") },
	 {0x72, 2, _T("OV_DELAY")						, true,106, 1, 0, 0, _T("?"),_T("[sec]") },
	{0x74, 2, _T("OV_TRIP")							, true, 1, 1, 0, 0, _T("?"), _T("[mV]") },
	{0x76, 2, _T("UV_TRIP")							, true, 1, 1, 0, 0, _T("?"), _T("[mV]") },
#endif
};

static unsigned int g_NUCUPS_memTerm[]={	 (unsigned int) 0xD
							,(unsigned int) 0x12
							,(unsigned int) 0x17
							,(unsigned int) 0x1E
							,(unsigned int) 0x26
							,(unsigned int) 0x30
							,(unsigned int) 0x3C
							,(unsigned int) 0x4B
							,(unsigned int) 0x5C
							,(unsigned int) 0x70
							,(unsigned int) 0x87
							,(unsigned int) 0xA1
							,(unsigned int) 0xBE
							,(unsigned int) 0xDF
							,(unsigned int) 0x102
							,(unsigned int) 0x129
							,(unsigned int) 0x152
							,(unsigned int) 0x17E
							,(unsigned int) 0x1AB
							,(unsigned int) 0x1DA
							,(unsigned int) 0x20A
							,(unsigned int) 0x238
							,(unsigned int) 0x266
							,(unsigned int) 0x294
							,(unsigned int) 0x2BF
							,(unsigned int) 0x2E9
							,(unsigned int) 0x311
							,(unsigned int) 0x336
							,(unsigned int) 0x359
							,(unsigned int) 0x37A
							,(unsigned int) 0x398
							,(unsigned int) 0x3B3
							,(unsigned int) 0x3CD
							,(unsigned int) 0x3E4
};

static unsigned int g_NUCUPS_memSCDTreshold[]={(unsigned int)4400
							,(unsigned int)6600
							,(unsigned int)8800
							,(unsigned int)11200
							,(unsigned int)13400
							,(unsigned int)15600
							,(unsigned int)17800
							,(unsigned int)20000
							,(unsigned int)-1
};

static unsigned int g_NUCUPS_memSCDDelay[]={(unsigned int)70
							,(unsigned int)100
							,(unsigned int)200
							,(unsigned int)400
							,(unsigned int)-1
};

static unsigned int g_NUCUPS_memOCDTreshold[]={(unsigned int)1600
							,(unsigned int)2200
							,(unsigned int)2800
							,(unsigned int)3400
							,(unsigned int)3800
							,(unsigned int)4400
							,(unsigned int)5000
							,(unsigned int)5600
							,(unsigned int)6200
							,(unsigned int)6600
							,(unsigned int)7200
							,(unsigned int)7800
							,(unsigned int)8400
							,(unsigned int)8800
							,(unsigned int)9400
							,(unsigned int)10000
							,(unsigned int)-1
};

static unsigned int g_NUCUPS_memOCDDelay[]={(unsigned int)8
							,(unsigned int)20
							,(unsigned int)40
							,(unsigned int)80
							,(unsigned int)160
							,(unsigned int)320
							,(unsigned int)640
							,(unsigned int)1280
							,(unsigned int)-1
};

static unsigned int g_NUCUPS_memUVDelay[]={(unsigned int)1
							,(unsigned int)4
							,(unsigned int)8
							,(unsigned int)16
							,(unsigned int)-1
};

static unsigned int g_NUCUPS_memOVDelay[]={(unsigned int)1
							,(unsigned int)2
							,(unsigned int)4
							,(unsigned int)8
							,(unsigned int)-1
};


HIDNUCUPS::HIDNUCUPS(USBHID *d) : HIDInterface(d)
{
	m_ulSettingsAddr = NUC_SETTINGS_ADDR_START;
}

HIDNUCUPS::~HIDNUCUPS()
{

}

void HIDNUCUPS::printValues()
{
    fprintf(stdout, "\n");
    
    fprintf(stdout, "State machine UPS: %d - %s\n",m_nStateMachine[0], SM_UPS[m_nStateMachine[0]]);
    fprintf(stdout, "State machine AFE: %d - %s\n",m_nStateMachine[1], SM_AFE[m_nStateMachine[1]]);
    fprintf(stdout, "State machine CHG: %d - %s\n",m_nStateMachine[2], SM_CHG[m_nStateMachine[2]]);
    fprintf(stdout, "State machine DTCT: %d - %s\n",m_nStateMachine[3], SM_DTCT[m_nStateMachine[3]]);
    fprintf(stdout, "State machine LOWP: %d - %s\n",m_nStateMachine[4], SM_LOWP[m_nStateMachine[4]]);

    fprintf(stdout, "\n");

    fprintf(stdout, "VIn: %f [V]\n", m_fVIn);
    fprintf(stdout, "VOut: %f [V]\n", m_fVOut);
    fprintf(stdout, "IOut: %f [A]\n", m_fIOut);
    fprintf(stdout, "POut: %f [W]\n", m_fPOut);
    fprintf(stdout, "VIgnition: %f [V]\n", m_fVIgnition);

    fprintf(stdout, "\n");

    fprintf(stdout, "BatteryLevel: %d [%%]\n" , m_chBatteryLevel);
    fprintf(stdout, "BatOverallVoltage: %f [V]\n", m_fBatOverallVoltage);
    fprintf(stdout, "BatPack: %f [V]\n", m_fBatPack);
    fprintf(stdout, "IChgDchg: %f [A]\n", m_fIChgDchg);
    fprintf(stdout, "CellDetected: [%d %d %d %d]\n",
        m_fCellDetected[0], m_fCellDetected[1], m_fCellDetected[2], m_fCellDetected[3]);
    fprintf(stdout, "CellVoltage: [%f %f %f %f] [V]\n", 
        m_fBatVoltage[0], m_fBatVoltage[1], m_fBatVoltage[2], m_fBatVoltage[3]);
    fprintf(stdout, "CellBalanceOn: [%d %d %d %d]\n", 
        m_fCellBalanceOn[0], m_fCellBalanceOn[1], m_fCellBalanceOn[2], m_fCellBalanceOn[3]);    
    fprintf(stdout, "CellTemperature: [%f %f %f %f] [degC]\n", 
        m_fTemperature[0], m_fTemperature[1], m_fTemperature[2], m_fTemperature[3]);

    fprintf(stdout, "\n");

    fprintf(stdout, "PrechargeTimer: %d [s]\n", m_nChgTimer[0]);
    fprintf(stdout, "ChargeEnableTimer: %d [s]\n", m_nChgTimer[1]);
    fprintf(stdout, "ChargeGlobalTimer: %d [m]\n", m_nChgTimer[2]);
    fprintf(stdout, "ToppingTimer: %d [s]\n", m_nChgTimer[3]);

    fprintf(stdout, "\n");

    fprintf(stdout, "InitTimer: %d [ms]\n", m_nTimer[1]);
    fprintf(stdout, "VinCounter: %d [ms]\n", m_nTimer[8]);
    fprintf(stdout, "IgnCounter: %d [ms]\n", m_nTimer[9]);
    fprintf(stdout, "IgnOnToOutputOnTimer: %d [s]\n", m_nTimer[2]);
    fprintf(stdout, "OutputOnToMOBPulseOnTimer: %d [ms]\n", m_nTimer[0]);
    fprintf(stdout, "MOBPulseWidthTimer: %d [ms]\n", m_nTimer[4]);
    fprintf(stdout, "IgnOffToMOBPulseOffTimer: %d [s]\n", m_nTimer[5]);
    fprintf(stdout, "IgnCancelTimer: %d [s]\n", m_nTimer[7]);
    fprintf(stdout, "OnBatteryTimer: %d [s]\n", m_nTimer[3]);
    fprintf(stdout, "HardOffTimer: %d [s]\n", m_nTimer[6]);

    fprintf(stdout, "\n");

    fprintf(stdout, "VerMajor: %d\nVerMinor: %d\n", m_nVerMajor, m_nVerMinor);
    fprintf(stdout, "State: %d\n", m_nState);
    fprintf(stdout, "ChargeEndedCondition: %d\n", m_chChargeEndedCondition);
    fprintf(stdout, "Mode: %d\n", m_chMode);
    fprintf(stdout, "CPUUsage: %d\n", m_nCPUUsage);
    
    fprintf(stdout, "\n");

    fprintf(stdout, "Input Flags: %d [%d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d]\n", 
	m_nInput, m_nInput>>15 & 1, m_nInput>>14 & 1, m_nInput>>13 & 1, m_nInput>>12 & 1, m_nInput>>11 & 1, m_nInput>>10 & 1, m_nInput>>9 & 1, m_nInput>>8 & 1,
	m_nInput>>7 & 1, m_nInput>>6 & 1, m_nInput>>5 & 1, m_nInput>>4 & 1, m_nInput>>3 & 1, m_nInput>>2 & 1, m_nInput>>1 & 1, m_nInput>>0 & 1);
    fprintf(stdout, "Input Flags[ALERT]: %d\n", m_nInput>>0 & 1);
    fprintf(stdout, "Input Flags[UNKNOWN1]: %d\n", m_nInput>>1 & 1);
    fprintf(stdout, "Input Flags[BB_CHG_ENABLE]: %d\n", m_nInput>>2 & 1);
    fprintf(stdout, "Input Flags[BUCK_mBUTTON]: %d\n", m_nInput>>3 & 1);
    fprintf(stdout, "Input Flags[USB Sense]: %d\n", m_nInput>>4 & 1);
    fprintf(stdout, "Input Flags[CFG Bootloader]: %d\n", m_nInput>>5 & 1);
    fprintf(stdout, "Input Flags[ICSP_PGD]: %d\n", m_nInput>>6 & 1);
    fprintf(stdout, "Input Flags[UNKNOWN2]: %d\n", m_nInput>>7 & 1);
    fprintf(stdout, "Input Flags[BB_STAT1]: %d\n", m_nInput>>8 & 1);
    fprintf(stdout, "Input Flags[BB_STAT2]: %d\n", m_nInput>>9 & 1);
    fprintf(stdout, "Input Flags[GOOD_CELL_CFG]: %d\n", m_nInput>>10 & 1);
    fprintf(stdout, "Input Flags[UNKNOWN3]: %d\n", m_nInput>>11 & 1);
    fprintf(stdout, "Input Flags[UNKNOWN4]: %d\n", m_nInput>>12 & 1);
    fprintf(stdout, "Input Flags[USB Mode]: %d\n", m_nInput>>13 & 1);
    fprintf(stdout, "Input Flags[Input Mode]: %d\n", m_nInput>>14 & 1);
    fprintf(stdout, "Input Flags[Battery Mode]: %d\n", m_nInput>>15 & 1);

    fprintf(stdout, "\n");

    fprintf(stdout, "Output Flags: %d [%d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d]\n", 
	m_nOutput, m_nOutput>>15 & 1, m_nOutput>>14 & 1, m_nOutput>>13 & 1, m_nOutput>>12 & 1, m_nOutput>>11 & 1, m_nOutput>>10 & 1, m_nOutput>>9 & 1, m_nOutput>>8 & 1,
	m_nOutput>>7 & 1, m_nOutput>>6 & 1, m_nOutput>>5 & 1, m_nOutput>>4 & 1, m_nOutput>>3 & 1, m_nOutput>>2 & 1, m_nOutput>>1 & 1, m_nOutput>>0 & 1);
    fprintf(stdout, "Output Flags[EN_THERMISTOR]: %d\n", m_nOutput>>0 & 1);
    fprintf(stdout, "Output Flags[nPSW]: %d\n", m_nOutput>>1 & 1);
    fprintf(stdout, "Output Flags[BUCK_ENABLE]: %d\n", m_nOutput>>2 & 1);
    fprintf(stdout, "Output Flags[BUCK_5V_ENABLE]: %d\n", m_nOutput>>3 & 1);
    fprintf(stdout, "Output Flags[BUCK_SYNC]: %d\n", m_nOutput>>4 & 1);
    fprintf(stdout, "Output Flags[BB_MODE]: %d\n", m_nOutput>>5 & 1);
    fprintf(stdout, "Output Flags[BB_ENABLE]: %d\n", m_nOutput>>6 & 1);
    fprintf(stdout, "Output Flags[WAKE]: %d\n", m_nOutput>>7 & 1);
    fprintf(stdout, "Output Flags[BB_PWM]: %d\n", m_nOutput>>8 & 1);
    fprintf(stdout, "Output Flags[BUCK_12VGOOD]: %d\n", m_nOutput>>9 & 1);
    fprintf(stdout, "Output Flags[I2C_SDA1]: %d\n", m_nOutput>>10 & 1);
    fprintf(stdout, "Output Flags[I2C_SCL1]: %d\n", m_nOutput>>11 & 1);
    fprintf(stdout, "Output Flags[CHARGER_ENABLE]: %d\n", m_nOutput>>12 & 1);
    fprintf(stdout, "Output Flags[UNKNOWN1]: %d\n", m_nOutput>>13 & 1);    
    fprintf(stdout, "Output Flags[UNKNOWN2]: %d\n", m_nOutput>>14 & 1);    
    fprintf(stdout, "Output Flags[UNKNOWN3]: %d\n", m_nOutput>>15 & 1);

    fprintf(stdout, "\n");

    fprintf(stdout, "State Flags: %d [%d %d %d %d %d %d %d %d]\n", 
	m_nStateFlags, m_nStateFlags>>7 & 1, m_nStateFlags>>6 & 1, m_nStateFlags>>5 & 1, m_nStateFlags>>4 & 1, m_nStateFlags>>3 & 1, m_nStateFlags>>2 & 1, m_nStateFlags>>1 & 1, m_nStateFlags>>0 & 1);
    fprintf(stdout, "State Flags[VIN_GOOD]: %d\n", m_nStateFlags>>0 & 1);
    fprintf(stdout, "State Flags[IGN_GOOD]: %d\n", m_nStateFlags>>1 & 1);
    fprintf(stdout, "State Flags[MOB_ALIVE_POUT]: %d\n", m_nStateFlags>>2 & 1);
    fprintf(stdout, "State Flags[SHOULDRESTART]: %d\n", m_nStateFlags>>3 & 1);
    fprintf(stdout, "State Flags[UNKNOWN1]: %d\n", m_nStateFlags>>4 & 1);
    fprintf(stdout, "State Flags[UNKNOWN2]: %d\n", m_nStateFlags>>5 & 1);
    fprintf(stdout, "State Flags[IGN_RAISED]: %d\n", m_nStateFlags>>6 & 1);
    fprintf(stdout, "State Flags[IGN_FAILED]: %d\n", m_nStateFlags>>7 & 1);

    fprintf(stdout, "\n");

    fprintf(stdout, "Shutdown Flags: %d [%d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d]\n", 
	m_nShutdownFlags, m_nShutdownFlags>>15 & 1, m_nShutdownFlags>>14 & 1, m_nShutdownFlags>>13 & 1, m_nShutdownFlags>>12 & 1, m_nShutdownFlags>>11 & 1, m_nShutdownFlags>>10 & 1, m_nShutdownFlags>>9 & 1, m_nShutdownFlags>>8 & 1,
	m_nShutdownFlags>>7 & 1, m_nShutdownFlags>>6 & 1, m_nShutdownFlags>>5 & 1, m_nShutdownFlags>>4 & 1, m_nShutdownFlags>>3 & 1, m_nShutdownFlags>>2 & 1, m_nShutdownFlags>>1 & 1, m_nShutdownFlags>>0 & 1);
    fprintf(stdout, "Shutdown Flags[Button]: %d\n", m_nShutdownFlags>>0 & 1);
    fprintf(stdout, "Shutdown Flags[Ignition]: %d\n", m_nShutdownFlags>>1 & 1);
    fprintf(stdout, "Shutdown Flags[PGood]: %d\n", m_nShutdownFlags>>2 & 1);
    fprintf(stdout, "Shutdown Flags[USBSense]: %d\n", m_nShutdownFlags>>3 & 1);
    fprintf(stdout, "Shutdown Flags[PowerSense]: %d\n", m_nShutdownFlags>>4 & 1);
    fprintf(stdout, "Shutdown Flags[OnBatteryTimer]: %d\n", m_nShutdownFlags>>5 & 1);
    fprintf(stdout, "Shutdown Flags[Undervoltage]: %d\n", m_nShutdownFlags>>6 & 1);
    fprintf(stdout, "Shutdown Flags[HardUndervoltage]: %d\n", m_nShutdownFlags>>7 & 1);
    fprintf(stdout, "Shutdown Flags[Overload]: %d\n", m_nShutdownFlags>>8 & 1);
    fprintf(stdout, "Shutdown Flags[DchgTemperature]: %d\n", m_nShutdownFlags>>9 & 1);

    fprintf(stdout, "\n");

    fprintf(stdout, "Charger Flags: %d [%d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d]\n", 
	m_nChargerFlags, m_nChargerFlags>>15 & 1, m_nChargerFlags>>14 & 1, m_nChargerFlags>>13 & 1, m_nChargerFlags>>12 & 1, m_nChargerFlags>>11 & 1, m_nChargerFlags>>10 & 1, m_nChargerFlags>>9 & 1, m_nChargerFlags>>8 & 1,
	m_nChargerFlags>>7 & 1, m_nChargerFlags>>6 & 1, m_nChargerFlags>>5 & 1, m_nChargerFlags>>4 & 1, m_nChargerFlags>>3 & 1, m_nChargerFlags>>2 & 1, m_nChargerFlags>>1 & 1, m_nChargerFlags>>0 & 1);
    fprintf(stdout, "Charger Flags[GoodCellCfg]: %d\n", m_nChargerFlags>>0 & 1);
    fprintf(stdout, "Charger Flags[CellOverVoltage]: %d\n", m_nChargerFlags>>1 & 1);
    fprintf(stdout, "Charger Flags[CellUnderVoltage]: %d\n", m_nChargerFlags>>2 & 1);
    fprintf(stdout, "Charger Flags[CellHardUnderVoltage]: %d\n", m_nChargerFlags>>3 & 1);
    fprintf(stdout, "Charger Flags[LimitTmperature]: %d\n", m_nChargerFlags>>4 & 1);
    fprintf(stdout, "Charger Flags[OverTemperature]: %d\n", m_nChargerFlags>>5 & 1);
    fprintf(stdout, "Charger Flags[ShouldStartCharge]: %d\n", m_nChargerFlags>>6 & 1);
    fprintf(stdout, "Charger Flags[ShouldStopCharge]: %d\n", m_nChargerFlags>>7 & 1);
    fprintf(stdout, "Charger Flags[ShouldTrickleCharge]: %d\n", m_nChargerFlags>>8 & 1);
    fprintf(stdout, "Charger Flags[PreChargeCondition]: %d\n", m_nChargerFlags>>9 & 1);
    fprintf(stdout, "Charger Flags[BatteryFullyCharged]: %d\n", m_nChargerFlags>>10 & 1);
    fprintf(stdout, "Charger Flags[ChargeCurrentIsValid]: %d\n", m_nChargerFlags>>11 & 1);
    fprintf(stdout, "Charger Flags[CellsChargedVoltageOK]: %d\n", m_nChargerFlags>>12 & 1);
    fprintf(stdout, "Charger Flags[BatteryCouldBeFullyCharged]: %d\n", m_nChargerFlags>>13 & 1);
    fprintf(stdout, "Charger Flags[DchgTemperature]: %d\n", m_nChargerFlags>>14 & 1);
    fprintf(stdout, "Charger Flags[UNKNOWN1]: %d\n", m_nChargerFlags>>15 & 1);
    
    fprintf(stdout, "\n");
}

void HIDNUCUPS::parseMessage(unsigned char *msg)
{
    if (!msg)
        return;

    switch (msg[0])
    {
    
    case NUC_IN_REPORT_IO_DATA:
        {
            unsigned int i;

            m_nInput   = (((unsigned int)msg[2]) << 8) | msg[1];
            m_nOutput  = (((unsigned int)msg[4]) << 8) | msg[3];

            m_nStateFlags  = (unsigned int)msg[5];
            
            i = (((unsigned int)msg[6]) << 8) | msg[7];
            m_fVIn = (float)i / 1000;

            i = (((unsigned int)msg[8]) << 8) | msg[9];
            m_fIOut = (float)i / 1000;

            i = (((unsigned int)msg[10]) << 8) | msg[11];
            m_fVOut = (float)i / 1000;

            i = (((unsigned int)msg[12]) << 8) | msg[13];
            m_fVIgnition = (float)i / 1000;

            i =  (((unsigned int)msg[14]) << 24) 
                |(((unsigned int)msg[15]) << 16) 
                |(((unsigned int)msg[16]) << 8) 
                | msg[17];
            m_fPOut = (float)i / 1000;

            for (i=0;i<4;i++)
                m_fTemperature[i] = convertOneValue2Float(msg, 2, 18+i*2, 55, 1);

            short sh = (((short)msg[26]) << 8) | msg[27];
            m_fIChgDchg = (float)sh / (float)1000.0;

            
        }break;
    case NUC_IN_REPORT_IO_DATA2:
        {
            m_nTimer[0]   = ( (((unsigned int)msg[1]) << 8) | msg[2] ) * 10;//Get_Output_On_To_MOB_Pulse_On
            m_nTimer[1]   = ( (((unsigned int)msg[3]) << 8) | msg[4] ) * 10;//Get_Init_Timer
            m_nTimer[2]   =   (((unsigned int)msg[5]) << 8) | msg[6];//Get_Ign_On_To_Output_On_Timer
            m_nTimer[3]   =   (((unsigned int)msg[7]) << 8) | msg[8];//OnBatteryTimer
            m_nTimer[4]   = ( (((unsigned int)msg[9]) << 8) | msg[10] ) * 10;//Get_MOB_Pulse_Width_Timer
            m_nTimer[5]   =   (((unsigned int)msg[11]) << 8) | msg[12];//Get_Ign_Off_To_MOBPulse_Off_Timer
            m_nTimer[6]   =   (((unsigned int)msg[13]) << 8) | msg[14];//Get_Hard_Off_timer
            m_nTimer[7]   =   (((unsigned int)msg[15]) << 8) | msg[16];//Get_Ign_Cancel_Timer
            m_nTimer[8]   = ( (((unsigned int)msg[17]) << 8) | msg[18] ) * 10;//Get_Vin_Counter
            m_nTimer[9]   = ( (((unsigned int)msg[19]) << 8) | msg[20] ) * 10;//Get_Ign_Counter

            m_nStateMachine[0] = (unsigned int)msg[21];
            m_nStateMachine[1] = (unsigned int)msg[26];
            m_nStateMachine[2] = (unsigned int)msg[27];
            m_nStateMachine[3] = (unsigned int)msg[28];
            m_nStateMachine[4] = (unsigned int)msg[31];

            m_nShutdownFlags  = (((unsigned int)msg[29]) << 8) | msg[30];

            m_nCPUUsage = (((unsigned int)msg[23]) << 8) | msg[24];

            m_chMode  = msg[22];

            m_nVerMajor			= (msg[25] >> 4) & 0x0F;
            m_nVerMinor			= msg[25] & 0x0F;
        }break;
    case NUC_IN_REPORT_IO_DATA3:
        {
            unsigned int i;

            for (i=0;i<32;i++)
                m_bDBG2[i] = msg[i];

            i = (((unsigned int)m_bDBG2[20]) << 8) | m_bDBG2[21];
            m_fBatVoltage[0] = (float)i / (float)1000.0;
            i = (((unsigned int)m_bDBG2[22]) << 8) | m_bDBG2[23];
            m_fBatVoltage[1] = (float)i / (float)1000.0;
            i = (((unsigned int)m_bDBG2[24]) << 8) | m_bDBG2[25];
            m_fBatVoltage[2] = (float)i / (float)1000.0;
            i = (((unsigned int)m_bDBG2[26]) << 8) | m_bDBG2[27];
            m_fBatVoltage[3] = (float)i / (float)1000.0;

            i = (((unsigned int)m_bDBG2[28]) << 8) | m_bDBG2[29];
            m_fBatOverallVoltage = (float)i / (float)1000.0;
            i = (((unsigned int)m_bDBG2[30]) << 8) | m_bDBG2[31];
            m_fBatPack = (float)i / (float)1000.0;

            m_fCellBalanceOn[0] = ((m_bDBG2[2]&0x01)!=0);
            m_fCellBalanceOn[1] = ((m_bDBG2[2]&0x02)!=0);
            m_fCellBalanceOn[2] = ((m_bDBG2[2]&0x04)!=0);
            m_fCellBalanceOn[3] = ((m_bDBG2[2]&0x10)!=0);
        }break;
    case NUC_IN_REPORT_IO_DATA4:
        {
            for (int i=0;i<32;i++)
                m_bDBG[i] = msg[i];

            m_nChargerFlags  =  (((unsigned int)msg[13])<<16) | (((unsigned int)msg[1]) << 8) | msg[2];
            m_nChgTimer[0]   =  (((unsigned int)msg[3]) << 8) | msg[4];
            m_nChgTimer[1]   =  (((unsigned int)msg[5]) << 8) | msg[6];
            m_nChgTimer[2]   =  (((unsigned int)msg[7]) << 8) | msg[8];
            m_nChgTimer[3]   =  (((unsigned int)msg[9]) << 8) | msg[10];

            m_chChargeEndedCondition = msg[11];

            m_chBatteryLevel = msg[12];

            m_fCellDetected[0] = ((m_bDBG[31]&0x01)!=0);
            m_fCellDetected[1] = ((m_bDBG[31]&0x02)!=0);
            m_fCellDetected[2] = ((m_bDBG[31]&0x04)!=0);
            m_fCellDetected[3] = ((m_bDBG[31]&0x08)!=0);
        }break;
    case NUC_IN_REPORT_IO_DATA5:
        {
            for (int i=0;i<32;i++)
                m_bDBG3[i] = msg[i];
        }break;
    case NUC_MEM_READ_IN:
        {            
            memcpy(m_chPackages + (m_ulSettingsAddr-NUC_SETTINGS_ADDR_START), msg+5, 16);
        }break;
    }

}

float HIDNUCUPS::convertOneValue2Float(unsigned char *buffer, int nLen, int nIndex, int nReadMode, double dMultiplier)
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
					value = (double)buffer[nIndex] * GetConstant((int)dMultiplier);
				else
					value = (double)( (((int)buffer[nIndex+1] ) << 8)|buffer[nIndex]) * GetConstant((int)dMultiplier);
			}break;
		case 30:
			{
				if (nLen == 1)
					value = (double)buffer[nIndex] * GetConstant(6);
				else
					value = (double)( (((int)buffer[nIndex+1] ) << 8)|buffer[nIndex]) * GetConstant(6);
			}break;
		case 15:
		case 55:
			{
				unsigned int termistor = 0;
				if (nReadMode == 15)
				{
					termistor = buffer[nIndex+1];
					termistor = (termistor << 8) | buffer[nIndex];
				}
				else
				{
					termistor = buffer[nIndex];
					termistor = (termistor << 8) | buffer[nIndex+1];
				}

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
						
						int temp = (int)ceil(dtemp) + t1;

						value = temp;
					}
				}
			}break;
	}

	return (float)value;
}

bool HIDNUCUPS::readOneValue(char *str, int nReadMode, double dMultiplier, int len, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4)
{
    return false;
}

bool HIDNUCUPS::readOneValue(int mesg_no, char* str, int nReadMode, double dMultiplier, int len, double dMin, double dMax, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4) 
{
	bool ok = false;
	c1 = 0;
	c2 = 0;

	double val = 0;
	
	switch (nReadMode)
	{
	case 3:
		{//hh:mm:ss
			if (_strcmpi(str, "Never")==0)
			{
				val = 0xFFFF;
				ok = true;
			}
			else if ((strlen(str)==8)&&(str[2]==':')&&(str[5]==':'))
			{
				_TCHAR* end = NULL;
				char temps[10];
				
				str_left(temps, str, 2);
				int hh  = _tcstol(temps, &end, 10);
				if ((end != temps)&&(hh>=0)&&(hh<18))
				{
					str_mid(temps, str, 3, 2);
					int mm  = _tcstol(temps, &end, 10);
					if ((end != temps)&&(mm>=0)&&(mm<60))
					{
						str_right(temps, str, 2);
						int ss  = _tcstol(temps, &end, 10);

						if ((end != temps)&&(ss>=0)&&(ss<60))
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
			if (strlen(str)!=0)
			{
				bool negative = false;
				if (str[0] == '-')
				{
					str = str+1;
					negative = true;
				}

				_TCHAR* end = NULL;
				val = _tcstod(str, &end);

				if (end != str)
				{
					if ( (dMin != dMax) && ((val < dMin) || (val > dMax)) ) return false;

					ok = true;

					val /= dMultiplier;
					if (val < 0) val = 0;
					double rem = val - (int) val;
					unsigned long uval = (unsigned long)val;
					if (rem >= 0.5) uval += 1;

					if (uval > 0x7F) uval = 0x7F;

					//for len=1?
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
			/*_TCHAR* end = NULL;
			val = _tcstod(str, &end);

			c2 = 0;
			c1 = HIDNUCUps::GetDataVOut(val);
			ok = true;*/
		}break;
	case 8:
		{
			if (_strcmpi(str, "Never")==0)
			{
				val = (double)0xFFFF * dMultiplier;
				ok = true;
			}
			else
			{
				TCHAR* end = NULL;
				val = _tcstod(str, &end);
				if (end != str)
				{
					if ( (dMin != dMax) && ((val < dMin) || (val > dMax)) ) return false;
					ok = true;
				}
			}
		}break;
	case 12:
	case 13:
		{
			c1 = 0;
			c2 = atoi(str);
			if ( (dMin != dMax) && ((c2 < dMin) || (c2 > dMax)) ) return false;
			ok = true;
		}break;
	case 14:
		{
			if (len == 1)
			{
				c1 = 0;
				c2 = bin2char(str, &ok);
			}
			else
			{
				c1 = bin2char(str, &ok);
				c2 = 0;
			}
		}break;
	case 15:
	case 55:
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

			if (nReadMode == 15)
			{
				c1 = (unsigned char)(((unsigned int)termistor) & 0xFF);
				c2 = (unsigned char)((((unsigned int)termistor)>>8) & 0xFF);
			}
			else
			{
				c1 = (unsigned char)((((unsigned int)termistor)>>8) & 0xFF);
				c2 = (unsigned char)(((unsigned int)termistor) & 0xFF);
			}
			ok = true;
		}break;
	case 16:
		{
			TCHAR* end = NULL;
			val = _tcstod(str, &end);

			if ((val != 0)&& (end != str))
			{
				if ( (dMin != dMax) && ((val < dMin) || (val > dMax)) ) return false;

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
			_TCHAR* end = NULL;
			val = _tcstod(str, &end);
			if (end != str)
			{
				if ( (dMin != dMax) && ((val < dMin) || (val > dMax)) ) return false;
				ok = true;
			}
			if ((val < 0) || (val > 255)) ok = false;
		}break;
	case 18:
		{
			_TCHAR* end = NULL;
			val = _tcstod(str, &end);
			if (end != str)
			{
				if ( (dMin != dMax) && ((val < dMin) || (val > dMax)) ) return false;
				ok = true;
				val = floor(1 / val * 1000000);
				if (val < 1428 ) val = 1428;
				else if (val > 5000) val = 5000;
			}
		}break;
	case 19:
		{//iout >>> n
			_TCHAR* end = NULL;
			val = _tcstod(str, &end);
			if (end != str)
			{
				if ( (dMin != dMax) && ((val < dMin) || (val > dMax)) ) return false;

				ok = true;
				val = floor( (val - 560) / 1280 + 2);
				//if ((val < 2) || (val > 31)) ok = false;
			}
		}break;
	case 40:
		{
			_TCHAR* end = NULL;
			val  = _tcstol(str, &end, 16);
			if ( (dMin != dMax) && ((val < dMin) || (val > dMax)) ) return false;
			ok = true;
		}break;//hex
	case 101:
		{
			int ival = atoi(str);
			if (ival < GetSpecialConstants(nReadMode, 0)) ival = GetSpecialConstants(nReadMode, 0);
			if (ival > GetSpecialConstants(nReadMode, 7)) ival = GetSpecialConstants(nReadMode, 7);
			for (int i=0;i<=7;i++)
			{
				if (GetSpecialConstants(nReadMode, i) >= ival)
				{
					ival = i;
					i = 1000;//break for
					ok = true;
				}
			}
			if (ok)
			{
				c1 = (m_chPackages[GetMessages()[mesg_no].nIndex] & 0xF8) | ival; 
				c2 = 0;
			}
		}break;
	case 102:
		{
			int ival = atoi(str);
			if (ival < GetSpecialConstants(nReadMode, 0)) ival = GetSpecialConstants(nReadMode, 0);
			if (ival > GetSpecialConstants(nReadMode, 3)) ival = GetSpecialConstants(nReadMode, 3);
			for (int i=0;i<=3;i++)
			{
				if (GetSpecialConstants(nReadMode, i) >= ival)
				{
					ival = i;
					i = 1000;//break for
					ok = true;
				}
			}
			if (ok)
			{
				c1 = (m_chPackages[GetMessages()[mesg_no].nIndex] & 0xE7) | (ival << 3);
				c2 = 0;
			}
		}break;
	case 103:
		{
			int ival = atoi(str);
			if (ival < GetSpecialConstants(nReadMode, 0)) ival = GetSpecialConstants(nReadMode, 0);
			if (ival > GetSpecialConstants(nReadMode, 15)) ival = GetSpecialConstants(nReadMode, 15);
			for (int i=0;i<=15;i++)
			{
				if (GetSpecialConstants(nReadMode, i) >= ival)
				{
					ival = i;
					i = 1000;//break for
					ok = true;
				}
			}
			if (ok)
			{
				c1 = (m_chPackages[GetMessages()[mesg_no].nIndex] & 0xF0) | ival;
				c2 = 0;
			}
		}break;
	case 104:
		{
			int ival = atoi(str);
			if (ival < GetSpecialConstants(nReadMode, 0)) ival = GetSpecialConstants(nReadMode, 0);
			if (ival > GetSpecialConstants(nReadMode, 7)) ival = GetSpecialConstants(nReadMode, 7);
			for (int i=0;i<=7;i++)
			{
				if (GetSpecialConstants(nReadMode, i) >= ival)
				{
					ival = i;
					i = 1000;//break for
					ok = true;
				}
			}
			if (ok)
			{
				c1 = (m_chPackages[GetMessages()[mesg_no].nIndex] & 0x8F) | (ival << 4);
				c2 = 0;
			}
		}break;
	case 105:
		{//uv-delay
			int ival = atoi(str);
			if (ival < GetSpecialConstants(nReadMode, 0)) ival = GetSpecialConstants(nReadMode, 0);
			if (ival > GetSpecialConstants(nReadMode, 3)) ival = GetSpecialConstants(nReadMode, 3);
			for (int i=0;i<=3;i++)
			{
				if (GetSpecialConstants(nReadMode, i) >= ival)
				{
					ival = i;
					i = 1000;//break for
					ok = true;
				}
			}
			if (ok)
			{
				c1 = (m_chPackages[GetMessages()[mesg_no].nIndex] & 0x3F) | (ival << 6);
				c2 = 0;
			}
		}break;
	case 106:
		{
			int ival = atoi(str);
			if (ival < GetSpecialConstants(nReadMode, 0)) ival = GetSpecialConstants(nReadMode, 0);
			if (ival > GetSpecialConstants(nReadMode, 3)) ival = GetSpecialConstants(nReadMode, 3);
			for (int i=0;i<=3;i++)
			{
				if (GetSpecialConstants(nReadMode, i) >= ival)
				{
					ival = i;
					i = 1000;//break for
					ok = true;
				}
			}
			if (ok)
			{
				c1 = (m_chPackages[GetMessages()[mesg_no].nIndex] & 0xCF) | (ival << 4);
				c2 = 0;
			}
		}break;

	case 1:
		//normal read - integer
	case 2:
	case 9:
	case 5:
	default:
		{//normal read - float
			_TCHAR* end = NULL;
			val = _tcstod(str, &end);

			if (end != str)
			{
				if ( (dMin != dMax) && ((val < dMin) || (val > dMax)) ) return false;
				ok = true;
			}
		}
	}

	if (ok)
	{
		if ((nReadMode != 4)&&(nReadMode != 7)&&(nReadMode != 9)&&(nReadMode != 11)&&(nReadMode != 12)&&(nReadMode != 13)&&(nReadMode != 14)&&(nReadMode != 15)&&(nReadMode != 16)&&(nReadMode < 100))
		{
			switch (nReadMode)
			{
				case 10: val /= GetConstant((int)dMultiplier); break;
				case 30: val /= GetConstant(6); break;
				default:val /= dMultiplier;
			}

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

				//DBGOutput("      uval1=%d %x\n",uval,uval);
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
				//DBGOutput("      uval2=%d %x\n",uval,uval);
			}
		}
	}
 
	return ok;
}

void HIDNUCUPS::convertOneValue2String(char *destination, int nLen, int nIndex, int nReadMode, double dMultiplier)
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
		case 2:   
		case 9:
			sprintf(destination, "%.3f",(float)value);
			break;
		case 20:  sprintf(destination, "%.2f",(float)value);break;
		case 10:
			{
				if (nLen == 1)
					value = (double)m_chPackages[nIndex] * GetConstant((int)dMultiplier);
				else if (nLen == 2)
					value = (double)( (((int)m_chPackages[nIndex+1] ) << 8)|m_chPackages[nIndex]) * GetConstant((int)dMultiplier);
				else if (nLen == 4)
					value = (double)( (((int)m_chPackages[nIndex+3] ) << 24) | (((int)m_chPackages[nIndex+2] ) << 16) | (((int)m_chPackages[nIndex+1] ) << 8) | m_chPackages[nIndex]) * HIDNUCUPS::GetConstant((int)dMultiplier);
				sprintf(destination, "%.3f",(float)value);
			}break;
		case 30:
			{
				if (nLen == 1)
					value = (double)m_chPackages[nIndex] * GetConstant(6);
				else if (nLen == 2)
					value = (double)( (((int)m_chPackages[nIndex+1] ) << 8)|m_chPackages[nIndex]) * GetConstant(6);
				else if (nLen == 4)
					value = (double)( (((int)m_chPackages[nIndex+3] ) << 24) | (((int)m_chPackages[nIndex+2] ) << 16) | (((int)m_chPackages[nIndex+1] ) << 8) | m_chPackages[nIndex]) * HIDNUCUPS::GetConstant(6);
				if (value > dMultiplier) value = dMultiplier;
				sprintf(destination, "%.3f",(float)value);
			}break;
		case 12:
		case 13:
			sprintf(destination, "%d", m_chPackages[nIndex]);
			break;
		case 14:
			char2bin(destination, m_chPackages[nIndex]);
			break;
		case 15:
		case 55:
			{
				unsigned int termistor = 0;

				if (nReadMode == 15)
				{
					termistor = m_chPackages[nIndex+1];
					termistor = (termistor << 8) | m_chPackages[nIndex];
				}
				else
				{
					termistor = m_chPackages[nIndex];
					termistor = (termistor << 8) | m_chPackages[nIndex+1];
				}

				if (termistor <= GetTermistorConsts()[0])
					strcpy(destination, "-40");
				else if (termistor >= GetTermistorConsts()[TERMISTOR_CONSTS-1])
					strcpy(destination, "125");
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
						sprintf(destination, "%d",pos*5-40);
					else
					{
						int t1 = pos*5-40;
						int t2 = (pos+1)*5-40;

						unsigned int d1 = GetTermistorConsts()[pos];
						unsigned int d2 = GetTermistorConsts()[pos+1];

						float dtemp = ((float)termistor - (float)d1)*((float)t2-(float)t1)/((float)d2-(float)d1);
						
						int temp = (int)ceil(dtemp) + t1;

						sprintf(destination, "%d",temp);
					}
				}
			}break;
		case 16:
			{//4 byte value
				if (dMultiplier == 1)
					 sprintf(destination,"%.0f", (float)value);
				else sprintf(destination,"%.2f", (float)value);
			}break;
		case 17:
			{
				sprintf(destination, "%d", (int)value);
			}break;
		case 18:
			{//khz
				if (value != 0)
				{
					value = 1.0/value * 1000000;
					sprintf(destination, "%d", (int)value);
				}
				else sprintf(destination, "0");
			}break;
		case 19:
			{//N >>> IOUT
				if (value >= 2)
				{
					value = 560 + (value - 2) * 1280;
					sprintf(destination, "%d", (int)value);
				}
				else sprintf(destination, "0");
			}break;
		case 3:
			{
				unsigned int ivalue = (unsigned int)value;
				if (ivalue == 0xFFFF)
					strcpy(destination, "Never");
				else sprintf(destination, "%02d:%02d:%02d"
						, ivalue/3600
						, (ivalue/60)%60
						, ivalue % 60
						);
			}break;
		case 4:
			{
				double dvalue = (m_chPackages[nIndex] & 0x7f) * dMultiplier;
				if ((m_chPackages[nIndex] & 0x80)!=0)
					 sprintf(destination,"-%.3f", (float)value);
				else sprintf(destination,"%.3f", (float)value);
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
				strcpy(destination, "-");
			}break;
		case 7:
			{//no multiplier - voltage value calcualted with formula
				//! sprintf(destination, "%.3f", GetVoltageVOut(m_chPackages[nIndex]));
			}break;
		case 8:
			{
				if (nLen == 1)
				{
					if (m_chPackages[nIndex] == 0xFF)
						 strcpy(destination, "Never");
					else sprintf(destination, "%d", (int)value);
				}
				else
				{
					if ((m_chPackages[nIndex] == 0xFF) && (m_chPackages[nIndex+1] == 0xFF))
						 strcpy(destination, "Never");
					else sprintf(destination, "%d", (int)value);
				}
			}break;
		case 40://hex
			if (nLen == 1)
				sprintf(destination, "%02x",(int)value);
			else if (nLen == 2)
				sprintf(destination, "%04x",(int)value);
			else if (nLen == 4) 
				sprintf(destination, "%08x",(int)value);
			break;
		case 101://SCD_TRESHOLD
			sprintf(destination, "%d", GetSpecialConstants(nReadMode, m_chPackages[nIndex] & 0x07));
			break;
		case 102://SCD_DELAY
			sprintf(destination, "%d", GetSpecialConstants(nReadMode, (m_chPackages[nIndex] >> 3) & 0x03));
			break;
		case 103://OCD_TRESHOLD
			sprintf(destination, "%d", GetSpecialConstants(nReadMode, m_chPackages[nIndex] & 0x0F));
			break;
		case 104://OCD_DELAY
			sprintf(destination, "%d", GetSpecialConstants(nReadMode, (m_chPackages[nIndex] >> 4) & 0x07));
			break;
		case 105://UV_DELAY
			sprintf(destination, "%d", GetSpecialConstants(nReadMode, (m_chPackages[nIndex] >> 6) & 0x03));
			break;
		case 106://OV_DELAY
			sprintf(destination, "%d", GetSpecialConstants(nReadMode, (m_chPackages[nIndex] >> 4) & 0x03));
			break;
		case 1:
		default:
			{
				sprintf(destination, "%d", (int)value);
			}
	}
}

bool HIDNUCUPS::setVariableData(int mesg_no, char *str)
{
    int len = GetMessages()[mesg_no].nLen;

	if (len!=0)
	{
		if (!GetMessages()[mesg_no].bEnabled) return 0;//read only variable

		unsigned char c1 = 0;
		unsigned char c2 = 0;
		unsigned char c3 = 0;
		unsigned char c4 = 0;

		if (readOneValue(mesg_no
						,str
						,GetMessages()[mesg_no].nReadMode
						,GetMessages()[mesg_no].dMultiplier
						,len
						,GetMessages()[mesg_no].dLimitMin
						,GetMessages()[mesg_no].dLimitMax
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
			return 1;
		}
		else return 0;
	}
	return 1;
}

ATXMSG* HIDNUCUPS::GetMessages()
{
	return g_NUCUPS_memMessages;
}

double HIDNUCUPS::GetConstant(int i)
{
	return g_NUCUPS_memConstants[i];
}

unsigned int* HIDNUCUPS::GetTermistorConsts()
{
	return g_NUCUPS_memTerm;
}

unsigned int HIDNUCUPS::GetSpecialConstants(unsigned int type, unsigned int cnt)
{
	switch (type)
	{
        case 101: if (cnt < 8) return g_NUCUPS_memSCDTreshold[cnt]; break;
        case 102: if (cnt < 4) return g_NUCUPS_memSCDDelay[cnt]; break;
        case 103: if (cnt < 16) return g_NUCUPS_memOCDTreshold[cnt]; break;
        case 104: if (cnt < 8) return g_NUCUPS_memOCDDelay[cnt]; break;
        case 105: if (cnt < 4) return g_NUCUPS_memUVDelay[cnt]; break;
        case 106: if (cnt < 4) return g_NUCUPS_memOVDelay[cnt]; break;
	}
	return -1;
}

unsigned char HIDNUCUPS::getUPSVariableData(unsigned int cnt, char *name, char *value, char *unit, char *comment)
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

void HIDNUCUPS::restartUPS()
{
    sendCommand(DCMD_RESTART_NUC, 1);
}

void HIDNUCUPS::restartUPSInBootloaderMode()
{
    sendMessage(NUC_ENTER_BOOTLOADER_OUT, 2, 0xFA, 0xCD);

}

void HIDNUCUPS::GetStatus()
{
    unsigned char recv[32];
	int ret;

	sendMessage(NUC_OUT_REPORT_IO_DATA, 0);
	recvMessage(recv);
	parseMessage(recv);

	ret = sendMessage(NUC_OUT_REPORT_IO_DATA2, 0);
	recvMessage(recv);
	parseMessage(recv);

	ret = sendMessage(NUC_OUT_REPORT_IO_DATA3, 0);
	recvMessage(recv);
	parseMessage(recv);

    ret = sendMessage(NUC_OUT_REPORT_IO_DATA4, 0);
	recvMessage(recv);
	parseMessage(recv);

/* 
	IO_DATA5 messages result in incomplete reads leading to timeouts on usb 
 	TODO: Investigate protocol for IO_DATA5  
*/

/* 
	ret = sendMessage(NUC_OUT_REPORT_IO_DATA5, 0);
	recvMessage(recv);
	parseMessage(recv);
*/
}

void HIDNUCUPS::ReadConfigurationMemory()
{
    unsigned char recv[32];

	m_ulSettingsAddr = NUC_SETTINGS_ADDR_START;
	memset(m_chPackages, 0, SETTINGS_PACKS * 16);

	while (m_ulSettingsAddr < NUC_SETTINGS_ADDR_END)
	{
		sendMessage(NUC_MEM_READ_OUT, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
		recvMessage(recv);
		parseMessage(recv);
		m_ulSettingsAddr += 16;
	}
}

void HIDNUCUPS::EraseConfigurationMemory()
{
    unsigned char recv[32];
	int ret;

	m_ulSettingsAddr = NUC_SETTINGS_ADDR_START;
	sendMessage(NUC_MEM_ERASE, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x40);

	int retries = 5;
	while ((ret = recvMessage(recv) <= 0) && retries > 0) {
		retries--;
		usleep(500*1000);
		fprintf(stderr, "Erase 0x%02x retry %d/5\n", recv[0], retries);
	}

	if (retries <= 0) {
		fprintf(stderr, "Error waiting for erase operation to finish\n");
	} else {
		fprintf(stderr, "Successfully erased (0x%02x) configuration memory\n", recv[0]);
	}
}

void HIDNUCUPS::WriteConfigurationMemory()
{
    unsigned char recv[32];
	int ret;

	m_ulSettingsAddr = NUC_SETTINGS_ADDR_START;
	while (m_ulSettingsAddr < NUC_SETTINGS_ADDR_END)
	{
		sendMessageWithBuffer(NUC_MEM_WRITE_OUT, 16, m_chPackages+(m_ulSettingsAddr-NUC_SETTINGS_ADDR_START), 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
		ret = recvMessage(recv);
		
		if (ret <= 0 || recv[0] != NUC_MEM_WRITE_IN) {
			fprintf(stderr, "Error (%d, 0x%02x) writing configuration variables , aborting ...\n", ret, recv[0]);
			break;
		} else {
			//fprintf(stderr, "Wrote page 0x%lx of configuration\n", m_ulSettingsAddr);
		}		
		m_ulSettingsAddr += 16;
	}

}
