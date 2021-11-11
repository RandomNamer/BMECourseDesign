// DIBTest.cpp : Defines the entry point for the console application.
//

/*
����
1. ��ɳ���ע���еĸ������⣿done
2. ���յ���debug������---
3. ���յ���ʱ���ڴ�鿴�ķ������ڴ���Ӵ��ں;ֲ��������ӡ�
4. ���ҳ��������ڣ��ڴ�й©��Bug��
	����A�����۾��ң� new��delete��Ҫ�ɶԳ���
	����B���������ߣ� malloc �� free����صļ���
 5. ��μ���ͼ��������֮��ľ��룺��㣨20,35������㣨1289,675����
 Solution: ����ͼ���У��ж��ֺ������ؼ����ķ�����
 - ŷ����þ��룺 \sqrt{(x_1-x_2)^2 + (y_1 - y_2)^2}�� ���Ϊ1421.
 - D4���룺|x_1 - X_2| + |y_1  y_2| 4-�ڽӾ�����ͬ�� ���Ϊ1909.
 - D8���룺 max{|x_1 - X_2| , |y_1  y_2|} 8-�ڽӾ�����ͬ�����Ϊ1269.
 6. �ѻҶ�lena512.bmp�ĳ�e=8�Ĳ�ɫͼ��
 Solution: ���α�ʷ������������������Ӧ�Ĳ�ɫ��������Ҫһ����256ɫ����ɫ���ұ�LUT���������256�׻Ҷȵ�24λ��ɫ��Ĺ��ɡ�
 �����д��һ��python�ű�[../NiseColor.py]��OpenCV��COLORMAP_JETαɫ������ȡ������д��ͷ�ļ��С�
 7. ��ͼ��ֱ����ת
 Solution: ���������ƣ�Ҳ�Ƿ���һ����ʴ�С���ڴ棬����Ӧλ�õ����ؿ���������
 */

#include "stdafx.h"
#include <iostream>

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC 
// #include<stdlib.h>
// #include<malloc.h>
// #include<crtdbg.h>

// #define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif




#include <Windows.h>
#include "ColorScheme.h"
constexpr int INIT_BUFFER_SIZE = 1024 * 1024;

using namespace std;

using std::cout;
using std::endl;
 
unsigned char * pData;
int width;
int height;
int bitCount;
RGBQUAD * pRGBQUAD;
BITMAPFILEHEADER bitMapFileHeader;
BITMAPINFOHEADER bitMapInfoHeader;

bool readBmpFileNew(char * filename)
{
	FILE* pf;
	pf = fopen(filename, "rb");
	if (NULL == pf)
	{
		cout << "�ļ���ʧ��!" << endl;
		//fclose(pf);
		return false;
	}
	unsigned char* buffer = new unsigned char[INIT_BUFFER_SIZE];
	if(0 == fread(&buffer, INIT_BUFFER_SIZE, 1, pf))
	{
		// cout << "File Read Count Zero, nothing is read!" << endl;
		// return false;
	}
	memcpy(&bitMapFileHeader, buffer, sizeof(BITMAPFILEHEADER));
	if (0x4D42 != bitMapFileHeader.bfType) {
		cout << "Error��Inappropriate file format." << endl;
		return false;
	}
	if(bitMapFileHeader.bfSize > INIT_BUFFER_SIZE)
	{
		const int newSize = bitMapFileHeader.bfSize;
		delete[] buffer;
		buffer = new unsigned char[newSize];
		if (0 == fread(&buffer, INIT_BUFFER_SIZE, 1, pf))
		{
			// cout << "File Read Count Zero, nothing is read!" << endl;
			// return false;
		}
		memcpy(&bitMapFileHeader, buffer, sizeof(BITMAPFILEHEADER));
		if (0x4D42 != bitMapFileHeader.bfType) {
			cout << "Error��Inappropriate file format." << endl;
			return false;
		}
	}
	fclose(pf);
	memcpy(&bitMapInfoHeader, buffer + sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));
	cout << "λͼ�ļ�ͷ��" << endl;
	cout << "λͼ�ļ�����: " << bitMapFileHeader.bfType << endl;
	cout << "λͼ�ļ���С: " << bitMapFileHeader.bfSize << endl;
	cout << "ƫ�Ƶ��ֽ���: " << bitMapFileHeader.bfOffBits << endl;
	width = bitMapInfoHeader.biWidth;
	height = bitMapInfoHeader.biHeight;
	bitCount = bitMapInfoHeader.biBitCount;
	cout << "\nλͼ��Ϣͷ��" << endl;
	cout << "��Ϣͷռ���ֽ�����" << bitMapInfoHeader.biSize << endl;
	cout << "λͼ��ȣ� " << bitMapInfoHeader.biWidth << endl;
	cout << "λͼ�߶ȣ� " << bitMapInfoHeader.biHeight << endl;
	cout << "λͼѹ�����ͣ� " << bitMapInfoHeader.biCompression << endl;
	cout << "λͼÿ����ռ��λ���� " << bitMapInfoHeader.biBitCount << endl; 
	cout << "λͼ����ռ���ֽ����� " << bitMapInfoHeader.biSizeImage << endl;

	if (8 == bitMapInfoHeader.biBitCount) 
	{
		pRGBQUAD = new RGBQUAD[256];
		fread(pRGBQUAD, sizeof(RGBQUAD), 256, pf);
	}

	int lineByte = (bitMapInfoHeader.biWidth * bitMapInfoHeader.biBitCount / 8 + 3) / 4 * 4;
	memcpy(pData, buffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), bitMapInfoHeader.biHeight * lineByte);
	delete[] buffer;
	return true;
}

bool readBmpFile(char * filename) 
{
	FILE * pf;
	pf = fopen(filename, "rb");
	if (NULL == pf)
	{
		cout << "�ļ���ʧ��!" << endl;
		//fclose(pf);
		return false;
	}
	
	fread(&bitMapFileHeader, sizeof(BITMAPFILEHEADER), 1, pf);//�˴��Ƿ���©������α�����
	if (0x4D42 != bitMapFileHeader.bfType) // ����жϵ�Ŀ����ʲô�� ���ݹٷ��ĵ���BITMAPFILEHEADER.bfType������������
	{
		cout << "Error��" << endl;
		return false;
	}
	fread(&bitMapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pf);//�˴��Ƿ���©������α����� û�п��Ƕ�ȡʧ�ܵ��������Ҫ�ں���ӶԶ�ȡ��ָ����ж�
	cout << "λͼ�ļ�ͷ��" << endl;
	cout << "λͼ�ļ�����: " << bitMapFileHeader.bfType << endl;
	cout << "λͼ�ļ���С: " << bitMapFileHeader.bfSize << endl;//����ж�bitMapFileHeader.bfSize�Ƿ����� bfSize = biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
	cout << "ƫ�Ƶ��ֽ���: " << bitMapFileHeader.bfOffBits << endl; //ʲô��ƫ���ֽ�������bitmapFileHeader��bitmap���ݵľ��롣
 
	width = bitMapInfoHeader.biWidth;
	height = bitMapInfoHeader.biHeight;
	bitCount = bitMapInfoHeader.biBitCount;
	cout << "\nλͼ��Ϣͷ��" << endl;
	cout << "��Ϣͷռ���ֽ�����" << bitMapInfoHeader.biSize << endl;
	cout << "λͼ��ȣ� " << bitMapInfoHeader.biWidth << endl;
	cout << "λͼ�߶ȣ� " << bitMapInfoHeader.biHeight << endl;
	cout << "λͼѹ�����ͣ� " << bitMapInfoHeader.biCompression << endl;
	cout << "λͼÿ����ռ��λ���� " << bitMapInfoHeader.biBitCount << endl;  //ʲô��ÿ����ռ��λ����
	cout << "λͼ����ռ���ֽ����� " << bitMapInfoHeader.biSizeImage << endl;//����ж�bitMapInfoHeader.biSizeImage�Ƿ�����
 
	if (8 == bitMapInfoHeader.biBitCount) //�˴��жϣ�Ŀ����ʲô��
	{
		pRGBQUAD = new RGBQUAD[256];
		fread(pRGBQUAD, sizeof(RGBQUAD), 256, pf);
	}

	int lineByte = (bitMapInfoHeader.biWidth * bitMapInfoHeader.biBitCount / 8 + 3) / 4 * 4; //�˴����߼���ʲô������ͣ�
	pData = new unsigned char[bitMapInfoHeader.biHeight * lineByte];	//�˴��Ƿ���©������α����� û�����ڴ����ʧ�ܣ����Ҳ��Ҫ�ж�Ӧ��delete����
	fread(pData, sizeof(unsigned char), bitMapInfoHeader.biHeight * lineByte, pf);//�˴��Ƿ���©������α�����
	fclose(pf);
	return true;
	//�˶κ���������fread�����������ȡ�ļ���Ч�ʲ��ߣ��������ȡһ���ļ�����ɱ��������ܵķ���
	// Solution�� ��ȡһ�η��ڻ������ڣ�Ȼ��Ի��������ڴ���в�����
}
 
bool writeBmpFile(char * filename, unsigned char * pData, int biWidth, int biHeight, int biBitCount)
{
	FILE * pf;
	pf = fopen(filename, "wb");
	if (NULL == pf)
	{
		cout << "�ļ���ʧ��!" << endl;
		//fclose(pf);  
		return false;
	}
 
	int colorTablesize = 0;
	if (biBitCount == 8)
		colorTablesize = 1024;
 
	//��˵���˴����߼���
	int lineByte = (biWidth * biBitCount / 8 + 3) / 4 * 4;
 
	//����λͼ�ļ�ͷ�ṹ��������д�ļ�ͷ��Ϣ��������˵��BITMAPFILEHEADER�����ֶκ��壿
	/*
	 * ���ݹٷ��ĵ��� byType��һ���㶨ֵ��
	 * bfSize���������ļ��Ĵ�С
	 * bfReversed�������յı���������0
	 * bfOffset��BMP�ļ���ͷ��ʵ�ʵ�ͼ������ƫ������
	 * BMP�ļ��Ľṹ��BITMAPFILEHEADER, BITMAPINFOHEADER, RGBQUAD Array, Color-Index Array�������С�
	 * ref��https://docs.microsoft.com/en-us/windows/win32/gdi/bitmap-storage
	 */
	BITMAPFILEHEADER bitMapFileHeader;
	bitMapFileHeader.bfType = 0x4D42;//bmp����
	bitMapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTablesize + lineByte*biHeight;
	bitMapFileHeader.bfReserved1 = 0;
	bitMapFileHeader.bfReserved2 = 0;
	bitMapFileHeader.bfOffBits = 54 + colorTablesize;
 
	//����λͼ��Ϣͷ�ṹ��������д��Ϣͷ��Ϣ��������˵��BITMAPINFOHEADER�����ֶκ��壿
	//ref�� https://docs.microsoft.com/en-us/previous-versions/dd183376(v=vs.85)
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


	//д�ļ�ͷ���ļ�
	fwrite(&bitMapFileHeader, sizeof(BITMAPFILEHEADER), 1, pf);
	//дλͼ��Ϣͷ���ڴ�
	fwrite(&bitMapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pf);
 
	//�˴��Ǻ��߼���
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
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);


	char * toReadfilename = "..\\lena512.bmp";
	//char* toReadfilename = "..\\flipVertical.bmp";
	bool ret = readBmpFile(toReadfilename);
	if (!ret)
	{
		cout << "��BMP�ļ�ʧ�ܣ�" << endl;
		return -1;
	}
	else
	{
		cout << "��BMP�ļ��ɹ���" << endl;
	}
	cout << endl;
 
	//д�뷭תͼ��
	char* toWritefilename = "..\\flipVertical.bmp";
	unsigned char* fcData = makeFlipVertical(pData, width, height, bitCount);
	ret = writeBmpFile(toWritefilename, fcData, width, height, bitCount);
	if (!ret)
	{
		cout << "д��ת�ļ�ʧ�ܣ�" << endl;
		return -1;
	}
	else
	{
		cout << "д��ת�ļ��ɹ���" << endl;
	}
	delete fcData;
	//д��αɫͼ��
	toWritefilename = "..\\fakeColor.bmp";
	fcData = makeFakeColorContents(pData, width, height);
	ret = writeBmpFile(toWritefilename, fcData, width, height, 24);
	if (!ret)
	{
		cout << "дαɫ�ļ�ʧ�ܣ�" << endl;
		return -1;
	}
	else
	{
		cout << "дαɫ�ļ��ɹ���" << endl;
	}
	delete fcData;
	fcData = NULL;
 
	delete[] pRGBQUAD;
	pRGBQUAD = NULL;
 
	pData = NULL;

	// int* a = new int;
	//  int* b = (int*)malloc(sizeof(int));
	_CrtDumpMemoryLeaks();
	// _CrtMemDumpAllObjectsSince(NULL);
	return 0;
	
}
