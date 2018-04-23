#include "MImage.h"
#include "png.h"
#include <stdlib.h>
#include <string.h>

#define CC_RGB_PREMULTIPLY_ALPHA(vr, vg, vb, va) \
	(unsigned)(((unsigned)((unsigned char)(vr)* ((unsigned char)(va)+1)) >> 8) | \
	((unsigned)((unsigned char)(vg)* ((unsigned char)(va)+1) >> 8) << 8) | \
	((unsigned)((unsigned char)(vb)* ((unsigned char)(va)+1) >> 8) << 16) | \
	((unsigned)(unsigned char)(va) << 24))

#define BreakIf(conf) if(conf) break;

static void pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
	tImageSource* isource = (tImageSource*)png_get_io_ptr(png_ptr);

	if ((int)(isource->offset + length) <= isource->size)
	{
		memcpy(data, isource->data + isource->offset, length);
		isource->offset += length;
	}
	else
	{
		png_error(png_ptr, "pngReaderCallback failed");
	}
}

MImage::MImage()
	: m_pData(NULL)
	, m_nHeight(0)
	, m_nWidth(0)
	, m_nBitsPerComponent(0)
	, m_bHasAlpha(false)
	, m_bPreMulti(false)
{
}

MImage::~MImage()
{
	if (m_pData)
	{
		delete[] m_pData;
	}
}

bool MImage::initWithFile(const char* fileName, MImageType _type)
{
	FILE* fp = NULL;	
	if ((fp = fopen(fileName, "rb")) == NULL)
	{
		printf("Open file error!");
		return false;
	}
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	unsigned char* pBuffer = new unsigned char[size];
	fread(pBuffer, size, 1, fp);
	fclose(fp);

	bool pRet = false;
	if (_type == typePNG)
	{
		pRet = initWithPngData(pBuffer, size);
	}
	delete[] pBuffer;
	return pRet;
}

bool MImage::initWithPngData(void* _data, size_t _len)
{
#define PNGSIGSIZE  8
	bool bRet = false;
	png_byte        header[PNGSIGSIZE] = { 0 };
	png_structp     png_ptr = 0;
	png_infop       info_ptr = 0;

	do
	{
		// png header len is 8 bytes
		BreakIf(_len < PNGSIGSIZE);

		// check the data is png or not
		memcpy(header, _data, PNGSIGSIZE);
		BreakIf(png_sig_cmp(header, 0, PNGSIGSIZE));

		// init png_struct
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		BreakIf(!png_ptr);

		// init png_info
		info_ptr = png_create_info_struct(png_ptr);
		BreakIf(!info_ptr);

		// set the read call back function
		tImageSource imageSource;
		imageSource.data = (unsigned char*)_data;
		imageSource.size = _len;
		imageSource.offset = 0;
		png_set_read_fn(png_ptr, &imageSource, pngReadCallback);

		// read png header info

		// read png file info
		png_read_info(png_ptr, info_ptr);

		m_nWidth = png_get_image_width(png_ptr, info_ptr);
		m_nHeight = png_get_image_height(png_ptr, info_ptr);
		m_nBitsPerComponent = png_get_bit_depth(png_ptr, info_ptr);
		png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);

		//CCLOG("color type %u", color_type);		
		// force palette images to be expanded to 24-bit RGB
		// it may include alpha channel
		if (color_type == PNG_COLOR_TYPE_PALETTE)
		{			
			png_set_palette_to_rgb(png_ptr);
		}
		// low-bit-depth grayscale images are to be expanded to 8 bits
		if (color_type == PNG_COLOR_TYPE_GRAY && m_nBitsPerComponent < 8)
		{
			png_set_expand_gray_1_2_4_to_8(png_ptr);
		}
		// expand any tRNS chunk data into a full alpha channel
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		{
			png_set_tRNS_to_alpha(png_ptr);
		}
		// reduce images with 16-bit samples to 8 bits
		if (m_nBitsPerComponent == 16)
		{
			png_set_strip_16(png_ptr);
		}
		// expand grayscale images to RGB
		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		{
			png_set_gray_to_rgb(png_ptr);
		}

		// read png data
		// m_nBitsPerComponent will always be 8
		m_nBitsPerComponent = 8;
		png_uint_32 rowbytes;
		png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep)* m_nHeight);

		png_read_update_info(png_ptr, info_ptr);

		rowbytes = png_get_rowbytes(png_ptr, info_ptr);

		m_pData = new unsigned char[rowbytes * m_nHeight];
		BreakIf(!m_pData);

		for (unsigned short i = 0; i < m_nHeight; ++i)
		{
			row_pointers[i] = m_pData + i*rowbytes;
		}
		png_read_image(png_ptr, row_pointers);

		png_read_end(png_ptr, NULL);

		png_uint_32 channel = rowbytes / m_nWidth;
		if (channel == 4)
		{
			m_bHasAlpha = true;
			unsigned int *tmp = (unsigned int *)m_pData;
			for (unsigned short i = 0; i < m_nHeight; i++)
			{
				for (unsigned int j = 0; j < rowbytes; j += 4)
				{
					*tmp++ = CC_RGB_PREMULTIPLY_ALPHA(row_pointers[i][j], row_pointers[i][j + 1],
						row_pointers[i][j + 2], row_pointers[i][j + 3]);
				}
			}

			m_bPreMulti = true;
		}

		free(row_pointers);

		bRet = true;
	} while (0);

	if (png_ptr)
	{
		png_destroy_read_struct(&png_ptr, (info_ptr) ? &info_ptr : 0, 0);
	}
	return bRet;
}

size_t MImage::getDataSize()
{
	int pixelLen = 3;
	if (m_bHasAlpha)
	{
		pixelLen = 4;
	}
	return m_nHeight*m_nWidth * pixelLen;
}

void write_png(const char *file_name, int width, int height, unsigned char* image)  
{  
    FILE *fp = fopen(file_name, "wb");
    if (fp == NULL)  
    	return;  

    png_structp png_ptr;  
    png_infop info_ptr;  
    png_colorp palette;     
   
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);  
  
    if (png_ptr == NULL)  
    {  
		printf("error1\n");
      fclose(fp);  
      return;  
    }    
   /* 分配内存并初始化图像信息数据。（必要）*/  
   info_ptr = png_create_info_struct(png_ptr);  
   if (info_ptr == NULL)  
   {  
	   printf("error2\n");
      fclose(fp);  
      png_destroy_write_struct(&png_ptr,  NULL);  
      return;  
   }  
  
   /* 设置错误处理。如果你在调用 png_create_write_struct() 时没 
    * 有设置错误处理函数，那么这段代码是必须写的。*/  
   if (setjmp(png_jmpbuf(png_ptr)))  
   {  
      /* 如果程序跑到这里了，那么写入文件时出现了问题 */  
	  printf("error3\n");
      fclose(fp);  
      png_destroy_write_struct(&png_ptr, &info_ptr);  
      return;  
   }  
  
   /* 设置输出控制，如果你使用的是 C 的标准 I/O 流 */  
   png_init_io(png_ptr, fp);  
  
   /* 这是一种复杂的做法 */  
  
   /* （必需）在这里设置图像的信息，宽度、高度的上限是 2^31。 
    * bit_depth 取值必需是 1、2、4、8 或者 16, 但是可用的值也依赖于 color_type。 
    * color_type 可选值有： PNG_COLOR_TYPE_GRAY、PNG_COLOR_TYPE_GRAY_ALPHA、 
    * PNG_COLOR_TYPE_PALETTE、PNG_COLOR_TYPE_RGB、PNG_COLOR_TYPE_RGB_ALPHA。 
    * interlace 可以是 PNG_INTERLACE_NONE 或 PNG_INTERLACE_ADAM7, 
    * 而 compression_type 和 filter_type 目前必需是 PNG_COMPRESSION_TYPE_BASE 
    * 和 and PNG_FILTER_TYPE_BASE。 
    */  
   png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA,  
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);  
  
   /* 写入文件头部信息（必需） */  
   png_write_info(png_ptr, info_ptr);  
  
   png_uint_32 k;  
  
   /* 在这个示例代码中，"image" 是一个一维的字节数组（每个元素占一个字节空间） */      
  
   unsigned char* row_pointers[2048];  
  
   if (height > PNG_UINT_32_MAX/(sizeof (png_bytep)))  
     png_error (png_ptr, "Image is too tall to process in memory");  
  
   /* 将这些像素行指针指向你的 "image" 字节数组中对应的位置，即：指向每行像素的起始处 */  
   for (k = 0; k < height; k++)  
     row_pointers[k] = image + k*width*4;  
  
   /* 一次调用就将整个图像写进文件 */  
   png_write_image(png_ptr, row_pointers);  
   /* 必需调用这个函数完成写入文件其余部分 */  
   png_write_end(png_ptr, info_ptr);  
   /* 写完后清理并释放已分配的内存 */  
   png_destroy_write_struct(&png_ptr, &info_ptr);  
   /* 关闭文件 */  
   fclose(fp);  
  
   
}  
