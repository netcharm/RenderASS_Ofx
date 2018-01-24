#pragma once

#ifndef COMMON_OFX
#define COMMON_OFX

#if defined __APPLE__ || defined linux || defined __FreeBSD__
#  define EXPORT __attribute__((visibility("default")))
#elif defined _WIN32
#  define EXPORT OfxExport
#else
#  error Not building on your operating system quite yet
#endif

#include <locale>
#include <new>
#include <cstring>
#include <stdexcept>
#include <windows.h>
#include <string>
#include <string.h>
#include <mbstring.h>
#include <math.h>
#include <time.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <Shlobj.h>

#include <iconv.h>

#include "ofxCore.h"
#include "ofxDialog.h"
#include "ofxImageEffect.h"
#include "ofxInteract.h"
#include "ofxKeySyms.h"
#include "ofxMemory.h"
#include "ofxMessage.h"
#include "ofxMultiThread.h"
#include "ofxOld.h"
#include "ofxOpenGLRender.h"
#include "ofxParam.h"
#include "ofxParametricParam.h"
#include "ofxPixels.h"
#include "ofxProgress.h"
#include "ofxProperty.h"
#include "ofxTimeLine.h"

#include "../include/ofxUtilities.H"
//#include "../include/ofxsImageBlender.H"
//#include "../include/ofxsProcessing.H"

#define BUFSIZE 1024

#ifndef _R
#define _R(c)  ((c)  >> 24)
#endif
#ifndef _G
#define _G(c)  (((c) >> 16)&0xFF)
#endif
#ifndef _B
#define _B(c)  (((c) >>  8)&0xFF)
#endif
#ifndef _A
#define _A(c)  ((c)        &0xFF)
#endif

// throws this if it can't fetch an image
class NoImageEx {};

enum ContextEnum {
	eIsGenerator,
	eIsFilter,
	eIsPaint,
	eIsGeneral
};

typedef struct OfxPluginInfo {
	const int Version_Majon;
	const int Version_Minor;
	const int Version_Revision;
	const int Version_BuildNo;

	const char* PluginAuthor;
#ifdef _DEBUG
	const char* PluginLabel;
#else
	const char* PluginLabel;
#endif
	const char* PluginDescription;
#ifdef _DEBUG
	const char* PluginIdentifier;
#else
	const char* PluginIdentifier;
#endif
}OfxPluginInfo;

typedef struct OfxTimeLineSuiteV2 {
	// converts what host displays in it's user interface to local effect time, could be a large number if host project starts at 12:00:00:00)
	OfxStatus(*getProjectTime)(void *instance, double EffectTime, double *ProjectTime);
	//  for example in an NLE this refers to In and out point handles of the video track on which the effect is applied, this is in effects local time. This is different then frame range and 0 to Duration.
	OfxStatus(*getEffectTrimPoints)(void *instance, double *InPoint, double *OutPoint);
	// this is in effects local time, if one asks to go to time -5000, it might not be defined
	// because of this not being supported a lot, this is example of wanting to check if function pointer is NULL as means of seeing if supported
	OfxStatus(*gotoEffectTime)(void *instance, double *time);  
} OfxTimeLineSuiteV2;

struct Times {
	double Current;
	double Min;
	double Max;
};

char* w2c(const wchar_t* wsp);
char* w2c(const std::wstring ws);
bool s2c(const std::string s, char* c);
int utf2gbk(char *buf, size_t len);
int gbk2utf(char *buf, size_t len);
char* gbk(char *buf);
char* utf(char *buf);

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

// pointers to various bits of the host
OfxHost                           *gHost;
OfxImageEffectSuiteV1             *gEffectHost = 0;
OfxPropertySuiteV1                *gPropHost = 0;
OfxParameterSuiteV1               *gParamHost = 0;
OfxParametricParameterSuiteV1     *gParametricParameterSuite = 0;
OfxInteractSuiteV1                *gInteractHost = 0;
OfxImageEffectOpenGLRenderSuiteV1 *gOpenGLRenderSuite = 0;
OfxMultiThreadSuiteV1             *gThreadHost = 0;
OfxMemorySuiteV1                  *gMemoryHost = 0;
OfxMessageSuiteV1                 *gMessageSuite = 0;
OfxMessageSuiteV2                 *gMessageSuiteV2 = 0;
OfxProgressSuiteV1                *gProgressSuiteV1 = 0;
OfxProgressSuiteV2                *gProgressSuiteV2 = 0;
OfxTimeLineSuiteV1                *gTimeLineHost1 = 0;
OfxTimeLineSuiteV2                *gTimeLineHost2 = 0;

// look up a pixel in the image, does bounds checking to see if it is in the image rectangle
inline OfxRGBAColourB * pixelAddress(OfxRGBAColourB *img, OfxRectI rect, int x, int y, int bytesPerLine) {
	if (x < rect.x1 || x >= rect.x2 || y < rect.y1 || y > rect.y2)
		return 0;
	OfxRGBAColourB *pix = (OfxRGBAColourB *)(((char *)img) + (y - rect.y1) * bytesPerLine);
	pix += x - rect.x1;
	return pix;
}

inline errno_t copy_source(OfxImageEffectHandle instance,
	const OfxRectI rw,
	const void *sp, const OfxRectI sr, const int srb,
	const void *dp, const OfxRectI dr, const int drb) {

	if(sp && dp) 
		return(memcpy_s(&dp, srb*(sr.y2 - sr.y1), sp, drb*(dr.y2 - dr.y1)));
	else if (dp) {
		memset(&dp, 0, drb*(dr.y2 - dr.y1));
		return 0;
	}
	return 1;
	/*
	// cast data pointers to 8 bit RGBA
	OfxRGBAColourB *src = (OfxRGBAColourB *)sp;
	OfxRGBAColourB *dst = (OfxRGBAColourB *)dp;

	// and do some inverting
	for (int y = rw.y1; y < rw.y2; y++) {
		if (gEffectHost->abort(instance)) break;

		OfxRGBAColourB *dstPix = pixelAddress(dst, dr, rw.x1, y, drb);

		for (int x = rw.x1; x < rw.x2; x++) {

			OfxRGBAColourB *srcPix = pixelAddress(src, sr, x, y, srb);

			if (src && srcPix) {
				dstPix->r = srcPix->r;
				dstPix->g = srcPix->g;
				dstPix->b = srcPix->b;
				dstPix->a = srcPix->a;
			}
			else {
				dstPix->r = 0;
				dstPix->g = 0;
				dstPix->b = 0;
				dstPix->a = 0;
			}
			dstPix++;
		}
	}
	*/
}

static OfxPlugin * GetRenderASS(void);

EXPORT OfxPlugin * OfxGetPlugin(int nth)
{
	if (nth == 0)
		//return RenderASS::GetPlugin();
		return GetRenderASS();
	return 0;
}

EXPORT int OfxGetNumberOfPlugins(void)
{
	return 1;
}

#endif
