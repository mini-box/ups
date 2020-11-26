#define MINIBOX_VENDOR_ID	0x04d8

#define DCDCUSB_PRODUCT_ID 	0xd003
#define OPENUPS_PRODUCT_ID	0xd004
#define OPENUPS2_PRODUCT_ID	0xd005
#define NUCUPS_PRODUCT_ID	0xd007

struct deviceId {
	const char *name;
	const char *desc;
	const unsigned int vendorid;
	const unsigned int productid;
	const unsigned int max_transfer_size;
	const struct deviceCmd *cmds;
};

struct deviceCmd {
	const char *cmd;
	const char *desc;
};

static const struct deviceCmd dcdc_usb_cmds[] = {
	{ "set_vout", "Set Output Voltage" },
	{}
};

static const struct deviceId deviceIds[] = {
	{
		"dcdcusb",
		"DCDC USB",
		MINIBOX_VENDOR_ID,
		DCDCUSB_PRODUCT_ID,
		24,
		dcdc_usb_cmds
	},
	{
		"openups",
		"Open UPS",
		MINIBOX_VENDOR_ID,
		OPENUPS_PRODUCT_ID,
		32,
		{}
	},
	{
		"openups2",
		"Open UPS2",
		MINIBOX_VENDOR_ID,
		OPENUPS2_PRODUCT_ID,
		32,
		{}
	},
	{
		"nucups",
		"DC-DC NUC UPS",
		MINIBOX_VENDOR_ID,
		NUCUPS_PRODUCT_ID,
		32,
		{}
	},
	{}
};
