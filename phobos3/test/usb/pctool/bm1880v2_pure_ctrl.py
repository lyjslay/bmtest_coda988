#!/usr/local/bin/python
from usb.core import USBError
from bm_usb_util.bm_usb_pyserial import bm_usb_pyserial
from bm_usb_util.bm_usb_libusb import bm_usb_libusb
import os
import sys
import time
import filecmp
import bm_usb_util.bm_usb_pkt as pkt
from array import array
import random

CTRL_TEST_64P0 = 255
CTRL_TEST_ZERO = 239
CTRL_TEST_STALL_STATUS = 138
CTRL_TEST_STALL_IN = 137
CTRL_TEST_STALL_OUT = 136

test_cnt = 16000
patterns = ['ctrl0B', 'ctrl1B', 'ctrl64B', 'ctrl64_0B', 'ctrl67B', 'ctrlInStall', 'ctrlOutStall', 'ctrlStsStall']
#patterns = ['ctrlStsStall']
check_files = ['Rctrl00']

bmtest_vidpid = 'VID:PID=0559:4000'
CH9_USB_REQ_TYPE_VENDOR = 0x40
CH9_USB_REQ_RECIPIENT_INTERFACE = 1
CH9_USB_EP_DIR_IN = 0x80
bmRequestTypeIn = CH9_USB_REQ_TYPE_VENDOR | CH9_USB_REQ_RECIPIENT_INTERFACE | CH9_USB_EP_DIR_IN
bmRequestTypeOut = CH9_USB_REQ_TYPE_VENDOR | CH9_USB_REQ_RECIPIENT_INTERFACE


if __name__ == '__main__':
    print ("bm1880_emmc_download v1.1")

    bm_usb_libusb = bm_usb_libusb()
    if len(sys.argv) > 1:
        bm_usb_libusb.parse_arg()
        
    device = bm_usb_libusb.libusb_query([bmtest_vidpid], 10)
    print ("Done")

    for i in range(0, test_cnt):        
        #pat = patterns[i%len(patterns)]
        pat = patterns[random.randint(0, len(patterns)-1)]
        content_size = os.path.getsize(pat)

        if content_size == 0:
            cmd = device.ctrl_transfer(bmRequestTypeIn, 0, 0, 1, CTRL_TEST_ZERO, 5000)
            continue
        elif content_size == CTRL_TEST_STALL_STATUS:
            try:
                cmd = device.ctrl_transfer(bmRequestTypeOut, 0, 0, 2, 0, 5000)
            except USBError as e:
                if "[Errno 32]" in str(e):
                    print ("ctrl status stall test pass!")
                else:
                    print ("ctrl status stall test fail!")
                    sys.exit(-1)
            continue
        
        content_file = open(pat, 'rb')
        data = array('B')
        data.fromfile(content_file, content_size)
        try:
            send_len = device.ctrl_transfer(bmRequestTypeOut, 0, 0, 1, data, 5000)
        except USBError as e:
            if content_size == CTRL_TEST_STALL_OUT and "[Errno 32]" in str(e):
                print ("ctrl out stall test pass!")
                content_file.close()
                continue
            else:
                sys.exit(-1)
        content_file.close()
        if send_len != content_size:
            print ("control write size mismatch %d != %d" %(content_size, send_len))
            sys.exit(-1)

        del data
        data = array('B')    
        chk = check_files[i%len(check_files)]
        content_file = open(chk, 'wb')
        try:
            data = device.ctrl_transfer(bmRequestTypeIn, 0, 0, 1, content_size, 500000)
        except USBError as e:
            if content_size == CTRL_TEST_STALL_IN and "[Errno 32]" in str(e):
                print ("ctrl in stall test pass!")
                content_file.close()
                continue
            else:
                sys.exit(-1)        
        if data is not None:
            read_len = len(data)
            if read_len != content_size and content_size != CTRL_TEST_64P0:
                print ("control read size mismatch %d != %d" %(content_size, send_len))
                sys.exit(-1)
            data.write(content_file)
        content_file.close()
        if content_size == CTRL_TEST_64P0:
            if read_len == 64:
                print ("ctrl 64 plus 0 pass")
            else:
                print ("ctrl 64 plus 0 fail")                
            continue

        if (filecmp.cmp(pat, chk) is True):
            print ("%d ctrl test pass" %content_size)
        else:
            print ("%d ctrl test fail" %content_size)
            sys.exit(-1)               
    del bm_usb_libusb    
    

    

