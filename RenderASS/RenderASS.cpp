// RenderASS.cpp : 定义 DLL 应用程序的导出函数。
//
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
#include <cstring>
#include <stdexcept>
#include <new>
#include <tchar.h>
#include "ofxImageEffect.h"
#include "ofxMemory.h"
#include "ofxMultiThread.h"
#include "ofxPixels.h"
#include "../include/ofxUtilities.H"

#if defined __APPLE__ || defined linux || defined __FreeBSD__
#  define EXPORT __attribute__((visibility("default")))
#elif defined _WIN32
#  define EXPORT OfxExport
#else
#  error Not building on your operating system quite yet
#endif

#include <ass.h>
#include "libass_helper.h"

AssRender *ass;


// pointers to various bits of the host
OfxHost               *gHost;
OfxImageEffectSuiteV1 *gEffectHost = 0;
OfxPropertySuiteV1    *gPropHost = 0;
OfxParameterSuiteV1   *gParamHost = 0;
OfxMemorySuiteV1      *gMemoryHost = 0;
OfxMultiThreadSuiteV1 *gThreadHost = 0;
OfxMessageSuiteV1     *gMessageSuite = 0;
OfxInteractSuiteV1    *gInteractHost = 0;

// some flags about the host's behaviour
int gHostSupportsMultipleBitDepths = false;

enum ContextEnum {
	eIsGenerator,
	eIsFilter,
	eIsGeneral
};
// private instance data type
struct MyInstanceData {
	ContextEnum context;

	// handles to the clips we deal with
	OfxImageClipHandle sourceClip;
	OfxImageClipHandle outputClip;

	// handles to a our parameters
	OfxParamHandle assFileName;

	OfxParamHandle assUseMargin;
	OfxParamHandle assMarginT;
	OfxParamHandle assMarginB;
	OfxParamHandle assMarginL;
	OfxParamHandle assMarginR;
	OfxParamHandle assSpace;
	OfxParamHandle assPosition;
	OfxParamHandle assFontScale;
	OfxParamHandle assFontHints;

	OfxParamHandle assDefaultFontName;
	OfxParamHandle assDefaultFontSize;
	OfxParamHandle assDefaultFontColor;
	OfxParamHandle assDefaultFontOutline;
	OfxParamHandle assDefaultBackground;
};

/* mandatory function to set up the host structures */


// Convinience wrapper to get private data 
static MyInstanceData *
getMyInstanceData(OfxImageEffectHandle effect)
{
	MyInstanceData *myData = (MyInstanceData *)ofxuGetEffectInstanceData(effect);
	return myData;
}


static double 
getFrameRate(OfxImageEffectHandle effect, OfxImageClipHandle clip)
{
	OfxPropertySetHandle props;
	gEffectHost->clipGetPropertySet(clip, &props);
	double fps = 30.000;
	gPropHost->propGetDouble(props, kOfxImageEffectPropFrameRate, 0, &fps);
	if(ass) ass->SetFPS(fps);
	return(fps);
}
/** @brief Called at load */

////////////////////////////////////////////////////////////////////////////////
// Called at load
static OfxStatus
onLoad(void)
{
	// fetch the host suites out of the global host pointer
	if (!gHost) return kOfxStatErrMissingHostFeature;

	gEffectHost = (OfxImageEffectSuiteV1 *)gHost->fetchSuite(gHost->host, kOfxImageEffectSuite, 1);
	gPropHost = (OfxPropertySuiteV1 *)gHost->fetchSuite(gHost->host, kOfxPropertySuite, 1);
	if (!gEffectHost || !gPropHost)
		return kOfxStatErrMissingHostFeature;

	return kOfxStatOK;
}

/// called at unload
static OfxStatus
onUnLoad(void)
{
	if (ass != NULL) {
		//ass->~AssRender();
	}
	return kOfxStatOK;
}

//  instance construction
static OfxStatus
createInstance(OfxImageEffectHandle effect)
{
	// get a pointer to the effect properties
	OfxPropertySetHandle effectProps;
	gEffectHost->getPropertySet(effect, &effectProps);

	// get a pointer to the effect's parameter set
	OfxParamSetHandle paramSet;
	gEffectHost->getParamSet(effect, &paramSet);

	// make my private instance data
	MyInstanceData *myData = new MyInstanceData;
	char *context = 0;

	// is this instance a general effect ?
	gPropHost->propGetString(effectProps, kOfxImageEffectPropContext, 0, &context);
	if (strcmp(context, kOfxImageEffectContextGenerator) == 0) {
		myData->context = eIsGenerator;
	}
	else if (strcmp(context, kOfxImageEffectContextFilter) == 0) {
		myData->context = eIsFilter;
	}
	else {
		myData->context = eIsGeneral;
	}

	// cache away param handles
	gParamHost->paramGetHandle(paramSet, "assFileName", &myData->assFileName, 0);

	gParamHost->paramGetHandle(paramSet, "assDefaultFontName", &myData->assDefaultFontName, 0);
	gParamHost->paramGetHandle(paramSet, "assDefaultFontSize", &myData->assDefaultFontSize, 0);
	gParamHost->paramGetHandle(paramSet, "assDefaultFontColor", &myData->assDefaultFontColor, 0);
	gParamHost->paramGetHandle(paramSet, "assDefaultFontOutline", &myData->assDefaultFontOutline, 0);
	gParamHost->paramGetHandle(paramSet, "assDefaultBackground", &myData->assDefaultBackground, 0);

	gParamHost->paramGetHandle(paramSet, "assFontScale", &myData->assFontScale, 0);
	gParamHost->paramGetHandle(paramSet, "assFontHints", &myData->assFontHints, 0);

	gParamHost->paramGetHandle(paramSet, "assUseMargin", &myData->assUseMargin, 0);
	gParamHost->paramGetHandle(paramSet, "assMarginT", &myData->assMarginT, 0);
	gParamHost->paramGetHandle(paramSet, "assMarginB", &myData->assMarginB, 0);
	gParamHost->paramGetHandle(paramSet, "assMarginL", &myData->assMarginL, 0);
	gParamHost->paramGetHandle(paramSet, "assMarginR", &myData->assMarginR, 0);
	gParamHost->paramGetHandle(paramSet, "assSpace", &myData->assSpace, 0);
	gParamHost->paramGetHandle(paramSet, "assPosition", &myData->assPosition, 0);

	// cache away clip handles
	if (myData->context != eIsGenerator)
		gEffectHost->clipGetHandle(effect, kOfxImageEffectSimpleSourceClipName, &myData->sourceClip, 0);
	else
		myData->sourceClip = NULL;

	gEffectHost->clipGetHandle(effect, kOfxImageEffectOutputClipName, &myData->outputClip, 0);

	// set my private instance data
	ofxuSetEffectInstanceData(effect, (void *)myData);

	//char* boundle;
	//gPropHost->propGetString(effectProps, kOfxPluginPropFilePath, 0, &boundle);

	//int fps = 0;
	//gPropHost->propGetInt(effectProps, kOfxImageEffectPropFrameRate, 0, &fps);

	//int ufps = 0;
	//gPropHost->propGetInt(effectProps, kOfxImageEffectPropUnmappedFrameRate, 0, &ufps);

	//int sfps = 0;
	//gPropHost->propGetInt(effectProps, kOfxImageEffectPropSetableFrameRate, 0, &sfps);

	if (ass == NULL) {
		ass = new AssRender(ASS_HINTING_NONE, 1.0, "UTF-8");
	}

	return kOfxStatOK;
}

////////////////////////////////////////////////////////////////////////////////
// function called when the instance has been changed by anything
static OfxStatus
instanceChanged(OfxImageEffectHandle effect,
	OfxPropertySetHandle inArgs,
	OfxPropertySetHandle outArgs)
{
	// see why it changed
	char *changeReason;
	gPropHost->propGetString(inArgs, kOfxPropChangeReason, 0, &changeReason);

	MyInstanceData *myData = new MyInstanceData;
	myData = getMyInstanceData(effect);
	if (!myData) return kOfxStatReplyDefault;

	char *propType;
	char *propName;
	gPropHost->propGetString(inArgs, kOfxPropType, 0, &propType);
	gPropHost->propGetString(inArgs, kOfxPropName, 0, &propName);

	if (strcmp(propName, "assFileName") == 0) {
		//char fn[MAX_PATH];
		//memset(fn, 0, MAX_PATH);
		char *fn;
		gParamHost->paramGetValue(myData->assFileName, &fn);
		if (ass != NULL && fn[0] != 0) {
			ass->LoadAss(fn, "UTF-8");
		}
	}
	else if (strcmp(propName, "assDefaultFontName") == 0) {
		char* fontname = NULL;
		gParamHost->paramGetValue(myData->assDefaultFontName, &fontname);
		try
		{
			if (ass != NULL) {
				char fn[512];
				memset(fn, 0, 512);
				strcpy_s(fn, fontname);
				utf2gbk(fn, strlen(fontname));
				ass->SetDefaultFont(fn, 24);
			}
		}
		catch (const std::exception&)
		{

		}	
	}
	else if (strcmp(propName, "assDefaultFontSize") == 0) {
		int fsize = 0;
		gParamHost->paramGetValue(myData->assDefaultFontSize, &fsize);
		int dstfsize = fsize;
	}
	else if (strcmp(propName, "assUseMargin") == 0) {
		int used_margin = 0;
		gParamHost->paramGetValue(myData->assUseMargin, &used_margin);
		if (ass) ass->SetMargin((bool)used_margin);
	}
	else if (strcmp(propName, "assMarginT") == 0) {
		double margin_t = 0, margin_b = 0, margin_l = 0, margin_r = 0;
		gParamHost->paramGetValue(myData->assMarginT, &margin_t);
		gParamHost->paramGetValue(myData->assMarginB, &margin_b);
		gParamHost->paramGetValue(myData->assMarginL, &margin_l);
		gParamHost->paramGetValue(myData->assMarginR, &margin_r);
		if (ass) ass->SetMargin(margin_t, margin_b, margin_l, margin_r);
	}
	else if (strcmp(propName, "assMarginB") == 0) {
		double margin_t = 0, margin_b = 0, margin_l = 0, margin_r = 0;
		gParamHost->paramGetValue(myData->assMarginT, &margin_t);
		gParamHost->paramGetValue(myData->assMarginB, &margin_b);
		gParamHost->paramGetValue(myData->assMarginL, &margin_l);
		gParamHost->paramGetValue(myData->assMarginR, &margin_r);
		if (ass) ass->SetMargin(margin_t, margin_b, margin_l, margin_r);
	}
	else if (strcmp(propName, "assMarginL") == 0) {
		double margin_t = 0, margin_b = 0, margin_l = 0, margin_r = 0;
		gParamHost->paramGetValue(myData->assMarginT, &margin_t);
		gParamHost->paramGetValue(myData->assMarginB, &margin_b);
		gParamHost->paramGetValue(myData->assMarginL, &margin_l);
		gParamHost->paramGetValue(myData->assMarginR, &margin_r);
		if (ass) ass->SetMargin(margin_t, margin_b, margin_l, margin_r);
	}
	else if (strcmp(propName, "assMarginR") == 0) {
		double margin_t = 0, margin_b = 0, margin_l = 0, margin_r = 0;
		gParamHost->paramGetValue(myData->assMarginT, &margin_t);
		gParamHost->paramGetValue(myData->assMarginB, &margin_b);
		gParamHost->paramGetValue(myData->assMarginL, &margin_l);
		gParamHost->paramGetValue(myData->assMarginR, &margin_r);
		if (ass) ass->SetMargin(margin_t, margin_b, margin_l, margin_r);
	}
	else if (strcmp(propName, "assSpace") == 0) {
		double spacing = 0;
		gParamHost->paramGetValue(myData->assSpace, &spacing);
		if (ass) ass->SetSpace(spacing);
	}
	else if (strcmp(propName, "assPosition") == 0) {
		double position = 0;
		gParamHost->paramGetValue(myData->assPosition, &position);
		if (ass) ass->SetSpace(position);
	}
	else if (strcmp(propName, "assFontScale") == 0) {
		double scale = 1.0;
		gParamHost->paramGetValue(myData->assFontScale, &scale);
		if (ass) ass->SetSpace(scale);
	}
	else if (strcmp(propName, "assFontHints") == 0) {
		int hints = 0;
		gParamHost->paramGetValue(myData->assFontHints, &hints);
		if (ass) {
			switch (hints)
			{
			case 1:
				ass->SetHints(ASS_HINTING_LIGHT);
				break;
			case 2:
				ass->SetHints(ASS_HINTING_NORMAL);
				break;
			case 3:
				ass->SetHints(ASS_HINTING_NATIVE);
				break;
			default:
				ass->SetHints(ASS_HINTING_NONE);
				break;
			}
		}
	}
	// don't trap any others
	return kOfxStatReplyDefault;
}

// instance destruction
static OfxStatus
destroyInstance(OfxImageEffectHandle effect)
{
	// get my instance data
	MyInstanceData *myData = getMyInstanceData(effect);

	// and delete it
	if (myData)
		delete myData;
	return kOfxStatOK;
}


////////////////////////////////////////////////////////////////////////////////
// the plugin's description routine
static OfxStatus
describe(OfxImageEffectHandle effect)
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
	gPropHost->propSetString(effectProps, kOfxPropLabel, 0, "Render ASS");
	gPropHost->propSetString(effectProps, kOfxImageEffectPluginPropGrouping, 0, "NetCharm");
	//gPropHost->propSetInt(effectProps, kOfxImageEffectPropSupportsOverlays, 0, 1);

	// define the contexts we can be used in
	gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedContexts, 0, kOfxImageEffectContextFilter);
	gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedContexts, 1, kOfxImageEffectContextGenerator);
	//gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedContexts, 2, kOfxImageEffectContextGeneral);

	return kOfxStatOK;
}

//  describe the plugin in context
static OfxStatus
describeInContext(OfxImageEffectHandle  effect, OfxPropertySetHandle inArgs)
{
	OfxPropertySetHandle props;
	// define the single output clip in both contexts
	gEffectHost->clipDefine(effect, kOfxImageEffectOutputClipName, &props);

	// set the component types we can handle on out output
	gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 0, kOfxImageComponentRGBA);
	gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 1, kOfxImageComponentRGB);
	//gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 1, kOfxImageComponentRGB);
	//gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 2, kOfxImageComponentNone);
	
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

	//OfxPropertySetHandle clipProps;
	//// define the single output clip in both contexts
	//gEffectHost->clipDefine(effect, kOfxImageEffectOutputClipName, &clipProps);

	//// set the component types we can handle on out output
	//gPropHost->propSetString(clipProps, kOfxImageEffectPropSupportedComponents, 0, kOfxImageComponentRGBA);
	////gPropHost->propSetString(clipProps, kOfxImageEffectPropSupportedComponents, 1, kOfxImageComponentAlpha);
	////gPropHost->propSetString(clipProps, kOfxImageClipPropFieldExtraction, 0, kOfxImageFieldSingle);

	//if (!isGeneratorContext) {
	//	OfxPropertySetHandle clipProps;
	//	// define the single source clip in filter and general contexts
	//	gEffectHost->clipDefine(effect, kOfxImageEffectSimpleSourceClipName, &clipProps);

	//	// set the component types we can handle on our main input
	//	gPropHost->propSetString(clipProps, kOfxImageEffectPropSupportedComponents, 0, kOfxImageComponentRGBA);
	//	//gPropHost->propSetString(clipProps, kOfxImageEffectPropSupportedComponents, 1, kOfxImageComponentAlpha);
	//	//gPropHost->propSetString(clipProps, kOfxImageClipPropFieldExtraction, 0, kOfxImageFieldSingle);
	//}

	//return kOfxStatOK;

	////////////////////////////////////////////////////////////////////////////////
	// define the parameters for this context

	// get a pointer to the effect's parameter set
	OfxParamSetHandle paramSet;
	gEffectHost->getParamSet(effect, &paramSet);

	// params properties
	OfxPropertySetHandle paramProps;

	gParamHost->paramDefine(paramSet, kOfxParamTypeString, "assFileName", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assFileName");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "ASS File");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Loaded ASS File");
	gPropHost->propSetString(paramProps, kOfxParamPropStringMode, 0, kOfxParamStringIsFilePath);
	gPropHost->propSetInt(paramProps, kOfxParamPropStringFilePathExists, 0, 1);

	// make ass properties group
	gParamHost->paramDefine(paramSet, kOfxParamTypeGroup, "AssPosProperties", &paramProps);
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "ASS Positions");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "ASS Positions");

	// make an ass spacing
	gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assSpace", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosProperties");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assSpace");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Spacing");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Ass Spacing level bottom");
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
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Position");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Ass Position level bottom");
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
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Used Margin");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Ass Margin Used");
	gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 0);

	// make an ass margin top
	gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assMarginT", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosProperties");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assMarginT");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Margin Top");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Ass Margin Top");
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 100.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 100.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 1.0);
	gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 1);

	// make an ass margin bottom
	gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assMarginB", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosProperties");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assMarginB");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Margin Bottom");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Ass Margin Bottom");
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 100.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 100.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 1.0);
	gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 1);

	// make an ass margin left
	gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assMarginL", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosProperties");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assMarginL");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Margin Left");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Ass Margin Left");
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 100.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 100.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 1.0);
	gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 1);

	// make an ass margin top
	gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assMarginR", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssPosProperties");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assMarginR");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Margin Right");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Ass Margin Right");
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, 0.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 100.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 100.0);
	gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 1.0);
	gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 1);

	// make ass font properties group
	gParamHost->paramDefine(paramSet, kOfxParamTypeGroup, "AssFontProperties", &paramProps);
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "ASS Font Properties");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "ASS Font Properties");

	// make an ass font scale
	gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "assFontScale", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "AssFontProperties");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assFontScale");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Font Scale");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "ASS Font Scale");
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
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Font Hinting");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Ass Font Hintint Mode");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 0, "None");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 1, "Light");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 2, "Normal");
	gPropHost->propSetString(paramProps, kOfxParamPropChoiceOption, 3, "Native");
	gPropHost->propSetInt(paramProps, kOfxParamPropDefault, 0, 0);


	/*
	*  Default ASS Font Setings
	*/
	// make default ass properties group
	gParamHost->paramDefine(paramSet, kOfxParamTypeGroup, "defaultAssProperties", &paramProps);
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Default ASS Properties");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Default ASS Properties");

	// make an font name parameter
	gParamHost->paramDefine(paramSet, kOfxParamTypeString, "assDefaultFontName", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultFontName");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Font Name");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Default Font Name");
	gPropHost->propSetString(paramProps, kOfxParamPropDefault, 0, "Tahoma");

	// make an font size parameter
	gParamHost->paramDefine(paramSet, kOfxParamTypeInteger, "assDefaultFontSize", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultFontSize");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Font Size");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Default Font Size");
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
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Font Color");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Default Font Color");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultFontColor");
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.18);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 1, 0.18);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 2, 0.18);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 3, 0.90);

	// make an rgba font outline colour parameter
	gParamHost->paramDefine(paramSet, kOfxParamTypeRGBA, "assDefaultFontOutline", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultFontOutline");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Outline Color");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Default Outline Color");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultFontOutline");
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 1.00);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 1, 0.85);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 2, 0.00);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 3, 0.75);

	// make an rgba font background colour parameter
	gParamHost->paramDefine(paramSet, kOfxParamTypeRGBA, "assDefaultBackground", &paramProps);
	gPropHost->propSetString(paramProps, kOfxParamPropParent, 0, "defaultAssProperties");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultBackground");
	gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Background Color");
	gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Default Bacgground Color");
	gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "assDefaultBackground");
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.50);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 1, 0.50);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 2, 0.50);
	gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 3, 0.50);

	return kOfxStatOK;
}

// are the settings of the effect performing an identity operation
static OfxStatus
isIdentity(OfxImageEffectHandle effect,
	OfxPropertySetHandle inArgs,
	OfxPropertySetHandle outArgs)
{	
	// retrieve any instance data associated with this effect
	MyInstanceData *myData = getMyInstanceData(effect);
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

	if (strcmp(propName, "assFileName") == 0) {
		char *fn;
		gParamHost->paramGetValue(myData->assFileName, &fn);
		if (ass != NULL && fn[0] != 0) {
			ass->LoadAss(fn, "UTF-8");
		}
	}

	// we should not be called on a generator
	if (myData->context != eIsGenerator) {

		// get the render window and the time from the inArgs
		OfxTime time;
		OfxRectI renderWindow;

		gPropHost->propGetDouble(inArgs, kOfxPropTime, 0, &time);
		gPropHost->propGetIntN(inArgs, kOfxImageEffectPropRenderWindow, 4, &renderWindow.x1);

		OfxRGBAColourD col;
		gParamHost->paramGetValueAtTime(myData->assDefaultFontColor, time, &col.r, &col.g, &col.b, &col.a);

		return kOfxStatOK;
	}

	// In this case do the default, which in this case is to render
	return kOfxStatReplyDefault;
}

// Set our clip preferences
static OfxStatus
getClipPreferences(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
{
	try {
		// retrieve any instance data associated with this effect
		MyInstanceData *myData = getMyInstanceData(effect);

		OfxPropertySetHandle props;
		//OfxImageClipHandle clipHandle;
		//gEffectHost->clipGetHandle(effect, kOfxImageEffectOutputClipName, &clipHandle, &props);
		gEffectHost->getPropertySet(effect, &props);

		// fetch output clip
		OfxImageClipHandle outputClip;
		gEffectHost->clipGetHandle(effect, kOfxImageEffectOutputClipName, &outputClip, 0);
		gEffectHost->clipGetPropertySet(outputClip, &props);

		//double fps = 29.970;
		double fps = 30.000;
		//gPropHost->propGetDouble(props, kOfxImageEffectPropFrameRate, 0, &fps);
		gPropHost->propGetDouble(props, kOfxImageEffectPropFrameRate, 0, &fps);

		if (ass != NULL) ass->SetFPS(fps);

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
		}
	}
	catch (std::exception ex) {

	}

	return kOfxStatOK;
}

// look up a pixel in the image, does bounds checking to see if it is in the image rectangle
inline OfxRGBAColourB *
pixelAddress(OfxRGBAColourB *img, OfxRectI rect, int x, int y, int bytesPerLine)
{
	if (x < rect.x1 || x >= rect.x2 || y < rect.y1 || y > rect.y2)
		return 0;
	OfxRGBAColourB *pix = (OfxRGBAColourB *)(((char *)img) + (y - rect.y1) * bytesPerLine);
	pix += x - rect.x1;
	return pix;
}

// throws this if it can't fetch an image
class NoImageEx {};

inline void 
blend_frame(OfxImageEffectHandle instance, 
	ASS_Image* img,
	OfxRectI renderWindow,
	void *srcPtr, OfxRectI srcRect, int srcRowBytes,
	void *dstPtr, OfxRectI dstRect, int dstRowBytes) {

	if (!img || img->w <= 0 || img->h <= 0) return;

	// cast data pointers to 8 bit RGBA
	OfxRGBAColourB *src = (OfxRGBAColourB *)srcPtr;
	OfxRGBAColourB *dst = (OfxRGBAColourB *)dstPtr;

	OfxRectI dstRectAss;
	dstRectAss.x1 = img->dst_x;
	dstRectAss.x2 = img->dst_x + img->w;
	dstRectAss.y1 = dstRect.y2 - (img->dst_y + img->h);
	dstRectAss.y2 = dstRect.y2 - img->dst_y;
	//dstRectAss.y1 = img->dst_y;
	//dstRectAss.y2 = (img->dst_y + img->h);

	unsigned int stride = (unsigned int)img->stride;
	unsigned int imglen = (unsigned int)(stride*img->h);
	unsigned int a = 255 - (_A(img->color));
	unsigned int r = (unsigned int)_R(img->color);
	unsigned int g = (unsigned int)_G(img->color);
	unsigned int b = (unsigned int)_B(img->color);

	const unsigned char *src_map = img->bitmap;
	unsigned int ok, ak, sk;

	for (int y = renderWindow.y1; y < renderWindow.y2; y++) {
		if (gEffectHost->abort(instance)) break;
				
		if (!img || img->w <= 0 || img->h <= 0) break;

		//OfxRGBAColourB *dstPix = pixelAddress(dst, dstRect, renderWindow.x1, renderWindow.y2 - y - 1, dstRowBytes);
		OfxRGBAColourB *dstPix = pixelAddress(dst, dstRect, renderWindow.x1, y, dstRowBytes);
		for (int x = renderWindow.x1; x < renderWindow.x2; x++) {
			try
			{
				if (!img || img->w <= 0 || img->h <= 0) break;

				long idx_x = x - dstRectAss.x1;
				long idx_y = img->h - (y - dstRectAss.y1) - 1;

				//OfxRGBAColourB *srcPix = pixelAddress(src, srcRect, x, renderWindow.y2 - y - 1, srcRowBytes);
				OfxRGBAColourB *srcPix = pixelAddress(src, srcRect, x, y, srcRowBytes);
				if (dst && dstPix && x >= dstRectAss.x1 && x < dstRectAss.x2 && y >= dstRectAss.y1 && y < dstRectAss.y2)
				{
					unsigned long idx = idx_x + idx_y*stride;
					if (idx >= imglen) break;
					ok = (unsigned)src_map[idx];
					ak = (ok) * a / 255;
					sk = 255 - ak;
					//if (sk > 0 && sk < 255)
					//if (ok > 0)
					{
						dstPix->a = (unsigned char)sk;
						dstPix->b = (unsigned char)((ak*b + sk*srcPix->b) / 255);
						dstPix->g = (unsigned char)((ak*g + sk*srcPix->g) / 255);
						dstPix->r = (unsigned char)((ak*r + sk*srcPix->r) / 255);
					}
					//else
					//{
					//		dstPix->a = 00;
					//		dstPix->b = b;
					//		dstPix->g = g;
					//		dstPix->r = r;
					//}
				}
				else
				{
					if (dst && dstPix && src && srcPix) {
						dstPix->a = srcPix->a;
						dstPix->b = srcPix->b;
						dstPix->g = srcPix->g;
						dstPix->r = srcPix->r;
					}
				}
			}
			catch (const std::exception&)
			{

			}
			dstPix++;
		}
	}
}

inline void
copy_source(OfxImageEffectHandle instance,
	OfxRectI renderWindow,
	void *srcPtr, OfxRectI srcRect, int srcRowBytes,
	void *dstPtr, OfxRectI dstRect, int dstRowBytes) {

	// cast data pointers to 8 bit RGBA
	OfxRGBAColourB *src = (OfxRGBAColourB *)srcPtr;
	OfxRGBAColourB *dst = (OfxRGBAColourB *)dstPtr;

	// and do some inverting
	for (int y = renderWindow.y1; y < renderWindow.y2; y++) {
		if (gEffectHost->abort(instance)) break;

		OfxRGBAColourB *dstPix = pixelAddress(dst, dstRect, renderWindow.x1, y, dstRowBytes);

		for (int x = renderWindow.x1; x < renderWindow.x2; x++) {

			OfxRGBAColourB *srcPix = pixelAddress(src, srcRect, x, y, srcRowBytes);

			if (srcPix) {
				dstPix->r = srcPix->r;
				dstPix->g = srcPix->g;
				dstPix->b = srcPix->b;
				dstPix->a = srcPix->a;
			}
			else {
				dstPix->r = 0;
				dstPix->g = 0;
				dstPix->b = 0;
				dstPix->a = 0;
			}
			dstPix++;
		}
	}

}

// the process code that the host sees
static OfxStatus render(OfxImageEffectHandle instance,
	OfxPropertySetHandle inArgs,
	OfxPropertySetHandle /*outArgs*/)
{
	// get the render window and the time from the inArgs
	OfxTime time;
	OfxRectI renderWindow;
	OfxStatus status = kOfxStatOK;

	gPropHost->propGetDouble(inArgs, kOfxPropTime, 0, &time);
	gPropHost->propGetIntN(inArgs, kOfxImageEffectPropRenderWindow, 4, &renderWindow.x1);

	// fetch output clip
	OfxImageClipHandle outputClip;
	gEffectHost->clipGetHandle(instance, kOfxImageEffectOutputClipName, &outputClip, 0);
	getFrameRate(instance, outputClip);

	OfxPropertySetHandle outputImg = NULL, sourceImg = NULL;
	try {
		// fetch image to render into from that clip
		OfxPropertySetHandle outputImg;
		if (gEffectHost->clipGetImage(outputClip, time, NULL, &outputImg) != kOfxStatOK) {
			throw NoImageEx();
		}

		// fetch output image info from that handle
		int dstRowBytes;
		OfxRectI dstRect;
		void *dstPtr;
		gPropHost->propGetInt(outputImg, kOfxImagePropRowBytes, 0, &dstRowBytes);
		gPropHost->propGetIntN(outputImg, kOfxImagePropBounds, 4, &dstRect.x1);
		gPropHost->propGetInt(outputImg, kOfxImagePropRowBytes, 0, &dstRowBytes);
		gPropHost->propGetPointer(outputImg, kOfxImagePropData, 0, &dstPtr);

		MyInstanceData *myData = getMyInstanceData(instance);
		if (!myData) throw(new NoImageEx());

		ASS_Image *img = NULL;
		if (abs(time - ass->last_time) > 0)	{
			ass->last_time = time;
			img = ass->GetAss((double)time, renderWindow.x2 - renderWindow.x1, renderWindow.y2 - renderWindow.y1);
		}

		if (img) {
			if (myData->context != eIsGenerator) {
				// fetch main input clip
				OfxImageClipHandle sourceClip;
				gEffectHost->clipGetHandle(instance, kOfxImageEffectSimpleSourceClipName, &sourceClip, 0);

				// fetch image at render time from that clip
				if (gEffectHost->clipGetImage(sourceClip, time, NULL, &sourceImg) != kOfxStatOK) {
					throw NoImageEx();
				}

				// fetch image info out of that handle
				int srcRowBytes;
				OfxRectI srcRect;
				void *srcPtr;
				gPropHost->propGetInt(sourceImg, kOfxImagePropRowBytes, 0, &srcRowBytes);
				gPropHost->propGetIntN(sourceImg, kOfxImagePropBounds, 4, &srcRect.x1);
				gPropHost->propGetInt(sourceImg, kOfxImagePropRowBytes, 0, &srcRowBytes);
				gPropHost->propGetPointer(sourceImg, kOfxImagePropData, 0, &srcPtr);

				if (!img)
				{

				}
				else {

				}
				while (img) {
					try
					{
						if (img->w <= 0 || img->h <= 0) continue;

						blend_frame(instance, img, renderWindow, srcPtr, srcRect, srcRowBytes, dstPtr, dstRect, dstRowBytes);

						if (img->next && (unsigned long)img->next < 0xcccc0000) {
							if (img->next->w || !img->next->h) break;
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

				//if (imglist->img_shadow)
				//{
				//	blend_frame(instance, imglist->img_shadow, renderWindow, srcPtr, srcRect, srcRowBytes, dstPtr, dstRect, dstRowBytes);
				//}
				//if (imglist->img_outline)
				//{
				//	blend_frame(instance, imglist->img_outline, renderWindow, srcPtr, srcRect, srcRowBytes, dstPtr, dstRect, dstRowBytes);
				//}
				//if (imglist->img_text)
				//{
				//	blend_frame(instance, imglist->img_text, renderWindow, srcPtr, srcRect, srcRowBytes, dstPtr, dstRect, dstRowBytes);
				//}
			}
			else {
				//if (imglist->img_outline)
				//{
				//	//blend_frame(instance, imglist->img_outline, renderWindow, NULL, dstRect, 0, dstPtr, dstRect, dstRowBytes);
				//}
				//if (imglist->img_shadow)
				//{
				//	blend_frame(instance, imglist->img_shadow, renderWindow, NULL, dstRect, 0, dstPtr, dstRect, dstRowBytes);
				//}
				//if (imglist->img_text)
				//{
				//	blend_frame(instance, imglist->img_text, renderWindow, NULL, dstRect, 0, dstPtr, dstRect, dstRowBytes);
				//}
				// we are finished with the source images so release them
				while (img) {
					try
					{
						if (img->w <= 0 || img->h <= 0)
						{
							img = img->next;
							continue;
						}

						blend_frame(instance, img, renderWindow, NULL, dstRect, 0, dstPtr, dstRect, dstRowBytes);

						if (img->next && (unsigned long)img->next < 0xcccc0000) {
							if (img->next->w || !img->next->h) break;
							if ((unsigned long)img->next->w > 0xcccc0000 ||
								(unsigned long)img->next->h > 0xcccc0000 ||
								(unsigned long)img->next->type > 0xcccc0000) break;
							if (img->next->w <= 0 || img->next->h <= 0 || img->next->type < 0) break;
							img = img->next;
						}
						else break;
					}
					catch (const std::exception&)
					{

					} {}
				}
			}
		}
		else {
			// fetch main input clip
			OfxImageClipHandle sourceClip;
			gEffectHost->clipGetHandle(instance, kOfxImageEffectSimpleSourceClipName, &sourceClip, 0);

			// fetch image at render time from that clip
			if (gEffectHost->clipGetImage(sourceClip, time, NULL, &sourceImg) != kOfxStatOK) {
				throw NoImageEx();
			}

			// fetch image info out of that handle
			int srcRowBytes;
			OfxRectI srcRect;
			void *srcPtr;
			gPropHost->propGetInt(sourceImg, kOfxImagePropRowBytes, 0, &srcRowBytes);
			gPropHost->propGetIntN(sourceImg, kOfxImagePropBounds, 4, &srcRect.x1);
			gPropHost->propGetInt(sourceImg, kOfxImagePropRowBytes, 0, &srcRowBytes);
			gPropHost->propGetPointer(sourceImg, kOfxImagePropData, 0, &srcPtr);

			copy_source(instance, renderWindow, srcPtr, srcRect, srcRowBytes, dstPtr, dstRect, dstRowBytes);
		}
	}
	catch (NoImageEx &) {
		// if we were interrupted, the failed fetch is fine, just return kOfxStatOK
		// otherwise, something wierd happened
		if (!gEffectHost->abort(instance)) {
			status = kOfxStatFailed;
		}
	}
	catch (std::exception) {

	}

	if (sourceImg)
		gEffectHost->clipReleaseImage(sourceImg);
	if (outputImg)
		gEffectHost->clipReleaseImage(outputImg);

	// all was well
	return status;
}



////////////////////////////////////////////////////////////////////////////////
// The main entry point function
static OfxStatus
pluginMain(const char *action, const void *handle, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
{
	try {
		// cast to appropriate type
		OfxImageEffectHandle effect = (OfxImageEffectHandle)handle;

		if (strcmp(action, kOfxActionLoad) == 0) {
			return onLoad();
		}
		//else if (strcmp(action, kOfxActionUnload) == 0) {
		//	return onUnLoad();
		//}
		else if (strcmp(action, kOfxActionDescribe) == 0) {
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
			return isIdentity(effect, inArgs, outArgs);
		}
		else if (strcmp(action, kOfxImageEffectActionGetClipPreferences) == 0) {
			return getClipPreferences(effect, inArgs, outArgs);
		}
		else if (strcmp(action, kOfxImageEffectActionRender) == 0) {
			return render(effect, inArgs, outArgs);
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
static void
setHostFunc(OfxHost *hostStruct)
{
	gHost = hostStruct;
}

////////////////////////////////////////////////////////////////////////////////
// the plugin struct 
static OfxPlugin RenderAssPlugin =
{
	kOfxImageEffectPluginApi,
	1,
	"cn.netcharm.Ofx.RenderASS",
	1,
	0,
	setHostFunc,
	pluginMain
};

// the two mandated functions
EXPORT OfxPlugin *
OfxGetPlugin(int nth)
{
	if (nth == 0)
		return &RenderAssPlugin;
	return 0;
}

EXPORT int
OfxGetNumberOfPlugins(void)
{
	return 1;
}
