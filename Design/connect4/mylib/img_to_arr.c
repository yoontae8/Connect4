#include "img_header.h"

//save the given bmp file to 800 * 480 array
void saveImgInArray(char* filename, u32 **bmpArr) {
    FILE *read_fp = NULL;
    BITMAPFILEHEADER BmpFileHd;
    BITMAPINFOHEADER BmpInfoHd;
    int i, j;


    if((read_fp = fopen(filename, "rb")) == NULL) {
	printf("File Open Error\n");
	exit(1);
    }

    //read bmp file header
    fread(&BmpFileHd, sizeof(BITMAPFILEHEADER), 1, read_fp);
    if(BmpFileHd.bfType != 0x4D42) {
	printf("Not Bitmap file or Unsupported Bitmap file\n");
	exit(1);
    }

    //read bmp info header
    fread(&BmpInfoHd, sizeof(BITMAPINFOHEADER), 1, read_fp);
    if(BmpInfoHd.biBitCount != 32) {
	printf("Not 32 bit Bitmap image file\n");
	exit(1);
    }

    fseek(read_fp, BmpFileHd.bfOffBits, SEEK_SET);

    //read bmp file content and save in array
    for(i = 0; i < 480; i++) {
	fread( (void*)bmpArr[i], sizeof(u32), 800, read_fp);
    }

    //flip the content of the array upside down to make it right
    for(i = 0; i < 240; i++) {
	for(j = 0; j < 800; j++) {
	    u32 temp;
	    temp = bmpArr[i][j];
	    bmpArr[i][j] = bmpArr[479-i][j];
	    bmpArr[479-i][j] = temp;
	}
    }
    fclose(read_fp);
}
