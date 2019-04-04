#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include "bmpFile.h"

typedef unsigned char BYTE;
typedef struct _IMAGE_BUF
{
	int             width;
	int             height;
	int             Total;
	unsigned char*  buf;
}IMAGE_BUF;


int clamp(int val) {
	if (val > 255) return 255;
	if (val < 0) return 0;
	return val;
}

//构建BMP位图文件头  
void ContructBhh(int nWidth, int nHeight, BITMAPFILEHEADER& bhh) //add 2010-9-04  
{
	int widthStep = (((nWidth * 24) + 31) & (~31)) / 8; //每行实际占用的大小（每行都被填充到一个4字节边界）  
	bhh.bfType = ((WORD)('M' << 8) | 'B');  //'BM'  
	bhh.bfSize = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + widthStep * nHeight;
	bhh.bfReserved1 = 0;
	bhh.bfReserved2 = 0;
	bhh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
}

// 构建BMP文件信息头
void ConstructBih(int nWidth, int nHeight, BITMAPINFOHEADER& bih)
{
	int widthStep = (((nWidth * 24) + 31) & (~31)) / 8;
	bih.biSize = 40;       // header size  
	bih.biWidth = nWidth;
	bih.biHeight = nHeight;
	bih.biPlanes = 1;
	bih.biBitCount = 24;     // RGB encoded, 24 bit  
	bih.biCompression = BI_RGB;   // no compression 非压缩  
	bih.biSizeImage = widthStep*nHeight * 3;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;
}

bool SaveDIB2Bmp(int fileNum, const char * fileName, const char * filePath, int iWidth, int iHeight, BYTE *pBuffer) {
	BITMAPINFOHEADER bih;
	ConstructBih(iWidth, iHeight, bih);
	BITMAPFILEHEADER bhh;
	ContructBhh(iWidth, iHeight, bhh);

	CHAR BMPFileName[1024];
	int widthStep = (((iWidth * 24) + 31) & (~31)) / 8; //每行实际占用的大小（每行都被填充到一个4字节边界）  
	int DIBSize = widthStep * iHeight;  //buffer的大小 （字节为单位）  

										//save  
	char path[1024];
	char str[1024];
	char num[10];
	sprintf(num, "%d", fileNum);
	sprintf(str, fileName);
	strcat(str, num);
	strcat(str, ".bmp");  //frame100.bmp  
	sprintf(path, "%s", filePath);
	strcat(path, str); //Path//frame100.bmp  

	strcpy(BMPFileName, path);
	FILE *file;

	if (file = fopen(BMPFileName, "wb"))
	{
		//写入文件  
		fwrite((LPSTR)&bhh, sizeof(BITMAPFILEHEADER), 1, file);
		fwrite((LPSTR)&bih, sizeof(BITMAPINFOHEADER), 1, file);
		fwrite(pBuffer, sizeof(char), DIBSize, file);
		fclose(file);
		return true;
	}
	return false;
}

void yuToRGB(const BYTE *yData, const BYTE *uData, const BYTE *vData, const int width, const int height) {
	BYTE *rgb24Data = new BYTE[width*height * 3];
	int index = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			BYTE y = yData[i*width + j];
			BYTE u = uData[(i / 2)*(width / 2) + j / 2];
			BYTE v = vData[(i / 2)*(width / 2) + j / 2];

			int data = (int)(y + 1.772 * (u - 128));//b分量
			rgb24Data[index] = ((data < 0) ? 0 : (data > 255 ? 255 : data));

			data = (int)(y - 0.34414 * (u - 128) - 0.71414 * (v - 128));//g分量
			rgb24Data[index + 1] = ((data < 0) ? 0 : (data > 255 ? 255 : data));

			data = (int)(y + 1.402 * (v - 128));//r分量
			rgb24Data[index + 2] = ((data < 0) ? 0 : (data > 255 ? 255 : data));
			index += 3;
		}
	}
	static int id = 0;
	SaveDIB2Bmp(id++, "./test", "./image", width, height, (BYTE*)rgb24Data);
	delete[] rgb24Data;
}

void test() {
	FILE *file = fopen("./176_144.yuv", "r");
	if (file == NULL) {
		printf("open file failed!!!\n");
		return;
	}

	const int WIDTH = 176;
	const int HEIGHT = 144;
	const int FRAME_SIZE = (WIDTH * HEIGHT * 3 / 2);

	fseek(file, 0L, SEEK_END);
	const int FRAME_NUMBER = ftell(file) / FRAME_SIZE;
	fseek(file, 0L, SEEK_SET);
	
	for (int i = 0; i < FRAME_NUMBER; i++) {
		BYTE farme_data[FRAME_SIZE];
		size_t len = fread(farme_data, 1, FRAME_SIZE, file);
 		yuToRGB(farme_data, (BYTE*)(farme_data + WIDTH * HEIGHT), (BYTE*)(farme_data + WIDTH * HEIGHT * 5 / 4) , WIDTH, HEIGHT);
		printf("index = %d %d\n", i, len);
	}
	fclose(file);
}

using namespace std;
int main() {
	test();
	system("pause");
	return 0;
}