#include "util.h"
#include "usbhid.h"
#include "HArray.h"
#include "HIDOpenUPS2.h"
#include "HIDInterface.h"

#include <stdarg.h>
#include <math.h>
#include <strings.h>
#include <ctype.h>


struct deviceId {
	const char *name;
	unsigned int vendorid;
	unsigned int productid;
};

static const struct deviceId deviceIds[] = {
	{
		"openups2",
		0x04d8,
		0xd005
	},
	{}
};

int main(void)
{

	int ret;
	USBHID *d = new USBHID(0x04d8, 0xd005);

	d->open();
	if (!d->isOpened())
	{
		return 0;
	}

	HIDOpenUPS2 *ups = new HIDOpenUPS2(d);
	ups->GetStatus();


	ups->ReadConfigurationMemory();
	fprintf(stderr, "Read configuration\n");

	ups->varsToFile("settings.cfg.before", false);
	fprintf(stderr, "Saved to file\n");

	ups->EraseConfigurationMemory();
	fprintf(stderr, "Erase configuration\n");

	ups->fileToVars("settings.cfg.changed");
	fprintf(stderr, "Loaded file\n");
	ups->WriteConfigurationMemory();
	fprintf(stderr, "Wrote configuration\n");
	
	ups->varsToFile("settings.cfg.after", false);
	ups->restartUPS();

	d->close();
	return 1;
}




	/* Read device configuration variables */
/*	
	m_ulSettingsAddr = SETTINGS_ADDR_START;
	memset(m_chPackages, 0, SETTINGS_PACKS * 16);

	while (m_ulSettingsAddr < SETTINGS_ADDR_END)
	{
		sendMessage(d, UPS12V_MEM_READ_OUT, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
		recvMessage(d, recv);
		parseMessage(recv);
		m_ulSettingsAddr += 16;
	}
*/
	/* Erase device configuration variables */
/*	
	m_ulSettingsAddr = SETTINGS_ADDR_START;
	sendMessage(d, UPS12V_MEM_ERASE, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x40);

	int retries = 5;
	while ((ret = recvMessage(d, recv) <= 0) && retries > 0) {
		retries--;
		usleep(500);
		fprintf(stderr, "Erase 0x%02x retry %d/5\n", recv[0], retries);
	}

	if (retries <= 0) {
		fprintf(stderr, "Error waiting for erase operation to finish\n");
	} else {
		fprintf(stderr, "Successfully erased (0x%02x) configuration memory\n", recv[0]);
	}
*/
	/* Dump configuration variables */
/*
	int var_max = 256;
	char name[256];
	char unit[64];
	char value[256];
	char comment[1024];

	for (int i = 0; i < var_max; i++)
	{
		if (getUPSVariableData(i, name, value, unit, comment))
		{
			fprintf(stderr, "%d. %s: %s %s %s\n", i, name, value, unit, comment);
			fprintf(stderr, "%s\n", "---------------------------------------------------------------------------------------------------------------");
		}
	}


	varsToFile("settings.cfg.before", false);
	fileToVars("settings.cfg.changed");	
*/	
	/* Write device configuration variables */
/*	
	m_ulSettingsAddr = SETTINGS_ADDR_START;
	while (m_ulSettingsAddr < SETTINGS_ADDR_END)
	{
		sendMessageWithBuffer(d, UPS12V_MEM_WRITE_OUT, 16, m_chPackages+(m_ulSettingsAddr-SETTINGS_ADDR_START), 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
		ret = recvMessage(d, recv);
		
		if (ret <= 0 || recv[0] != UPS12V_MEM_WRITE_IN) {
			fprintf(stderr, "Error (%d, 0x%02x) writing configuration variables , aborting ...\n", ret, recv[0]);
			break;
		} else {
			fprintf(stderr, "Wrote page 0x%lx of configuration\n", m_ulSettingsAddr);
		}
		
		m_ulSettingsAddr += 16;
	}

	varsToFile("settings.cfg.after", false);
*/	


/*	
	restartUPS(d);
	recvMessage(d, recv);
	fprintf(stderr, "Restart UPS returned: 0x%02x\n", recv[0]);
	*/