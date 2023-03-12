/*
 *    Author: Nicu Pavel <npavel@mini-box.com>
 *    Copyright (c) 2020 Mini-Box.com
 *    Licensed under the terms of the GNU Lesser General Public License v2.1
 *  	http://www.opensource.org/licenses/lgpl-2.1.php 
 * 
 */

#include "HIDDCDCUSB.h"
#include "util.h"
#include <math.h>
#include <unistd.h>

#define A_SLEEP 5
#define A_TIMEOUT 3000
#define BUF_LINE_LEN 47
#define STR_LINE_LEN 57

#define PKG_LEN_LCD 64
#define PKG_LEN_FLASH 37

unsigned int g_DCDCUSB_memTerm[] = {(unsigned int)0x01};

ATXMSG g_DCDCUSB_memMessages[DCDCUSB_MAX_MESSAGE_CNT] =
	{
		{52, 1, _T("VOLTAGE_0"), true, 7, 0, 0, 0, _T("Set Output Voltage 0. Changes will take effect after a power cycle."), _T("[V]")},
		{53, 1, _T("VOLTAGE_1"), true, 7, 0, 0, 0, _T("Set Output Voltage 1. Changes will take effect after a power cycle."), _T("[V]")},
		{54, 1, _T("VOLTAGE_2"), true, 7, 0, 0, 0, _T("Set Output Voltage 2. Changes will take effect after a power cycle."), _T("[V]")},
		{55, 1, _T("VOLTAGE_3"), true, 7, 0, 0, 0, _T("Set Output Voltage 3. Changes will take effect after a power cycle."), _T("[V]")},
		{56, 1, _T("VOLTAGE_4"), true, 7, 0, 0, 0, _T("Set Output Voltage 4. Changes will take effect after a power cycle."), _T("[V]")},
		{57, 1, _T("VOLTAGE_5"), true, 7, 0, 0, 0, _T("Set Output Voltage 5. Changes will take effect after a power cycle."), _T("[V]")},
		{58, 1, _T("VOLTAGE_6"), true, 7, 0, 0, 0, _T("Set Output Voltage 6. Changes will take effect after a power cycle."), _T("[V]")},
		{59, 1, _T("VOLTAGE_7"), true, 7, 0, 0, 0, _T("Set Output Voltage 7. Changes will take effect after a power cycle."), _T("[V]")},

		{0, 1, _T("IGNITION_HIGH_LIMIT"), true, 2, 0.1558, 0, 0, _T("Above this voltage IGN is considered ON"), _T("[V]")},
		{1, 1, _T("IGNITION_LOW_LIMIT"), true, 2, 0.1558, 0, 0, _T("Below this voltage IGN is considered OFF"), _T("[V]")},
		{2, 1, _T("IGNITION_DEBOUNCE"), false, 1, 10, 0, 0, _T("Ignition filter de-bouncing"), _T("[ms]")},

		{30, 1, _T("AUTOMOTIVE_IGNITION_CANCEL_TIME"), true, 1, 1, 0, 0, _T("After DCDC-USB \"boots\" in AUTOMOTIVE mode IGN is cancelled for this time"), _T("[sec]")},
		{14, 1, _T("AUTOMOTIVE_THUMP_TIMEOUT"), true, 1, 1, 0, 0, _T("THUMP timeout, switching off the audio amplifier during motherboard startup/shutdown"), _T("[sec]")},

		{3, 1, _T("DELAY_BEFORE_PSU_STARTUP"), true, 1, 1, 0, 0, _T("Initial startup delay to avoid re-cranking problems"), _T("[s]")},
		{4, 1, _T("MIN_VIN_VOLTAGE_AT_STARTUP"), true, 2, 0.1558, 0, 0, _T("Input voltage STARTUP ZONE treshold "), _T("[V]")},
		{5, 1, _T("MIN_VIN_VOLTAGE_WHILE_RUNNING"), true, 2, 0.1558, 0, 0, _T("Input voltage RUNNING ZONE treshold "), _T("[V]")},
		{6, 1, _T("MAX_ALLOWED_VIN_VOLTAGE"), false, 2, 0.1558, 0, 0, _T("Input voltage, HIGH SHUTDOWN treshold [35V]"), _T("[V]")},

		{63, 1, _T("MIN_VIN_VOLTAGE_DEEP_DISCHARGE"), true, 2, 0.1558, 0, 0, _T("Minimum VIN voltage measured during off delay"), _T("[V]")},

		{7, 1, _T("ON_PULSE_DELAY"), true, 1, 1, 0, 0, _T("Delay between output is ON and startup pulse is sent\n(v4.3+)"), _T("[s]")},
		//	{ 8, 1, _T("Min. limit for Voltage")			, true, 2, 0.1558,0,0	, _T("MIN voltage treshold"), _T("[V]") },
		{9, 1, _T("REGULATOR_OFFSET"), true, 1, 1, 0, 0, _T("Regulator offset (firmware version >= v2.0)"), _T("")},
		{10, 2, _T("PSW_PUSH_DOWN_TIME"), true, 1, 10, 0, 0, _T("How long the PSW button is pressed down"), _T("[ms]")},
		//{11, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		//	{ 12, 1, _T("Emergency timeout")				, true, 1, 1,0,0					, _T("Delay"), _T("[s]") },
		{13, 1, _T("CONFIGURATION_FLAGS"), true, 9, 1, 0, 0, _T("Configuration flags: bit0=Regulator Enabled/Disabled(v2.0+)\nbit1=Power cycle On/Off(v2.1+)\nbit2=On pulse enable/disable bit3=Off pulse enable/disable(v3.1+)\nbit4=USB sense enable/disable(v4.0+)\nbit5=Ignition pulse enable/disable(v4.2+)"), _T("")},
		{15, 1, _T("MAX_REGULATION_STEP_NUMBER"), true, 1, 1, 0, 0, _T("Maximum allowed regulation step number (0-255) (firmware version >= v2.0)"), _T("")},

		{16, 2, _T("OFF_DELAY_TIME_0"), false, 3, 1, 0, 0, _T("Time between IGN OFF to PSW OFF impulse in Time Configuration 0 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{17, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		{18, 2, _T("HARD_OFF_TIME_0"), false, 3, 1, 0, 0, _T("Time Between PSW OFF to Output OFF,Hardware OFF in Time Configuration 0 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{19, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		{20, 2, _T("OFF_DELAY_TIME_1"), true, 3, 1, 0, 0, _T("Time between IGN OFF to PSW OFF impulse in Time Configuration 1 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{21, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		{22, 2, _T("HARD_OFF_TIME_1"), true, 3, 1, 0, 0, _T("Time Between PSW OFF to Output OFF,Hardware OFF in Time Configuration 1 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{23, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		{24, 2, _T("OFF_DELAY_TIME_2"), true, 3, 1, 0, 0, _T("Time between IGN OFF to PSW OFF impulse in Time Configuration 2 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{25, 0, _T("FILLER")							, true, 1, 1		, _T(""), _T("") },
		{26, 2, _T("HARD_OFF_TIME_2"), true, 3, 1, 0, 0, _T("Time Between PSW OFF to Output OFF,Hardware OFF in Time Configuration 2 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{27, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		{28, 2, _T("OFF_DELAY_TIME_3"), true, 3, 1, 0, 0, _T("Time between IGN OFF to PSW OFF impulse in Time Configuration 3 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{29, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },

		{32, 2, _T("HARD_OFF_TIME_3"), true, 3, 1, 0, 0, _T("Time Between PSW OFF to Output OFF,Hardware OFF in Time Configuration 3 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{33, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		{34, 2, _T("OFF_DELAY_TIME_4"), true, 3, 1, 0, 0, _T("Time between IGN OFF to PSW OFF impulse in Time Configuration 4 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{35, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		{36, 2, _T("HARD_OFF_TIME_4"), true, 3, 1, 0, 0, _T("Time Between PSW OFF to Output OFF,Hardware OFF in Time Configuration 4 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{37, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		{38, 2, _T("OFF_DELAY_TIME_5"), true, 3, 1, 0, 0, _T("Time between IGN OFF to PSW OFF impulse in Time Configuration 5 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{39, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		{40, 2, _T("HARD_OFF_TIME_5"), true, 3, 1, 0, 0, _T("Time Between PSW OFF to Output OFF,Hardware OFF in Time Configuration 5 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{41, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		{42, 2, _T("OFF_DELAY_TIME_6"), true, 3, 1, 0, 0, _T("Time between IGN OFF to PSW OFF impulse in Time Configuration 6 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{43, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		{44, 2, _T("HARD_OFF_TIME_6"), true, 3, 1, 0, 0, _T("Time Between PSW OFF to Output OFF,Hardware OFF in Time Configuration 6 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{45, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },

		{48, 2, _T("OFF_DELAY_TIME_7"), true, 3, 1, 0, 0, _T("Time between IGN OFF to PSW OFF impulse in Time Configuration 7 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{49, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },
		{50, 2, _T("HARD_OFF_TIME_7"), true, 3, 1, 0, 0, _T("Time Between PSW OFF to Output OFF,Hardware OFF in Time Configuration 7 (max. 17:59:59 or Never)"), _T("[hh:mm:ss]")},
		//{51, 0, _T("FILLER")							, true, 1, 1,0,0		, _T(""), _T("") },

		{46, 1, _T("UPS_MODE_SHUTDOWN_TIME"), true, 1, 1, 0, 0, _T("After OFF impulse is sent wait \"UPS Mode: Shutdown Time\" till Output OFF,Hardware OFF"), _T("[sec]")},
		{47, 1, _T("UPS_MODE_IGNITION_OFF_TIME"), true, 1, 1, 0, 0, _T("If IGN is OFF for \"UPS Mode: Ignition OFF time\" time the PSU will send OFF impulse"), _T("[sec]")},

		{60, 1, _T("UPS_MODE_IGNITION_ON"), true, 2, 0.1558, 0, 0, _T("PSU will be ON if IGN>\"UPS Mode: Ignition ON\" (and VIN>\"UPS Mode: VIn Good\")"), _T("[V]")},
		{61, 1, _T("UPS_MODE_IGNITION_OFF"), true, 2, 0.1558, 0, 0, _T("PSU will be OFF if IGN<\"UPS Mode: Ignition OFF\" for \"UPS Mode: Ignition OFF time\" seconds"), _T("[V]")},
		{62, 1, _T("UPS_MODE_VIN_GOOD"), true, 2, 0.1558, 0, 0, _T("PSU will be ON if VIN>\"UPS Mode: VIn Good\" (and IGN>\"UPS Mode: Ignition ON\")"), _T("[V]")},
};

double HIDDCDCUSB::GetVOut(unsigned char data)
{
	double rpot = ((double)data) * DCDCUSB_CT_RP / (double)257 + DCDCUSB_CT_RW;
	double voltage = (double)80 * ((double)1 + DCDCUSB_CT_R1 / (rpot + DCDCUSB_CT_R2));
	voltage = floor(voltage);
	return voltage / 100;
}

unsigned char HIDDCDCUSB::ConvertVoltageToChar(double vout)
{
	if (vout < 5)
		vout = 5;
	
	if (vout > 25)
		vout = 25;

	double rpot = (double)0.8 * DCDCUSB_CT_R1 / (vout - (double)0.8) - DCDCUSB_CT_R2;
	double result = (257 * (rpot - DCDCUSB_CT_RW) / DCDCUSB_CT_RP);

	if (result < 0)
		result = 0;
	if (result > 255)
		result = 255;

	return (unsigned char)result;
}

HIDDCDCUSB::HIDDCDCUSB(USBHID *d) : HIDInterface(d)
{
	m_ulSettingsAddr = DCDCUSB_SETTINGS_ADDR_START;
}

HIDDCDCUSB::~HIDDCDCUSB()
{
}

ATXMSG *HIDDCDCUSB::GetMessages()
{
	return g_DCDCUSB_memMessages;
}

double HIDDCDCUSB::GetConstant(int i)
{
	return 1.0;
}

unsigned int *HIDDCDCUSB::GetTermistorConsts()
{
	return g_DCDCUSB_memTerm;
}

void HIDDCDCUSB::printValues()
{
	fprintf(stdout, "Mode: %d %s\n", m_nMode, m_strMode);
	fprintf(stdout, " Configured Voltage Ouput: %.02f\n", m_fVOutSet);
	fprintf(stdout, " Voltage Config: %d\n V In: %.02f\n V Ign: %.02f\n V Out: %.02f\n", m_nVoltageCfg, m_fVIn, m_fIgn, m_fVOut);
	fprintf(stdout, " Power Switch: %d\n Output: %d\n Aux V In: %d\n", m_bPowerSwitch, m_bOutput, m_bAuxVin);
	fprintf(stdout, " Version: %d.%d\n State: 0x%02x\n", m_nVerMajor, m_nVerMinor, m_nState);
	fprintf(stdout, " Voltage (flags): %s\n Status 1 (flags): %s\n Status 2 (flags): %s\n", m_strFlagsVoltage, m_strFlagsStatus1, m_strFlagsStatus2);
	fprintf(stdout, " Timer (flags): %s\n Timer Config: 0x%02x\n Voltage Config: %s\n", m_strFlagsTimer, m_nTimeCfg, m_strVoltCfg);
	fprintf(stdout, " Timer Wait: %s\n Timer V Out: %s\n Timer PW Switch: %s\n", m_strTimerWait, m_strTimerVout, m_strTimerPwSwitch);
	fprintf(stdout, " Timer V Aux: %s\n Timer Off Delay: %s\n Timer Hard Off: %s\n", m_strTimerVAux, m_strTimerOffDelay, m_strTimerHardOff);
}

void HIDDCDCUSB::parseMessage(unsigned char *msg)
{
	if (!msg)
		return;

	switch (msg[0])
	{
	case DCDCUSB_INTERNAL_MESG:
	{
		switch (msg[1])
		{
		case DCDCUSB_INTERNAL_MESG_DISCONNECTED:
		{
			fprintf(stderr, "DCDCUSB DEVICE DISCONNECTED!");
		}
		break;
		}
	}
	break;
	case DCDCUSB_RECV_ALL_VALUES:
	{
		m_nState = msg[2];
		m_nTimeCfg = (msg[1] >> 5) & 0x7;
		m_nVoltageCfg = (msg[1] >> 2) & 0x7;

		float val;
		//VIN
		m_fVIn = (float)msg[3] * (float)0.1558;

		//Ign
		m_fIgn = (float)msg[4] * (float)0.1558;

		//VOut
		m_fVOut = (float)msg[5] * (float)0.1170;

		int status1 = msg[6];
		if (status1 & 0x4)
			m_bPowerSwitch = true;
		else
			m_bPowerSwitch = false;

		if (status1 & 0x8)
			m_bOutput = true;
		else
			m_bOutput = false;

		if (status1 & 0x10)
			m_bAuxVin = true;
		else
			m_bAuxVin = false;

		char2bin(m_strFlagsStatus1, msg[6]);
		char2bin(m_strFlagsStatus2, msg[7]);
		char2bin(m_strFlagsVoltage, msg[8]);
		char2bin(m_strFlagsTimer, msg[9]);

		m_nScriptPointer = msg[10];

		convertOneValue2String(m_strTimerWait, 2, 11, 3, 1);
		convertOneValue2String(m_strTimerVout, 2, 13, 3, 1);
		convertOneValue2String(m_strTimerVAux, 2, 15, 3, 1);
		convertOneValue2String(m_strTimerPwSwitch, 2, 17, 3, 1);
		convertOneValue2String(m_strTimerOffDelay, 2, 19, 3, 1);
		convertOneValue2String(m_strTimerHardOff, 2, 21, 3, 1);

		m_bVersion = msg[23];
		m_nVerMajor = (msg[23] >> 5) & 0x07;
		m_nVerMinor = msg[23] & 0x1F;
		m_nMode = (msg[1]) & 0x3;

		switch ((msg[1]) & 0x3)
		{
		case 0:
			strcpy(m_strMode, "(Dumb)");
			break;
		case 1:
			strcpy(m_strMode, "(Automotive)");
			break;
		case 2:
			strcpy(m_strMode, "(Script)");
			break;
		case 3:
			strcpy(m_strMode, "(UPS)");
			break;
		}
	}
	break;
	case DCDCUSB_CMD_IN:
	{
		if (msg[1] != 0)
		{
			if (msg[2] == DCDCUSB_CMD_READ_REGULATOR_STEP)
				fprintf(stdout, "Regulator: undefined");
			else
				fprintf(stderr, "Error %d for message %02X", msg[1], msg[2]);
		}
		else
		{
			switch (msg[2])
			{
			case DCDCUSB_CMD_WRITE_VOUT:
			case DCDCUSB_CMD_READ_VOUT:
			case DCDCUSB_CMD_INC_VOUT:
			case DCDCUSB_CMD_DEC_VOUT:
			{
				m_fVOutSet = HIDDCDCUSB::GetVOut(msg[3]);				
			}
			break;
			case DCDCUSB_CMD_READ_REGULATOR_STEP:
			{
				fprintf(stdout, "Regulator Step: %d", msg[3]);
			}
			break;
			}
		}
	}
	break;
	case DCDCUSB_MEM_READ_IN:
	{
		//TX[24]:A1 C0 3F 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
		//RX[24]:A2 C0 3F 00 10 26 24 32 05 26 23 E0 AA 00 03 00 32 0A 00 AA AA 00 00 01
		memcpy(m_chPackages + (m_ulSettingsAddr - DCDCUSB_SETTINGS_ADDR_START), msg + 5, 16);
	}
	break;
	}
}

float HIDDCDCUSB::convertOneValue2Float(unsigned char *buffer, int nLen, int nIndex, int nReadMode, double dMultiplier)
{
	double value = -1;

	if (nLen == 1)
	{
		value = (double)buffer[nIndex] * dMultiplier;
	}
	else
	{
		value = (double)((((int)buffer[nIndex]) << 8) | buffer[nIndex + 1]) * dMultiplier;
	}

	switch (nReadMode)
	{
	case 5:
	{
		if (nLen == 1)
			value = (double)((char)buffer[nIndex]) * dMultiplier;
		else
			value = (double)((((int)((char)buffer[nIndex])) << 8) | buffer[nIndex + 1]) * dMultiplier;
	}
	break;

	case 7:
	{
		//no multiplier - voltage value calculated with formula
		value = HIDDCDCUSB::GetVOut(buffer[nIndex]);
	}
	break;
	}

	return (float)value;
}

bool HIDDCDCUSB::readOneValue(char *str, int nReadMode, double dMultiplier, int len, unsigned char &c1, unsigned char &c2, unsigned char &c3, unsigned char &c4)
{
	bool ok = false;
	c1 = 0;
	c2 = 0;

	double val = 0;

	switch (nReadMode)
	{
	case 3:
	{ //hh:mm:ss
		if (strcmp(str, "Never") == 0)
		{
			val = 0xFFFF;
			ok = true;
		}
		else if ((strlen(str) == 8) && (str[2] == ':') && (str[5] == ':'))
		{
			_TCHAR *end = NULL;
			char temps[10];
			str_left(temps, str, 2);
			int hh = _tcstol(temps, &end, 10);
			if ((end != temps) && (hh >= 0) && (hh < 18))
			{
				str_mid(temps, str, 3, 2);
				int mm = _tcstol(temps, &end, 10);
				if ((end != temps) && (mm >= 0) && (mm < 60))
				{
					str_right(temps, str, 2);
					int ss = _tcstol(temps, &end, 10);

					if ((end != temps) && (ss >= 0) && (ss < 60))
					{
						val = hh * 3600 + mm * 60 + ss;
						ok = true;
					}
				}
			}
		}
	}
	break;
	case 4:
	{ //"signed" float
		if (strlen(str) != 0)
		{
			bool negative = false;
			char temps[10];
			char temps2[10];

			if (str[0] == '-')
			{
				str_right(temps, str, strlen(str) - 1);
				str = temps;
				negative = true;
			}

			_TCHAR *end = NULL;
			str_left(temps2, str, 8);
			val = _tcstod(temps2, &end);

			if ((end != temps2))
			{
				ok = true;

				val /= dMultiplier;
				if (val < 0)
					val = 0;
				double rem = val - (int)val;
				unsigned long uval = (unsigned long)val;
				if (rem >= 0.5)
					uval += 1;

				if (uval > 0x7F)
					uval = 0x7F;

				c1 = 0;
				c2 = (unsigned char)uval;
				if (negative)
					c2 |= 0x80;
			}
		}
	}
	break;
	case 6:
	{
		val = 0xFF;
		ok = true;
	}
	break;
	case 7:
	{
		_TCHAR *end = NULL;
		char temps[10];
		str_left(temps, str, 8);
		val = _tcstod(temps, &end);

		if ((val < 5) || (val > 25.53))
		{
			ok = false;
		}
		else
		{
			c1 = 0;
			c2 = ConvertVoltageToChar(val);
			ok = true;
		}
	}
	break;
	case 8:
	{
		if (strcmp(str, "Never") == 0)
		{
			val = (double)0xFFFF * dMultiplier;
			ok = true;
		}
		else
		{
			TCHAR *end = NULL;
			char temps[10];
			str_left(temps, str, 8);
			val = _tcstod(temps, &end);
			if (end != temps)
				ok = true;
		}
	}
	break;
	case 9:
	{
		c1 = 0;
		c2 = bin2char(str, &ok);
	}
	break;
	case 1:
		//normal read - integer
	case 2:
	case 5:
	default:
	{ //normal read - float
		_TCHAR *end = NULL;
		char temps[10];
		str_left(temps, str, 8);
		val = _tcstod(temps, &end);

		//					CString err;
		//					err.Format("Convert: (%f) (%d) (%s)",val, errno, str.Left(8));
		//					AfxMessageBox(err);

		if (end != temps)
			ok = true;
	}
	}

	if (ok)
	{
		if ((nReadMode != 4) && (nReadMode != 7) && (nReadMode != 9))
		{
			val /= dMultiplier;

			if (nReadMode != 5)
			{ //unsigned
				if (val < 0)
					val = 0;
				double rem = val - (int)val;
				unsigned long uval = (unsigned long)val;
				if (rem >= 0.5)
					uval += 1;

				if (len == 1)
				{
					if (uval > 0xFF)
						uval = 0xFF;

					c1 = 0;
					c2 = (unsigned char)uval;
				}
				else
				{
					if (uval > 0xFFFF)
						uval = 0xFFFF;

					c1 = (unsigned char)((((unsigned int)uval) >> 8) & 0xFF);
					c2 = (unsigned char)(((unsigned int)uval) & 0xFF);
				}
			}
			else
			{
				double rem = -val + (int)val;
				long uval = (long)val;
				if (rem >= 0.5)
					uval += 1;

				if (len == 1)
				{
					if (uval > 127)
						uval = 127;
					if (uval < -128)
						uval = -128;

					c1 = 0;
					c2 = (char)uval;
				}
				else
				{
					if (uval > 32767)
						uval = 32767;
					if (uval < -32768)
						uval = -32768;

					c1 = (char)((((int)uval) >> 8) & 0xFF);
					c2 = (char)(((int)uval) & 0xFF);
				}
			}
		}
	}

	//fprintf(stderr, "readOneValue: readMode: %d c1: 0x%02x c2: 0x%02x\n", nReadMode, c1, c2);

	return ok;
}

void HIDDCDCUSB::convertOneValue2String(char *destination, int nLen, int nIndex, int nReadMode, double dMultiplier)
{
	destination[0] = 0; //empty string
	double value = 0;

	if (nLen == 1)
	{
		value = (double)m_chPackages[nIndex] * dMultiplier;
	}
	else
	{
		value = (double)((((int)m_chPackages[nIndex]) << 8) | m_chPackages[nIndex + 1]) * dMultiplier;
	}

	switch (nReadMode)
	{
	case 2:
		sprintf(destination, "%.2f", (float)value);
		break;
	case 3:
	{
		unsigned int ivalue = (unsigned int)value;
		if (ivalue == 0xFFFF)
			strcpy(destination, "Never");
		else
			sprintf(destination, "%02d:%02d:%02d", ivalue / 3600, (ivalue / 60) % 60, ivalue % 60);
	}
	break;
	case 4:
	{
		double dvalue = (m_chPackages[nIndex] & 0x7f) * dMultiplier;

		if ((m_chPackages[nIndex] & 0x80) != 0)
			sprintf(destination, "-%.3f", (float)dvalue);
		else
			sprintf(destination, "%.3f", (float)value);
	}
	break;
	case 5:
	{
		if (nLen == 1)
			value = (double)((char)m_chPackages[nIndex]) * dMultiplier;
		else
			value = (double)((((int)((char)m_chPackages[nIndex])) << 8) | m_chPackages[nIndex + 1]) * dMultiplier;

		sprintf(destination, "%d", (int)value);
	}
	break;
	case 6:
	{
		strcpy(destination, "-");
	}
	break;
	case 7:
	{ //no multiplier - voltage value calcualted with formula
		sprintf(destination, "%.3f", HIDDCDCUSB::GetVOut(m_chPackages[nIndex]));
	}
	break;
	case 8:
	{
		if (nLen == 1)
		{
			if (m_chPackages[nIndex] == 0xFF)
				strcpy(destination, "Never");
			else
				sprintf(destination, "%d", (int)value);
		}
		else
		{
			if ((m_chPackages[nIndex] == 0xFF) && (m_chPackages[nIndex + 1] == 0xFF))
				strcpy(destination, "Never");
			else
				sprintf(destination, "%d", (int)value);
		}
	}
	break;
	case 9:
		char2bin(destination, m_chPackages[nIndex]);
		break;
	case 1:
	default:
	{
		sprintf(destination, "%d", (int)value);
	}
	}

	//DBGOutput("convertOneValue2String %d %s %f [%x][%x]\n",nIndex,destination,(float)value,(int)m_chPackages[nIndex],(int)m_chPackages[nIndex+1]);
}

bool HIDDCDCUSB::setVariableData(int mesg_no, char *str)
{
	int len = GetMessages()[mesg_no].nLen;

	if (len != 0)
	{
		unsigned char c1 = 0;
		unsigned char c2 = 0;
		unsigned char c3 = 0;
		unsigned char c4 = 0;

		if (readOneValue(str, GetMessages()[mesg_no].nReadMode, GetMessages()[mesg_no].dMultiplier, len, c1, c2, c3, c4))
		{
			switch (len)
			{
			case 1:
				m_chPackages[GetMessages()[mesg_no].nIndex] = c2;
				break;
			case 2:
				m_chPackages[GetMessages()[mesg_no].nIndex] = c1;
				m_chPackages[GetMessages()[mesg_no].nIndex + 1] = c2;
				break;
			case 4:
				m_chPackages[GetMessages()[mesg_no].nIndex] = c1;
				m_chPackages[GetMessages()[mesg_no].nIndex + 1] = c2;
				m_chPackages[GetMessages()[mesg_no].nIndex + 2] = c3;
				m_chPackages[GetMessages()[mesg_no].nIndex + 3] = c4;
				break;
			}
			return 1;
		}
		else
			return 0;
	}
	return 1;
}

unsigned char HIDDCDCUSB::getUPSVariableData(unsigned int cnt, char *name, char *value, char *unit, char *comment)
{
	if ((cnt < 0) || (cnt >= DCDCUSB_MAX_MESSAGE_CNT))
		return 0;

	if (HIDDCDCUSB::GetMessages()[cnt].nLen == 0)
		return 0;

	if (name)
		strcpy(name, HIDDCDCUSB::GetMessages()[cnt].strName);
	if (unit)
		strcpy(unit, HIDDCDCUSB::GetMessages()[cnt].strUnit);
	if (comment)
		strcpy(comment, HIDDCDCUSB::GetMessages()[cnt].strText);

	if (value)
	{
		convertOneValue2String(value, HIDDCDCUSB::GetMessages()[cnt].nLen,
							   HIDDCDCUSB::GetMessages()[cnt].nIndex,
							   HIDDCDCUSB::GetMessages()[cnt].nReadMode,
							   HIDDCDCUSB::GetMessages()[cnt].dMultiplier);
	}

	return HIDDCDCUSB::GetMessages()[cnt].nReadMode;
}

void HIDDCDCUSB::restartUPS()
{
}

void HIDDCDCUSB::restartUPSInBootloaderMode()
{
}

void HIDDCDCUSB::setVOutVolatile(char *param) {

	if (!param) 
		return;

	double voltage;
	char *end, temps[10];
	unsigned char val;
	
	voltage = strtod(param, &end);
	fprintf(stderr, "Setting VOut voltage to: %lf\n", voltage);
	val = ConvertVoltageToChar(voltage);

	sendMessage(DCDCUSB_CMD_OUT, 3, DCDCUSB_CMD_WRITE_VOUT, val, 0);    
}

void HIDDCDCUSB::setVOutVolatile(float vout)
{
	unsigned char val = ConvertVoltageToChar(vout);
	sendMessage(DCDCUSB_CMD_OUT, 3, DCDCUSB_CMD_WRITE_VOUT, val, 0); 
}

void HIDDCDCUSB::incDecVOutVolatile(unsigned char inc)
{
}

void HIDDCDCUSB::GetStatus()
{
	unsigned char recv[32];
	int ret;

	sendMessage(DCDCUSB_GET_ALL_VALUES, 0);
	recvMessage(recv);
	parseMessage(recv);

	sendMessage(DCDCUSB_CMD_OUT, 3, DCDCUSB_CMD_READ_VOUT, 0, 0);
	recvMessage(recv);
	parseMessage(recv);
}

void HIDDCDCUSB::ReadConfigurationMemory()
{
	unsigned char recv[32];

	m_ulSettingsAddr = DCDCUSB_SETTINGS_ADDR_START;
	memset(m_chPackages, 0, DCDCUSB_SETTINGS_PACKS * 16);	

	while (m_ulSettingsAddr < DCDCUSB_SETTINGS_ADDR_END)
	{
		int ret = sendMessage(DCDCUSB_MEM_READ_OUT, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
		recvMessage(recv);
		parseMessage(recv);
		m_ulSettingsAddr += 16;
	}
}

void HIDDCDCUSB::EraseConfigurationMemory()
{
	unsigned char recv[32];
	int ret;

	m_ulSettingsAddr = DCDCUSB_SETTINGS_ADDR_START;
	sendMessage(DCDCUSB_MEM_ERASE, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x40);

	int retries = 5;
	while ((ret = recvMessage(recv) <= 0) && retries > 0)
	{
		retries--;
		usleep(500);
		fprintf(stderr, "Erase 0x%02x retry %d/5\n", recv[0], retries);
	}

	if (retries <= 0)
	{
		fprintf(stderr, "Error waiting for erase operation to finish\n");
	}
	else
	{
		fprintf(stderr, "Successfully erased (0x%02x) configuration memory\n", recv[0]);
	}
}

void HIDDCDCUSB::WriteConfigurationMemory()
{
	unsigned char recv[32];
	int ret;

	m_ulSettingsAddr = DCDCUSB_SETTINGS_ADDR_START;
	while (m_ulSettingsAddr < DCDCUSB_SETTINGS_ADDR_END)
	{
		sendMessageWithBuffer(DCDCUSB_MEM_WRITE_OUT, 16, m_chPackages + (m_ulSettingsAddr - DCDCUSB_SETTINGS_ADDR_START), 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
		ret = recvMessage(recv);

		if (ret <= 0)
		{
			fprintf(stderr, "Error (%d, 0x%02x) writing configuration variables , aborting ...\n", ret, recv[0]);
			break;
		}
		else
		{
			//fprintf(stderr, "Wrote page 0x%lx of configuration\n", m_ulSettingsAddr);
		}
		m_ulSettingsAddr += 16;
	}
}

bool HIDDCDCUSB::executeCommand(char *cmdexpr) {
	fprintf(stdout, "Executing command and params: %s\n", cmdexpr);
	struct EXEC *exec = parseCommand(cmdexpr);
	bool executed = false;

	if ((strcmp(exec->cmd, "set_vout") == 0) && (exec->params_count > 0)) {
		setVOutVolatile(exec->params[0]);		
		executed = true;
	} else {
		fprintf(stderr, "Unknown command: %s\n", exec->cmd);
	}

	free(exec);
	return executed;
}
