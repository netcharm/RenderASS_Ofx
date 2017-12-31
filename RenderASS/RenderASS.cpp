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

/** @brief Called at load */

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
	gParamHost->paramGetHandle(paramSet, "assFile", &myData->assFileName, 0);
	gParamHost->paramGetHandle(paramSet, "assDefaultFontName", &myData->assDefaultFontName, 0);
	gParamHost->paramGetHandle(paramSet, "assDefaultFontSize", &myData->assDefaultFontSize, 0);
	gParamHost->paramGetHandle(paramSet, "assDefaultFontColor", &myData->assDefaultFontColor, 0);
	gParamHost->paramGetHandle(paramSet, "assDefaultFontOutline", &myData->assDefaultFontOutline, 0);
	gParamHost->paramGetHandle(paramSet, "assDefaultBackground", &myData->assDefaultBackground, 0);

	// cache away clip handles
	if (myData->context != eIsGenerator)
		gEffectHost->clipGetHandle(effect, kOfxImageEffectSimpleSourceClipName, &myData->sourceClip, 0);
	gEffectHost->clipGetHandle(effect, kOfxImageEffectOutputClipName, &myData->outputClip, 0);

	// set my private instance data
	gPropHost->propSetPointer(effectProps, kOfxPropInstanceData, 0, (void *)myData);

	return kOfxStatOK;
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

// are the settings of the effect performing an identity operation
static OfxStatus
isIdentity(OfxImageEffectHandle effect,
	OfxPropertySetHandle inArgs,
	OfxPropertySetHandle outArgs)
{
	// retrieve any instance data associated with this effect
	MyInstanceData *myData = getMyInstanceData(effect);

	// we should not be called on a generator
	if (myData->context != eIsGenerator) {

		// get the render window and the time from the inArgs
		OfxTime time;
		OfxRectI renderWindow;

		gPropHost->propGetDouble(inArgs, kOfxPropTime, 0, &time);
		gPropHost->propGetIntN(inArgs, kOfxImageEffectPropRenderWindow, 4, &renderWindow.x1);

		// get my rectangle in cannonical coords
		//OfxRectD rect;
		//getCannonicalRect(effect, time, rect);
		//gParamHost->paramGetValueAt

		OfxRGBAColourD col;
		gParamHost->paramGetValueAtTime(myData->assDefaultFontColor, time, &col.r, &col.g, &col.b, &col.a);

		// if the rectangle is transparent or out of the window, then we can do a pass through on to the source clip
		//if (col.a <= 0.0 || rect.x1 > renderWindow.x2 || rect.y1 > renderWindow.y2 ||
		//	rect.x2 < renderWindow.x1 || rect.y2 < renderWindow.y1) {
		//	// set the property in the out args indicating which is the identity clip
		//	gPropHost->propSetString(outArgs, kOfxPropName, 0, kOfxImageEffectSimpleSourceClipName);
		//	return kOfxStatOK;
		//}
		return kOfxStatOK;
	}

	// In this case do the default, which in this case is to render
	return kOfxStatReplyDefault;
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
					dstPix->r = 255 - srcPix->r;
					dstPix->g = 255 - srcPix->g;
					dstPix->b = 255 - srcPix->b;
					dstPix->a = 255 - srcPix->a;
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

		// we are finished with the source images so release them
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

//  describe the plugin in context
static OfxStatus
describeInContext(OfxImageEffectHandle  effect, OfxPropertySetHandle inArgs)
{
	OfxPropertySetHandle props;
	// define the single output clip in both contexts
	gEffectHost->clipDefine(effect, kOfxImageEffectOutputClipName, &props);

	// set the component types we can handle on out output
	gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 0, kOfxImageComponentRGBA);

	// define the single source clip in both contexts
	gEffectHost->clipDefine(effect, kOfxImageEffectSimpleSourceClipName, &props);

	// set the component types we can handle on our main input
	gPropHost->propSetString(props, kOfxImageEffectPropSupportedComponents, 0, kOfxImageComponentRGBA);

	// get the context from the inArgs handle
	char *context;
	gPropHost->propGetString(inArgs, kOfxImageEffectPropContext, 0, &context);
	bool isGeneratorContext = strcmp(context, kOfxImageEffectContextGenerator) == 0;

	OfxPropertySetHandle clipProps;
	// define the single output clip in both contexts
	gEffectHost->clipDefine(effect, kOfxImageEffectOutputClipName, &clipProps);

	// set the component types we can handle on out output
	gPropHost->propSetString(clipProps, kOfxImageEffectPropSupportedComponents, 0, kOfxImageComponentRGBA);
	gPropHost->propSetString(clipProps, kOfxImageEffectPropSupportedComponents, 1, kOfxImageComponentAlpha);
	gPropHost->propSetString(clipProps, kOfxImageClipPropFieldExtraction, 0, kOfxImageFieldSingle);

	if (!isGeneratorContext) {
		// define the single source clip in filter and general contexts
		gEffectHost->clipDefine(effect, kOfxImageEffectSimpleSourceClipName, &clipProps);

		// set the component types we can handle on our main input
		gPropHost->propSetString(clipProps, kOfxImageEffectPropSupportedComponents, 0, kOfxImageComponentRGBA);
		gPropHost->propSetString(clipProps, kOfxImageEffectPropSupportedComponents, 1, kOfxImageComponentAlpha);
		gPropHost->propSetString(clipProps, kOfxImageClipPropFieldExtraction, 0, kOfxImageFieldSingle);
	}

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
	
	OfxPropertySetHandle groupParamProps;

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
	//gPropHost->propSetInt(effectProps, kOfxImageEffectPluginPropFieldRenderTwiceAlways, 0, 0);

	// say we cannot support multiple pixel depths and let the clip preferences action deal with it all.
	gPropHost->propSetInt(effectProps, kOfxImageEffectPropSupportsMultipleClipDepths, 0, 0);

	// set the bit depths the plugin can handle
	gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedPixelDepths, 0, kOfxBitDepthByte);
	gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedPixelDepths, 1, kOfxBitDepthShort);
	gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedPixelDepths, 2, kOfxBitDepthFloat);

	// set plugin label and the group it belongs to
	gPropHost->propSetString(effectProps, kOfxPropLabel, 0, "Render ASS");
	gPropHost->propSetString(effectProps, kOfxImageEffectPluginPropGrouping, 0, "NetCharm");
	gPropHost->propSetInt(effectProps, kOfxImageEffectPropSupportsOverlays, 0, 1);

	// define the contexts we can be used in
	gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedContexts, 0, kOfxImageEffectContextGenerator);
	gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedContexts, 1, kOfxImageEffectContextFilter);
	gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedContexts, 2, kOfxImageEffectContextGeneral);

	return kOfxStatOK;
}

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
		else if (strcmp(action, kOfxActionDescribe) == 0) {
			return describe(effect);
		}
		else if (strcmp(action, kOfxImageEffectActionDescribeInContext) == 0) {
			return describeInContext(effect, inArgs);
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
	catch (const std::exception& e) {
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
