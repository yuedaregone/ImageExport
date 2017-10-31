// DotMatrix.cpp : 定义控制台应用程序的入口点。
//
#include <iostream>
#include <map>
#include <vector>
#include "MImage.h"
#include <fstream>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("eg.");
		printf("export *.png");
		return -1;
	}

	char buff[1024] = { 0 };
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
	unsigned char* _data = img.getData();
	size_t _size = img.getDataSize();
	unsigned short w = img.getWidth();
	int off = 3;
	if (img.isHasAlpha())
	{
		off = 4;
	}

	FILE* stream = fopen(fileTxt.c_str(), "w+");	
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
		if (i !=0)
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
		fprintf(stream, "%d", _data[i+1]);
		fputc(',', stream);
		fprintf(stream, "%d", _data[i+2]);		
	}
	fputs("]];", stream);
	fclose(stream);
	return 0;
}

