/*
 *    Author: Nicu Pavel <npavel@mini-box.com>
 *    Copyright (c) 2020 Mini-Box.com
 *    Licensed under the terms of the GNU Lesser General Public License v2.1
 *  	http://www.opensource.org/licenses/lgpl-2.1.php 
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <ctype.h>

#include "HIDInterface.h"
#include "HArray.h"

#define OPENUPS2_CMD_OUT					0xB1


HIDInterface::HIDInterface(USBHID *hidDevice) {
	d = hidDevice;
	memset(m_chPackages, 0, SETTINGS_PACKS * 16);
}

HIDInterface::~HIDInterface() {
	
}

void HIDInterface::printConfiguration(bool withComments)
{
	int var_max = 256;
	char name[256];
	char unit[64];
	char value[256];
	char comment[1024];

	for (int i = 0; i < var_max; i++)
	{
		if (getUPSVariableData(i, name, value, unit, comment))
		{
			if (withComments) {
				fprintf(stdout, "%d. %s: %s # %s %s\n", i, name, value, unit, comment);
				fprintf(stdout, "%s\n", "---------------------------------------------------------------------------------------------------------------");
			} else {
				fprintf(stdout, "%s=%s\n", name, value);
			}			
		}
	}
}

int HIDInterface::sendMessageWithBuffer(unsigned char cType, unsigned int buflen, unsigned char* buffer, unsigned int len, ...)
{
	HArray mesg(buflen + len + 1);
	mesg.getBuf()[0] = cType;

	va_list args;
	va_start(args, len);
	unsigned int cnt = 0;
	unsigned char i = va_arg(args, unsigned int);

	while (cnt < len)
	{
		mesg.getBuf()[1+cnt] = i;
		cnt++;
		if (cnt < len)
			i = va_arg(args, unsigned int);
	}
	
	va_end(args);
	
	memcpy(mesg.getBuf() + 1 + len, buffer, buflen);

	int ret = d->writeInterrupt(mesg.getBuf(), mesg.length(), true);
	//fprintf(stderr, "[0x%02x] written %d/%d\n", cType, ret, len + 1);
	return ret;
}

int HIDInterface::sendMessage(unsigned char cType, unsigned int len, ...) 
{
	HArray mesg(len + 1);
	//HArray mesg(24);
	mesg.getBuf()[0] = cType;

	va_list args;
	va_start(args, len);
	unsigned int cnt = 0;

	unsigned char i = va_arg(args, unsigned int);
	while (cnt < len)
	{
		mesg.getBuf()[1 + cnt] = i;
		cnt++;
		if (cnt < len)
			i = va_arg(args, unsigned int);
	}
	va_end(args);

	int ret = d->writeInterrupt(mesg.getBuf(), mesg.length(), true);
	//fprintf(stderr, "[0x%02x] written %d/%d\n", cType, ret, len + 1);
	return ret;
}

int HIDInterface::sendCommand(unsigned char command, unsigned char value)
{
	return sendMessage(COMMON_CMD_OUT, 3, command, value, 0); // OpenUPS, OpenUPS2 and NUCUPS share same CMD_OUT value
}
int HIDInterface::sendCommandEx(unsigned char command, unsigned char value1, unsigned char value2)
{
	return sendMessage(COMMON_CMD_OUT, 3, command, value1, value2);
}

int HIDInterface::recvMessage(unsigned char *buffer)
{
	int ret = d->readInterrupt(buffer);	
		    	
	if (ret <= 0 ) {
		fprintf(stderr, "Read error [0x%02x]: %d\n", buffer[0], ret);
	}
	return ret;
}

int HIDInterface::recvMessage(unsigned char *buffer, unsigned int len)
{
	int ret = d->readInterrupt(buffer, len);	
		    	
	if (ret <= 0 ) {
		fprintf(stderr, "Read error [0x%02x]: %d/%d\n", buffer[0], ret, len);
	}
	return ret;
}

int HIDInterface::GetMessageIdxByName(const char* name)
{
	if (!name)
		return -1;

	for (int i=0;i < MAX_MESSAGE_CNT;i++)
	{
		ATXMSG msg = GetMessages()[i];
		// fprintf(stderr, "GetMessage %d: %s\n", i, msg.strName);
		if (msg.nLen!=0)
			if (strcasecmp(msg.strName, name) == 0) 
				return i;
	}
	
	return -1;
}

int HIDInterface::varsToFile(const char *filename, bool withComments) {
	if (!filename) {
		return -1;
	}

	FILE *f = fopen(filename, "w");
	
	if (!f) {
		return -2;
	}

	int var_max = 256;
	char name[256];
	char unit[64];
	char value[256];
	char comment[1024];

	for (int i = 0; i < var_max; i++)
	{
		if (getUPSVariableData(i, name, value, unit, comment))
		{
			if (withComments) {
				// Replace \r \n with comment #, assumes windows style line break		
				char *p = comment;
				while (p && *p != '\0') {				
					if (*p == '\r') {
						if (*(p + 1) == '\n') {
							*p = '\n';
							*(p + 1) = '#';
						} 					
					}				
					p++;
				}			
				fprintf(f, "# %s %s %s\n\n", name, unit, comment);
			}
			fprintf(f, "%s=%s\n", name, value);
		}
	}
	fclose(f);
	return 0;
}

int HIDInterface::fileToVars(const char *filename) {
	if (!filename) {
		return -1;
	}

	FILE *f = fopen(filename, "r");

	if (!f) {
		fprintf(stderr, "Failed to open %s for writting\n", filename);
		return -2;
	}

	int ret;
	char name[256];
	char val[256];
	char buf[1024];

	#define MAX_LINE 1024
	char line[MAX_LINE];
	int len;
	int lineno = 0, last = 0, vars = 0;

	while (fgets(line, MAX_LINE, f) != NULL) {
		len = strlen(line) - 1;
		if (line[len] != '\n' && !feof(f)) {
			fprintf(stderr, "Line %d too long\n", lineno);
			fclose(f);
			return -3;
		}
		lineno++;
		
		/* Remove \n and spaces from the end */
		while ((len >= 0) && ((line[len]=='\n') || (isspace(line[len])))) {
            line[len] = 0 ;
            len--;
        }

        if (len < 0) {
            len = 0;
			continue;
        }
		if (sscanf(line, "%32[^=]=%32[^\n]%*c", name, val) == 2){
			fprintf(stdout, "Found var: %s value: %s\n", name, val);
			int idx;
			idx = GetMessageIdxByName(name);
			if (idx > 0) {
				if (setVariableData(idx, val)) {
					fprintf(stdout, "Succesfully set %s to %s\n", name, val);
				} else {
					fprintf(stderr, "Error setting %s to %s\n", name, val);
				}				
			}
			vars++;
		} 
	}
	fprintf(stdout, "Found %d vars\n", vars);
	fclose(f);

	return 0;
}
