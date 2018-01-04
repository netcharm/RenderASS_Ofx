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

#include <png.h>

#include <iconv.h>

#include "libass_helper.h"

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
	for (int i = 0; i < s.length(); i++) {
		c[i] = s[i];
	}
	return true;
}

#define BUFSIZE 1024
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

//
ASS_Image_List::ASS_Image_List(ASS_Image * img)
{
	int img_count = 0;
	while (img) {
		img_count++;
		try
		{
			if (img->w <= 0 || img->h <= 0)
			{
				//img = img->next;
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

			if (img->next && (unsigned long)img->next < 0xcccc0000) {
				if (!img->next->w || !img->next->h) break;
				if ((unsigned long)img->next->w > 0xcccc0000 ||
					(unsigned long)img->next->h > 0xcccc0000 ||
					(unsigned long)img->next->type > 0xcccc0000) break;
				img = img->next;
			}
			else break;
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
AssRender::AssRender(ASS_Hinting hints, double scale, const char *charset) {	
	memset(ass_file, 0, MAX_PATH);
	//if (!InitLibass(ASS_HINTING_LIGHT, scale, 1280, 720))
	if (!InitLibass(ASS_HINTING_NATIVE, scale, 1280, 720))
	{
		//throw("AssRender: failed to initialize libass");
	}		
}

AssRender::~AssRender() {
	ass_free_track(t);
	ass_renderer_done(ar);
	ass_library_done(al);
}

bool AssRender::InitLibass(ASS_Hinting hints, double scale, int width, int height) {
	al = ass_library_init();
	if (!al)
		return false;

	//char tmp[MAX_PATH];
	//SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, tmp);

	// not needed?
	//ass_set_fonts_dir(al, tmp);
	//ass_set_fonts_dir(al, "C:\\Windows\\Fonts");
	ass_set_extract_fonts(al, 2);
	ass_set_style_overrides(al, NULL);

	ar = ass_renderer_init(al);
	if (!ar)
		return false;

	ass_set_frame_size(ar, width, height);
	//ass_set_margins(ar, 0, 0, 0, 0);
	//ass_set_use_margins(ar, 0);
	//ass_set_aspect_ratio(ar,);  // todo: implement this
	ass_set_hinting(ar, hints);
	ass_set_font_scale(ar, scale);
	ass_set_line_position(ar, position);
	ass_set_line_spacing(ar, spacing);
	//ass_set_shaper(ar, ASS_SHAPING_COMPLEX);

	std::string path(&cur_dll_path[0]);
	path.append(_T("fontconfig\\fonts.conf"));
	s2c(path, fontconf);

	memset(default_fontname, 0, 512);
	strcpy_s(default_fontname, "Arial");

	//ass_set_fonts(ar, "Arial", "Sans", 1, w2c(path), 1);
	//ass_set_fonts(ar, "C:\\Windows\\Fonts\\Arial.ttf", "Sans", 1, fontconf, 1);
	ass_set_fonts(ar, default_fontname, "Sans", 1, fontconf, 1);
	//ass_set_fonts(ar, "Arial", "Sans", 0, NULL, 0);

	return true;
}

bool AssRender::SetDefaultFont(char * fontname, int fontsize)
{
	if (!ar) return false;
	if (strlen(fontname) > 0 && strcmp(fontname, default_fontname) != 0) {
		memset(default_fontname, 0, 512);
		strcpy_s(default_fontname, fontname);
		ass_set_fonts(ar, default_fontname, "Sans", 1, fontconf, 1);
	}
	return true;
}

bool AssRender::SetMargin(bool used)
{
	if (!ar) return false;
	margin = used;
	ass_set_use_margins(ar, margin);
	return true;
}

bool AssRender::SetMargin(int t, int b, int l, int r)
{
	if (!ar) return false;
	if (renderHeight > 0) {
		margin_t = (double)margin_t / (double)(renderHeight);
		margin_b = (double)margin_b / (double)(renderHeight);
	}
	if (renderWidth > 0) {
		margin_l = (double)margin_l / (double)(renderHeight);
		margin_r = (double)margin_r / (double)(renderHeight);
	}
	ass_set_margins(ar, t, b, l, r);
	return true;
}

bool AssRender::SetMargin(double t, double b, double l, double r)
{
	if (!ar) return false;
	margin_t = t, margin_b = b, margin_l = l, margin_r = r;
	int tt = 0, tb = 0, tl = 0, tr = 0;
	if (renderHeight > 0) {
		tt = (int)(renderHeight*margin_t);
		tb = (int)(renderHeight*margin_b);
	}
	if (renderWidth > 0) {
		tl = (int)(renderWidth*margin_l);
		tr = (int)(renderWidth*margin_r);
	}
	ass_set_margins(ar, tt, tb, tl, tr);
	return true;
}

bool AssRender::SetMargin(bool used, int t, int b, int l, int r)
{
	if (!ar) return false;
	SetMargin(used);
	SetMargin(t, b, l, r);
	return true;
}

bool AssRender::SetMargin(bool used, double t, double b, double l, double r)
{
	if (!ar) return false;
	SetMargin(used);
	SetMargin(t, b, l, r);
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
	}
	return true;
}

bool AssRender::ReScale(double scale)
{
	if(!ar) return false;
	if (scale > 0) {
		fontscale = scale;
		ass_set_font_scale(ar, fontscale);
	}
	return true;
}

bool AssRender::SetFPS(double fr)
{
	if(fr>0) fps = fr;
	return false;
}

bool AssRender::SetHints(ASS_Hinting hints)
{
	if (!ar) return false;
	fonthinting = hints;
	ass_set_hinting(ar, fonthinting);
	return true;
}

bool AssRender::SetSpace(int pixels)
{
	if (!ar) return false;
	if (pixels > 0 && renderHeight > 0) {
		spacing = (double)pixels / (double)renderHeight;
		ass_set_line_spacing(ar, pixels);
	}
	return true;
}

bool AssRender::SetSpace(double percentage)
{
	if (!ar) return false;
	spacing = percentage;
	if (renderHeight > 0) {
		int pixels = (int)(renderHeight*spacing);
		ass_set_line_spacing(ar, pixels);
	}
	return true;
}

bool AssRender::SetPosition(int percentage)
{
	if (!ar) return false;
	if (percentage >= 0 && percentage <= 100) {
		position = percentage;
		ass_set_line_position(ar, percentage);
	}
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

	if(t) ass_flush_events(t);

	char ass_buf[MAX_PATH];
	memset(ass_buf, 0, MAX_PATH);
	strcpy_s(ass_buf, assfile);
	utf2gbk(ass_buf, strlen(ass_buf));

	t = ass_read_file(al, ass_buf, charset);
	if (!t) {
		throw("AssRender: could not read %s", ass_file);
		return false;
	}
	else {
		//ass_set_fonts_dir(al, ass_path);
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
	if (!ar) return(NULL);
	try
	{
		//ass_set_storage_size(ar, width, height);
		//ass_set_frame_size(ar, width, height);
		//int64_t now = (int64_t)(n * 1000);
		int64_t ts = (int64_t)(n / fps * 1000);
		//if (!t) return NULL;
		int detChange = 0;
		ASS_Image *img = ass_render_frame(ar, t, ts, &detChange);
		return img;
	}
	catch (const std::exception&)
	{
		return NULL;
	}
}

ASS_Image* AssRender::GetAss(double n, ASS_Image* src)
{
	if (src != NULL) {
		ass_set_frame_size(ar, src->w, src->h);
	}
	//int64_t now = (int64_t)(n * 1000);
	int64_t ts = (int64_t)(n / fps * 1000);
	int detChange = 0;
	ASS_Image *img = ass_render_frame(ar, t, ts, &detChange);

	return img;
}

ASS_Image* AssRender::GetAss(int64_t n, ASS_Image* src)
{
	ASS_Image *img = ass_render_frame(ar, t, n, NULL);
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

