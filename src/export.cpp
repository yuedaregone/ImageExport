// DotMatrix.cpp : 定义控制台应用程序的入口点。
//
#include <iostream>
#include <map>
#include <vector>
#include "MImage.h"
#include <fstream>
#include <stdlib.h>
#include <string.h>

void OutPutToJS(const std::string& fileTxt, MImage& img)
{
	unsigned char* _data = img.getData();
	size_t _size = img.getDataSize();
	unsigned short w = img.getWidth();
	int off = 3;
	if (img.isHasAlpha())
	{
		off = 4;
	}

	FILE* stream = fopen(fileTxt.c_str(), "w+");

	char buff[1024] = { 0 };
	memset(buff, 0, 1024);
	sprintf(buff, "var width=%d;\n", w);
	fputs(buff, stream);
	memset(buff, 0, 1024);
	sprintf(buff, "var height=%d;\n", img.getHeight());
	fputs(buff, stream);
	fputs("var xs = [[", stream);

	int k = 0;
	int h = w*off;
	for (int i = 0; i < _size; i += off)
	{
		if (i != 0)
		{
			if (i % h == 0)
			{
				fputs("],\n[", stream);
				k = 0;
			}
			else
			{
				fputc(',', stream);
			}
		}
		fprintf(stream, "%d", _data[i]);
		fputc(',', stream);
		fprintf(stream, "%d", _data[i + 1]);
		fputc(',', stream);
		fprintf(stream, "%d", _data[i + 2]);
	}
	fputs("]];", stream);
	fclose(stream);
}

void OutPutToH(const std::string& fileTxt, MImage& img)
{
	unsigned char* _data = img.getData();
	size_t _size = img.getDataSize();
	unsigned short w = img.getWidth();
	unsigned short h = img.getHeight();
	int off = 3;
	if (img.isHasAlpha())
	{
		off = 4;
	}

	FILE* stream = fopen(fileTxt.c_str(), "w+");
	fputs("#pragma once\n", stream);
	fputs("//format:RGB(A)\n\n", stream);
	char buff[1024] = { 0 };
	memset(buff, 0, 1024);
	std::string fileName = fileTxt.substr(0, fileTxt.find_last_of("."));
	sprintf(buff, "struct Img_%s\n{\n\tint width = %d;\n\tint height = %d;\n\t", fileName.c_str(), w, h);
	fputs(buff, stream);
	memset(buff, 0, 1024);
	sprintf(buff, "unsigned int data[%d * %d] = {", w, h);
	fputs(buff, stream);	

	for (int i = 0; i < _size; i += off)
	{
		if (i != 0)
			fputs(",", stream);

		if (i % w == 0)		
			fputs("\n\t\t", stream);		

		fputs("0x", stream);
		for (int j = 0; j < off; ++j)
		{
			fprintf(stream, "%02X", (int)_data[i + j]);
		}
	}
	fputs("\n\t};\n};", stream);
	fclose(stream);
}


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("eg.\n");
		printf("\texport *.png -js\n");
		printf("\texport *.png -h");
		return -1;
	}
	
	std::string strPath = argv[0];
	strPath = strPath.substr(0, strPath.rfind("/"));	

	std::string png_file = argv[1];
	std::string subfix = png_file.substr(png_file.find_last_of(".") + 1, png_file.length());
	if (subfix.compare("png") != 0)
	{
		printf("Input file must be png!");
		return -1;
	}

	std::string fileTxt = png_file;
	fileTxt = fileTxt.substr(0, fileTxt.find_last_of("."));	
	fileTxt += ".txt";
		
	MImage img;
	img.initWithFile(png_file.c_str(), MImage::typePNG);	

	if (argc > 2 && strcmp(argv[2], "-js") == 0)
	{
		OutPutToJS(fileTxt, img);
	}
	else if (argc > 2 && strcmp(argv[2], "-h") == 0)
	{
		OutPutToH(fileTxt, img);
	}
	else
	{
		OutPutToH(fileTxt, img);
	}
	return 0;
}

