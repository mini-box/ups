#ifndef _USBHID_H_
#define _USBHID_H_


#include <usb.h>

class USBHID {
    public:
        USBHID(unsigned int vendorid, unsigned int productid);
        ~USBHID();

        struct usb_dev_handle* open(void);
        char *getProduct();
        char *getManufacturer();
        char *getSerial();
        bool isOpened();
        int readInterrupt(unsigned char *buff, int size);
        int writeInterrupt(unsigned char *buff, int size);
        int close();

    private:
        unsigned int id;
        unsigned int vendorid;
        unsigned int productid;
        bool connected;
        struct usb_device *device;
        struct usb_dev_handle *handle;

        struct usb_device *find(void); 
        int release();
};

#endif //_USBHID_H_