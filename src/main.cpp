/*
 *    Author: Nicu Pavel <npavel@mini-box.com>
 *    Copyright (c) 2020 Mini-Box.com
 *    Licensed under the terms of the GNU Lesser General Public License v2.1
 *  	http://www.opensource.org/licenses/lgpl-2.1.php 
 */


#include "devices.h"
#include "version.h"
#include "util.h"
#include "usbhid.h"
#include "HArray.h"
#include "HIDDCDCUSB.h"
#include "HIDOpenUPS.h"
#include "HIDOpenUPS2.h"
#include "HIDNUCUPS.h"
#include "HIDInterface.h"

#include <stdarg.h>
#include <math.h>
#include <strings.h>
#include <ctype.h>
#include <libgen.h>


static const struct deviceId* findDeviceByName(const char *name) 
{
	for(int i = 0; deviceIds[i].name != NULL; i++) {
		if (strcasecmp(name, deviceIds[i].name) == 0) {
			return &deviceIds[i];
		}
	}

	return NULL;
}

static int printDeviceCommands(const struct deviceCmd *cmds) 
{
	int i;

	if (!cmds) {
		return 0;
	}
	fprintf(stderr, "\n");
	for (i = 0; cmds[i].cmd != NULL; i++) {
		fprintf(stderr, "\t%s - %s\n", cmds[i].cmd, cmds[i].desc);
	}

	return i;	
}

int usage(char *progname)
{
	fprintf(stderr,
                "Usage: %s [<options>]\n"
                "Options:\n"
                "  -t <device type>:    Select device model (see below)\n"
				"  -i <input file>:     Write settings from this file. Warning: Will reboot UPS!\n"
				"  -o <output file>:    Dump settings to this file\n"
				"  -c:                  Add comments for each configuration variable to output file\n"
				"  -s:                  Only output status don't read and show configuration variables\n"
				"  -e <cmd:param1:...>: Execute command.\n"
				"  -r			Restart UPS only and exit \n"
				"  -b			Restart UPS to bootloader and exit \n"
                "\n",
                progname);

	fprintf(stderr, "Known device models:\n");
	for(int i = 0; deviceIds[i].name != NULL; i++) {
		fprintf(stderr, "  %s \t- %s (vid: 0x%04x pid: 0x%04x)\n", deviceIds[i].name, deviceIds[i].desc, deviceIds[i].vendorid, deviceIds[i].productid);
		fprintf(stderr, "    Available Commands: ");
		if (printDeviceCommands(deviceIds[i].cmds) == 0) {
			fprintf(stderr, " None\n");
		};
	}
        return 3;
}

int main(int argc, char **argv)
{

	int ret, c;
	char *progname = basename(argv[0]);
	char *devicetype = NULL;
	char *infile = NULL;
	char *outfile = NULL;
	bool withComments = false;
	bool withConfiguration = true;
	bool doReset = false;
	bool goBootloader = false;
	char *execCmd = NULL;

	if (argc < 2)
		return usage(progname);

	while ((c = getopt(argc, argv, "t:i:o:e:cshrb")) != -1) {
		switch (c) {
			
			case 't':
				devicetype = optarg;
				break;
			case 'i':
				infile = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'c':
				withComments = true;
				break;
			case 's':
				withConfiguration = false;
				break;
			case 'e':
				execCmd = optarg;
				break;
			case 'h':
				usage(progname);
				break;
			case 'r':
				doReset = true;
				break;
			case 'b':
				goBootloader = true;
				break;
			default:
				usage(progname);
		}
	}

	fprintf(stdout, "%s version %s. Device selected: %s\n", progname, VERSION, devicetype);
	if (!devicetype) {
		fprintf(stderr, "No device type selected !\n");
		return usage(progname);
	}

	const struct deviceId *devid = findDeviceByName(devicetype);

	if (devid == NULL) {
		fprintf(stderr, "Device '%s' unknown\n", devicetype);
		return 1;
	} else {
		fprintf(stderr, "Selected device %s: %s\n", devid->name, devid->desc);
	}

	USBHID *d = new USBHID(devid->vendorid, devid->productid, devid->max_transfer_size);

	d->open();
	if (!d->isOpened())
	{
		fprintf(stderr, "Failed to open USB device\n");
		return 2;
	}

	HIDInterface *ups;

	if (devid->productid == DCDCUSB_PRODUCT_ID) {
		ups = new HIDDCDCUSB(d);
	} else if (devid->productid == OPENUPS_PRODUCT_ID) {
		ups = new HIDOpenUPS(d);
	} else if (devid->productid == OPENUPS2_PRODUCT_ID) {
		ups = new HIDOpenUPS2(d);
	} else if (devid->productid == NUCUPS_PRODUCT_ID) {
		ups = new  HIDNUCUPS(d);
	} else {
		fprintf(stderr, "Unknown product id !");
		d->close();
		return 3;
	}

	/* After opening wait 100ms before querying the device */
	usleep(1000*100);
	
        if (doReset) {
                ups->restartUPS();
                fprintf(stdout, "Restarted UPS\n");
                return 0;
        }

        if (goBootloader) {
                ups->restartUPSInBootloaderMode();
                fprintf(stdout, "Restarted UPS in Bootloader Mode\n");
                return 0;
        }

	
	ups->GetStatus();
	ups->printValues();	

	if (execCmd) {
		if (ups->executeCommand(execCmd)) {
			fprintf(stdout, "Command executed.\n");
			ups->GetStatus();
			ups->printValues();
		}
	}
	
	if (withConfiguration) {
		fprintf(stdout, "Read configuration\n");
		ups->ReadConfigurationMemory();		
		ups->printConfiguration(withComments);
		if (outfile) {
			ups->varsToFile(outfile, withComments);
			fprintf(stdout, "Saved configuration to file: %s\n", outfile);
		}
	}
			
	if (infile) {
		ups->EraseConfigurationMemory();
		fprintf(stdout, "Erased configuration\n");

		ups->fileToVars(infile);
		fprintf(stdout, "Loaded configuration file: %s\n", infile);
	
		ups->WriteConfigurationMemory();
		fprintf(stdout, "Wrote configuration\n");
	
		//ups->varsToFile("settings.cfg.after", false);	
		ups->restartUPS();
		fprintf(stdout, "Restarted UPS\n");
	}
	
	d->close();
	return 0;
}
