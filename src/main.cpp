#include "util.h"
#include "usbhid.h"
#include "HArray.h"
#include "HIDOpenUPS.h"
#include "HIDOpenUPS2.h"
#include "HIDInterface.h"

#include <stdarg.h>
#include <math.h>
#include <strings.h>
#include <ctype.h>


struct deviceId {
	const char *name;
	const char *desc;
	unsigned int vendorid;
	unsigned int productid;
};

static const struct deviceId deviceIds[] = {
	{
		"openups",
		"Open UPS",
		0x04d8,
		0xd004
	},
	{
		"openups2",
		"Open UPS2",
		0x04d8,
		0xd005
	},
	{
		"nucups",
		"DC-DC NUC UPS",
		0x04d8,
		0xd007
	},
	{}
};

static const struct deviceId* findDeviceByName(const char *name) 
{
	for(int i = 0; deviceIds[i].name != NULL; i++) {
		if (strcasecmp(name, deviceIds[i].name) == 0) {
			return &deviceIds[i];
		}
	}

	return NULL;
}

int usage(char *progname)
{
	fprintf(stderr,
                "Usage: %s [<options>]\n"
                "Options:\n"
                "  -t <device type>:     Select openups, openups2 or nucups as device type\n"                
                "\n",
                progname);
        return 3;
}

int main(int argc, char **argv)
{

	int ret, c;
	char *progname = basename(argv[0]);
	char *devicetype = NULL;

	if (argc < 2)
		return usage(progname);

	while ((c = getopt(argc, argv, "t:h")) != -1) {		
		switch (c) {
			
			case 't':
				devicetype = optarg;
				break;			
			case 'h':
				usage(progname);
				break;
			default:
				usage(progname);
		}
	}

	fprintf(stderr, "Device selected: %s\n", devicetype);
	if (!devicetype) {
		fprintf(stderr, "No device type selected !\n");
		return usage(progname);
	}

	const struct deviceId *devid = findDeviceByName(devicetype);

	if (devid == NULL) {
		fprintf(stderr, "Device '%s' not found\n", devicetype);
		return 1;
	} else {
		fprintf(stderr, "Found device %s: %s\n", devid->name, devid->desc);
	}

	USBHID *d = new USBHID(devid->vendorid, devid->productid);

	d->open();
	if (!d->isOpened())
	{
		return 2;
	}

	//HIDOpenUPS2 *ups = new HIDOpenUPS2(d);
	HIDOpenUPS *ups = new HIDOpenUPS(d);
	/* After opening wait 1 second before querying the device */
	usleep(1000);
	ups->GetStatus();


	ups->ReadConfigurationMemory();
	fprintf(stderr, "Read configuration\n");
	ups->printConfiguration();
	

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
	return 0;
}




	/* Read device configuration variables */
/*	
	m_ulSettingsAddr = SETTINGS_ADDR_START;
	memset(m_chPackages, 0, SETTINGS_PACKS * 16);

	while (m_ulSettingsAddr < SETTINGS_ADDR_END)
	{
		sendMessage(d, OPENUPS2_MEM_READ_OUT, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
		recvMessage(d, recv);
		parseMessage(recv);
		m_ulSettingsAddr += 16;
	}
*/
	/* Erase device configuration variables */
/*	
	m_ulSettingsAddr = SETTINGS_ADDR_START;
	sendMessage(d, OPENUPS2_MEM_ERASE, 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x40);

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
		sendMessageWithBuffer(d, OPENUPS2_MEM_WRITE_OUT, 16, m_chPackages+(m_ulSettingsAddr-SETTINGS_ADDR_START), 4, m_ulSettingsAddr & 0xFF, (m_ulSettingsAddr >> 8) & 0xFF, 0x00, 0x10);
		ret = recvMessage(d, recv);
		
		if (ret <= 0 || recv[0] != OPENUPS2_MEM_WRITE_IN) {
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