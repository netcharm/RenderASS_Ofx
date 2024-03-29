// RenderASS.cpp : 定义 DLL 应用程序的导出函数。
//
#pragma once
#define __STDC_WANT_LIB_EXT1__ 1
#include "stdafx.h"

/*
Ofx plugin that render ASS subtitle.

It is meant to illustrate certain features of the API, as opposed to being a perfectly
crafted piece of image processing software.

The main features are
- basic plugin definition
- basic property usage
- basic image access and rendering
*/
#include <libgnuintl.h>
#include "libass_helper.h"
#include "common_ofx.h"

/*使用gettext通常使用类似下面的一个带函数的宏定义
*你完全可以不用，直接使用 gettext(字符串)
*/
#define _(S) gettext(S)

/*PACKAGE是本程序最终的名字（运行时输入的命令）*/
#define PACKAGE "RenderASS"

// private instance data type
struct RenderAssInstanceData {
	ContextEnum context;
	AssRender * ass;
	bool pre_blend = false;

	int use_margin = 0;
	int use_defaultstyle = 0;
	double Offset = 0;
	double FrameStart = 0;
	double FrameEnd = 0;
	double FPS = 0;

	// handles to the clips we deal with
	OfxImageClipHandle sourceClip;
	OfxImageClipHandle outputClip;

	// handles to a our parameters
	OfxParamHandle assFileName;
	OfxParamHandle assOffset;

	OfxParamHandle assUseMargin;
	OfxParamHandle assMarginT;
	OfxParamHandle assMarginB;
	OfxParamHandle assMarginL;
	OfxParamHandle assMarginR;
	OfxParamHandle assSpace;
	OfxParamHandle assPosition;
	OfxParamHandle assFontScale;
	OfxParamHandle assFontHints;

	OfxParamHandle assUseDefaultStyle;
	OfxParamHandle assDefaultFontName;
	OfxParamHandle assDefaultFontSize;
	OfxParamHandle assDefaultFontColor;
	OfxParamHandle assDefaultFontColorAlpha;
	OfxParamHandle assDefaultFontColorAlt;
	OfxParamHandle assDefaultFontColorAltAlpha;
	OfxParamHandle assDefaultOutlineColor;
	OfxParamHandle assDefaultOutlineColorAlpha;
	OfxParamHandle assDefaultBackColor;
	OfxParamHandle assDefaultBackColorAlpha;
	OfxParamHandle assDefaultBold;
	OfxParamHandle assDefaultItalic;
	OfxParamHandle assDefaultUnderline;
	OfxParamHandle assDefaultStrikeOut;
	//OfxParamHandle assDefaultScaleX;
	//OfxParamHandle assDefaultScaleY;
	//OfxParamHandle assDefaultSpacing;
	//OfxParamHandle assDefaultAngle;
	OfxParamHandle assDefaultBorderStyle;
	OfxParamHandle assDefaultOutline;
	OfxParamHandle assDefaultShadow;
	//OfxParamHandle assDefaultAlignment;
	//OfxParamHandle assDefaultMarginL;
	//OfxParamHandle assDefaultMarginR;
	//OfxParamHandle assDefaultMarginV;

	OfxParamHandle assGlobalBlend;
	OfxParamHandle assGlobalBackColor;
	OfxParamHandle assGlobalBackColorAlpha;
};

static const char* PluginAuthor = "NetCharm";
#ifdef _DEBUG
static const char* PluginLabel = _("Render ASS Debug");
#else
static const char* PluginLabel = _("Render ASS");
#endif
static const char* PluginDescription = _("ASS/SSA (Advanced Substation Alpha/Substation Alpha) Render Filter");
#ifdef _DEBUG
static const char* PluginIdentifier = "cn.netcharm.Ofx.RenderASS-d";
#else
static const char* PluginIdentifier = "cn.netcharm.Ofx.RenderASS";
#endif

static const int Version_Majon = 1;
static const int Version_Minor = 0;
static const int Version_Revision = 3;
static const int Version_BuildNo = 40;

//static const int PluginVersion[4] = { Version_Majon, Version_Minor, Version_Revision, Version_BuildNo };

/* mandatory function to set up the host structures */
class RenderASS {
private:
	// some flags about the host's behaviour
	static const int gHostSupportsMultipleBitDepths = false;

	// Convinience wrapper to get private data 
	static bool setDuration(OfxImageEffectHandle effect) {
		RenderAssInstanceData *myData = getMyInstanceData(effect);

		if (myData && myData->context == eIsGenerator) {
			OfxPropertySetHandle effectProps;
			gEffectHost->clipGetPropertySet(myData->outputClip, &effectProps);
			// set output has N frames
			myData->ass->SetFPS(1000);
			double frames = myData->ass->GetFrames((int)myData->Offset);
			gPropHost->propSetDouble(effectProps, kOfxImageEffectPropFrameRange, 0, 0);
			gPropHost->propSetDouble(effectProps, kOfxImageEffectPropFrameRange, 1, frames);
			return true;
		}
		return false;
	}

	static RenderAssInstanceData * getMyInstanceData(OfxImageEffectHandle effect)
	{
		RenderAssInstanceData *myData = (RenderAssInstanceData *)ofxuGetEffectInstanceData(effect);
		return myData;
	}

	static RenderAssInstanceData * getMyInstanceParam(OfxImageEffectHandle effect) {

		// get a pointer to the effect properties
		OfxPropertySetHandle effectProps;
		gEffectHost->getPropertySet(effect, &effectProps);

		// get a pointer to the effect's parameter set
		OfxParamSetHandle paramSet;
		gEffectHost->getParamSet(effect, &paramSet);

		// get the parameter from the parameter set
		OfxParamHandle param;

		RenderAssInstanceData *myData = new RenderAssInstanceData;

		gParamHost->paramGetHandle(paramSet, "assFileName", &myData->assFileName, 0);
		gParamHost->paramGetHandle(paramSet, "assOffset", &myData->assOffset, 0);

		gParamHost->paramGetHandle(paramSet, "assUseMargin", &myData->assUseMargin, 0);
		gParamHost->paramGetHandle(paramSet, "assMarginT", &myData->assMarginT, 0);
		gParamHost->paramGetHandle(paramSet, "assMarginB", &myData->assMarginB, 0);
		gParamHost->paramGetHandle(paramSet, "assMarginL", &myData->assMarginL, 0);
		gParamHost->paramGetHandle(paramSet, "assMarginR", &myData->assMarginR, 0);
		gParamHost->paramGetHandle(paramSet, "assSpace", &myData->assSpace, 0);
		gParamHost->paramGetHandle(paramSet, "assPosition", &myData->assPosition, 0);

		gParamHost->paramGetHandle(paramSet, "assFontScale", &myData->assFontScale, 0);
		gParamHost->paramGetHandle(paramSet, "assFontHints", &myData->assFontHints, 0);

		gParamHost->paramGetHandle(paramSet, "assUseDefaultStyle", &myData->assUseDefaultStyle, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultFontName", &myData->assDefaultFontName, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultFontSize", &myData->assDefaultFontSize, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultFontColor", &myData->assDefaultFontColor, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultFontColorAlpha", &myData->assDefaultFontColorAlpha, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultFontColorAlt", &myData->assDefaultFontColorAlt, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultFontColorAltAlpha", &myData->assDefaultFontColorAltAlpha, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultOutlineColor", &myData->assDefaultOutlineColor, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultOutlineColorAlpha", &myData->assDefaultOutlineColorAlpha, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultBackColor", &myData->assDefaultBackColor, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultBackColorAlpha", &myData->assDefaultBackColorAlpha, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultBold", &myData->assDefaultBold, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultItalic", &myData->assDefaultItalic, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultUnderline", &myData->assDefaultUnderline, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultStrikeOut", &myData->assDefaultStrikeOut, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultBorderStyle", &myData->assDefaultBorderStyle, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultOutline", &myData->assDefaultOutline, 0);
		gParamHost->paramGetHandle(paramSet, "assDefaultShadow", &myData->assDefaultShadow, 0);

		gParamHost->paramGetHandle(paramSet, "assGlobalBlend", &myData->assGlobalBlend, 0);
		gParamHost->paramGetHandle(paramSet, "assGlobalBackColor", &myData->assGlobalBackColor, 0);
		gParamHost->paramGetHandle(paramSet, "assGlobalBackColorAlpha", &myData->assGlobalBackColorAlpha, 0);

		myData->ass = new AssRender();

		// cache away param handles
		char *str_ass;
		gParamHost->paramGetHandle(paramSet, "assFileName", &param, 0);
		gParamHost->paramGetValue(param, &str_ass);
		myData->ass->LoadAss(str_ass, "UTF-8");

		int offset = (int)myData->Offset;
		myData->Offset = ofxuGetTime(effect);
		gParamHost->paramGetHandle(paramSet, "assOffset", &param, 0);
		gParamHost->paramGetValue(param, &offset);
		if (offset < 1) offset = 0;
		if (myData) myData->Offset = (double)offset;
		if (myData->ass) myData->ass->SetOffset(offset);


		int margin_enabled = 0;
		gParamHost->paramGetHandle(paramSet, "assUseMargin", &param, 0);
		gParamHost->paramGetValue(param, &margin_enabled);
		myData->use_margin = margin_enabled;
		if (myData->ass) myData->ass->SetMargin(margin_enabled);

		double margin_t = 0.0;
		double margin_b = 0.0;
		double margin_l = 0.0;
		double margin_r = 0.0;
		gParamHost->paramGetHandle(paramSet, "assMarginT", &param, 0);
		gParamHost->paramGetValue(param, &margin_t);
		gParamHost->paramGetHandle(paramSet, "assMarginB", &param, 0);
		gParamHost->paramGetValue(param, &margin_b);
		gParamHost->paramGetHandle(paramSet, "assMarginL", &param, 0);
		gParamHost->paramGetValue(param, &margin_l);
		gParamHost->paramGetHandle(paramSet, "assMarginR", &param, 0);
		gParamHost->paramGetValue(param, &margin_r);
		if (myData->ass) myData->ass->SetMargin(margin_t, margin_b, margin_l, margin_r);

		double spacing = 0.0;
		gParamHost->paramGetHandle(paramSet, "assSpace", &param, 0);
		gParamHost->paramGetValue(param, &spacing);
		if (myData->ass) myData->ass->SetSpace(spacing);

		double position = 0.0;
		gParamHost->paramGetHandle(paramSet, "assPosition", &param, 0);
		gParamHost->paramGetValue(param, &position);
		if (myData->ass) myData->ass->SetSpace(position);


		double scale = 1.0;
		gParamHost->paramGetHandle(paramSet, "assFontScale", &param, 0);
		gParamHost->paramGetValue(param, &scale);
		if (myData->ass) myData->ass->SetSpace(scale);

		int hints = 0;
		gParamHost->paramGetHandle(paramSet, "assFontHints", &param, 0);
		gParamHost->paramGetValue(param, &position);
		if (myData->ass) {
			switch (hints)
			{
			case 1:
				myData->ass->SetHints(ASS_HINTING_LIGHT);
				break;
			case 2:
				myData->ass->SetHints(ASS_HINTING_NORMAL);
				break;
			case 3:
				myData->ass->SetHints(ASS_HINTING_NATIVE);
				break;
			default:
				myData->ass->SetHints(ASS_HINTING_NONE);
				break;
			}
		}

		int defaultstyle_enabled = 0;
		gParamHost->paramGetHandle(paramSet, "assUseDefaultStyle", &param, 0);
		gParamHost->paramGetValue(param, &defaultstyle_enabled);
		if (myData->ass) myData->ass->SetUseDefaultStyle(defaultstyle_enabled);
		myData->use_defaultstyle = defaultstyle_enabled;

		char* str_fontname;
		gParamHost->paramGetHandle(paramSet, "assDefaultFontName", &param, 0);
		gParamHost->paramGetValue(param, &str_fontname);
		if (myData->ass) myData->ass->SetDefaultFontName(str_fontname);

		int fontsize = 24;
		gParamHost->paramGetHandle(paramSet, "assDefaultFontSize", &param, 0);
		gParamHost->paramGetValue(param, &fontsize);
		if (myData->ass) myData->ass->SetDefaultFontSize(fontsize);
		//if (myData->ass && fontsize < 256) {
		//	myData->ass->SetDefaultFont(str_fontname, fontsize);
		//}

		RGBAColourD fc = { 0, 0, 0, 1 };
		gParamHost->paramGetHandle(paramSet, "assDefaultFontColor", &param, 0);
		gParamHost->paramGetValue(param, &fc.r, &fc.g, &fc.b, &fc.a);
		gParamHost->paramGetHandle(paramSet, "assDefaultFontColorAlpha", &param, 0);
		gParamHost->paramGetValue(param, &fc.a);
		if (myData->ass) myData->ass->SetDefaultFontColor(fc);

		RGBAColourD fca = { 1, 1, 1, 1 };
		gParamHost->paramGetHandle(paramSet, "assDefaultFontColorAlt", &param, 0);
		gParamHost->paramGetValue(param, &fca.r, &fca.g, &fca.b, &fca.a);
		gParamHost->paramGetHandle(paramSet, "assDefaultFontColorAltAlpha", &param, 0);
		gParamHost->paramGetValue(param, &fca.a);
		if (myData->ass) myData->ass->SetDefaultFontColorAlt(fca);

		RGBAColourD fo = { 0, 0, 0, 1 };
		gParamHost->paramGetHandle(paramSet, "assDefaultOutlineColor", &param, 0);
		gParamHost->paramGetValue(param, &fo.r, &fo.g, &fo.b, &fo.a);
		gParamHost->paramGetHandle(paramSet, "assDefaultOutlineColorAlpha", &param, 0);
		gParamHost->paramGetValue(param, &fo.a);
		if (myData->ass) myData->ass->SetDefaultOutlineColor(fo);

		RGBAColourD fb = { 0, 0, 0, 1 };
		gParamHost->paramGetHandle(paramSet, "assDefaultBackColor", &param, 0);
		gParamHost->paramGetValue(param, &fb.r, &fb.g, &fb.b, &fb.a);
		gParamHost->paramGetHandle(paramSet, "assDefaultBackColorAlpha", &param, 0);
		gParamHost->paramGetValue(param, &fb.a);
		if (myData->ass) myData->ass->SetDefaultBackColor(fb);

		int fbold = 0;
		gParamHost->paramGetHandle(paramSet, "assDefaultBold", &param, 0);
		gParamHost->paramGetValue(param, &fbold);
		if (myData->ass) myData->ass->SetDefaultBold(fbold);

		int fitalic = 0;
		gParamHost->paramGetHandle(paramSet, "assDefaultItalic", &param, 0);
		gParamHost->paramGetValue(param, &fitalic);
		if (myData->ass) myData->ass->SetDefaultItalic(fitalic);

		int funderline = 0;
		gParamHost->paramGetHandle(paramSet, "assDefaultUnderline", &param, 0);
		gParamHost->paramGetValue(param, &funderline);
		if (myData->ass) myData->ass->SetDefaultUnderline(funderline);

		int fstrikeout = 0;
		gParamHost->paramGetHandle(paramSet, "assDefaultStrikeOut", &param, 0);
		gParamHost->paramGetValue(param, &fstrikeout);
		if (myData->ass) myData->ass->SetDefaultStrikeOut(fstrikeout);

		int fborderstyle = 1;
		gParamHost->paramGetHandle(paramSet, "assDefaultBorderStyle", &param, 0);
		gParamHost->paramGetValue(param, &fborderstyle);
		if (myData->ass) myData->ass->SetDefaultBorderStyle(fborderstyle);

		int foutline = 2;
		gParamHost->paramGetHandle(paramSet, "assDefaultOutline", &param, 0);
		gParamHost->paramGetValue(param, &foutline);
		if (myData->ass) myData->ass->SetDefaultOutline(foutline);

		int fshadow = 2;
		gParamHost->paramGetHandle(paramSet, "assDefaultShadow", &param, 0);
		gParamHost->paramGetValue(param, &fshadow);
		if (myData->ass) myData->ass->SetDefaultShadow(fshadow);

		// Global setting
		gParamHost->paramGetHandle(paramSet, "assGlobalBlend", &param, 0);
		gParamHost->paramGetValue(param, &myData->pre_blend);

		RGBAColourD gbc = { 0, 0, 0, 1 };
		gParamHost->paramGetHandle(paramSet, "assGlobalBackColor", &param, 0);
		gParamHost->paramGetValue(param, &gbc.r, &gbc.g, &gbc.b, &gbc.a);
		gParamHost->paramGetHandle(paramSet, "assGlobalBackColorAlpha", &param, 0);
		gParamHost->paramGetValue(param, &gbc.a);
		if (myData->ass) myData->ass->SetGlobalBackColor(gbc);

		//if (myData && myData->ass)
		//  myData->ass->SetDefaultStyle(str_fontname, fontsize, fc, fca, fo, fb, fbold, fitalic, funderline, fstrikeout, fborderstyle, foutline, fshadow);

		char *context = 0;

		// is this instance a general effect ?
		gPropHost->propGetString(effectProps, kOfxImageEffectPropContext, 0, &context);
		if (strcmp(context, kOfxImageEffectContextGenerator) == 0) {
			myData->context = eIsGenerator;
		}
		else if (strcmp(context, kOfxImageEffectContextPaint) == 0) {
			myData->context = eIsPaint;
		}
		else if (strcmp(context, kOfxImageEffectContextFilter) == 0) {
			myData->context = eIsGeneral;
		}
		else {
			myData->context = eIsGeneral;
		}

		gEffectHost->clipGetHandle(effect, kOfxImageEffectOutputClipName, &myData->outputClip, 0);

		// cache away clip handles
		if (myData->context != eIsGenerator) {
			gEffectHost->clipGetHandle(effect, kOfxImageEffectSimpleSourceClipName, &myData->sourceClip, 0);
		} else {
			myData->sourceClip = NULL;
			setDuration(effect);
		}

		return myData;
	}

	static OfxStatus setMyInstanceParam(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
	{
		//RenderAssInstanceData *myData = new RenderAssInstanceData;
		RenderAssInstanceData *myData = getMyInstanceData(effect);
		if (!myData) return kOfxStatReplyDefault;

		char *propType = NULL;
		char *propName = NULL;
		gPropHost->propGetString(inArgs, kOfxPropType, 0, &propType);
		gPropHost->propGetString(inArgs, kOfxPropName, 0, &propName);

		if (!propType || !propName) return kOfxStatReplyDefault;

		if (strcmp(propType, kOfxTypeClip) == 0) {

		}
		else if (strcmp(propType, kOfxTypeParameter) == 0) {
			if (strcmp(propName, "assFileName") == 0) {
				char *fn;
				gParamHost->paramGetValue(myData->assFileName, &fn);
				if (myData->ass && fn[0] != 0) {
					myData->ass->LoadAss(fn, "UTF-8");
					setDuration(effect);
				}
			}
			else if (strcmp(propName, "assOffset") == 0) {
				int offset = (int)myData->Offset;
				gParamHost->paramGetValue(myData->assOffset, &offset);
				if (offset < 1) offset = 0;
				if (myData) myData->Offset = (double)offset;
				if (myData->ass) myData->ass->SetOffset(offset);
			}
			else if (strcmp(propName, "assUseDefaultStyle") == 0) {
				int used_defaultstyle = 0;
				gParamHost->paramGetValue(myData->assUseDefaultStyle, &used_defaultstyle);
				if (myData->ass) myData->ass->SetUseDefaultStyle(used_defaultstyle);
				myData->use_defaultstyle = used_defaultstyle;
			}
			else if (strcmp(propName, "assDefaultFontName") == 0) {
				char* fontname = NULL;
				gParamHost->paramGetValue(myData->assDefaultFontName, &fontname);
				try {
					if (myData->ass) {
						char fn[512];
						memset(fn, 0, 512);
						strcpy_s(fn, fontname);
						utf2gbk(fn, strlen(fontname));
						myData->ass->SetDefaultFontName(fn);
					}
				}
				catch (const std::exception&) {}
			}
			else if (strcmp(propName, "assDefaultFontSize") == 0) {
				int fsize = 0;
				gParamHost->paramGetValue(myData->assDefaultFontSize, &fsize);
				if (myData->ass) myData->ass->SetDefaultFontSize(fsize);
			}
			else if (strcmp(propName, "assDefaultFontColor") == 0) {
				RGBAColourD fc = { 0, 0, 0, 1 };
				gParamHost->paramGetValue(myData->assDefaultFontColor, &fc.r, &fc.g, &fc.b, &fc.a);
				gParamHost->paramGetValue(myData->assDefaultFontColorAlpha, &fc.a);
				if (myData->ass) myData->ass->SetDefaultFontColor(fc);
			}
			else if (strcmp(propName, "assDefaultFontColorAlpha") == 0) {
				RGBAColourD fc = { 0, 0, 0, 1 };
				gParamHost->paramGetValue(myData->assDefaultFontColor, &fc.r, &fc.g, &fc.b, &fc.a);
				gParamHost->paramGetValue(myData->assDefaultFontColorAlpha, &fc.a);
				if (myData->ass) myData->ass->SetDefaultFontColor(fc);
			}
			else if (strcmp(propName, "assDefaultFontColorAlt") == 0) {
				RGBAColourD fca = { 0, 0, 0, 1 };
				gParamHost->paramGetValue(myData->assDefaultFontColorAlt, &fca.r, &fca.g, &fca.b, &fca.a);
				gParamHost->paramGetValue(myData->assDefaultFontColorAltAlpha, &fca.a);
				if (myData->ass) myData->ass->SetDefaultFontColorAlt(fca);
			}
			else if (strcmp(propName, "assDefaultFontColorAltAlpha") == 0) {
				RGBAColourD fca = { 0, 0, 0, 1 };
				gParamHost->paramGetValue(myData->assDefaultFontColorAlt, &fca.r, &fca.g, &fca.b, &fca.a);
				gParamHost->paramGetValue(myData->assDefaultFontColorAltAlpha, &fca.a);
				if (myData->ass) myData->ass->SetDefaultFontColorAlt(fca);
			}
			else if (strcmp(propName, "assDefaultOutlineColor") == 0) {
				RGBAColourD fo = { 0, 0, 0, 1 };
				gParamHost->paramGetValue(myData->assDefaultOutlineColor, &fo.r, &fo.g, &fo.b, &fo.a);
				gParamHost->paramGetValue(myData->assDefaultOutlineColorAlpha, &fo.a);
				if (myData->ass) myData->ass->SetDefaultOutlineColor(fo);
			}
			else if (strcmp(propName, "assDefaultOutlineColorAlpha") == 0) {
				RGBAColourD fo = { 0, 0, 0, 1 };
				gParamHost->paramGetValue(myData->assDefaultOutlineColor, &fo.r, &fo.g, &fo.b, &fo.a);
				gParamHost->paramGetValue(myData->assDefaultOutlineColorAlpha, &fo.a);
				if (myData->ass) myData->ass->SetDefaultOutlineColor(fo);
			}
			else if (strcmp(propName, "assDefaultBackColor") == 0) {
				RGBAColourD fb = { 0, 0, 0, 1 };
				gParamHost->paramGetValue(myData->assDefaultBackColor, &fb.r, &fb.g, &fb.b, &fb.a);
				gParamHost->paramGetValue(myData->assDefaultBackColorAlpha, &fb.a);
				if (myData->ass) myData->ass->SetDefaultBackColor(fb);
			}
			else if (strcmp(propName, "assDefaultBackColorAlpha") == 0) {
				RGBAColourD fb = { 0, 0, 0, 1 };
				gParamHost->paramGetValue(myData->assDefaultBackColor, &fb.r, &fb.g, &fb.b, &fb.a);
				gParamHost->paramGetValue(myData->assDefaultBackColorAlpha, &fb.a);
				if (myData->ass) myData->ass->SetDefaultBackColor(fb);
			}
			else if (strcmp(propName, "assDefaultBold") == 0) {
				int fbold = 0;
				gParamHost->paramGetValue(myData->assDefaultBold, &fbold);
				if (myData->ass) myData->ass->SetDefaultBold(fbold);
			}
			else if (strcmp(propName, "assDefaultItalic") == 0) {
				int fitalic = 0;
				gParamHost->paramGetValue(myData->assDefaultItalic, &fitalic);
				if (myData->ass) myData->ass->SetDefaultItalic(fitalic);
			}
			else if (strcmp(propName, "assDefaultUnderline") == 0) {
				int funderline = 0;
				gParamHost->paramGetValue(myData->assDefaultUnderline, &funderline);
				if (myData->ass) myData->ass->SetDefaultUnderline(funderline);
			}
			else if (strcmp(propName, "assDefaultStrikeOut") == 0) {
				int fstrikeout = 0;
				gParamHost->paramGetValue(myData->assDefaultStrikeOut, &fstrikeout);
				if (myData->ass) myData->ass->SetDefaultStrikeOut(fstrikeout);
			}
			else if (strcmp(propName, "assDefaultBorderStyle") == 0) {
				int fborderstyle = 0;
				gParamHost->paramGetValue(myData->assDefaultBorderStyle, &fborderstyle);
				if (myData->ass) myData->ass->SetDefaultBorderStyle(fborderstyle);
			}
			else if (strcmp(propName, "assDefaultOutline") == 0) {
				int foutline = 0;
				gParamHost->paramGetValue(myData->assDefaultOutline, &foutline);
				if (myData->ass) myData->ass->SetDefaultOutline(foutline);
			}
			else if (strcmp(propName, "assDefaultShadow") == 0) {
				int fshadow = 0;
				gParamHost->paramGetValue(myData->assDefaultShadow, &fshadow);
				if (myData->ass) myData->ass->SetDefaultShadow(fshadow);
			}
			else if (strcmp(propName, "assUseMargin") == 0) {
				int used_margin = 0;
				gParamHost->paramGetValue(myData->assUseMargin, &used_margin);
				if (myData->ass) myData->ass->SetMargin(used_margin);
				myData->use_margin = used_margin;
			}
			else if (strcmp(propName, "assMarginT") == 0) {
				double margin_t = 0, margin_b = 0, margin_l = 0, margin_r = 0;
				gParamHost->paramGetValue(myData->assMarginT, &margin_t);
				gParamHost->paramGetValue(myData->assMarginB, &margin_b);
				gParamHost->paramGetValue(myData->assMarginL, &margin_l);
				gParamHost->paramGetValue(myData->assMarginR, &margin_r);
				if (myData->ass) myData->ass->SetMargin(margin_t, margin_b, margin_l, margin_r);
			}
			else if (strcmp(propName, "assMarginB") == 0) {
				double margin_t = 0, margin_b = 0, margin_l = 0, margin_r = 0;
				gParamHost->paramGetValue(myData->assMarginT, &margin_t);
				gParamHost->paramGetValue(myData->assMarginB, &margin_b);
				gParamHost->paramGetValue(myData->assMarginL, &margin_l);
				gParamHost->paramGetValue(myData->assMarginR, &margin_r);
				if (myData->ass) myData->ass->SetMargin(margin_t, margin_b, margin_l, margin_r);
			}
			else if (strcmp(propName, "assMarginL") == 0) {
				double margin_t = 0, margin_b = 0, margin_l = 0, margin_r = 0;
				gParamHost->paramGetValue(myData->assMarginT, &margin_t);
				gParamHost->paramGetValue(myData->assMarginB, &margin_b);
				gParamHost->paramGetValue(myData->assMarginL, &margin_l);
				gParamHost->paramGetValue(myData->assMarginR, &margin_r);
				if (myData->ass) myData->ass->SetMargin(margin_t, margin_b, margin_l, margin_r);
			}
			else if (strcmp(propName, "assMarginR") == 0) {
				double margin_t = 0, margin_b = 0, margin_l = 0, margin_r = 0;
				gParamHost->paramGetValue(myData->assMarginT, &margin_t);
				gParamHost->paramGetValue(myData->assMarginB, &margin_b);
				gParamHost->paramGetValue(myData->assMarginL, &margin_l);
				gParamHost->paramGetValue(myData->assMarginR, &margin_r);
				if (myData->ass) myData->ass->SetMargin(margin_t, margin_b, margin_l, margin_r);
			}
			else if (strcmp(propName, "assSpace") == 0) {
				double spacing = 0;
				gParamHost->paramGetValue(myData->assSpace, &spacing);
				if (myData->ass) myData->ass->SetSpace(spacing);
			}
			else if (strcmp(propName, "assPosition") == 0) {
				double position = 0;
				gParamHost->paramGetValue(myData->assPosition, &position);
				if (myData->ass) myData->ass->SetPosition(position);
			}
			else if (strcmp(propName, "assFontScale") == 0) {
				double scale = 1.0;
				gParamHost->paramGetValue(myData->assFontScale, &scale);
				if (myData->ass) myData->ass->SetFontScale(scale);
			}
			else if (strcmp(propName, "assFontHints") == 0) {
				int hints = 0;
				gParamHost->paramGetValue(myData->assFontHints, &hints);
				if (myData->ass) {
					switch (hints) {
					case 1:
						myData->ass->SetHints(ASS_HINTING_LIGHT);
						break;
					case 2:
						myData->ass->SetHints(ASS_HINTING_NORMAL);
						break;
					case 3:
						myData->ass->SetHints(ASS_HINTING_NATIVE);
						break;
					default:
						myData->ass->SetHints(ASS_HINTING_NONE);
						break;
					}
				}
			}
			else if (strcmp(propName, "assGlobalBlend") == 0) {
				gParamHost->paramGetValue(myData->assGlobalBlend, &myData->pre_blend);
			}
			else if (strcmp(propName, "assGlobalBackColor") == 0) {
				RGBAColourD fc = { 0, 0, 0, 1 };
				gParamHost->paramGetValue(myData->assGlobalBackColor, &fc.r, &fc.g, &fc.b, &fc.a);
				gParamHost->paramGetValue(myData->assGlobalBackColorAlpha, &fc.a);
				if (myData->ass) myData->ass->SetGlobalBackColor(fc);
			}
			else if (strcmp(propName, "assGlobalBackColorAlpha") == 0) {
				RGBAColourD fc = { 0, 0, 0, 1 };
				gParamHost->paramGetValue(myData->assGlobalBackColor, &fc.r, &fc.g, &fc.b, &fc.a);
				gParamHost->paramGetValue(myData->assGlobalBackColorAlpha, &fc.a);
				if (myData->ass) myData->ass->SetGlobalBackColor(fc);
			}
		}
		return kOfxStatReplyDefault;
	}

	static double getFrameRate(OfxImageEffectHandle effect, OfxImageClipHandle clip)
	{
		OfxPropertySetHandle props;
		gEffectHost->clipGetPropertySet(clip, &props);
		double fps = 0;
		gPropHost->propGetDouble(props, kOfxImageEffectPropFrameRate, 0, &fps);

		RenderAssInstanceData *myData = getMyInstanceData(effect);
		if (myData) {
			myData->FPS = fps;
			if (myData->ass) myData->ass->SetFPS(fps);

			if (myData->context == eIsGenerator) {
				double frames = myData->ass->GetFrames();
				gPropHost->propSetDouble(props, kOfxImageEffectInstancePropEffectDuration, 0, frames);
			}
		}
		return(fps);
	}

	static void getFrameRange(OfxImageEffectHandle effect, OfxImageClipHandle clip)
	{
		OfxPropertySetHandle props;
		gEffectHost->clipGetPropertySet(clip, &props);
		OfxRangeD frame;
		gPropHost->propGetDouble(props, kOfxImageEffectPropFrameRange, 0, &frame.min);
		gPropHost->propGetDouble(props, kOfxImageEffectPropFrameRange, 1, &frame.max);

		RenderAssInstanceData *myData = getMyInstanceData(effect);
		if (myData) {
			myData->FrameStart = frame.min;
			myData->FrameEnd = frame.max;
		}
	}

	static Times getProjectTimes(OfxImageEffectHandle effect, double t = 0)
	{
		Times times;
		times.Current = t;
		times.Min = 0;
		times.Max = 0;

		if (gTimeLineHost2) {
			OfxStatus ret = gTimeLineHost2->getProjectTime(effect, t, &times.Current);
			gTimeLineHost2->getEffectTrimPoints(effect, &times.Min, &times.Max);
		}
		else if (gTimeLineHost1) {
			OfxStatus ret = gTimeLineHost1->getTime(effect, &times.Current);
			gTimeLineHost1->getTimeBounds(effect, &times.Min, &times.Max);
		}
		return(times);
	}

	/** @brief Called at load */
public:
	////////////////////////////////////////////////////////////////////////////////
	// Called at load
	static OfxStatus onLoad(void)
	{
		// fetch the host suites out of the global host pointer
		if (!gHost) return kOfxStatErrMissingHostFeature;

		//gEffectHost = (OfxImageEffectSuiteV1 *)gHost->fetchSuite(gHost->host, kOfxImageEffectSuite, 1);
		//gPropHost = (OfxPropertySuiteV1 *)gHost->fetchSuite(gHost->host, kOfxPropertySuite, 1);
		//if (!gEffectHost || !gPropHost)
		//	return kOfxStatErrMissingHostFeature;

		gOpenGLRenderSuite = (OfxImageEffectOpenGLRenderSuiteV1 *)gHost->fetchSuite(gHost->host, kOfxOpenGLRenderSuite, 1);
		gParametricParameterSuite = (OfxParametricParameterSuiteV1 *)gHost->fetchSuite(gHost->host, kOfxParametricParameterSuite, 1);
		gMessageSuiteV2 = (OfxMessageSuiteV2 *)gHost->fetchSuite(gHost->host, kOfxMessageSuite, 2);
		gProgressSuiteV1 = (OfxProgressSuiteV1 *)gHost->fetchSuite(gHost->host, kOfxProgressSuite, 1);
		gProgressSuiteV2 = (OfxProgressSuiteV2 *)gHost->fetchSuite(gHost->host, kOfxProgressSuite, 2);			
		gTimeLineHost1 = (OfxTimeLineSuiteV1 *)gHost->fetchSuite(gHost->host, kOfxTimeLineSuite, 1);
		gTimeLineHost2 = (OfxTimeLineSuiteV2 *)gHost->fetchSuite(gHost->host, kOfxTimeLineSuite, 2);
		return ofxuFetchHostSuites();

		return kOfxStatOK;
	}

	/// called at unload
	static OfxStatus onUnLoad(void)
	{
		return kOfxStatOK;
	}

	// instance construction
	static OfxStatus createInstance(OfxImageEffectHandle effect) 
	{
		// make my private instance data
		RenderAssInstanceData *myData = getMyInstanceParam(effect);

		// set my private instance data
		ofxuSetEffectInstanceData(effect, (void *)myData);

		return kOfxStatOK;
	}

	////////////////////////////////////////////////////////////////////////////////
	// function called when the instance has been changed by anything
	static OfxStatus instanceChanged(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs) 
	{
		// see why it changed
		char *changeReason;
		gPropHost->propGetString(inArgs, kOfxPropChangeReason, 0, &changeReason);

		//RenderAssInstanceData *myData = new RenderAssInstanceData;
		RenderAssInstanceData *myData = getMyInstanceData(effect);
		if (!myData) return kOfxStatReplyDefault;

		char *propType = NULL;
		char *propName = NULL;
		gPropHost->propGetString(inArgs, kOfxPropType, 0, &propType);
		gPropHost->propGetString(inArgs, kOfxPropName, 0, &propName);

		if (!propType || !propName) return kOfxStatReplyDefault;

		if (strcmp(propType, kOfxTypeClip) == 0) {
			if (strcmp(propName, kOfxImageEffectOutputClipName) == 0 && myData->context == eIsGenerator) {
				double frames = myData->ass->GetFrames();
				gPropHost->propSetDouble(outArgs, kOfxImageEffectInstancePropEffectDuration, 0, frames);
			}
		}
		else if (strcmp(propType, kOfxTypeParameter) == 0) {
			return setMyInstanceParam(effect, inArgs, outArgs);
		}
		// don't trap any others
		return kOfxStatReplyDefault;
	}

	// instance destruction
	static OfxStatus destroyInstance(OfxImageEffectHandle effect) 
	{
		// get my instance data
		RenderAssInstanceData *myData = getMyInstanceData(effect);

		// and delete it
		if (myData) {
			if (myData->ass) myData->ass->~AssRender();
			delete myData;
		}

		return kOfxStatOK;
	}

	static OfxStatus syncData(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
	{
		return kOfxStatOK;
	}

	////////////////////////////////////////////////////////////////////////////////
	// the plugin's description routine
	static OfxStatus describe(OfxImageEffectHandle effect)
	{
		// first fetch the host APIs, this cannot be done before this call
		OfxStatus stat;
		if ((stat = ofxuFetchHostSuites()) != kOfxStatOK)
			return stat;

		// get the property handle for the plugin
		OfxPropertySetHandle effectProps;
		gEffectHost->getPropertySet(effect, &effectProps);

		// We can render both fields in a fielded image in one hit if there is no animation
		// So set the flag that allows us to do this
		gPropHost->propSetInt(effectProps, kOfxImageEffectPluginPropFieldRenderTwiceAlways, 0, 0);

		// say we cannot support multiple pixel depths and let the clip preferences action deal with it all.
		gPropHost->propSetInt(effectProps, kOfxImageEffectPropSupportsMultipleClipDepths, 0, 0);

		// set the bit depths the plugin can handle
		gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedPixelDepths, 0, kOfxBitDepthByte);
		//gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedPixelDepths, 1, kOfxBitDepthShort);
		//gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedPixelDepths, 2, kOfxBitDepthFloat);

		// set plugin label and the group it belongs to
		gPropHost->propSetString(effectProps, kOfxPropLabel, 0, utf(_(PluginLabel)));
		gPropHost->propSetString(effectProps, kOfxImageEffectPluginPropGrouping, 0, PluginAuthor);
		//gPropHost->propSetInt(effectProps, kOfxImageEffectPropSupportsOverlays, 0, 1);

		char ver_str[64] = "";
		//sprintf_s(ver_str, "%d.%d.%d.%d", PluginVersion[0], PluginVersion[1], PluginVersion[2], PluginVersion[3]);
		//gPropHost->propSetIntN(effectProps, kOfxPropVersion, 4, PluginVersion);
		sprintf_s(ver_str, "%d.%d.%d.%d", Version_Majon, Version_Minor, Version_Revision, Version_BuildNo);
		gPropHost->propSetInt(effectProps, kOfxPropVersion, 0, Version_Majon);
		gPropHost->propSetInt(effectProps, kOfxPropVersion, 1, Version_Minor);
		gPropHost->propSetInt(effectProps, kOfxPropVersion, 2, Version_Revision);
		gPropHost->propSetInt(effectProps, kOfxPropVersion, 3, Version_BuildNo);
		gPropHost->propSetString(effectProps, kOfxPropVersionLabel, 0, ver_str);
		gPropHost->propSetString(effectProps, kOfxPropPluginDescription, 0, utf(_(PluginDescription)));
		
		// define the contexts we can be used in
		gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedContexts, 0, kOfxImageEffectContextGenerator);

//#ifdef _DEBUG
		//gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedContexts, 1, kOfxImageEffectContextFilter);
		//gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedContexts, 2, kOfxImageEffectContextPaint);
		//gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedContexts, 2, kOfxImageEffectContextGeneral);		
//#endif // _DEBUG

		return kOfxStatOK;
	}

	// describe the plugin in context
	static OfxStatus describeInContext(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs)
	{
		OfxPropertySetHandle props;
		// define the single output clip in both contexts
		gEffectHost->clipDefine(effect, kOfxImageEffectOutputClipName, &props);

		// set the component types we can handle on out output
		gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 0, kOfxImageComponentRGBA);
		//gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 1, kOfxImageComponentRGB);
		//gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 2, kOfxImageComponentAlpha);
		//gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 3, kOfxImageComponentNone);

		// define the single source clip in both contexts
		gEffectHost->clipDefine(effect, kOfxImageEffectSimpleSourceClipName, &props);

		// set the component types we can handle on our main input
		gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 0, kOfxImageComponentRGBA);
		gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 1, kOfxImageComponentRGB);
		gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 2, kOfxImageComponentAlpha);
		gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 3, kOfxImageComponentNone);

		// get the context from the inArgs handle
		char *context;
		gPropHost->propGetString(inArgs, kOfxImageEffectPropContext, 0, &context);
		bool isGeneratorContext = strcmp(context, kOfxImageEffectContextGenerator) == 0;

		////////////////////////////////////////////////////////////////////////////////
		// define the parameters for this context

		// get a pointer to the effect's parameter set
		OfxParamSetHandle paramSet;
		gEffectHost->getParamSet(effect, &paramSet);

		// params properties
		OfxPropertySetHandle paramProps;

		// set params
		if (paramSet) {
			// make ass file name
			gParamHost->paramDefine(paramSet, kOfxParamTypeString, "assFileName", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assFileName");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("ASS File")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Loaded ASS File")));
			gPropHost->propSetString(paramProps, kOfxParamPropStringMode, 0, kOfxParamStringIsFilePath);
			gPropHost->propSetInt(paramProps, kOfxParamPropStringFilePathExists, 0, 1);

			// make an font size parameter
			gParamHost->paramDefine(paramSet, kOfxParamTypeInteger, "assOffset", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assOffset");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("ASS Offset")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("ASS Time Offset")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 0);
			gPropHost->propSetInt(paramProps, kOfxParamPropMin, 0, 0);
			gPropHost->propSetInt(paramProps, kOfxParamPropMax, 0, 512000);
			gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMin, 0, 0);
			gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMax, 0, 512000);
			gPropHost->propSetInt(paramProps, kOfxParamPropIncrement, 0, 100);

			// make ass properties group
			gParamHost->paramDefine(paramSet, kOfxParamTypeGroup, "AssPosProperties", &paramProps);
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("ASS Positions")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("ASS Positions")));

			// make an ass spacing
			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assSpace", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assSpace");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Spacing")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Ass Spacing level bottom")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 100.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 100.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 1.0);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 1);

			// make an ass position
			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assPosition", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assPosition");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Position")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Ass Position level bottom")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 2.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 100.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 100.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 1.0);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 1);

			// make an ass margin used
			gParamHost->paramDefine(paramSet, kOfxParamTypeBoolean, "assUseMargin", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assUseMargin");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Used Margin")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Ass Margin Used")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 0);

			// make ass properties group
			gParamHost->paramDefine(paramSet, kOfxParamTypeGroup, "AssPosMarginProperties", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosProperties");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Margins")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Margin Properties")));

			// make an ass margin top
			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assMarginT", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosMarginProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assMarginT");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Margin Top")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Ass Margin Top")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 100.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 100.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 1.0);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 1);

			// make an ass margin bottom
			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assMarginB", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosMarginProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assMarginB");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Margin Bottom")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Ass Margin Bottom")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 100.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 100.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 1.0);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 1);

			// make an ass margin left
			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assMarginL", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosMarginProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assMarginL");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Margin Left")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Ass Margin Left")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 100.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 100.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 1.0);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 1);

			// make an ass margin top
			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assMarginR", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosMarginProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assMarginR");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Margin Right")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Ass Margin Right")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 100.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 100.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 1.0);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 1);

			// make ass font properties group
			gParamHost->paramDefine(paramSet, kOfxParamTypeGroup, "AssFontProperties", &paramProps);
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("ASS Font Properties")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("ASS Font Properties")));

			// make an ass font scale
			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assFontScale", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssFontProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assFontScale");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Font Scale")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("ASS Font Scale")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 1.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.1);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 5.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.1);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 5.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 0.1);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 2);

			// make an ass font hinging mode
			gParamHost->paramDefine(paramSet, kOfxParamTypeChoice, "assFontHints", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssFontProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assFontHints");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Font Hinting")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Ass Font Hintint Mode")));
			gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 0, utf(_("None")));
			gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 1, utf(_("Light")));
			gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 2, utf(_("Normal")));
			gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 3, utf(_("Native")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 0);

			/*
			*  Default ASS Font Setings
			*/
			// make using default style
			gParamHost->paramDefine(paramSet, kOfxParamTypeBoolean, "assUseDefaultStyle", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assUseDefaultStyle");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Used Default Style")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Used Default Style to override ASS file")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 0);

			// make default ass properties group
			gParamHost->paramDefine(paramSet, kOfxParamTypeGroup, "defaultAssProperties", &paramProps);
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Default ASS Properties")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default ASS Properties")));

			// make an font name parameter
			gParamHost->paramDefine(paramSet, kOfxParamTypeString, "assDefaultFontName", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultFontName");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Font Name")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Font Name")));
			gPropHost->propSetString(paramProps, kOfxParamPropDefault, 0, "Tahoma");

			// make an font size parameter
			gParamHost->paramDefine(paramSet, kOfxParamTypeInteger, "assDefaultFontSize", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultFontSize");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Font Size")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Font Size")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 24);
			gPropHost->propSetInt(paramProps, kOfxParamPropMin, 0, 8);
			gPropHost->propSetInt(paramProps, kOfxParamPropMax, 0, 256);
			gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMin, 0, 8);
			gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMax, 0, 256);
			gPropHost->propSetInt(paramProps, kOfxParamPropIncrement, 0, 1);
			//gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 1);

			// make an rgba font colour parameter
			gParamHost->paramDefine(paramSet, kOfxParamTypeRGBA, "assDefaultFontColor", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultFontColor");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Font Color")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Font Color")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.18);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 1, 0.18);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 2, 0.18);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 3, 0.90);

			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assDefaultFontColorAlpha", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultFontColorAlpha");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Font Color Alpha")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Font Color Alpha")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.9);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 1.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 1.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 0.05);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 2);

			// make an rgba font alt colour parameter
			gParamHost->paramDefine(paramSet, kOfxParamTypeRGBA, "assDefaultFontColorAlt", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultFontColorAlt");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Alternate Font Color")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Alternate Font Color")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.18);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 1, 0.18);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 2, 0.18);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 3, 0.90);

			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assDefaultFontColorAltAlpha", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultFontColorAltAlpha");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Alt Font Color Alpha")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Alternate Font Color Alpha")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.9);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 1.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 1.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 0.05);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 2);

			// make an rgba font outline colour parameter
			gParamHost->paramDefine(paramSet, kOfxParamTypeRGBA, "assDefaultOutlineColor", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultOutlineColor");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Outline Color")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Outline Color")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 1.00);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 1, 0.85);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 2, 0.00);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 3, 0.75);

			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assDefaultOutlineColorAlpha", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultOutlineColorAlpha");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Outline Color Alpha")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Outline Color Alpha")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.75);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 1.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 1.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 0.05);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 2);

			// make an outline size parameter
			gParamHost->paramDefine(paramSet, kOfxParamTypeInteger, "assDefaultOutline", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultOutline");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Outline Width")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Outline Width")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 2);
			gPropHost->propSetInt(paramProps, kOfxParamPropMin, 0, 0);
			gPropHost->propSetInt(paramProps, kOfxParamPropMax, 0, 4);
			gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMin, 0, 0);
			gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMax, 0, 4);
			gPropHost->propSetInt(paramProps, kOfxParamPropIncrement, 0, 1);

			// make an rgba font background colour parameter
			gParamHost->paramDefine(paramSet, kOfxParamTypeRGBA, "assDefaultBackColor", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultBackColor");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Shadow Color")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Shadow Color")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.50);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 1, 0.50);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 2, 0.50);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 3, 0.50);

			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assDefaultBackColorAlpha", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultBackColorAlpha");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Shadow Color Alpha")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Shadow Color Alpha")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.5);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 1.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 1.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 0.05);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 2);

			// make an shadow size parameter
			gParamHost->paramDefine(paramSet, kOfxParamTypeInteger, "assDefaultShadow", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultShadow");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Shadow Size")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Shadow Size")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 2);
			gPropHost->propSetInt(paramProps, kOfxParamPropMin, 0, 0);
			gPropHost->propSetInt(paramProps, kOfxParamPropMax, 0, 4);
			gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMin, 0, 0);
			gPropHost->propSetInt(paramProps, kOfxParamPropDisplayMax, 0, 4);
			gPropHost->propSetInt(paramProps, kOfxParamPropIncrement, 0, 1);

			// make ass font style properties group
			gParamHost->paramDefine(paramSet, kOfxParamTypeGroup, "AssFontStyleProperties", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Font Style")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Font Style Properties")));

			// make default style with bold
			gParamHost->paramDefine(paramSet, kOfxParamTypeBoolean, "assDefaultBold", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssFontStyleProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultBold");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Bold")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Font Bold Style")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 0);

			// make default style with italic
			gParamHost->paramDefine(paramSet, kOfxParamTypeBoolean, "assDefaultItalic", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssFontStyleProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultItalic");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Italic")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Font Italic Style")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 0);

			// make default style with underline
			gParamHost->paramDefine(paramSet, kOfxParamTypeBoolean, "assDefaultUnderline", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssFontStyleProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultUnderline");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Underline")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Font Underline Style")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 0);

			// make default style with strike out
			gParamHost->paramDefine(paramSet, kOfxParamTypeBoolean, "assDefaultStrikeOut", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssFontStyleProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultStrikeOut");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("StrikeOut")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Font StrikeOut Style")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 0);

			// make default style with border style
			gParamHost->paramDefine(paramSet, kOfxParamTypeChoice, "assDefaultBorderStyle", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultBorderStyle");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Border Style")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Font Border Style Mode")));
			gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 0, utf(_("Outline + Drop Shadow")));
			gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 1, utf(_("Opaque Box")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 0);

			// make default ass properties group
			gParamHost->paramDefine(paramSet, kOfxParamTypeGroup, "assGlobalProperties", &paramProps);
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Global ASS Properties")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Global ASS Properties")));

			// make an rgba global background colour parameter
			gParamHost->paramDefine(paramSet, kOfxParamTypeBoolean, "assGlobalBlend", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "assGlobalProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assGlobalBlend");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Pre-Blend")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Blend Color")));
			gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 1);

			gParamHost->paramDefine(paramSet, kOfxParamTypeRGBA, "assGlobalBackColor", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "assGlobalProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assGlobalBackColor");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Background Color")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Background Color")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.50);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 1, 0.50);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 2, 0.50);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 3, 1.00);

			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assGlobalBackColorAlpha", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "assGlobalProperties");
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assGlobalBackColorAlpha");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, utf(_("Background Color Alpha")));
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, utf(_("Default Background Color Alpha")));
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 1.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 1.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 1.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 0.05);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 2);
		}
		return kOfxStatOK;
	}

	// are the settings of the effect performing an identity operation
	static OfxStatus isIdentity(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
	{
		// retrieve any instance data associated with this effect
		RenderAssInstanceData *myData = getMyInstanceData(effect);
		if (!myData) return kOfxStatReplyDefault;

		// fetch a handle to the point param from the parameter set
		OfxParamSetHandle paramSet;
		gEffectHost->getParamSet(effect, &paramSet);

		OfxParamHandle param;
		OfxPropertySetHandle paramProps;
		gParamHost->paramGetHandle(paramSet, kOfxParamTypeString, &param, &paramProps);

		char *propType;
		char *propName;
		gPropHost->propGetString(inArgs, kOfxPropType, 0, &propType);
		gPropHost->propGetString(inArgs, kOfxPropName, 0, &propName);

		int offset = (int)myData->Offset;
		gParamHost->paramGetValue(myData->assOffset, &offset);
		if (offset < 1) myData->Offset = 0;
		if (myData) myData->Offset = offset;

		// we should not be called on a generator
		if (myData->context != eIsGenerator) {

			// get the render window and the time from the inArgs
			OfxTime time;
			OfxRectI renderWindow;

			gPropHost->propGetDouble(inArgs, kOfxPropTime, 0, &time);
			gPropHost->propGetIntN(inArgs, kOfxImageEffectPropRenderWindow, 4, &renderWindow.x1);

			if (myData->ass) myData->ass->Resize(renderWindow.x2 - renderWindow.x1, renderWindow.y2 - renderWindow.y1);

			return kOfxStatOK;
		}

		// In this case do the default, which in this case is to render
		return kOfxStatReplyDefault;
	}

	// Set our clip preferences
	static OfxStatus getClipPreferences(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
	{
		try {
			// retrieve any instance data associated with this effect
			RenderAssInstanceData *myData = getMyInstanceData(effect);

			OfxPropertySetHandle props;
			//OfxImageClipHandle clipHandle;
			//gEffectHost->clipGetHandle(effect, kOfxImageEffectOutputClipName, &clipHandle, &props);
			gEffectHost->getPropertySet(effect, &props);

			// fetch output clip
			OfxImageClipHandle outputClip;
			gEffectHost->clipGetHandle(effect, kOfxImageEffectOutputClipName, &outputClip, 0);
			gEffectHost->clipGetPropertySet(outputClip, &props);			

			if (myData) {
				if (myData->context != eIsGenerator) {
					// get the component type and bit depth of our main input
					int  bitDepth;
					bool isRGBA;
					ofxuClipGetFormat(myData->sourceClip, bitDepth, isRGBA, true); // get the unmapped clip component

																				   // get the strings used to label the various bit depths
					const char *bitDepthStr = bitDepth == 8 ? kOfxBitDepthByte : (bitDepth == 16 ? kOfxBitDepthShort : kOfxBitDepthFloat);
					const char *componentStr = isRGBA ? kOfxImageComponentRGBA : kOfxImageComponentAlpha;

					// set out output to be the same same as the input, component and bitdepth
					gPropHost->propSetString(outArgs, "OfxImageClipPropComponents_Output", 0, componentStr);
					if (gHostSupportsMultipleBitDepths)
						gPropHost->propSetString(outArgs, "OfxImageClipPropDepth_Output", 0, bitDepthStr);
				} 
				else {
					setDuration(effect);

					// set out output to be the same same as the input, component and bitdepth
					gPropHost->propSetString(outArgs, "OfxImageClipPropComponents_Output", 0, kOfxBitDepthByte);
					if (gHostSupportsMultipleBitDepths)
						gPropHost->propSetString(outArgs, "OfxImageClipPropDepth_Output", 0, kOfxImageComponentRGBA);

					// as a generator we create premultiplied output
					//gPropHost->propSetString(outArgs, kOfxImageEffectPropPreMultiplication, 0, kOfxImageUnPreMultiplied);
					//gPropHost->propSetString(outArgs, kOfxImageEffectPropPreMultiplication, 0, kOfxImageOpaque);
					gPropHost->propSetString(outArgs, kOfxImageEffectPropPreMultiplication, 0, kOfxImagePreMultiplied);
					gPropHost->propSetInt(outArgs, kOfxImageEffectFrameVarying, 0, 1);
					gPropHost->propSetInt(outArgs, kOfxImageClipPropContinuousSamples, 0, 1);			
				}
			}
		}
		catch (std::exception ex) {

		}

		return kOfxStatOK;
	}

	// Tells the host how many frames we can fill, only called in the general context.
	// This is actually redundant as this is the default behaviour, but for illustrative
	// purposes.
	static OfxStatus getTimeDomain(OfxImageEffectHandle effect, OfxPropertySetHandle /*inArgs*/, OfxPropertySetHandle outArgs)
	{
		RenderAssInstanceData *myData = getMyInstanceData(effect);

		//double sourceRange[2];
		OfxRangeD frame;

		// get the frame range of the source clip
		OfxPropertySetHandle props; 
		gEffectHost->clipGetPropertySet(myData->sourceClip, &props);
		//gPropHost->propGetDoubleN(props, kOfxImageEffectPropFrameRange, 2, sourceRange);
		gPropHost->propGetDouble(props, kOfxImageEffectPropFrameRange, 0, &frame.min);
		gPropHost->propGetDouble(props, kOfxImageEffectPropFrameRange, 1, &frame.max);

		if (myData && myData->ass) {
			double frames = myData->ass->GetFrames((int)myData->Offset);
			//sourceRange[0] = 0;
			//sourceRange[1] = frames;
			frame.min = 0;
			frame.max = frames;
		}
		// set it on the out args
		//gPropHost->propSetDoubleN(outArgs, kOfxImageEffectPropFrameRange, 2, sourceRange);
		gPropHost->propSetDouble(outArgs, kOfxImageEffectPropFrameRange, 0, frame.min);
		gPropHost->propSetDouble(outArgs, kOfxImageEffectPropFrameRange, 1, frame.max);

		return kOfxStatOK;
	}

	// the process code that the host sees
	static OfxStatus render(OfxImageEffectHandle instance, OfxPropertySetHandle inArgs, OfxPropertySetHandle /*outArgs*/)
	{
		// get the render window and the time from the inArgs
		OfxTime time;
		OfxTime time_p;
		OfxRectI renderWindow;
		char* renderDepth;
		int colorDepth = 4;
		OfxStatus status = kOfxStatOK;

		time_p = ofxuGetTime(instance);
		time = ofxuGetTime(inArgs);
		Times tp = getProjectTimes(instance, time);

		gPropHost->propGetIntN(inArgs, kOfxImageEffectPropRenderWindow, 4, &renderWindow.x1);
		gPropHost->propGetString(inArgs, kOfxImageEffectPropComponents, 0, &renderDepth);

		if (renderDepth == kOfxBitDepthNone) colorDepth = 0;
		else if (renderDepth == kOfxImageComponentRGBA) colorDepth = 4;
		else if (renderDepth == kOfxImageComponentRGB) colorDepth = 3;
		else if (renderDepth == kOfxImageComponentAlpha) colorDepth = 1;

		// fetch output clip
		OfxImageClipHandle outputClip;
		gEffectHost->clipGetHandle(instance, kOfxImageEffectOutputClipName, &outputClip, 0);
		getFrameRate(instance, outputClip);

		OfxPropertySetHandle outputImg = NULL, sourceImg = NULL;
		try {
			bool blend = false;
			RenderAssInstanceData *myData = getMyInstanceData(instance);
			if (!myData) throw(new NoImageEx());
			if (myData->Offset < 0.5) time_p = time;
			else time_p = time + myData->Offset;
			//getFrameRange(instance, outputClip);

			// fetch image to render into from that clip
			int dstRowBytes;
			int dstBitDepth;
			bool dstIsAlpha;
			OfxRectI dstRect;
			void *dstPtr;
			outputImg = ofxuGetImage(outputClip, time, dstRowBytes, dstBitDepth, dstIsAlpha, dstRect, dstPtr);
			if (!outputImg) throw NoImageEx();

			if (myData->context != eIsGenerator) {
				// fetch main input clip
				OfxImageClipHandle sourceClip;
				gEffectHost->clipGetHandle(instance, kOfxImageEffectSimpleSourceClipName, &sourceClip, 0);

				// fetch image at render time from that clip
				int srcRowBytes;
				int srcBitDepth;
				bool srcIsAlpha;
				OfxRectI srcRect;
				void *srcPtr;
				sourceImg = ofxuGetImage(sourceClip, time, srcRowBytes, srcBitDepth, srcIsAlpha, srcRect, srcPtr);
				if (!sourceImg) throw NoImageEx();

				errno_t err = copy_source(instance, renderWindow, srcPtr, srcRect, srcRowBytes, dstPtr, dstRect, dstRowBytes);
				blend = true;
			} else {
				colorDepth = 4;
				errno_t err = copy_source(instance, renderWindow, NULL, dstRect, 0, dstPtr, dstRect, dstRowBytes);
				blend = true;
			}

			if (myData->ass)
				myData->ass->GetAss((double)time_p, renderWindow.x2 - renderWindow.x1, renderWindow.y2 - renderWindow.y1, colorDepth, dstPtr, blend && myData->pre_blend);
		}
		catch (NoImageEx &) {
			// if we were interrupted, the failed fetch is fine, just return kOfxStatOK
			// otherwise, something wierd happened
			if (!gEffectHost->abort(instance)) {
				status = kOfxStatFailed;
			}
		}

		if (sourceImg)
			gEffectHost->clipReleaseImage(sourceImg);
		if (outputImg)
			gEffectHost->clipReleaseImage(outputImg);

		// all was well
		return status;
	}

	static OfxStatus renderSeqBegin(OfxImageEffectHandle instance, OfxPropertySetHandle inArgs, OfxPropertySetHandle /*outArgs*/)
	{
		//if (myData->ass != NULL) {
		//	myData->ass->~AssRender();
		//}
		return kOfxStatOK;
	}

	static OfxStatus renderSeqEnd(OfxImageEffectHandle instance, OfxPropertySetHandle inArgs, OfxPropertySetHandle /*outArgs*/)
	{
		//if (myData->ass != NULL) {
		//	myData->ass->~AssRender();
		//}
		return kOfxStatOK;
	}

	////////////////////////////////////////////////////////////////////////////////
	// The main entry point function
	static OfxStatus pluginMain(const char *action, const void *handle, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
	{
		if (!action) return kOfxStatReplyDefault;

		try {
			// cast to appropriate type
			OfxImageEffectHandle effect = (OfxImageEffectHandle)handle;

			if (strcmp(action, kOfxActionLoad) == 0) {
				return onLoad();
			}
			else if (strcmp(action, kOfxActionUnload) == 0) {
				return onUnLoad();
			}
			if (!effect) return kOfxStatReplyDefault;

			if (strcmp(action, kOfxActionDescribe) == 0) {
				return describe(effect);
			}
			else if (strcmp(action, kOfxImageEffectActionDescribeInContext) == 0) {
				return describeInContext(effect, inArgs);
			}
			else if (strcmp(action, kOfxActionCreateInstance) == 0) {
				return createInstance(effect);
			}
			else if (strcmp(action, kOfxActionDestroyInstance) == 0) {
				return destroyInstance(effect);
			}
			else if (strcmp(action, kOfxActionInstanceChanged) == 0) {
				return instanceChanged(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionIsIdentity) == 0) {
				//return isIdentity(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionGetClipPreferences) == 0) {
				return getClipPreferences(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionRender) == 0) {
				return render(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxActionSyncPrivateData) == 0) {
				//return syncData(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionBeginSequenceRender) == 0) {
				//return renderSeqBegin(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionEndSequenceRender) == 0) {
				//return renderSeqEnd(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionGetRegionOfDefinition) == 0) {
				//return renderSeqEnd(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionGetRegionsOfInterest) == 0) {
				//return renderSeqEnd(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionGetTimeDomain) == 0) {
				return getTimeDomain(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionGetFramesNeeded) == 0) {
				//return renderSeqEnd(effect, inArgs, outArgs);
			}		
		}
		catch (std::bad_alloc) {
			// catch memory
			//std::cout << "OFX Plugin Memory error." << std::endl;
			return kOfxStatErrMemory;
		}
		catch (const std::exception) {
			//catch (const std::exception& e) {
				// standard exceptions
				//std::cout << "OFX Plugin error: " << e.what() << std::endl;
			return kOfxStatErrUnknown;
		}
		catch (int err) {
			// ho hum, gone wrong somehow
			return err;
		}
		catch (...) {
			// everything else
			//std::cout << "OFX Plugin error" << std::endl;
			return kOfxStatErrUnknown;
		}

		// other actions to take the default value
		return kOfxStatReplyDefault;
	}

	// function to set the host structure
	static void setHostFunc(OfxHost *hostStruct)
	{
		gHost = hostStruct;
	}

	static OfxPlugin * GetPlugin(void)
	{
		OfxPlugin plugin = {
			kOfxImageEffectPluginApi,
			1,
			PluginIdentifier,
			Version_Majon,
			Version_Minor,
			setHostFunc,
			pluginMain
		};
		return &plugin;
	}

};

//////////////////////////////////////////////////////////////////////////////
// the plugin struct 
static OfxPlugin RenderAssPlugin =
{
	kOfxImageEffectPluginApi,
	1,
	PluginIdentifier,
	Version_Majon,
	Version_Minor,
	RenderASS::setHostFunc,
	RenderASS::pluginMain
};

static OfxPlugin * GetRenderASS(void)
{
	//return RenderASS::GetPlugin();
	char locale[MAX_PATH];
	std::string path(&cur_dll_path[0]);
	path.append("locale");
	s2c(path, locale);

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, locale);
	textdomain(PACKAGE);

	//char* t = _("ASS File");

	return &RenderAssPlugin;
}
