# <center> Explanations regarding output images

## 0. How to compile

`gcc -Wall -O2 -o YCbCr YCbCr.c -lm`

## 1. Problem b
When S = 2, we can see some unnatural artifacts at the edge of the red parrot.
When S = 4, we can see small color blocks at the grass and the edge of both parrots.
When S = 8, color blocks become bigger all over the image.

## 2. Problem c
S = 1, 768 \* 512 \* 24 = 9437184 bits
S = 2, 768 \* 512 \* (8 + 16/4) = 4718592 bits
S = 4, 768 \* 512 \* (8 + 16/16) = 3538944 bits
S = 8, 768 \* 512 \* (8 + 16/64) = 3244032 bits 

