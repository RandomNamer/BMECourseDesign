// DIBTest.cpp : Defines the entry point for the console application.
//

/*
����
1. ��ɳ���ע���еĸ������⣿
2. ���յ���debug������
3. ���յ���ʱ���ڴ�鿴�ķ�����
4. ���ҳ��������ڣ��ڴ�й©��Bug��
	����A�����۾��ң�
	����B���������ߣ�
 5. ��μ���ͼ��������֮��ľ��룺��㣨20,35������㣨1289,675����
 6. �ѻҶ�lena512.bmp�ĳ�biBitCount=8�Ĳ�ɫͼ��
 7.
*/

#include "stdafx.h"
#include <iostream>
using namespace std;

#include <Windows.h>

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
		cout << "�ļ���ʧ��!" << endl;
		fclose(pf);
		return false;
	}
	
	fread(&bitMapFileHeader, sizeof(BITMAPFILEHEADER), 1, pf);//�˴��Ƿ���©������α�����
	if (0x4D42 != bitMapFileHeader.bfType) // ����жϵ�Ŀ����ʲô��
	{
		cout << "Error��" << endl;
		return false;
	}
	fread(&bitMapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pf);//�˴��Ƿ���©������α�����
	cout << "λͼ�ļ�ͷ��" << endl;
	cout << "λͼ�ļ�����: " << bitMapFileHeader.bfType << endl;
	cout << "λͼ�ļ���С: " << bitMapFileHeader.bfSize << endl;//����ж�bitMapFileHeader.bfSize�Ƿ�����
	cout << "ƫ�Ƶ��ֽ���: " << bitMapFileHeader.bfOffBits << endl; //ʲô��ƫ���ֽ�����
 
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
	pData = new unsigned char[bitMapInfoHeader.biHeight * lineByte];	//�˴��Ƿ���©������α�����
	fread(pData, sizeof(unsigned char), bitMapInfoHeader.biHeight * lineByte, pf);//�˴��Ƿ���©������α�����
	fclose(pf);
	return true;
	//�˶κ���������fread�����������ȡ�ļ���Ч�ʲ��ߣ��������ȡһ���ļ�����ɱ��������ܵķ�����
}
 
bool writeBmpFile(char * filename, unsigned char * pData, int biWidth, int biHeight, int biBitCount)
{
	FILE * pf;
	pf = fopen(filename, "wb");
	if (NULL == pf)
	{
		cout << "�ļ���ʧ��!" << endl;
		fclose(pf);  
		return false;
	}
 
	int colorTablesize = 0;
	if (biBitCount == 8)
		colorTablesize = 1024;
 
	//��˵���˴����߼���
	int lineByte = (biWidth * biBitCount / 8 + 3) / 4 * 4;
 
	//����λͼ�ļ�ͷ�ṹ��������д�ļ�ͷ��Ϣ��������˵��BITMAPFILEHEADER�����ֶκ��壿
	BITMAPFILEHEADER bitMapFileHeader;
	bitMapFileHeader.bfType = 0x4D42;//bmp����
	bitMapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTablesize + lineByte*biHeight;
	bitMapFileHeader.bfReserved1 = 0;
	bitMapFileHeader.bfReserved2 = 0;
	bitMapFileHeader.bfOffBits = 54 + colorTablesize;
 
	//����λͼ��Ϣͷ�ṹ��������д��Ϣͷ��Ϣ��������˵��BITMAPINFOHEADER�����ֶκ��壿
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
 
int main()
{

	char * toReadfilename = "..\\lena512.bmp";
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
 
	char * toWritefilename = "c:\\write.bmp";
	ret = writeBmpFile(toWritefilename, pData, width, height, bitCount);
	if (!ret)
	{
		cout << "дBMP�ļ�ʧ�ܣ�" << endl;
		return -1;
	}
	else
	{
		cout << "дBMP�ļ��ɹ���" << endl;
	}
 
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
