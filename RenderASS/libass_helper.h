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
#  include <ass_types.h>
}

#define _R(c)  ((c)>>24)
#define _G(c)  (((c)>>16)&0xFF)
#define _B(c)  (((c)>>8)&0xFF)
#define _A(c)  ((c)&0xFF)

extern TCHAR cur_dll_path[MAX_PATH];

DWORD GetModulePath(HMODULE hModule, LPTSTR pszBuffer, DWORD dwSize);

class AssRender {
private:
	bool InitLibass(ASS_Hinting hints, double scale, int width, int height);

	ASS_Library *al;
	ASS_Renderer *ar;
	ASS_Track *t;

	double fps = 29.970;
	char ass_file[MAX_PATH];

public:
	AssRender(ASS_Hinting hints, double scale, const char *charset);
	~AssRender();

	bool __stdcall LoadAss(const char* assfile, const char *_charset);
	bool __stdcall Resize(double scale, int width, int height);
	bool __stdcall SetFrameRate(double fr);

	ASS_Image* __stdcall RenderFrame(double n, int width, int height);
	ASS_Image* __stdcall RenderFrame(double n, ASS_Image* src);
	ASS_Image* __stdcall RenderFrame(int64_t n, ASS_Image* src);
	ASS_Image* __stdcall RenderFrame(int n);
};

char* w2c(const wchar_t* wsp);
char* w2c(const std::wstring ws);
