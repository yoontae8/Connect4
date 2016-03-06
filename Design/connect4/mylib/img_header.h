#ifndef _IMGTOARR_
#define _IMGTOARR_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned int u64;

typedef struct tagBITMAPFILEHEADER {
    u16 bfType;
    u32 bfSize;
    u16 bfReserved1;
    u16 bfReserved2;
    u32 bfOffBits;
} __attribute((packed))BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    u32 biSize;
    u64 biWidth;
    u64 biHeight;
    u16 biplanes;
    u16 biBitCount;
    u32 biCompression;
    u32 biSizeImage;
    u64 biXPelsPerMeter;
    u64 biYPelsPerMeter;
    u32 biClrUsed;
    u32 biClrImportant;
} __attribute((packed)) BITMAPINFOHEADER;

void saveImgInArray(char* filename, u32 **bmpArr);
#endif
