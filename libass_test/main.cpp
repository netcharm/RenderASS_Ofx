#include <cstring>
#include <stdexcept>
#include <new>
#include <iostream>
#include <tchar.h>
#include <windows.h>

#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <libgnuintl.h>
#include <iconv.h>
#include <ass.h>
#include "libass_helper.h"

#using "System.Drawing.dll"
#using "WindowsBase.dll"

using namespace System;
using namespace System::Drawing;
using namespace System::IO;
using namespace System::Runtime::InteropServices;

/*使用gettext通常使用类似下面的一个带函数的宏定义
*你完全可以不用，直接使用 gettext(字符串)
*/
#define _(S) gettext(S)

/*PACKAGE是本程序最终的名字（运行时输入的命令）*/
#define PACKAGE "RenderASS"

const int bDepth = 4; /// red, green, blue
const int fileHeaderSize = 14;
const int infoHeaderSize = 108;
const int bWidth = 1280;
const int bHeight = 720;

void changePixelColorOrder(const unsigned char *image, int height, int width);
unsigned char* createBitmapFileHeader(int height, int width);
array<unsigned char>^ createBitmapFileHeaderClr(int height, int width);
unsigned char* createBitmapInfoHeader(int height, int width);
array<unsigned char>^  createBitmapInfoHeaderClr(int height, int width);
void generateBitmapImage(const unsigned char *image, int height, int width, char* imageFileName);
Image^ generateBitmapImageClr(unsigned char *image, int height, int width);

void generateBitmapImage(const unsigned char *image, int height, int width, char* imageFileName) {

	unsigned char* fileHeader = createBitmapFileHeader(height, width);
	unsigned char* infoHeader = createBitmapInfoHeader(height, width);
	unsigned char padding[3] = { 0, 0, 0 };
	int paddingSize = (4 - (width*bDepth) % 4) % 4;

	//FILE* imageFile = fopen(imageFileName, "wb"); 
	FILE* imageFile;
	errno_t err; 
	err = fopen_s(&imageFile, imageFileName, "wb");

	fwrite(fileHeader, 1, fileHeaderSize, imageFile);
	fwrite(infoHeader, 1, infoHeaderSize, imageFile);

	for (int i = 0; i<height; i++) {
		fwrite(image + (i*width*bDepth), bDepth, width, imageFile);
		fwrite(padding, 1, paddingSize, imageFile);
	}

	fclose(imageFile);
}

Image^ generateBitmapImageClr(unsigned char *image, int height, int width) {

	int lineSize = width*bDepth;
	int imageSize = lineSize*height;
	int paddingSize = (4 - (lineSize) % 4) % 4;

	array<unsigned char>^ fileHeader = createBitmapFileHeaderClr(height, width);
	array<unsigned char>^ infoHeader = createBitmapInfoHeaderClr(height, width);
	array<unsigned char>^ padding = gcnew array<unsigned char>(3) { 0, 0, 0 };

	array<unsigned char> ^lines = gcnew array<unsigned char>(imageSize);
	Marshal::Copy((IntPtr)image, lines, 0, imageSize);

	MemoryStream ^ms = gcnew MemoryStream();
	//ms->Capacity = fileHeader->Length + infoHeader->Length + imageSize + height*paddingSize;

	ms->Write(fileHeader, 0, fileHeader->Length);
	ms->Write(infoHeader, 0, infoHeader->Length);

	for (int i = 0; i<height; i++) {
		try {
			ms->Write(lines, i*lineSize, lineSize);
			ms->Write(padding, 1, paddingSize);
		}
		finally
		{
			// Free the unmanaged memory.
		}	
	}
	ms->Seek(0, SeekOrigin::Begin);
	//Image^ img = Bitmap::FromStream(ms);
	Bitmap^ img = gcnew Bitmap(ms);
	//Bitmap^ bmp = gcnew Bitmap(img->Width, img->Height, Imaging::PixelFormat::Format32bppArgb);
	//Graphics^ g = Graphics::FromImage(bmp);
	////g->Clear(Color::Transparent);
	////g->FillRectangle(gcnew SolidBrush(Color::Transparent), 0, 0, bmp->Width, bmp->Height);
	//g->DrawImage(img, 0, 0);
	//delete g;

	//Bitmap::Bitmap(ms, true, true);
	return img;
	//return Bitmap::Bitmap(ms);
}

unsigned char* createBitmapFileHeader(int height, int width) {
	int fileSize = fileHeaderSize + infoHeaderSize + bDepth*height*width;

	static unsigned char fileHeader[] = {
		0,0, /// signature
		0,0,0,0, /// image file size in bytes
		0,0,0,0, /// reserved
		0,0,0,0, /// start of pixel array
	};

	fileHeader[0] = (unsigned char)('B');
	fileHeader[1] = (unsigned char)('M');
	fileHeader[2] = (unsigned char)(fileSize);
	fileHeader[3] = (unsigned char)(fileSize >> 8);
	fileHeader[4] = (unsigned char)(fileSize >> 16);
	fileHeader[5] = (unsigned char)(fileSize >> 24);
	fileHeader[10] = (unsigned char)(fileHeaderSize + infoHeaderSize);

	return fileHeader;
}

array<unsigned char>^ createBitmapFileHeaderClr(int height, int width) {
	array<unsigned char>^ fileHeader = gcnew array<unsigned char>(fileHeaderSize) {
		0,0, /// signature
		0,0,0,0, /// image file size in bytes
		0,0,0,0, /// reserved
		0,0,0,0, /// start of pixel array
	};

	int fileSize = fileHeaderSize + infoHeaderSize + bDepth*height*width;

	fileHeader[0] = (unsigned char)('B');
	fileHeader[1] = (unsigned char)('M');
	fileHeader[2] = (unsigned char)(fileSize);
	fileHeader[3] = (unsigned char)(fileSize >> 8);
	fileHeader[4] = (unsigned char)(fileSize >> 16);
	fileHeader[5] = (unsigned char)(fileSize >> 24);
	fileHeader[10] = (unsigned char)(fileHeaderSize + infoHeaderSize);

	return fileHeader;
}

unsigned char* createBitmapInfoHeader(int height, int width) {
	static unsigned char infoHeader[] = {
		0,0,0,0, /// header size
		0,0,0,0, /// image width
		0,0,0,0, /// image height
		0,0, /// number of color planes
		0,0, /// bits per pixel
		0,0,0,0, /// compression
		0,0,0,0, /// image size
		0,0,0,0, /// horizontal resolution
		0,0,0,0, /// vertical resolution
		0,0,0,0, /// colors in color table
		0,0,0,0, /// important color count
	};

	infoHeader[0] = (unsigned char)(infoHeaderSize);
	infoHeader[4] = (unsigned char)(width);
	infoHeader[5] = (unsigned char)(width >> 8);
	infoHeader[6] = (unsigned char)(width >> 16);
	infoHeader[7] = (unsigned char)(width >> 24);
	infoHeader[8] = (unsigned char)(height);
	infoHeader[9] = (unsigned char)(height >> 8);
	infoHeader[10] = (unsigned char)(height >> 16);
	infoHeader[11] = (unsigned char)(height >> 24);
	infoHeader[12] = (unsigned char)(1);
	infoHeader[14] = (unsigned char)(bDepth * 8);

	return infoHeader;
}

array<unsigned char>^  createBitmapInfoHeaderClr(int height, int width) {
	array<unsigned char>^  infoHeader = gcnew array<unsigned char>(infoHeaderSize) {
		0,0,0,0, /// header size
		0,0,0,0, /// image width
		0,0,0,0, /// image height
		0,0, /// number of color planes
		0,0, /// bits per pixel
		0,0,0,0, /// compression
		0,0,0,0, /// image size
		0,0,0,0, /// horizontal resolution
		0,0,0,0, /// vertical resolution
		0,0,0,0, /// colors in color table
		0,0,0,0, /// important color count
		
			0, 0, 0xFF, 0, /// Red channel bit mask (valid because BI_BITFIELDS is specified)
			0, 0xFF, 0, 0, /// Green channel bit mask (valid because BI_BITFIELDS is specified) 
			0xFF, 0, 0, 0, /// Blue channel bit mask (valid because BI_BITFIELDS is specified)
			0, 0, 0, 0xFF, /// Alpha channel bit mask 	
			0x20, 0x6E, 0x69, 0x57, /// LCS_WINDOWS_COLOR_SPACE 
			0, 0, 0, 0, 0, 0, 0, 0, 0, /// CIEXYZTRIPLE Color Space endpoints Unused for LCS "Win " or "sRGB" 
			0, 0, 0, 0, /// Red Gamma Unused for LCS "Win " or "sRGB"
			0, 0, 0, 0, /// Green Gamma Unused for LCS "Win " or "sRGB" 
			0, 0, 0, 0, /// Blue Gamma Unused for LCS "Win " or "sRGB"
	};

	int lineSize = width*bDepth;
	int paddingSize = (4 - (lineSize) % 4) % 4;
	int imageSize = (lineSize+paddingSize)*height;

	infoHeader[0] = (unsigned char)(infoHeaderSize);
	
	infoHeader[4] = (unsigned char)(width);
	infoHeader[5] = (unsigned char)(width >> 8);
	infoHeader[6] = (unsigned char)(width >> 16);
	infoHeader[7] = (unsigned char)(width >> 24);
	
	infoHeader[8] = (unsigned char)(height);
	infoHeader[9] = (unsigned char)(height >> 8);
	infoHeader[10] = (unsigned char)(height >> 16);
	infoHeader[11] = (unsigned char)(height >> 24);
	
	infoHeader[12] = (unsigned char)(1);
	
	infoHeader[14] = (unsigned char)(bDepth * 8);
	
	infoHeader[16] = (unsigned char)(3);
	
	infoHeader[20] = (unsigned char)(imageSize);
	infoHeader[21] = (unsigned char)(imageSize >> 8);
	infoHeader[22] = (unsigned char)(imageSize >> 16);
	infoHeader[23] = (unsigned char)(imageSize >> 32);
	
	infoHeader[24] = (unsigned char)(0x13);
	infoHeader[25] = (unsigned char)(0x0B);

	infoHeader[28] = (unsigned char)(0x13);
	infoHeader[29] = (unsigned char)(0x0B);

	return infoHeader;
}

void changePixelColorOrder(const unsigned char *image, int height, int width) {
	int dstRowBytes = width*bDepth;

	OfxRGBAColourB *dst = (OfxRGBAColourB *)image;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			try
			{
				unsigned int a = dst->a;
				unsigned int r = dst->r;
				unsigned int g = dst->g;
				unsigned int b = dst->b;

				dst->a = a; ///alpha
				dst->r = b; ///red
				dst->g = g; ///green
				dst->b = r; ///blue				
			}
			catch (const std::exception&)
			{

			}
			dst++;
		}
	}
}

// look up a pixel in the image, does bounds checking to see if it is in the image rectangle
inline OfxRGBAColourB *
pixelAddress(OfxRGBAColourB *img, OfxRectI rect, int x, int y, int bytesPerLine)
{
	if (x < rect.x1 || x >= rect.x2 || y < rect.y1 || y > rect.y2)
		return 0;
	OfxRGBAColourB *pix = (OfxRGBAColourB *)(((char *)img) + (y - rect.y1) * bytesPerLine);
	pix += x - rect.x1;
	return pix;
}

inline
bool blend_image(ASS_Image* img, const void* image) {

	int dstRowBytes = bWidth*bDepth;
	OfxRectI dstRect;
	dstRect.x1 = 0;
	dstRect.x2 = bWidth;
	dstRect.y1 = 0;
	dstRect.y2 = bHeight;

	OfxRectI dstRectAss;
	dstRectAss.x1 = img->dst_x;
	dstRectAss.x2 = img->dst_x + img->w;
	dstRectAss.y1 = bHeight - (img->dst_y + img->h);
	dstRectAss.y2 = bHeight - img->dst_y;

	const unsigned char *src_map = img->bitmap;
	unsigned int ok, ak, sk;

	unsigned int stride = (unsigned int)img->stride;
	unsigned int imglen = (unsigned int)(stride*img->h);
	unsigned int a = 255 - (_A(img->color));
	unsigned int r = (unsigned int)_R(img->color);
	unsigned int g = (unsigned int)_G(img->color);
	unsigned int b = (unsigned int)_B(img->color);
	
	OfxRGBAColourB *dst = (OfxRGBAColourB *)image;
	for (int y = 0; y < bHeight; y++) {
		if (y < dstRectAss.y1 || y > dstRectAss.y2) continue;
		OfxRGBAColourB *dstPix = pixelAddress(dst, dstRect, 0, y, dstRowBytes);		
		dstPix += dstRectAss.x1;
		for (int x = dstRectAss.x1; x < dstRectAss.x2; x++) {
			try
			{
				long idx_x = x - dstRectAss.x1;
				long idx_y = img->h - (y - dstRectAss.y1) - 1;

				if ( x >= dstRectAss.x1 && x < dstRectAss.x2 && y >= dstRectAss.y1 && y < dstRectAss.y2)
				{
					unsigned long idx = idx_x + idx_y*stride;
					if (idx >= imglen) break;
					ok = (unsigned)src_map[idx];
					ak = ok*a / 255;
					sk = 255 - ak;

					dstPix->a = (unsigned char)sk; ///alpha
					dstPix->r = (unsigned char)((ak*r + sk*dstPix->r) / 255); ///red
					dstPix->g = (unsigned char)((ak*g + sk*dstPix->g) / 255); ///green
					dstPix->b = (unsigned char)((ak*b + sk*dstPix->b) / 255); ///blue				
				}
				else {
					dstPix->a = 0x00; ///alpha
					dstPix->r = 0x00; ///red
					dstPix->g = 0x00; ///green
					dstPix->b = 0x00; ///blue				
				}
			}
			catch (const std::exception&)
			{

			}
			dstPix++;
		}
		dstPix += bWidth - dstRectAss.x2;
	}
	return true;
}

int main(int args, char** argv) {

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, "locale");
	textdomain(PACKAGE);

	printf(_("ASS File"));

	//_fullpath(cur_dll_path, argv[0], sizeof(cur_dll_path));
	char* buffer;
	buffer = _getcwd(NULL, 0);
	strcat(buffer, "\\");
	//strcat_s(buffer, 1, "\\\0");
	strcpy_s(cur_dll_path, buffer);

	unsigned int count = bHeight*bWidth*bDepth;
	const char* buf;
	buf = (const char*)calloc(count, sizeof(const char));
	
	AssRender* ass = new AssRender(ASS_HINTING_NONE, 1.0, "UTF-8");
	ass->LoadAss("test.ass", "UTF-8");
	ass->SetFPS(25);
	for (int i = 0; i < 25; i++) {
		//memset(buf, 0, count);
		OfxRGBAColourB *image = (OfxRGBAColourB *)buf;

		for (int h = 0; h<bHeight; h++) {
			for (int w = 0; w<bWidth; w++) {
				image->a = 0x00; ///alpha
				image->r = 0x00; ///red
				image->g = 0x00; ///green
				image->b = 0x00; ///blue
				image++;
			}
		}

		int c = ass->GetAss((double)i, bWidth, bHeight, bDepth, buf, true);

		if (c > 0) {
			changePixelColorOrder((unsigned char *)buf, bHeight, bWidth);

			//char bName[MAX_PATH] = "";
			//sprintf(bName, "ass_%06d_%02d.bmp", i + 1, c);
			//generateBitmapImage((unsigned char *)buf, bHeight, bWidth, bName);

			char pName[MAX_PATH] = "";
			sprintf(pName, "ass_%06d_%02d.png", i + 1, c);
			Image^ img = generateBitmapImageClr((unsigned char *)buf, bHeight, bWidth);
			img->Save(Marshal::PtrToStringAnsi((IntPtr)(char *)pName), Imaging::ImageFormat::Png);
		}
	}
	free((void*)buf);

	std::cout << std::endl <<  _("Press enter key exit...") << std::endl;
	getchar();
	return 0;
}