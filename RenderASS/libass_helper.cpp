#include "stdafx.h"

// importing libass
#include <shlobj.h>
#include <tchar.h>
#include <time.h>
#include <string>

#include "libass_helper.h"


#define _R(c)  ((c)>>24)
#define _G(c)  (((c)>>16)&0xFF)
#define _B(c)  (((c)>>8)&0xFF)
#define _A(c)  ((c)&0xFF)

char cur_dll_path[MAX_PATH]; // hack

AssRender::AssRender(ASS_Hinting hints, double scale, const char *charset) {
	if (!InitLibass(hints, scale))
	{
		//throw("AssRender: failed to initialize libass");
	}		
}

AssRender::~AssRender() {
	ass_free_track(t);
	ass_renderer_done(ar);
	ass_library_done(al);
}

ASS_Image* AssRender::RenderFrame(int64_t n)
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


bool AssRender::InitLibass(ASS_Hinting hints, double scale) {
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

	//ass_set_frame_size(ar, vi.width, vi.height);
	ass_set_frame_size(ar, 1280, 720);
	//ass_set_margins(ar, 0, 0, 0, 0);
	//ass_set_use_margins(ar, 0);
	//ass_set_aspect_ratio(ar,);  // todo: implement this
	ass_set_font_scale(ar, scale);
	ass_set_hinting(ar, hints);

	std::string path(cur_dll_path);
	path.append("fontconfig\\fonts.conf");
	ass_set_fonts(ar, "Arial", "Sans", 1, path.c_str(), 1);

	return true;
}


const char * SelectAssFile(void)
{
	int msgboxID = MessageBox(
		NULL,
		(LPCWSTR)L"Select ASS File",
		(LPCWSTR)L"ASS Selected",
		MB_ICONINFORMATION | MB_OK | MB_DEFBUTTON1
	);

	return("");
}

