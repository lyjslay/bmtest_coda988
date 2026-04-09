#!/usr/local/bin/python
from bm_usb_util.bm_usb_pyserial import bm_usb_pyserial
from bm_usb_util.bm_usb_libusb import bm_usb_libusb
import os
import sys
import time
import filecmp
import bm_usb_util.bm_usb_pkt as pkt
from array import array
import random

test_cnt = 100
patterns = ['bulk00', 'bulk01', 'bulk02', 'bulk03', 'bulk04', 'bulk05', 'bulk06', 'bulk07', 'bulk08', 'BulkInStall']
check_files = ['Rbulk00']

bmtest_vidpid = 'VID:PID=0559:4012'

if __name__ == '__main__':
    print ("bm1880_emmc_download v1.1")

    bm_usb_libusb = bm_usb_libusb()
    if len(sys.argv) > 1:
        bm_usb_libusb.parse_arg()

    device = bm_usb_libusb.libusb_query([bmtest_vidpid], 10)
    print ("Done")

    for i in range(0, test_cnt):        
        print ("====== Test %d ======" %i)
        pat = patterns[random.randint(0, len(patterns)-1)]
        file_size = os.path.getsize(pat)
     
        cmd = array('B')
        cmd.append(0)   # direction write
        cmd.append(0)
        cmd.append(0)
        cmd.append(0)
    
        cmd.append(file_size & 0xFF)             # file size
        cmd.append((file_size >> 8) & 0xFF)
        cmd.append((file_size >> 16) & 0xFF)
        cmd.append((file_size >> 24) & 0xFF) 
        print ("Sending %s..." % pat)
        bm_usb_libusb._usb_send_req(cmd, 0, 0)
        dest_addr = 0
        if (bm_usb_libusb.usb_send_file(pat, dest_addr, 0) < 0):
            if file_size == 0xDEAD:
                print("stall out test")
                continue
            
        chk = check_files[i%len(check_files)]
    
        del cmd

        cmd = array('B')
        cmd.append(1)   # direction write
        cmd.append(0)
        cmd.append(0)
        cmd.append(0)
        cmd.append(file_size & 0xFF)             # file size
        cmd.append((file_size >> 8) & 0xFF)
        cmd.append((file_size >> 16) & 0xFF)
        cmd.append((file_size >> 24) & 0xFF)
        print ("Reading %s..." % chk)
        bm_usb_libusb._usb_send_req(cmd, 0, 0)    
        if (bm_usb_libusb.usb_read_file(chk, file_size) < 0):
            if file_size == 0xBEEF:
                print("stall in test")
                continue            

        if (filecmp.cmp(pat, chk) is True):
            print ("bulk-out test pass")
        else:
            print ("bulk-out test fail")    
            sys.exit(-1)

    del bm_usb_libusb    
    

    

