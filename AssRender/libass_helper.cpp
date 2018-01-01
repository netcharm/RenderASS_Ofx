// importing libass
#include <cstring>
#include <stdexcept>
#include <new>
#include <tchar.h>
#include <shlobj.h>
#include <tchar.h>
#include <time.h>
//#include <string>

#include "libass_helper.h"


AssRender::AssRender(ASS_Hinting hints, double scale, const char *charset) {	
	if (!InitLibass(ASS_HINTING_LIGHT, scale, 1280, 720))
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
	strcpy_s(ass_file, assfile);

	char charset[128]; // 128 bytes ought to be enough for anyone
	strcpy_s(charset, _charset);

	t = ass_read_file(al, ass_file, charset);
	if (!t) {
		throw("AssRender: could not read %s", ass_file);
		return false;
	}
	else { return true; }
}

bool AssRender::Resize(double scale, int width, int height)
{
	//InitLibass(ASS_HINTING_LIGHT, scale, width, height);
	ass_set_frame_size(ar, width, height);
	return true;
}

bool AssRender::SetFrameRate(double fr)
{
	//fps = fr;
	return false;
}

ASS_Image* AssRender::RenderFrame(double n, ASS_Image* src)
{
	if (src != NULL) {
		ass_set_frame_size(ar, src->w, src->h);
	}		
	//int64_t now = (int64_t)(n * 1000);
	int64_t ts = (int64_t)(n / fps * 1000);
	ASS_Image *img = ass_render_frame(ar, t, ts, NULL);

	// this here loop shamelessly stolen from aegisub's subtitles_provider_libass.cpp
	// and slightly modified to render upside-down
	//while (img) {
	//	if (img->w == 0 || img->h == 0)
	//		continue;

	//}
	return img;
}

ASS_Image* AssRender::RenderFrame(double n, int width, int height)
{
	ass_set_frame_size(ar, width, height);
	//int64_t now = (int64_t)(n * 1000);
	int64_t ts = (int64_t)(n / fps * 1000);
	ASS_Image *img = ass_render_frame(ar, t, ts, NULL);

	// this here loop shamelessly stolen from aegisub's subtitles_provider_libass.cpp
	// and slightly modified to render upside-down
	//while (img) {
	//	if (img->w == 0 || img->h == 0)
	//		continue;

	//}
	return img;
}

ASS_Image* AssRender::RenderFrame(int64_t n, ASS_Image* src)
{
	ASS_Image *img = ass_render_frame(ar, t, n, NULL);

	// this here loop shamelessly stolen from aegisub's subtitles_provider_libass.cpp
	// and slightly modified to render upside-down
	while (img) {
		if (img->w == 0 || img->h == 0)
			continue;

	}
	return img;
}

ASS_Image* AssRender::RenderFrame(int n)
{
	//// it's a casting party!
	//int64_t ts = (int64_t)n * (int64_t)1000 * (int64_t)vi.fps_denominator / (int64_t)vi.fps_numerator;

	//ASS_Image *img = ass_render_frame(ar, t, ts, NULL);

	//// this here loop shamelessly stolen from aegisub's subtitles_provider_libass.cpp
	//// and slightly modified to render upside-down
	//while (img) {
	//	if (img->w == 0 || img->h == 0)
	//		continue;

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
	ass_set_extract_fonts(al, 0);
	ass_set_style_overrides(al, NULL);

	ar = ass_renderer_init(al);
	if (!ar)
		return false;

	ass_set_frame_size(ar, width, height);
	//ass_set_margins(ar, 0, 0, 0, 0);
	//ass_set_use_margins(ar, 0);
	//ass_set_aspect_ratio(ar,);  // todo: implement this
	ass_set_font_scale(ar, scale);
	ass_set_hinting(ar, hints);

	std::wstring path(dll_path);
	path.append(_T("fontconfig\\fonts.conf"));
	ass_set_fonts(ar, "Arial", "Sans", 1, w2c(path), 1);

	return true;
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
