#pragma once
//  Copyright (c) 2009 Karl Blomster
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#ifndef _LIBASS_HELPER_
#define _LIBASS_HELPER_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>

extern "C" {
#include <ass.h>
#include <ass_types.h>
}

#define M_PI   3.14159265358979323846   // pi

#define _R(c)  ((c)>>24)
#define _G(c)  (((c)>>16)&0xFF)
#define _B(c)  (((c)>>8)&0xFF)
#define _A(c)  ((c)&0xFF)

extern TCHAR cur_dll_path[MAX_PATH];

typedef struct RGBAColourD {
	double r, g, b, a;
}RGBAColourD;

typedef struct RGBA {
	unsigned char r, g, b, a;
}RGBA;

typedef struct ARECT {
	int x1, y1, x2, y2;
} ARECT;

//
DWORD GetModulePath(HMODULE hModule, LPTSTR pszBuffer, DWORD dwSize);

char* w2c(const wchar_t* wsp);
char* w2c(const std::wstring ws);
int utf2gbk(char *buf, size_t len);

unsigned char d2b(double);
RGBA color_d2b(RGBAColourD color);
unsigned int color_b2i(RGBA color);
unsigned int color_d2i(RGBAColourD color);

//
class ASS_Image_List {
public:
	ASS_Image_List(ASS_Image* img);
	~ASS_Image_List();

	ASS_Image *img_text = NULL;
	ASS_Image *img_outline = NULL;
	ASS_Image *img_shadow = NULL;
};

//
class AssRender {
private:
	ASS_Library *al = NULL;
	ASS_Renderer *ar = NULL;
	ASS_Track *at = NULL;

	char ass_file[MAX_PATH];
	char fontconf[MAX_PATH];

	double fps = 0;
	int renderWidth = 0;
	int renderHeight = 0;
	int renderDepth = 0;

	ASS_Hinting fonthinting = ASS_HINTING_NONE;
	double fontscale = 1.0;

	ASS_Style default_style;
	int use_defaultstyle = 0;
	char default_fontname[512];
	int default_fontsize = 24;

	RGBA default_fontcolor = { 0,0,0,255 };
	RGBA default_fontcoloralt = { 200,200,200,255 };
	RGBA default_outlinecolor = { 255,255,255,255 };
	RGBA default_backcolor = { 180,180,180,255 };

	int default_bold = 0;
	int default_italic = 0;
	int default_underline = 0;
	int default_strikeout = 0;
	
	int default_scalex = 100;
	int default_scaley = 100;
	int default_spacing = 0;
	double default_angle = 0;
	
	int default_borderstyle = 1;
	int default_outline = 2;
	int default_shadow = 0;
	int default_alignment = 2;
	
	double default_marginl = 0;
	double default_marginr = 0;
	double default_marginv = 0;
	double default_alpha = 100;
	
	int default_encoding = 1;

	int margin = 0;
	double margin_t = 0, margin_b = 0, margin_l = 0, margin_r = 0;
	double spacing = 2, position = 2;

	void msg_callback(const int level, const char *fmt, const va_list args, const void *);
	bool InitLibass(const ASS_Hinting hints, const double scale, const int width, const int height);
	inline RGBA * pixelAddress(RGBA *img, ARECT rect, int x, int y, int bytesPerLine);
	inline bool blend_image(ASS_Image* img, const void* image, bool blend);
public:
	AssRender(void);
	AssRender(ASS_Hinting hints, double scale, const char *charset);
	~AssRender();

	double last_time = 0;

	bool __stdcall Resize(int width, int height);
	bool __stdcall ReScale(double scale);
	bool __stdcall SetFPS(double fr);
	bool __stdcall SetHints(ASS_Hinting hints);

	bool __stdcall SetDefaultStyle(void);
	bool __stdcall SetDefaultStyle(const char * fontname, int fontsize, 
		RGBAColourD color, RGBAColourD coloralt, RGBAColourD outlinecolor, RGBAColourD backcolor,
		int bold, int italic, int underline, int strikeout,
		int bordestyle, int outline, int shadow);

	bool __stdcall SetUseDefaultStyle(int used);
	bool __stdcall SetDefaultFont(const char * fontname, int fontsize);
	bool __stdcall SetDefaultFontName(const char * fontname);
	bool __stdcall SetDefaultFontSize(int fontsize);
	bool __stdcall SetDefaultFontColor(RGBA color);
	bool __stdcall SetDefaultFontColor(RGBAColourD color);
	bool __stdcall SetDefaultFontColorAlt(RGBA color);
	bool __stdcall SetDefaultFontColorAlt(RGBAColourD color);
	bool __stdcall SetDefaultOutlineColor(RGBA color);
	bool __stdcall SetDefaultOutlineColor(RGBAColourD color);
	bool __stdcall SetDefaultBackColor(RGBA color);
	bool __stdcall SetDefaultBackColor(RGBAColourD color);
	bool __stdcall SetDefaultBold(int bold);
	bool __stdcall SetDefaultItalic(int italic);
	bool __stdcall SetDefaultUnderline(int underline);
	bool __stdcall SetDefaultStrikeOut(int strikeout);
	bool __stdcall SetDefaultBorderStyle(int borderstyle);
	bool __stdcall SetDefaultOutline(int outline);
	bool __stdcall SetDefaultShadow(int shadow);

	bool __stdcall SetSpace(int pixels);
	bool __stdcall SetSpace(double percentage);
	bool __stdcall SetPosition(int pixels);
	bool __stdcall SetPosition(double percentage);
	bool __stdcall SetMargin(int used);
	bool __stdcall SetMargin(int t, int b, int l, int r);
	bool __stdcall SetMargin(double t, double b, double l, double r);
	bool __stdcall SetMargin(int used, int t, const int b, int l, int r);
	bool __stdcall SetMargin(int used, double t, double b, double l, double r);

	bool __stdcall LoadAss(const char* assfile, const char *_charset);

	int __stdcall GetAss(double n, int width, int height, int depth, const void* image, bool blend);
	ASS_Image* __stdcall GetAss(double n, int width, int height);
	ASS_Image* __stdcall GetAss(double n, const ASS_Image* src);
	ASS_Image* __stdcall GetAss(int64_t n, const ASS_Image* src);
	ASS_Image* __stdcall GetAss(int n);
	ASS_Image_List* __stdcall GetAss(double n, int width, int height, bool ass_type);
};

#endif
