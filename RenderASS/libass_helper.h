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


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
extern "C" {
#  include <ass.h>
//#  include <ass_outline.h>
#  include <ass_types.h>
}

#define M_PI       3.14159265358979323846   // pi

#define _R(c)  ((c)>>24)
#define _G(c)  (((c)>>16)&0xFF)
#define _B(c)  (((c)>>8)&0xFF)
#define _A(c)  ((c)&0xFF)

extern TCHAR cur_dll_path[MAX_PATH];

//
DWORD GetModulePath(HMODULE hModule, LPTSTR pszBuffer, DWORD dwSize);

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
	bool InitLibass(ASS_Hinting hints, double scale, int width, int height);

	ASS_Library *al;
	ASS_Renderer *ar;
	ASS_Track *t;

	char ass_file[MAX_PATH];
	char fontconf[MAX_PATH];

	double fps = 29.970;
	int renderWidth = 0;
	int renderHeight = 0;
	double fontscale = 1.0;
	ASS_Hinting fonthinting = ASS_HINTING_NONE;

	bool margin = false;
	double margin_t = 0, margin_b = 0, margin_l = 0, margin_r = 0;
	double spacing = 2, position = 2;

public:
	AssRender(ASS_Hinting hints, double scale, const char *charset);
	~AssRender();

	bool __stdcall Resize(int width, int height);
	bool __stdcall ReScale(double scale);
	bool __stdcall SetFPS(double fr);
	bool __stdcall SetHints(ASS_Hinting hints);

	bool __stdcall SetDefaultFont(char * fontname, int fontsize);
	bool __stdcall SetSpace(int pixels);
	bool __stdcall SetSpace(double percentage);
	bool __stdcall SetPosition(int percentage);
	bool __stdcall SetMargin(bool used);
	bool __stdcall SetMargin(int t, int b, int l, int r);
	bool __stdcall SetMargin(double t, double b, double l, double r);
	bool __stdcall SetMargin(bool used, int t, int b, int l, int r);
	bool __stdcall SetMargin(bool used, double t, double b, double l, double r);

	bool __stdcall LoadAss(const char* assfile, const char *_charset);

	ASS_Image_List* __stdcall GetAss(double n, int width, int height, bool ass_type);
	ASS_Image* __stdcall GetAss(double n, int width, int height);
	ASS_Image* __stdcall GetAss(double n, ASS_Image* src);
	ASS_Image* __stdcall GetAss(int64_t n, ASS_Image* src);
	ASS_Image* __stdcall GetAss(int n);
};

char* w2c(const wchar_t* wsp);
char* w2c(const std::wstring ws);
