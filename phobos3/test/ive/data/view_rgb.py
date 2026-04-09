#!/bin/python

import cv2 as cv2        # OpenCV import
import numpy as np
from PIL import Image
import sys
from matplotlib import pyplot as plt

D = np.fromfile(sys.argv[1], dtype='uint8')
w, h = int(sys.argv[2]), int(sys.argv[3])

print(D.shape)
px = w*h*3
BGR = D[0:px].reshape(h,w,3)
print(BGR.shape)

RGB = cv2.cvtColor(BGR, cv2.COLOR_BGR2RGB)
img = Image.fromarray(RGB)
img.show()

if len(sys.argv) >= 6:
  # np.set_printoptions(threshold=sys.maxsize, formatter={'int':hex}, linewidth=84)
  np.set_printoptions(threshold=sys.maxsize, formatter={'int':'{:02x}'.format}, linewidth=50)

  x, y = int(sys.argv[4]), int(sys.argv[5])
  if x < 0:
    print(BGR[y])
  else:
    print(BGR[y,x])
