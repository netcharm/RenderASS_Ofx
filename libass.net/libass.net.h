// libass.net.h

#pragma once

using namespace System;
using namespace System::Drawing;
using namespace System::IO;
using namespace System::Runtime::InteropServices;

#include "libass_helper.h"

namespace libass {

	enum ASSHinting {
		ASS_HINTING_NONE = 0,
		ASS_HINTING_LIGHT = 1,
		ASS_HINTING_NORMAL = 2,
		ASS_HINTING_NATIVE = 3
	};

	public ref class Render
	{
	private:
		int renderWidth;
		int renderHeight;
		int renderDepth = 4;
		// TODO:  在此处添加此类的方法。
	public:
		Render(void);
		Render(ASSHinting hints, double scale, String^ encoding);
		~Render();

		bool SetDefaultFont(String^ fontname, int fontsize);
		bool SetDefaultFontName(String^ fontname);
		bool SetDefaultFontSize(int fontsize);
		bool SetDefaultFontColor(Color^ color);
		bool SetDefaultFontColorAlt(Color^ color);
		bool SetDefaultOutlineColor(Color^ color);
		bool SetDefaultBackColor(Color^ color);
		bool SetDefaultBold(int bold);
		bool SetDefaultItalic(int italic);
		bool SetDefaultUnderline(int underline);
		bool SetDefaultStrikeOut(int strikeout);
		bool SetDefaultBorderStyle(int borderstyle);
		bool SetDefaultOutline(int outline);
		bool SetDefaultShadow(int shadow);

		bool SetDefaultStyle(void);
		bool SetDefaultStyle(String^ fontname, int fontsize,
			Color^ color, Color^ coloralt, Color^ outlinecolor, Color^ backcolor,
			int bold, int italic, int underline, int strikeout,
			int borderstyle, int outline, int shadow);
		bool SetUseDefaultStyle(int used);

		bool SetSpace(int pixels);
		bool SetSpace(double percentage);
		bool SetPosition(int pixels);
		bool SetPosition(double percentage);
		bool SetMargin(int used);
		bool SetMargin(int t, int b, int l, int r);
		bool SetMargin(double t, double b, double l, double r);
		bool SetMargin(int used, int t, const int b, int l, int r);
		bool SetMargin(int used, double t, double b, double l, double r);
		bool SetFontScale(double scale);
		bool SetHints(ASS_Hinting hints);

		bool Resize(int width, int height);
		bool ReScale(double scale);
		bool SetFPS(double fps);

		double GetDuration(void);
		double GetDuration(double start);
		double GetFrames(void);
		double GetFrames(double times);
		double GetFrames(int framestart);
		bool SetOffset(double offset);

		bool LoadAss(String^ filename, String^ encoding);
		bool LoadAss(String^ contents);
		bool LoadAss(Stream^ stream);

		Image^ GetAss(int n);
		Image^ GetAss(double n, int width, int height);
	};
}
