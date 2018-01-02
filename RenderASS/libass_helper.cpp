#include "stdafx.h"

// importing libass
#include <shlobj.h>
#include <tchar.h>
#include <time.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <mbstring.h>

#include <png.h>

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

bool AssRender::LoadAss(const char * assfile, const char *_charset)
{
	char ass_path[MAX_PATH];

	strcpy_s(ass_file, assfile);
	strcpy_s(ass_path, assfile);

	char* ap = strrchr(ass_path, '\\');
	if (ap) ap[0] = 0;

	ass_set_fonts_dir(al, ass_path);

	char charset[128]; // 128 bytes ought to be enough for anyone
	strcpy_s(charset, _charset);

	t = ass_read_file(al, ass_file, charset);
	if (!t) {
		throw("AssRender: could not read %s", ass_file);
		return false;
	}
	else { 
		return true; 
	}
}

bool AssRender::Resize(double scale, int width, int height)
{
	//InitLibass(ASS_HINTING_LIGHT, scale, width, height);
	ass_set_frame_size(ar, width, height);
	return true;
}

bool AssRender::SetFrameRate(double fr)
{
	if(fr>0) fps = fr;
	return false;
}

bool AssRender::SetDefaultFont(char * fontname, int fontsize)
{
	ass_set_fonts(ar, fontname, "Sans", 1, fontconf, 1);
	return true;
}

ASS_Image_List* AssRender::RenderFrame(double n, int width, int height, bool ass_type){

	ASS_Image* img = RenderFrame(n, width, height);
	ASS_Image_List* imglist = new ASS_Image_List(img);
	return(imglist);
}

ASS_Image* AssRender::RenderFrame(double n, int width, int height)
{
	ass_set_frame_size(ar, width, height);
	//int64_t now = (int64_t)(n * 1000);
	int64_t ts = (int64_t)(n / fps * 1000);
	if (!t) return NULL;
	int detChange = 2;
	ASS_Image *img = ass_render_frame(ar, t, ts, &detChange);
	//ASS_Image *img = ass_render_frame(ar, t, ts, NULL);

	// this here loop shamelessly stolen from aegisub's subtitles_provider_libass.cpp
	// and slightly modified to render upside-down
	//while (img) {
	//	if (img->w == 0 || img->h == 0)
	//		continue;

	//}
	return img;
}

ASS_Image* AssRender::RenderFrame(double n, ASS_Image* src)
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

ASS_Image* AssRender::RenderFrame(int64_t n, ASS_Image* src)
{
	ASS_Image *img = ass_render_frame(ar, t, n, NULL);
	return img;
}

ASS_Image* AssRender::RenderFrame(int n)
{
	//// it's a casting party!
	//int64_t ts = (int64_t)n * (int64_t)1000 * (int64_t)vi.fps_denominator / (int64_t)vi.fps_numerator;

	//ASS_Image *img = ass_render_frame(ar, t, ts, NULL);

	//}
	//return img;
	return NULL;
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
	ass_set_line_position(ar, 5);
	ass_set_line_spacing(ar, 16);
	//ass_set_shaper(ar, ASS_SHAPING_COMPLEX);

	std::string path(&cur_dll_path[0]);
	path.append(_T("fontconfig\\fonts.conf"));
	s2c(path, fontconf);
	//ass_set_fonts(ar, "Arial", "Sans", 1, w2c(path), 1);
	//ass_set_fonts(ar, "C:\\Windows\\Fonts\\Arial.ttf", "Sans", 1, fontconf, 1);
	ass_set_fonts(ar, "Arial", "Sans", 1, fontconf, 1);
	//ass_set_fonts(ar, "Arial", "Sans", 0, NULL, 0);

	return true;
}

ASS_Image_List::ASS_Image_List(ASS_Image * img)
{
	int img_count = 0;
	while (img && img_count < 3) {
		img_count++;
		try
		{
			if (img->w <= 0 || img->h <= 0)
			{
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

			unsigned long uia = (unsigned long)(&img->next);
			if (uia <= 0 && uia > 0x00FFFFFFFFFFL) break;
			if (!img->next || img->next->w <= 0 || img->next->h <= 0 || img->next->type < 0) break;
			img = img->next;
		}
		catch (const std::exception&)
		{

		} {}
	}
}

ASS_Image_List::~ASS_Image_List()
{
	img_text = NULL;
	img_outline = NULL;
	img_shadow = NULL;
	//if (img_text) delete img_text;
	//if (img_outline) delete img_outline;
	//if (img_shadow) delete img_shadow;
}
