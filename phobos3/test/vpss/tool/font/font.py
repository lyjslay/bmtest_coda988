import os,sys
import time
import threading
import cv2  
import numpy as np  
import argparse

def newfont(w, h, fx, fy, fontsize):
    img_1 = np.zeros([w,h,1],dtype=np.uint8)
    img_1.fill(0)

    #font = cv2.FONT_HERSHEY_SIMPLEX
    str1 = 'WORLD'

    #cv2.putText(img_1 text, (100,300)坐标,font,2字体大小,(255,255,255)颜色,3字体宽度 )

    cv2.putText(img_1, str1, (fx,fy),font,fontsize,(255,255,255),3 )
    # or img[:] = 255
    cv2.imshow('Single Channel Window', img_1)
    print("image shape: ", img_1.shape)
    cv2.imwrite('font.bmp',img_1)

    outarray=np.asarray(img_1)
    outarray.tofile('font.bin')

    size = int(w*h/8)
    arry = np.zeros([1,size,1],dtype=np.uint8)
    arry.fill(0)
    count = 0
    t_value = 0
    value = 0
    for i in img_1.flatten():

        if (i != 0):
            value = 1
        else:
            value = 0
        t_value |= (value<<(count%8)) 
        count += 1        

        if(count%8 == 0):
            index = int(count/8 -1)
            #print ('debug info')
            #print (t_value)
            #print (index)
            #print (type(t_value))
            #print (arry[0,0])
            
            arry[0,index] = t_value
            #print (arry[0,index])
            t_value = 0

    #print (arry)
    arry.tofile('font2.bin')
     


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--shape', dest='shape', type=str,required=True)
    parser.add_argument('--fontxy', dest='fxy', type=str,required=True)
    parser.add_argument('--fontsize', dest='fsize', type=int,required=True)

    args = parser.parse_args()
    
    shape = args.shape
    shape = shape.split(',')
    fxy = args.fxy
    fxy = fxy.split(',')
    fsize = args.fsize

    print(fxy)

    print(shape)

    print(fsize)

    newfont(int(shape[0]), int(shape[1]), int(fxy[0]), int(fxy[1]), fsize)

