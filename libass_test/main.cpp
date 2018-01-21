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


/*使用gettext通常使用类似下面的一个带函数的宏定义
*你完全可以不用，直接使用 gettext(字符串)
*/
#define _(S) gettext(S)

/*PACKAGE是本程序最终的名字（运行时输入的命令）*/
#define PACKAGE "RenderASS"

#define BUFSIZE 1024

#define _R(c)  ((c)>>24)
#define _G(c)  (((c)>>16)&0xFF)
#define _B(c)  (((c)>>8)&0xFF)
#define _A(c)  ((c)&0xFF)

TCHAR cur_dll_path[MAX_PATH] = ""; // hack

const int bDepth = 4; /// red, green, blue
const int fileHeaderSize = 14;
const int infoHeaderSize = 40;
const int bWidth = 1280;
const int bHeight = 720;

/** @brief Defines an 8 bit per component RGBA pixel */
typedef struct OfxRGBAColourB {
	//unsigned char r, g, b, a;
	unsigned char b, g, r, a;
	//unsigned char a, r, g, b;
	//unsigned char a, b, g, r;
	//unsigned char a, g, r, b;
}OfxRGBAColourB;

typedef struct OfxRectI {
	int x1, y1, x2, y2;
} OfxRectI;

char* w2c(const wchar_t* wsp) {
	size_t size = wcslen(wsp) * 2 + 2;
	char * csp = new char[size];
	size_t c_size;
	wcstombs_s(&c_size, csp, size, wsp, size);
	return(csp);
}

char* w2c(const std::wstring ws) {
	const wchar_t *wsp = ws.c_str();
	size_t size = wcslen(wsp) * 2 + 2;
	char * csp = new char[size];
	size_t c_size;
	wcstombs_s(&c_size, csp, size, wsp, size);
	return(csp);
}

bool s2c(const std::string s, char* c) {
	memset(c, 0, sizeof(c));
	for (unsigned int i = 0; i < s.length(); i++) {
		c[i] = s[i];
	}
	return true;
}

int utf2gbk(char *buf, size_t len)
{
	iconv_t cd = iconv_open("GBK", "UTF-8");
	if (cd == (iconv_t)-1) {
		perror("获取字符转换描述符失败！\n");
		return -1;
	}
	size_t sz = BUFSIZE * BUFSIZE;
	char *tmp_str = (char *)malloc(sz);
	// 不要将原始的指针传进去，那样会改变原始指针的  
	size_t inlen = len;
	size_t outlen = sz;
	const char *in = buf;
	char* out = tmp_str;
	if (tmp_str == NULL) {
		iconv_close(cd);
		fprintf(stderr, "分配内存失败！\n");
		return -1;
	}
	memset(tmp_str, 0, sz);
	if (iconv(cd, &in, &inlen, &out, &outlen) == (size_t)-1) {
		iconv_close(cd);
		return -1;
	}
	strcpy_s(buf, MAX_PATH, tmp_str);
	iconv_close(cd);
	return 0;
}

char* gbk(char *buf)
{
	iconv_t cd = iconv_open("GBK", "UTF-8");
	if (cd == (iconv_t)-1) {
		perror("获取字符转换描述符失败！\n");
		return buf;
	}
	size_t sz = BUFSIZE * BUFSIZE;
	char *tmp_str = (char *)malloc(sz);
	// 不要将原始的指针传进去，那样会改变原始指针的  
	//size_t inlen = len;
	size_t inlen = strlen(buf) + 1;
	size_t outlen = sz;
	const char *in = buf;
	char* out = tmp_str;
	if (tmp_str == NULL) {
		iconv_close(cd);
		fprintf(stderr, "分配内存失败！\n");
		return buf;
	}
	memset(tmp_str, 0, sz);
	if (iconv(cd, &in, &inlen, &out, &outlen) == (size_t)-1) {
		iconv_close(cd);
		return buf;
	}
	//strcpy_s(buf, MAX_PATH, tmp_str);
	iconv_close(cd);
	return tmp_str;
}

int gbk2utf(char *buf, size_t len)
{
	iconv_t cd = iconv_open("UTF-8", "GBK");
	if (cd == (iconv_t)-1) {
		perror("获取字符转换描述符失败！\n");
		return -1;
	}
	size_t sz = BUFSIZE * BUFSIZE;
	char *tmp_str = (char *)malloc(sz);
	// 不要将原始的指针传进去，那样会改变原始指针的  
	size_t inlen = len;
	size_t outlen = sz;
	const char *in = buf;
	char* out = tmp_str;
	if (tmp_str == NULL) {
		iconv_close(cd);
		fprintf(stderr, "分配内存失败！\n");
		return -1;
	}
	memset(tmp_str, 0, sz);
	if (iconv(cd, &in, &inlen, &out, &outlen) == (size_t)-1) {
		iconv_close(cd);
		return -1;
	}
	strcpy_s(buf, MAX_PATH, tmp_str);
	iconv_close(cd);
	return 0;
}

char* utf(char *buf) {
	iconv_t cd = iconv_open("UTF-8", "GBK");
	if (cd == (iconv_t)-1) {
		perror("获取字符转换描述符失败！\n");
		return buf;
	}
	size_t sz = BUFSIZE * BUFSIZE;
	char *tmp_str = (char *)malloc(sz);
	// 不要将原始的指针传进去，那样会改变原始指针的  
	//size_t inlen = len;
	size_t inlen = strlen(buf) + 1;
	size_t outlen = sz;
	const char *in = buf;
	char* out = tmp_str;
	if (tmp_str == NULL) {
		iconv_close(cd);
		fprintf(stderr, "分配内存失败！\n");
		return buf;
	}
	memset(tmp_str, 0, sz);
	if (iconv(cd, &in, &inlen, &out, &outlen) == (size_t)-1) {
		iconv_close(cd);
		return buf;
	}
	//strcpy_s(buf, MAX_PATH, tmp_str);
	iconv_close(cd);
	return tmp_str;
}

void generateBitmapImage(const unsigned char *image, int height, int width, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int width);
unsigned char* createBitmapInfoHeader(int height, int width);
void changePixelColorOrder(const unsigned char *image, int height, int width);

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
	for (int i = 0; i < 50; i++) {
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
			char bName[MAX_PATH] = "";
			sprintf(bName, "ass_%06d_%02d.bmp", i + 1, c);
			changePixelColorOrder((unsigned char *)buf, bHeight, bWidth);
			generateBitmapImage((unsigned char *)buf, bHeight, bWidth, bName);
		}
	}
	free((void*)buf);

	std::cout << std::endl <<  _("Press enter key exit...") << std::endl;
	getchar();
	return 0;
}