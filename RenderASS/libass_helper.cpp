#pragma once
#include "stdafx.h"

// importing libass
#include <shlobj.h>
#include <tchar.h>
#include <time.h>
#include <string>
#include <locale>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <mbstring.h>
#include <math.h>
#include <png.h>

#include <cstring>
#include <stdexcept>
#include <new>
#include <iostream>
#include <tchar.h>
#include <windows.h>

#include <direct.h>
#include <malloc.h>
#include <libgnuintl.h>
#include <iconv.h>

#include "libass_helper.h"
using namespace System;
using namespace System::IO;
using namespace System::Runtime::InteropServices;
// subtitles_helper/bin/Debug/subtitles_helper.dll
//#using "subtitles_helper.dll"
//using namespace SubTitles;

#using "System.Drawing.dll"

using namespace System;
using namespace System::Drawing;
using namespace System::IO;
using namespace System::Runtime::InteropServices;

//#ifndef cur_dll_path
TCHAR cur_dll_path[MAX_PATH];// = ""; // hack
//#endif

void msg_callback(int level, const char *fmt, va_list args, void *) {
	if (level >= 7) return;
	char buf[1024];
#ifdef _WIN32
	vsprintf_s(buf, sizeof(buf), fmt, args);
#else
	vsnprintf(buf, sizeof(buf), fmt, args);
#endif
	//if (level < 2) // warning/error
	//	LOG_I("subtitle/provider/libass") << buf;
	//else // verbose
	//	LOG_D("subtitle/provider/libass") << buf;
}

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

RGBA color_d2b(RGBAColourD color)
{
	return{ d2b(color.r * 255), d2b(color.g * 255), d2b(color.b * 255), d2b(color.a * 255) };
}

uint32_t color_b2i(RGBA color)
{
	uint32_t r = (color.r << 24) & 0xFF000000;
	uint32_t g = (color.g << 16) & 0x00FF0000;
	uint32_t b = (color.b <<  8) & 0x0000FF00;
	uint32_t a = (255 - color.a) & 0x000000FF;
	return(r + g + b + a);
	//return( + (color.g << 16) & 0x00FF0000 + (color.b << 8) & 0x0000FF00 + (255 - color.a) & 0x000000FF);
	return(((255 - color.r) << 24) + ((255 - color.g) << 16) + ((255 - color.b) << 8) + (255 - color.a));
}

uint32_t color_d2i(RGBAColourD color)
{
	return(color_b2i(color_d2b(color)));
	//return((d2b(color.a * 255) << 24) + (d2b(color.b * 255) << 16) + (d2b(color.g * 255) << 8) + d2b(color.r * 255));
}

//
ASS_Image_List::ASS_Image_List(ASS_Image * img)
{
	int img_count = 0;
	while (img) {
		img_count++;
		try
		{
			if ((__int64)img >= 0xcccc0000) break;
			if (img->w <= 0 || img->h <= 0 || img->w > 8192 || img->h > 8192) {
				img = img->next;
				continue;
			}

			switch (img->type)
			{
			case ASS_Image::IMAGE_TYPE_CHARACTER:
				img_text = img;
				break;
			case ASS_Image::IMAGE_TYPE_OUTLINE:
				img_outline = img;
				break;
			case ASS_Image::IMAGE_TYPE_SHADOW:
				img_shadow = img;
				break;
			default:
				break;
			}

			if (img->next && (__int64)img->next < 0xcccccccccccc0000) {
				if (!img->next->w || !img->next->h) break;
				if ((__int64)img->next->w > 0xcccccccccccc0000 ||
					(__int64)img->next->h > 0xcccccccccccc0000) break;
			}
			else break;
			Sleep(10);
			img = img->next;
		}
		catch (const std::exception&)
		{

		} {}
	}
}

//
ASS_Image_List::~ASS_Image_List()
{
	img_text = NULL;
	img_outline = NULL;
	img_shadow = NULL;
	//if (img_text) delete img_text;
	//if (img_outline) delete img_outline;
	//if (img_shadow) delete img_shadow;
}

//
// 
//
AssRender::AssRender(void) {
	memset(ass_file, 0, MAX_PATH);
	if (!InitLibass(ASS_HINTING_NONE, 1.0, 100, 60)) {
		//throw("AssRender: failed to initialize libass");
	}
}

AssRender::AssRender(ASS_Hinting hints, double scale, const char *charset) {
	memset(ass_file, 0, MAX_PATH);
	if (!InitLibass(hints, scale, 100, 60))
	{
		//throw("AssRender: failed to initialize libass");
	}		
}

AssRender::~AssRender() {
	if(at) ass_free_track(at);
	if(ar) ass_renderer_done(ar);
	if(al) ass_library_done(al);
}

// look up a pixel in the image, does bounds checking to see if it is in the image rectangle
inline RGBA *AssRender::pixelAddress(RGBA *img, ARECT rect, int x, int y, int bytesPerLine)
{
	if (x < rect.x1 || x >= rect.x2 || y < rect.y1 || y > rect.y2)
		return 0;
	RGBA *pix = (RGBA *)(((char *)img) + (y - rect.y1) * bytesPerLine);
	pix += x - rect.x1;
	return pix;
}

// render ass image to source clip frame
inline bool AssRender::blend_image(ASS_Image* img, const void* image, bool blend=true) {

	int dstRowBytes = renderWidth*renderDepth;
	ARECT dstRect;
	dstRect.x1 = 0;
	dstRect.x2 = renderWidth;
	dstRect.y1 = 0;
	dstRect.y2 = renderHeight;

	ARECT dstRectAss;
	dstRectAss.x1 = img->dst_x;
	dstRectAss.x2 = img->dst_x + img->w;
	dstRectAss.y1 = renderHeight - (img->dst_y + img->h);
	dstRectAss.y2 = renderHeight - img->dst_y;

	const unsigned char *src_map = img->bitmap;
	unsigned int ok, ak, sk;

	unsigned int stride = (unsigned int)img->stride;
	unsigned int imglen = (unsigned int)(stride*img->h);
	unsigned int a = 255 - (_A(img->color));
	unsigned int r = (unsigned int)_R(img->color);
	unsigned int g = (unsigned int)_G(img->color);
	unsigned int b = (unsigned int)_B(img->color);
	
	RGBA *dst = (RGBA *)image;
	for (int y = 0; y < renderHeight; y++) {
		if (y < dstRectAss.y1 || y > dstRectAss.y2) continue;
		RGBA *dstPix = pixelAddress(dst, dstRect, 0, y, dstRowBytes);		
		//dstPix += dstRectAss.x1;
		//for (int x = dstRectAss.x1; x < dstRectAss.x2; x++) {
		for (int x = 0; x < renderWidth; x++) {
			try
			{
				long idx_x = x - dstRectAss.x1;
				long idx_y = img->h - (y - dstRectAss.y1) - 1;

				if (dst && dstPix && x >= dstRectAss.x1 && x < dstRectAss.x2 && y >= dstRectAss.y1 && y < dstRectAss.y2)
				{
					unsigned long idx = idx_x + idx_y*stride;
					if (idx >= imglen) break;
					ok = (unsigned)src_map[idx];
					ak = ok*a / 255;
					if (blend) {
						sk = 255 - ak;
						dstPix->a = (unsigned char)ak;
						dstPix->b = (unsigned char)((ak*b + sk*dstPix->b) / 255);
						dstPix->g = (unsigned char)((ak*g + sk*dstPix->g) / 255);
						dstPix->r = (unsigned char)((ak*r + sk*dstPix->r) / 255);
					} else {
						dstPix->a = (unsigned char)ak;
						dstPix->b = b;
						dstPix->g = g;
						dstPix->r = r;
					}
				}
			}
			catch (const std::exception&)
			{

			}
			dstPix++;
		}
		//dstPix += renderWidth - dstRectAss.x2;
	}
	return true;
}

bool AssRender::InitLibass(ASS_Hinting hints, double scale, int width, int height) {
	al = ass_library_init();
	if (!al) return false;

	//ass_set_message_cb(al, msg_callback, nullptr);

	//char tmp[MAX_PATH];
	//SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, tmp);

	// not needed?
	//ass_set_fonts_dir(al, tmp);
	//ass_set_fonts_dir(al, "C:\\Windows\\Fonts");
	//ass_set_extract_fonts(al, 2);
	ass_set_style_overrides(al, NULL);

	ar = ass_renderer_init(al);
	if (!ar) return false;

	ass_set_frame_size(ar, width, height);
	//ass_set_margins(ar, 0, 0, 0, 0);
	//ass_set_use_margins(ar, 0);
	//ass_set_aspect_ratio(ar,);  // todo: implement this
	ass_set_line_position(ar, position);
	ass_set_line_spacing(ar, spacing);
	//ass_set_shaper(ar, ASS_SHAPING_COMPLEX);

	std::string path(&cur_dll_path[0]);
	path.append(_T("fontconfig\\fonts.conf"));
	s2c(path, fontconf);

	memset(default_fontname, 0, 512);
	strcpy_s(default_fontname, "Arial");

	ass_set_hinting(ar, hints);
	ass_set_font_scale(ar, scale);
	ass_set_fonts(ar, default_fontname, "Sans", 1, fontconf, 1);
	
	ass_fonts_update(ar);
	
	// Key: if comment this line, maybe crashed the host app
	//ass_set_cache_limits(ar, 2048, 64);
	ass_set_cache_limits(ar, 0, 0);

	//ass_get_available_font_providers(al, )

	SetDefaultStyle();
	return true;
}

bool AssRender::SetGlobalBackColor(RGBA color)
{
	global_backcolor = color;
	return true;
}

bool AssRender::SetGlobalBackColor(RGBAColourD color)
{
	return SetGlobalBackColor(color_d2b(color));
}

bool AssRender::SetDefaultFont(const char * fontname, int fontsize)
{
	if (!ar) return false;
	return SetDefaultFontName(fontname);
	return true;
}

bool AssRender::SetDefaultFontName(const char * fontname)
{
	if (!ar) return false;
	if (strlen(fontname) > 0 && strcmp(fontname, default_fontname) != 0) {
		memset(default_fontname, 0, 512);
		strcpy_s(default_fontname, fontname);
		ass_set_fonts(ar, default_fontname, NULL, 1, fontconf, 1);
		ass_fonts_update(ar);
		default_style->FontName = default_fontname;
		if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	}
	return false;
}

bool AssRender::SetDefaultFontSize(int fontsize)
{
	if (fontsize > 0 && fontsize < 256) {
		default_fontsize = fontsize;
		default_style->FontSize = fontsize;
		if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
		return true;
	}
	return false;
}

bool AssRender::SetDefaultFontColor(RGBA color)
{
	default_fontcolor = color;	
	default_style->PrimaryColour = color_b2i(color);
	if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	return true;
}

bool AssRender::SetDefaultFontColor(RGBAColourD color)
{
	return SetDefaultFontColor(color_d2b(color));
}

bool AssRender::SetDefaultFontColorAlt(RGBA color)
{
	default_fontcoloralt = color;
	default_style->SecondaryColour = color_b2i(color);
	if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	return true;
}

bool AssRender::SetDefaultFontColorAlt(RGBAColourD color)
{
	return SetDefaultFontColorAlt(color_d2b(color));
}

bool AssRender::SetDefaultOutlineColor(RGBA color)
{
	default_outlinecolor = color;
	default_style->OutlineColour = color_b2i(color);
	if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	return true;
}

bool AssRender::SetDefaultOutlineColor(RGBAColourD color)
{
	return SetDefaultOutlineColor(color_d2b(color));
}

bool AssRender::SetDefaultBackColor(RGBA color)
{
	default_backcolor = color;
	default_style->BackColour = color_b2i(color);
	if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	return false;
}

bool AssRender::SetDefaultBackColor(RGBAColourD color)
{
	return SetDefaultBackColor(color_d2b(color));
	return false;
}

bool AssRender::SetDefaultBold(int bold)
{
	if (bold)
		default_bold = 1;
	else
		default_bold = 0;
	default_style->Bold = default_bold;
	if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	return true;
}

bool AssRender::SetDefaultItalic(int italic)
{
	if (italic)
		default_italic = 1;
	else
		default_italic = 0;
	default_style->Italic = default_italic;
	if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	return true;
}

bool AssRender::SetDefaultUnderline(int underline)
{
	if (underline)
		default_underline = 1;
	else
		default_underline = 0;
	default_style->Underline = default_underline;
	if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	return true;
}

bool AssRender::SetDefaultStrikeOut(int strikeout)
{
	if (strikeout)
		default_strikeout = 1;
	else
		default_strikeout = 0;
	default_style->StrikeOut = default_strikeout;
	if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	return true;
}

bool AssRender::SetDefaultBorderStyle(int borderstyle)
{
	default_borderstyle = 0;

	switch (borderstyle)
	{
	case 1:
		default_borderstyle = 3;
		break;
	default:
		default_borderstyle = 0;
		break;
	}
	default_style->BorderStyle = default_borderstyle;
	if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	return true;
}

bool AssRender::SetDefaultOutline(int outline)
{
	default_outline = outline;
	if (default_outline < 0) default_outline = 0;
	if (default_outline > 4) default_outline = 4;
	default_style->Outline = default_outline;
	if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	return true;
}

bool AssRender::SetDefaultShadow(int shadow)
{
	default_shadow = shadow;
	if (default_shadow < 0) default_shadow = 0;
	if (default_shadow > 4) default_shadow = 4;
	default_style->Shadow = default_shadow;
	if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	return true;
}

bool AssRender::SetDefaultStyle(void)
{
	if (!default_style) default_style = new ASS_Style;

	default_style->Name = "Default";

	default_style->FontName = "Tahoma";
	default_style->FontSize = 48;

	default_style->PrimaryColour = 0x00000000;
	default_style->SecondaryColour = 0xFFFFFFC0;
	default_style->OutlineColour = 0xFFFFFFCC;
	default_style->BackColour = 0x66666680;

	default_style->Bold = 0;
	default_style->Italic = 0;
	default_style->Underline = 0;
	default_style->StrikeOut = 0;

	default_style->ScaleX = 100.0;
	default_style->ScaleY = 100.0;
	default_style->Spacing = 0.0;
	default_style->Angle = 0.0;

	default_style->BorderStyle = 1;
	default_style->Outline = 2.0;
	default_style->Shadow = 2.0;
	default_style->Alignment = 1;

	default_style->MarginL = 0;
	default_style->MarginR = 0;
	default_style->MarginV = 0;
	default_style->Encoding = 1;

	default_style->treat_fontname_as_pattern = 1;
	default_style->Blur = 0;
	default_style->Justify = 0;

	return false;
}

bool AssRender::SetDefaultStyle(const char * fontname, int fontsize,
	RGBAColourD color, RGBAColourD coloralt, RGBAColourD outlinecolor, RGBAColourD backcolor,
	int bold, int italic, int underline, int strikeout,
	int borderstyle, int outline, int shadow)
{
	memset(default_fontname, 0, 512);
	strcpy_s(default_fontname, fontname);
	utf2gbk(default_fontname, strlen(default_fontname));
	default_fontsize = fontsize;

	default_fontcolor = color_d2b(color);
	default_fontcoloralt = color_d2b(coloralt);
	default_outlinecolor = color_d2b(outlinecolor);
	default_backcolor = color_d2b(backcolor);

	default_bold = bold;
	default_italic = italic;
	default_underline = underline;
	default_strikeout = strikeout;

	default_borderstyle = borderstyle;
	default_outline = outline;
	default_shadow = shadow;
	if (default_outline < 0) default_outline = 0;
	if (default_outline > 4) default_outline = 4;
	if (default_shadow < 0) default_shadow = 0;
	if (default_shadow > 4) default_shadow = 4;

	default_style->FontName = default_fontname;
	default_style->FontSize = fontsize;

	default_style->PrimaryColour = color_d2i(color);
	default_style->SecondaryColour = color_d2i(coloralt);
	default_style->OutlineColour = color_d2i(outlinecolor);
	default_style->BackColour = color_d2i(backcolor);

	default_style->Bold = bold;
	default_style->Italic = italic;
	default_style->Underline = underline;
	default_style->StrikeOut = strikeout;

	default_style->BorderStyle = borderstyle;
	default_style->Outline = outline;
	default_style->Shadow = shadow;

	if (use_defaultstyle) SetUseDefaultStyle(use_defaultstyle);
	return true;
}

bool AssRender::SetUseDefaultStyle(int used)
{
	if (!al || !ar || !at) return false;
	use_defaultstyle = used;

	if (use_defaultstyle) {
		//ass_set_selective_style_override_enabled(ar, ASS_OVERRIDE_BIT_STYLE);
		ass_set_selective_style_override_enabled(ar, ASS_OVERRIDE_BIT_FONT_NAME + ASS_OVERRIDE_BIT_COLORS + ASS_OVERRIDE_BIT_ATTRIBUTES + ASS_OVERRIDE_BIT_BORDER);
		ass_set_selective_style_override(ar, default_style);
	}
	else {
		ass_set_selective_style_override_enabled(ar, ASS_OVERRIDE_DEFAULT);
		ass_set_style_overrides(al, NULL);
	}
	ass_process_force_style(at);

	return true;
}

bool AssRender::SetSpace(int pixels)
{
	if (!ar) return false;
	if (pixels > 0 && renderHeight > 0) {
		spacing = (double)pixels / (double)renderHeight * 100;
		ass_set_line_spacing(ar, pixels);
		return true;
	}
	return false;
}

bool AssRender::SetSpace(double percentage)
{
	if (!ar) return false;
	spacing = percentage;
	if (renderHeight > 0) {
		double pixels = (renderHeight*spacing / 100.0);
		ass_set_line_spacing(ar, pixels);
		return true;
	}
	return false;
}

bool AssRender::SetPosition(double percentage)
{
	if (!ar) return false;
	if (percentage >= 0 && percentage <= 100) {
		position = percentage;
		ass_set_line_position(ar, percentage);
		return true;
	}
	return false;
}

bool AssRender::SetPosition(int pixels)
{
	if (!ar) return false;
	if (renderHeight > 0) {
		position = pixels / renderHeight * 100;
		ass_set_line_position(ar, position);
		return true;
	}
	return false;
}

bool AssRender::SetMargin(int used)
{
	if (!ar) return false;
	margin = used;
	//ass_set_use_margins(ar, margin);
	if (margin) SetMargin(margin_t, margin_b, margin_l, margin_r);
	else ass_set_margins(ar, 0, 0, 0, 0);
	return true;
}

bool AssRender::SetMargin(int t, int b, int l, int r)
{
	if (!ar) return false;
	if (renderHeight > 0) {
		margin_t = (double)t / (double)(renderHeight) * 100;
		margin_b = (double)b / (double)(renderHeight) * 100;
	}
	if (renderWidth > 0) {
		margin_l = (double)l / (double)(renderHeight) * 100;
		margin_r = (double)r / (double)(renderHeight) * 100;
	}
	if (margin) ass_set_margins(ar, t, b, l, r);
	else ass_set_margins(ar, 0, 0, 0, 0);
	return true;
}

bool AssRender::SetMargin(double t, double b, double l, double r)
{
	if (!ar) return false;
	margin_t = t, margin_b = b, margin_l = l, margin_r = r;
	int tt = 0, tb = 0, tl = 0, tr = 0;
	if (renderHeight > 0) {
		tt = (int)(renderHeight*margin_t / 100);
		tb = (int)(renderHeight*margin_b / 100);
	}
	if (renderWidth > 0) {
		tl = (int)(renderWidth*margin_l / 100);
		tr = (int)(renderWidth*margin_r / 100);
	}
	if(margin) ass_set_margins(ar, tt, tb, tl, tr);
	return true;
}

bool AssRender::SetMargin(int used, int t, int b, int l, int r)
{
	if (!ar) return false;
	SetMargin(used);
	SetMargin(t, b, l, r);
	return true;
}

bool AssRender::SetMargin(int used, double t, double b, double l, double r)
{
	if (!ar) return false;
	SetMargin(used);
	SetMargin(t, b, l, r);
	return true;
}

bool AssRender::SetFontScale(double scale)
{
	if (!al || !ar) return false;
	ass_set_font_scale(ar, scale);
	return true;
}

bool AssRender::SetHints(ASS_Hinting hints)
{
	if (!ar) return false;
	fonthinting = hints;
	ass_set_hinting(ar, fonthinting);
	return true;
}

bool AssRender::Resize(int width, int height)
{
	if (!ar) return false;
	if (width > 0 && height > 0) {
		renderWidth = width;
		renderHeight = height;
		//ass_set_storage_size(ar, width, height);
		ass_set_frame_size(ar, width, height);
		return true;
	}
	return false;
}

bool AssRender::ReScale(double scale)
{
	if (!ar) return false;
	if (scale > 0) {
		fontscale = scale;
		ass_set_font_scale(ar, fontscale);
		return true;
	}
	return false;
}

bool AssRender::SetFPS(double fr)
{
	if (fr > 0) {
		fps = fr;
		frames = duration * fps / 1000;
		return true;
	}
	return false;
}

double AssRender::GetDuration(void)
{
	if (!al || !ar || !at) return 0.0;
	long long length = ass_step_sub(at, 0, at->n_events - 2);
	//long long now = 0;
	//if (fps && start) now = (long long)(start / fps);
	//long long length = ass_step_sub(at, now, at->n_events - 2);
	return (double)length;
}

double AssRender::GetDuration(double start)
{
	if (!al || !ar || !at || !fps) return 0.0;
	long long length = (long long)(duration - (start / fps * 1000));
	return (double)length;
}

double AssRender::GetFrames(void)
{
	if (!al || !ar || !at || !fps) return 0.0;
	return GetFrames(duration);
}

double AssRender::GetFrames(double times)
{
	if (!al || !ar || !at || !fps) return 0.0;
	double frame = times*fps / 1000.0;
	return frame;
}

double AssRender::GetFrames(int framestart)
{
	if (!al || !ar || !at || !fps) return 0.0;
	double frame = frames - framestart;
	return frame;
}

bool AssRender::SetOffset(double offset)
{
	start = offset;
	//GetFrames(GetDuration(start));
	return false;
}

bool AssRender::LoadAss(const char * assfile, const char *_charset)
{
	if (!al) return false;

	char ass_path[MAX_PATH];

	strcpy_s(ass_file, assfile);
	strcpy_s(ass_path, assfile);

	char* ap = strrchr(ass_path, '\\');
	if (ap) ap[0] = 0;

	char charset[128]; // 128 bytes ought to be enough for anyone
	strcpy_s(charset, _charset);

	char ass_buf[MAX_PATH];
	memset(ass_buf, 0, MAX_PATH);
	strcpy_s(ass_buf, assfile);
	utf2gbk(ass_buf, strlen(ass_buf));

	if (at) ass_flush_events(at);
	if (at) ass_free_track(at);
	at = NULL;

	/*
	//using namespace System::Runtime::InteropServices;
	System::String * str = S"Hello world\n";
	char* str2 = (char*)(void*)Marshal::StringToHGlobalAnsi(str);
	printf(str2);
	Marshal::FreeHGlobal(str2);
	*/

	//String^ assFileName = gcnew String(ass_buf);
	//Srt ^srt = gcnew Srt();
	//String^ assContents = srt->Load(assFileName);
	//
	//char* str2 = (char*)(void*)Marshal::StringToHGlobalAnsi(assContents);
	//if (1) {
	//}
	//Marshal::FreeHGlobal((IntPtr)str2);

	at = ass_read_file(al, ass_buf, charset);
	if (!at) {
		//throw("AssRender: could not read %s", ass_file);
		return false;
	}
	else {
		//ass_process_force_style(at);
		//resize_read_order_bitmap(at, 8192);
		//for (int i = 0; i < 2000; i++)
		//	GetAss((double)i, renderWidth, renderHeight);
		frames = 0;
		duration = GetDuration() + 2000;
		frames = GetFrames(duration);

		return true;
	}
}

bool AssRender::LoadAss(char* ass_buf, int length, const char * _charset)
{
	if (!al) return false;

	char ass_path[MAX_PATH];

	char* ap = strrchr(ass_path, '\\');
	if (ap) ap[0] = 0;

	char charset[128]; // 128 bytes ought to be enough for anyone
	strcpy_s(charset, _charset);

	if (at) ass_flush_events(at);
	if (at) ass_free_track(at);
	at = NULL;

	at = ass_read_memory(al, ass_buf, length, charset);
	if (!at) {
		//throw("AssRender: could not read %s", ass_file);
		return false;
	}
	else {
		//ass_process_force_style(at);
		//resize_read_order_bitmap(at, 8192);
		//for (int i = 0; i < 2000; i++)
		//	GetAss((double)i, renderWidth, renderHeight);
		frames = 0;
		duration = GetDuration() + 2000;
		frames = GetFrames(duration);

		return true;
	}
}

ASS_Image_List* AssRender::GetAss(double n, int width, int height, bool ass_type){

	ASS_Image* img = GetAss(n, width, height);
	ASS_Image_List* imglist = new ASS_Image_List(img);
	return(imglist);
}

ASS_Image* AssRender::GetAss(double n, int width, int height)
{
	if (!ar || !al || !at) return NULL;
	renderWidth = width < 0 ? 0 : width;
	renderHeight = height < 0 ? 0 : height;
	if (renderWidth <= 0 || renderHeight <= 0) return NULL;
	if (fps <= 0) return NULL;
	int detChange = 0;
	try
	{
		//ass_set_storage_size(ar, renderHeight, renderHeight);
		ass_set_frame_size(ar, renderWidth, renderHeight);
		//int64_t now = (int64_t)(n * 1000);
		int64_t ts = (int64_t)(n / fps * 1000);
		if (ts < 0) ts = 0;
		//ASS_Image *img = ass_render_frame(ar, at, ts, &detChange);
		ASS_Image *img = ass_render_frame(ar, at, ts, NULL);
		return img;
	}
	catch (const std::exception&)
	{
		return NULL;
	}
}

int AssRender::GetAss(double n, int width, int height, int depth, const void* image, bool blend=true)
{
	renderDepth = depth;
	ASS_Image* assImg = GetAss(n, width, height);
	int c = 0;
	while (assImg) {
		if ((__int64)assImg >= 0xcccccccccccc0000) break;
		if (assImg->w <= 0 || assImg->h <= 0 || assImg->w > 8192 || assImg->h > 8192) {
			assImg = assImg->next;
			continue;
		}

		blend_image(assImg, image, blend);

		if (assImg->next && (__int64)assImg->next < 0xcccccccccccc0000) {
			if (!assImg->next->w || !assImg->next->h) break;
			if ((__int64)assImg->next->w > 0xcccccccccccc0000 ||
				(__int64)assImg->next->h > 0xcccccccccccc0000) break;
		}
		else break;
		assImg = assImg->next;
		c++;
	}
	assImg = NULL;
	return c;
}

ASS_Image* AssRender::GetAss(double n, const ASS_Image* src)
{
	if (src != NULL) {
		ass_set_frame_size(ar, src->w, src->h);
	}
	//int64_t now = (int64_t)(n * 1000);
	int64_t ts = (int64_t)(n / fps * 1000);
	int detChange = 0;
	ASS_Image *img = ass_render_frame(ar, at, ts, &detChange);

	return img;
}

ASS_Image* AssRender::GetAss(int64_t n, const ASS_Image* src)
{
	ASS_Image *img = ass_render_frame(ar, at, n, NULL);
	return img;
}

ASS_Image* AssRender::GetAss(int n)
{
	//// it's a casting party!
	//int64_t ts = (int64_t)n * (int64_t)1000 * (int64_t)vi.fps_denominator / (int64_t)vi.fps_numerator;

	//ASS_Image *img = ass_render_frame(ar, t, ts, NULL);

	//}
	//return img;
	return NULL;
}

