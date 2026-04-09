#!/bin/python

import cv2 as cv2        # OpenCV import
import numpy as np
from PIL import Image
import sys
from matplotlib import pyplot as plt

YUV_SP420 = np.fromfile(sys.argv[1], dtype='uint8')
w, h = int(sys.argv[2]), int(sys.argv[3])

print(YUV_SP420.shape)
px = w*h
Y = YUV_SP420[0:px].reshape(h,w)
print(Y.shape)
UV = YUV_SP420[px:].reshape(h//2,w)

img = Image.fromarray(Y)
img.show()

if len(sys.argv) >= 6:
  # np.set_printoptions(threshold=sys.maxsize, formatter={'int':hex}, linewidth=84)
  np.set_printoptions(threshold=sys.maxsize, formatter={'int':'{:02x}'.format}, linewidth=50)

  x, y = int(sys.argv[4]), int(sys.argv[5])
  if x < 0:
    print(Y[y])
    print(UV[y//2])
  else:
    print(Y[y,x])
    print(UV[y//2,x])
