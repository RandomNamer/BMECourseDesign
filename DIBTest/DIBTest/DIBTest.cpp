// DIBTest.cpp : Defines the entry point for the console application.
//

/*
任务：
1. 完成程序注释中的各项问题？
2. 掌握单点debug方法；
3. 掌握调试时，内存查看的方法。
4. 请找出主函数内，内存泄漏的Bug？
	方法A：用眼睛找；
	方法B：借助工具；
 5. 如何计算图像上两点之间的距离：像点（20,35）到像点（1289,675）？
 Solution: 数字图像中，有多种衡量像素间距离的方法：
 - 欧几里得距离： \sqrt{(x_1-x_2)^2 + (y_1 - y_2)^2}， 结果为1421.
 - D4距离：|x_1 - X_2| + |y_1  y_2| 4-邻接距离相同， 结果为1909.
 - D8距离： max{|x_1 - X_2| , |y_1  y_2|} 8-邻接距离相同，结果为1269.
 6. 把灰度lena512.bmp改成e=8的彩色图像
 Solution: 添加伪彩方案：根据亮度添加相应的彩色，但是需要一个有256色的颜色查找表（LUT），以完成256阶灰度到24位彩色间的过渡。
 这里编写了一个python脚本[../NiseColor.py]把OpenCV的COLORMAP_JET伪色方案提取出来，写在头文件中。
 7. 把图像垂直方向翻转
 Solution: 与上述类似，也是分配一块合适大小的内存，将对应位置的像素拷贝过来。
 */

#include "stdafx.h"
#include <iostream>
using namespace std;

#include <Windows.h>
#include "ColorScheme.h"

using std::cout;
using std::endl;
 
unsigned char * pData;
int width;
int height;
int bitCount;
RGBQUAD * pRGBQUAD;
BITMAPFILEHEADER bitMapFileHeader;
BITMAPINFOHEADER bitMapInfoHeader; 

bool readBmpFile(char * filename) 
{
	FILE * pf;
	pf = fopen(filename, "rb");
	if (NULL == pf)
	{
		cout << "文件打开失败!" << endl;
		//fclose(pf);
		return false;
	}
	
	fread(&bitMapFileHeader, sizeof(BITMAPFILEHEADER), 1, pf);//此处是否有漏洞，如何保护？
	if (0x4D42 != bitMapFileHeader.bfType) // 这个判断的目的是什么？ 根据官方文档，BITMAPFILEHEADER.bfType必须是这样。
	{
		cout << "Error！" << endl;
		return false;
	}
	fread(&bitMapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pf);//此处是否有漏洞，如何保护？ 没有考虑读取失败的情况，需要在后面加对读取出指针的判断
	cout << "位图文件头：" << endl;
	cout << "位图文件类型: " << bitMapFileHeader.bfType << endl;
	cout << "位图文件大小: " << bitMapFileHeader.bfSize << endl;//如何判断bitMapFileHeader.bfSize是否有误： bfSize = biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
	cout << "偏移的字节数: " << bitMapFileHeader.bfOffBits << endl; //什么叫偏移字节数？从bitmapFileHeader到bitmap数据的距离。
 
	width = bitMapInfoHeader.biWidth;
	height = bitMapInfoHeader.biHeight;
	bitCount = bitMapInfoHeader.biBitCount;
	cout << "\n位图信息头：" << endl;
	cout << "信息头占用字节数：" << bitMapInfoHeader.biSize << endl;
	cout << "位图宽度： " << bitMapInfoHeader.biWidth << endl;
	cout << "位图高度： " << bitMapInfoHeader.biHeight << endl;
	cout << "位图压缩类型： " << bitMapInfoHeader.biCompression << endl;
	cout << "位图每像素占用位数： " << bitMapInfoHeader.biBitCount << endl;  //什么叫每像素占用位数？
	cout << "位图数据占用字节数： " << bitMapInfoHeader.biSizeImage << endl;//如何判断bitMapInfoHeader.biSizeImage是否有误？
 
	if (8 == bitMapInfoHeader.biBitCount) //此处判断，目的是什么？
	{
		pRGBQUAD = new RGBQUAD[256];
		fread(pRGBQUAD, sizeof(RGBQUAD), 256, pf);
	}

	int lineByte = (bitMapInfoHeader.biWidth * bitMapInfoHeader.biBitCount / 8 + 3) / 4 * 4; //此处的逻辑是什么？请解释？
	pData = new unsigned char[bitMapInfoHeader.biHeight * lineByte];	//此处是否有漏洞，如何保护？ 没考虑内存分配失败？这个也需要有对应的delete回收
	fread(pData, sizeof(unsigned char), bitMapInfoHeader.biHeight * lineByte, pf);//此处是否有漏洞，如何保护？
	fclose(pf);
	return true;
	//此段函授有三处fread，如果批量读取文件，效率不高，请给出读取一次文件，完成本函数功能的方案
	// Solution： 读取一次放在缓冲区内，然后对缓冲区的内存进行操作。
}
 
bool writeBmpFile(char * filename, unsigned char * pData, int biWidth, int biHeight, int biBitCount)
{
	FILE * pf;
	pf = fopen(filename, "wb");
	if (NULL == pf)
	{
		cout << "文件打开失败!" << endl;
		//fclose(pf);  
		return false;
	}
 
	int colorTablesize = 0;
	if (biBitCount == 8)
		colorTablesize = 1024;
 
	//请说明此处的逻辑？
	int lineByte = (biWidth * biBitCount / 8 + 3) / 4 * 4;
 
	//申请位图文件头结构变量，填写文件头信息，请逐项说明BITMAPFILEHEADER各个字段含义？
	/*
	 * 根据官方文档， byType是一个恒定值；
	 * bfSize就是整个文件的大小
	 * bfReversed是两个空的变量，都是0
	 * bfOffset是BMP文件开头到实际的图像矩阵的偏移量。
	 * BMP文件的结构是BITMAPFILEHEADER, BITMAPINFOHEADER, RGBQUAD Array, Color-Index Array连续排列。
	 * ref：https://docs.microsoft.com/en-us/windows/win32/gdi/bitmap-storage
	 */
	BITMAPFILEHEADER bitMapFileHeader;
	bitMapFileHeader.bfType = 0x4D42;//bmp类型
	bitMapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTablesize + lineByte*biHeight;
	bitMapFileHeader.bfReserved1 = 0;
	bitMapFileHeader.bfReserved2 = 0;
	bitMapFileHeader.bfOffBits = 54 + colorTablesize;
 
	//申请位图信息头结构变量，填写信息头信息，请逐项说明BITMAPINFOHEADER各个字段含义？
	//ref： https://docs.microsoft.com/en-us/previous-versions/dd183376(v=vs.85)
	BITMAPINFOHEADER bitMapInfoHeader;
	bitMapInfoHeader.biBitCount = biBitCount;
	bitMapInfoHeader.biClrImportant = 0;
	bitMapInfoHeader.biClrUsed = 0;
	bitMapInfoHeader.biCompression = 0;
	bitMapInfoHeader.biHeight = biHeight;
	bitMapInfoHeader.biPlanes = 1;
	bitMapInfoHeader.biSize = 40;
	bitMapInfoHeader.biSizeImage = lineByte * biHeight;
	bitMapInfoHeader.biWidth = biWidth;
	bitMapInfoHeader.biXPelsPerMeter = 0;
	bitMapInfoHeader.biYPelsPerMeter = 0;


	//写文件头进文件
	fwrite(&bitMapFileHeader, sizeof(BITMAPFILEHEADER), 1, pf);
	//写位图信息头进内存
	fwrite(&bitMapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pf);
 
	//此处是何逻辑？
	if (biBitCount == 8)
	{
		fwrite(pRGBQUAD, sizeof(RGBQUAD), 256, pf);
	}
 
	fwrite(pData, sizeof(unsigned char), biHeight * lineByte, pf);
	fclose(pf);
	return true;
}

unsigned char* makeFakeColorContents(unsigned char* data, int width, int height) {
	// int bitCount = 24;
	unsigned char* srcCurPix = data;
	unsigned char* dst = new unsigned char[width * height * 3];
	unsigned char* dstCurPix = dst;
	for (int x = 0; x < height; x++) {
		for (int y = 0; y < width; y++) {
			srcCurPix = data + (x * width + y);
			dstCurPix = dst + (x * width + y) * 3;
			dstCurPix[0] = LUT_B[*srcCurPix];
			dstCurPix[1] = LUT_G[*srcCurPix];
			dstCurPix[2] = LUT_R[*srcCurPix];
		}
	}
	return dst;
}

unsigned char* makeFlipVertical(unsigned char* data, int width, int height, int bitCount) {
	unsigned char* srcCurPix;
	unsigned char* dstCurPix;
	int lineByte = (width * bitCount / 8 + 3) / 4 * 4;
	unsigned char* dst = new unsigned char[lineByte * height];
	for (int x = 0; x < height; x++) {
		for (int y = 0; y < width; y++) {
			srcCurPix = data + ((height - 1- x) * width + y);
			dstCurPix = dst + (x * width + y);
			dstCurPix[0] = *srcCurPix;
		}
	}
	return dst;
}

 
int main()
{

	char * toReadfilename = "..\\lena512.bmp";
	//char* toReadfilename = "..\\flipVertical.bmp";
	bool ret = readBmpFile(toReadfilename);
	if (!ret)
	{
		cout << "读BMP文件失败！" << endl;
		return -1;
	}
	else
	{
		cout << "读BMP文件成功！" << endl;
	}
	cout << endl;
 
	//写入翻转图像
	char* toWritefilename = "..\\flipVertical.bmp";
	unsigned char* fcData = makeFlipVertical(pData, width, height, bitCount);
	ret = writeBmpFile(toWritefilename, fcData, width, height, bitCount);
	if (!ret)
	{
		cout << "写翻转文件失败！" << endl;
		return -1;
	}
	else
	{
		cout << "写翻转文件成功！" << endl;
	}
	delete fcData;
	//写入伪色图像：
	toWritefilename = "..\\fakeColor.bmp";
	fcData = makeFakeColorContents(pData, width, height);
	ret = writeBmpFile(toWritefilename, fcData, width, height, 24);
	if (!ret)
	{
		cout << "写伪色文件失败！" << endl;
		return -1;
	}
	else
	{
		cout << "写伪色文件成功！" << endl;
	}
	delete fcData;
	fcData = NULL;
 
	delete[] pRGBQUAD;
	pRGBQUAD = NULL;
 
	pData = NULL;
	return 0;
}

/*int _tmain(int argc, _TCHAR* argv[])
{

	return 0;
}
*/
