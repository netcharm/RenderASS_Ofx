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
		//ThrowError("AssRender: failed to initialize libass");
	}		
}

AssRender::~AssRender() {
	ass_free_track(t);
	ass_renderer_done(ar);
	ass_library_done(al);
	if (lp.log)
		fclose(lp.log);
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