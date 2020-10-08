#include "version.h"
#include "util.h"
#include "usbhid.h"
#include "HArray.h"
#include "HIDOpenUPS.h"
#include "HIDOpenUPS2.h"
#include "HIDNUCUPS.h"
#include "HIDInterface.h"

#include <stdarg.h>
#include <math.h>
#include <strings.h>
#include <ctype.h>

#define MINIBOX_VENDOR_ID	0x04d8

#define OPENUPS_PRODUCT_ID	0xd004
#define OPENUPS2_PRODUCT_ID	0xd005
#define NUCUPS_PRODUCT_ID	0xd007

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
		MINIBOX_VENDOR_ID,
		OPENUPS_PRODUCT_ID
	},
	{
		"openups2",
		"Open UPS2",
		MINIBOX_VENDOR_ID,
		OPENUPS2_PRODUCT_ID
	},
	{
		"nucups",
		"DC-DC NUC UPS",
		MINIBOX_VENDOR_ID,
		NUCUPS_PRODUCT_ID
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
                "  -t <device type>: Select openups, openups2 or nucups as device type\n"
				"  -i <input file>:  Write settings from this file. Warning: Will reboot UPS!\n"
				"  -o <output file>: Dump settings to this file\n"
				"  -c:               Add comments for each configuration variable to output file\n"
				"  -s:               Only output status don't show configuration variables\n"
                "\n",
                progname);
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

	if (argc < 2)
		return usage(progname);

	while ((c = getopt(argc, argv, "t:i:o:csh")) != -1) {
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
		fprintf(stderr, "Device '%s' unknown\n", devicetype);
		return 1;
	} else {
		fprintf(stderr, "Selected device %s: %s\n", devid->name, devid->desc);
	}

	USBHID *d = new USBHID(devid->vendorid, devid->productid);

	d->open();
	if (!d->isOpened())
	{
		fprintf(stderr, "Failed to open USB device\n");
		return 2;
	}

	HIDInterface *ups;

	if (devid->productid == OPENUPS_PRODUCT_ID) {
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

	/* After opening wait 1 second before querying the device */
	usleep(1000);
	
	ups->GetStatus();
	
	ups->ReadConfigurationMemory();
	fprintf(stdout, "Read configuration\n");
	if (withConfiguration)
		ups->printConfiguration(withComments);
	
	if (outfile) {
		ups->varsToFile(outfile, withComments);
		fprintf(stdout, "Saved configuration to file: %s\n", outfile);
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
