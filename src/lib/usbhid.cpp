/*
 *    Author: Nicu Pavel <npavel@mini-box.com>
 *    Copyright (c) 2020 Mini-Box.com
 *    Licensed under the terms of the GNU Lesser General Public License v2.1
 *  	http://www.opensource.org/licenses/lgpl-2.1.php 
 */

#include "usbhid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#undef DEBUG_RECV

USBHID::USBHID(unsigned int vendorid, unsigned int productid, unsigned int max_transfer_size, unsigned int index)
{
    this->vendorid = vendorid;
    this->productid = productid;
    this->index = index;
    this->device = NULL;
    this->handle = NULL;
    this->connected = false;
    this->max_transfer_size = max_transfer_size;

    usb_init();
    usb_set_debug(0);
    usb_find_busses();
    usb_find_devices();
}

USBHID::~USBHID() {}

struct usb_dev_handle *USBHID::open(void)
{
    char buf[65535];

    this->device = this->find();

    if (this->device == NULL)
    {
        fprintf(stderr, "Device [0x%04x: 0x%04x] not detected !\n", this->vendorid, this->productid);
        return NULL;
    }
    this->handle = usb_open(this->device);
    int ret = usb_get_driver_np(this->handle, 0, buf, sizeof(buf));

    if (ret == 0)
    {
        fprintf(stderr, "interface 0 already claimed by driver \"%s\", attempting to detach it\n", buf);
        ret = usb_detach_kernel_driver_np(this->handle, 0);
        fprintf(stderr, "usb_detach_kernel_driver_np returned %d\n", ret);
    }
    usleep(1000);

    ret = usb_claim_interface(this->handle, 0);
    if (ret != 0)
    {
        printf("claim failed with error %d\n", ret);
        return NULL;
    }

    ret = usb_set_altinterface(this->handle, 0);

    fprintf(stdout, "Product: %s, Manufacturer: %s, Firmware Version: %s\n", this->getProduct(), this->getManufacturer(), this->getSerial());

    connected = true;
    return this->handle;
}

struct usb_device *USBHID::find(void)
{
    struct usb_bus *bus;
    unsigned int i = 0;

    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        struct usb_device *dev;
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == this->vendorid && dev->descriptor.idProduct == this->productid)
            {
                if (i == this->index)
                {
                    return dev;
                } else {
                    i++;
                }
            }
        }
    }
    return NULL;
}

char *USBHID::getProduct()
{
    int len;
    int BUFLEN = 256;
    char *buffer;

    buffer = (char *)malloc(BUFLEN);

    if (buffer == NULL)
        return NULL;

    if (this->device->descriptor.iProduct)
    {
        len = usb_get_string_simple(this->handle, this->device->descriptor.iProduct, buffer, BUFLEN);

        if (len > 0)
            return buffer;
    }

    free(buffer);
    return NULL;
}

char *USBHID::getManufacturer()
{
    int len;
    int BUFLEN = 256;
    char *buffer;

    buffer = (char *)malloc(BUFLEN);

    if (buffer == NULL)
        return NULL;

    if (this->device->descriptor.iManufacturer)
    {
        len = usb_get_string_simple(this->handle, this->device->descriptor.iManufacturer, buffer, BUFLEN);

        if (len > 0)
            return buffer;
    }

    free(buffer);
    return NULL;
}

char *USBHID::getSerial()
{
    int len;
    int BUFLEN = 256;
    char *buffer;

    buffer = (char *)malloc(BUFLEN);

    if (buffer == NULL)
        return NULL;

    if (this->device->descriptor.iSerialNumber)
    {
        len = usb_get_string_simple(this->handle, this->device->descriptor.iSerialNumber, buffer, BUFLEN);

        if (len > 0)
            return buffer;
    }

    free(buffer);
    return NULL;
}

int USBHID::readInterrupt(unsigned char *buff, int size)
{
    if (buff == NULL) {
        fprintf(stderr, "Invalid read buffer, skipping read\n");
        return -1;
    }

    int ret = usb_interrupt_read((usb_dev_handle *)this->handle, USB_ENDPOINT_IN + 1,
                                 reinterpret_cast<char *>(buff), size, 
                                 USBHID_RECV_TIMEOUT);
    
#ifdef DEBUG_RECV
    for (int j = 0; j < size; j++) {
		fprintf(stderr, " 0x%02x ", buff[j]);
	}
	fprintf(stderr, "\n");
#endif
    if (ret < 0)
        fprintf(stderr, "hid_interrupt_read failed with return code %d\n", ret);

    return ret;
}

int USBHID::readInterrupt(unsigned char *buff) {
    return readInterrupt(buff, max_transfer_size);
}

int USBHID::writeInterrupt(unsigned char *buff, int size, bool use_transfer_size)
{
    if (buff == NULL) {
        fprintf(stderr, "Invalid write buffer, skipping write\n");
        return -1;
    }

    unsigned char *packet;
    if (use_transfer_size) {
        if ((packet = (unsigned char*) malloc(max_transfer_size)) == NULL) {
        return -2;
        }

        if (size > max_transfer_size) {
            fprintf(stderr, "Warning: Multi-packet write not implemented !");
            size = max_transfer_size;
        } 
        memset(packet, 0x00, max_transfer_size);
        memcpy(packet, buff, size);
        size = max_transfer_size;
    } else {
        packet = buff;
    }
    

    int ret = usb_interrupt_write((usb_dev_handle *)this->handle, USB_ENDPOINT_OUT + 1,
                              reinterpret_cast<char *>(packet), size, 
                              USBHID_SEND_TIMEOUT);

    if (ret < 0)
        fprintf(stderr, "hid_interrupt_write failed with return code %d\n", ret);
    
    if (use_transfer_size) {
        free(packet);
    }

    return ret;
}

bool USBHID::isOpened() {
    return connected;
}

int USBHID::release()
{
    int ret = -1;
    ret = usb_release_interface(this->handle, 0);
    if (ret < 0)
        printf("failed to release interface: %d\n", ret);

    return ret;
}

int USBHID::close()
{

    release();
    int ret = usb_close(this->handle);

    if (ret < 0)
    {
        fprintf(stderr, "hid_close failed with return code %d\n", ret);
        return 1;
    }

    connected = false;
    return 0;
}
