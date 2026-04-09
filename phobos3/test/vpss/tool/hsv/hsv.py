import os,sys
import time
import threading
import cv2  
import numpy as np  
import argparse

def rgb2hsv(rgb_file):
	img1=cv2.imread(rgb_file,cv2.IMREAD_COLOR)

	HSV1 =cv2.cvtColor(img1, cv2.COLOR_BGR2HSV)
	aH1, aS1, aV1 =cv2.split(HSV1)
	H1 =np.array(aH1).flatten()
	S1 =np.array(aS1).flatten()
	V1 =np.array(aV1).flatten()

	cv2.namedWindow('rgb2hsv', cv2.WINDOW_AUTOSIZE)
	cv2.imshow('rgb2hsv', HSV1)
	cv2.waitKey(0)
	cv2.destroyAllWindows()
	cv2.waitKey(1)
	cv2.imwrite("rgb2hsv.bmp", HSV1);

def hsv2rgb(hsv_file):
	img1=cv2.imread(hsv_file,cv2.IMREAD_COLOR)

	BGR =cv2.cvtColor(img1, cv2.COLOR_HSV2BGR)


	cv2.namedWindow('hsv2rgb', cv2.WINDOW_AUTOSIZE)
	cv2.imshow('hsv2rgb', BGR)
	cv2.waitKey(0)
	cv2.destroyAllWindows()
	cv2.waitKey(1)
	cv2.imwrite("hsv2rgb.bmp", BGR);

if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument('--file', dest='file', type=str,required=True)
	parser.add_argument('--hsv2rgb', dest='hsv2rgb', type=int,required=True)
	args = parser.parse_args()

	file = args.file
	h2r = args.hsv2rgb
	
	if (h2r == 1 ):
		hsv2rgb(file)
	else:
		rgb2hsv(file)
