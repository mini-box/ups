#ifndef _USBHID_H_
#define _USBHID_H_

#include <usb.h>

#define USBHID_RECV_TIMEOUT 1000
#define USBHID_SEND_TIMEOUT 1000

class USBHID {
    public:
        USBHID(unsigned int vendorid, unsigned int productid, unsigned int max_transfer_size, unsigned int index);
        ~USBHID();

        struct usb_dev_handle* open(void);
        char *getProduct();
        char *getManufacturer();
        char *getSerial();
        bool isOpened();
        int readInterrupt(unsigned char *buff);
        int readInterrupt(unsigned char *buff, int size);
        int writeInterrupt(unsigned char *buff, int size, bool use_transfer_size);
        int close();

    private:
        unsigned int id;
        unsigned int vendorid;
        unsigned int productid;
        unsigned int max_transfer_size;
        unsigned int index;
        bool connected;
        struct usb_device *device;
        struct usb_dev_handle *handle;

        struct usb_device *find(void); 
        int release();
};

#endif //_USBHID_H_
