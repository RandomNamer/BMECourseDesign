import cv2 
import numpy as np

src = np.zeros((256, 256, 3), np.uint8)
for i in range(256):
    strip = np.zeros((256,3),np.uint8)
    strip.fill(i)
    src[i] = strip
dst = cv2.applyColorMap(src, cv2.COLORMAP_JET)
code_r = code_g = code_b = '[ '
for i in range(256):
    # print(dst[i][0][0])
    code_r+=(str(dst[i][0][0]) + ', ')
    code_g+=(str(dst[i][0][1])  + ', ')
    code_b+=(str(dst[i][0][2])  + ', ')
code_r+=']'
code_g+=']'
code_b+=']'
print("r_map:\n", code_r)
print("g_map:\n", code_g)
print("b_map:\n", code_b)

