// Tawawa.cpp : 定义 DLL 应用程序的导出函数。
//
#pragma once
#include "stdafx.h"

#ifndef PLUGIN_TAWAWA
#define PLUGIN_TAWAWA

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
#include <windows.h>
#include <math.h>
#include <Shlobj.h>
#include "ofxImageEffect.h"
#include "ofxMemory.h"
#include "ofxMultiThread.h"
#include "ofxPixels.h"
#include "../include/ofxUtilities.H"


#include "common_ofx.h"


// private instance data type
struct TawawaInstanceData {
	ContextEnum context;
	double strength_r = 0;
	double strength_g = 0;
	double strength_b = 0;

	// handles to the clips we deal with
	OfxImageClipHandle sourceClip;
	OfxImageClipHandle outputClip;

	OfxParamHandle StrengthR;
	OfxParamHandle StrengthG;
	OfxParamHandle StrengthB;
};

// some flags about the host's behaviour
int gHostSupportsMultipleBitDepths = false;

const char* PluginAuthor = "NetCharm";
#ifdef _DEBUG
const char* PluginLabel = "TawawaBlue Debug";
#else
const char* PluginLabel = "TawawaBlue";
#endif
const char* PluginDescription = "Tawawa Blue Tone";
#ifdef _DEBUG
const char* PluginIdentifier = "cn.netcharm.Ofx.Tawawa-d";
#else
const char* PluginIdentifier = "cn.netcharm.Ofx.Tawawa";
#endif

const unsigned int Version_Majon = 1;
const unsigned int Version_Minor = 0;
const unsigned int Version_Revision = 0;
const unsigned int Version_BuildNo = 0;

int PluginVersion[4] = { Version_Majon, Version_Minor, Version_Revision, Version_BuildNo };



/* mandatory function to set up the host structures */
class Tawawa {
private:

	// Convinience wrapper to get private data 
	static TawawaInstanceData * getMyInstanceData(OfxImageEffectHandle effect)
	{
		TawawaInstanceData *myData = (TawawaInstanceData *)ofxuGetEffectInstanceData(effect);
		return myData;
	}

	static TawawaInstanceData * getMyInstanceParam(OfxImageEffectHandle effect) {

		// get a pointer to the effect properties
		OfxPropertySetHandle effectProps;
		gEffectHost->getPropertySet(effect, &effectProps);

		// get a pointer to the effect's parameter set
		OfxParamSetHandle paramSet;
		gEffectHost->getParamSet(effect, &paramSet);

		// get the parameter from the parameter set
		//OfxParamHandle param;
		OfxParamHandle param;

		TawawaInstanceData *myData = new TawawaInstanceData;

		gParamHost->paramGetHandle(paramSet, "StrengthR", &myData->StrengthR, 0);
		gParamHost->paramGetHandle(paramSet, "StrengthG", &myData->StrengthG, 0);
		gParamHost->paramGetHandle(paramSet, "StrengthB", &myData->StrengthB, 0);

		gParamHost->paramGetHandle(paramSet, "StrengthR", &param, 0);
		double StrengthR = 0;
		gParamHost->paramGetValue(param, &StrengthR);
		myData->strength_r = 1 - StrengthR / 100;

		gParamHost->paramGetHandle(paramSet, "StrengthG", &param, 0);
		double StrengthG = 0;
		gParamHost->paramGetValue(param, &StrengthG);
		myData->strength_g = 1 - StrengthG / 100;

		gParamHost->paramGetHandle(paramSet, "StrengthB", &param, 0);
		double StrengthB = 0;
		gParamHost->paramGetValue(param, &StrengthB);
		myData->strength_b = 1 - StrengthB / 100;

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

		// cache away clip handles
		if (myData->context != eIsGenerator)
			gEffectHost->clipGetHandle(effect, kOfxImageEffectSimpleSourceClipName, &myData->sourceClip, 0);
		else
			myData->sourceClip = NULL;

		gEffectHost->clipGetHandle(effect, kOfxImageEffectOutputClipName, &myData->outputClip, 0);

		return myData;
	}

	static double getFrameRate(OfxImageEffectHandle effect, OfxImageClipHandle clip)
	{
		OfxPropertySetHandle props;
		gEffectHost->clipGetPropertySet(clip, &props);
		double fps = 0;
		gPropHost->propGetDouble(props, kOfxImageEffectPropFrameRate, 0, &fps);

		TawawaInstanceData *myData = getMyInstanceData(effect);
		if (myData) {
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

		TawawaInstanceData *myData = getMyInstanceData(effect);
		if (myData) {
		}
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

		return ofxuFetchHostSuites();

		return kOfxStatOK;
	}

	/// called at unload
	static OfxStatus onUnLoad(void)
	{
		return kOfxStatOK;
	}

	//  instance construction
	static OfxStatus createInstance(OfxImageEffectHandle effect)
	{
		// make my private instance data
		TawawaInstanceData *myData = getMyInstanceParam(effect);

		// set my private instance data
		ofxuSetEffectInstanceData(effect, (void *)myData);

		return kOfxStatOK;
	}

	////////////////////////////////////////////////////////////////////////////////
	// function called when the instance has been changed by anything
	static OfxStatus instanceChanged(OfxImageEffectHandle effect,
		OfxPropertySetHandle inArgs,
		OfxPropertySetHandle outArgs)
	{
		// see why it changed
		char *changeReason;
		gPropHost->propGetString(inArgs, kOfxPropChangeReason, 0, &changeReason);

		//TawawaInstanceData *myData = new TawawaInstanceData;
		TawawaInstanceData *myData = getMyInstanceData(effect);
		if (!myData) return kOfxStatReplyDefault;

		char *propType = NULL;
		char *propName = NULL;
		gPropHost->propGetString(inArgs, kOfxPropType, 0, &propType);
		gPropHost->propGetString(inArgs, kOfxPropName, 0, &propName);

		if (!propType || !propName) return kOfxStatReplyDefault;

		if (strcmp(propType, kOfxTypeClip) == 0) {

		}
		else if (strcmp(propType, kOfxTypeParameter) == 0) {
			if (strcmp(propName, "StrengthR") == 0) {
				double StrengthR = 0;
				gParamHost->paramGetValue(myData->StrengthR, &StrengthR);
				if (abs(StrengthR) < 0.001) StrengthR = 0;
				myData->strength_r = 1 - StrengthR / 100;
			}
			else if (strcmp(propName, "StrengthG") == 0) {
				double StrengthG = 0;
				gParamHost->paramGetValue(myData->StrengthG, &StrengthG);
				if (abs(StrengthG) < 0.001) StrengthG = 0;
				myData->strength_g = 1 - StrengthG / 100;
			}
			else if (strcmp(propName, "StrengthB") == 0) {
				double StrengthB = 0;
				gParamHost->paramGetValue(myData->StrengthB, &StrengthB);
				if (abs(StrengthB) < 0.001) StrengthB = 0;
				myData->strength_b = 1 - StrengthB / 100;
			}
		}
		// don't trap any others
		return kOfxStatReplyDefault;
	}

	// instance destruction
	static OfxStatus destroyInstance(OfxImageEffectHandle effect)
	{
		// get my instance data
		TawawaInstanceData *myData = getMyInstanceData(effect);

		// and delete it
		if (myData) {
			delete myData;
		}

		return kOfxStatOK;
	}

	static OfxStatus syncData(OfxImageEffectHandle effect,
		OfxPropertySetHandle inArgs,
		OfxPropertySetHandle outArgs)
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
		gPropHost->propSetString(effectProps, kOfxPropLabel, 0, PluginLabel);
		gPropHost->propSetString(effectProps, kOfxImageEffectPluginPropGrouping, 0, PluginAuthor);
		//gPropHost->propSetInt(effectProps, kOfxImageEffectPropSupportsOverlays, 0, 1);

		char ver_str[64] = "";
		sprintf_s(ver_str, "%d.%d.%d.%d", PluginVersion[0], PluginVersion[1], PluginVersion[2], PluginVersion[3]);
		gPropHost->propSetIntN(effectProps, kOfxPropVersion, 4, PluginVersion);
		gPropHost->propSetString(effectProps, kOfxPropVersionLabel, 0, ver_str);
		gPropHost->propSetString(effectProps, kOfxPropPluginDescription, 0, PluginDescription);

		// define the contexts we can be used in
		gPropHost->propSetString(effectProps, kOfxImageEffectPropSupportedContexts, 0, kOfxImageEffectContextFilter);

		return kOfxStatOK;
	}

	//  describe the plugin in context
	static OfxStatus describeInContext(OfxImageEffectHandle  effect, OfxPropertySetHandle inArgs)
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
			// make Tawawa Blue Strength Red
			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "StrengthR", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "StrengthR");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Strength Red");
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Modify Red Strength");
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, -25.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 25.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, -25.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 25.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 0.1);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 2);

			// make Tawawa Blue Strength Red
			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "StrengthG", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "StrengthG");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Strength Green");
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Modify Green Strength");
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, -25.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 25.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, -25.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 25.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 0.1);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 2);
		
			// make Tawawa Blue Strength Red
			gParamHost->paramDefine(paramSet, kOfxParamTypeDouble, "StrengthB", &paramProps);
			gPropHost->propSetString(paramProps, kOfxParamPropScriptName, 0, "StrengthB");
			gPropHost->propSetString(paramProps, kOfxPropLabel, 0, "Strength Blue");
			gPropHost->propSetString(paramProps, kOfxParamPropHint, 0, "Modify Blue Strength");
			gPropHost->propSetDouble(paramProps, kOfxParamPropDefault, 0, 0.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMin, 0, -25.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropMax, 0, 25.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, -25.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 25.0);
			gPropHost->propSetDouble(paramProps, kOfxParamPropIncrement, 0, 0.1);
			gPropHost->propSetInt(paramProps, kOfxParamPropDigits, 0, 2);
		}
		return kOfxStatOK;
	}

	// are the settings of the effect performing an identity operation
	static OfxStatus isIdentity(OfxImageEffectHandle effect,
		OfxPropertySetHandle inArgs,
		OfxPropertySetHandle outArgs)
	{
		// retrieve any instance data associated with this effect
		TawawaInstanceData *myData = getMyInstanceData(effect);
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

		// we should not be called on a generator
		if (myData->context != eIsGenerator) {

			// get the render window and the time from the inArgs
			OfxTime time;
			OfxRectI renderWindow;

			gPropHost->propGetDouble(inArgs, kOfxPropTime, 0, &time);
			gPropHost->propGetIntN(inArgs, kOfxImageEffectPropRenderWindow, 4, &renderWindow.x1);

			return kOfxStatOK;
		}

		// In this case do the default, which in this case is to render
		return kOfxStatReplyDefault;
	}

	// Set our clip preferences
	static OfxStatus getClipPreferences(OfxImageEffectHandle effect,
		OfxPropertySetHandle inArgs,
		OfxPropertySetHandle outArgs)
	{
		try {
			// retrieve any instance data associated with this effect
			TawawaInstanceData *myData = getMyInstanceData(effect);

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
			}
		}
		catch (std::exception ex) {

		}

		return kOfxStatOK;
	}

	// Tells the host how many frames we can fill, only called in the general context.
	// This is actually redundant as this is the default behaviour, but for illustrative
	// purposes.
	static OfxStatus getTimeDomain(OfxImageEffectHandle  effect, OfxPropertySetHandle /*inArgs*/, OfxPropertySetHandle outArgs)
	{
		TawawaInstanceData *myData = getMyInstanceData(effect);

		double sourceRange[2];

		// get the frame range of the source clip
		OfxPropertySetHandle props; gEffectHost->clipGetPropertySet(myData->sourceClip, &props);
		gPropHost->propGetDoubleN(props, kOfxImageEffectPropFrameRange, 2, sourceRange);

		// set it on the out args
		gPropHost->propSetDoubleN(outArgs, kOfxImageEffectPropFrameRange, 2, sourceRange);

		return kOfxStatOK;
	}


	static inline void blend_frame(OfxImageEffectHandle instance,
		double* strength,
		const OfxRectI renderWindow,
		const void *srcPtr, const OfxRectI srcRect, const int srcRowBytes,
		const void *dstPtr, const OfxRectI dstRect, const int dstRowBytes) {

		if (renderWindow.x1 < 0 || renderWindow.y1 < 0) return;
		if (renderWindow.x2 - renderWindow.x1 <= 0 || renderWindow.y2 - renderWindow.y1 <= 0) return;

		// cast data pointers to 8 bit RGBA
		OfxRGBAColourB *src = (OfxRGBAColourB *)srcPtr;
		OfxRGBAColourB *dst = (OfxRGBAColourB *)dstPtr;
		unsigned char R, G, B, A;
		double Y, U, V;

		/*
		Y = 0.257R + 0.504G + 0.098B + 16
		U = 0.148R - 0.291G + 0.439B + 128
		V = 0.439R - 0.368G - 0.071B + 128
		B = 1.164(Y - 16)                   + 2.018(U - 128)
		G = 1.164(Y - 16) - 0.813(V - 128)  - 0.391(U - 128)
		R = 1.164(Y - 16) + 1.596(V - 128)

		from Keith Jack's excellent book "Video Demystified" (ISBN 1-878707-09-4).
		*/

		double o_r = strength[0];
		double o_g = strength[1];
		double o_b = strength[2];
		for (int y = renderWindow.y1; y < renderWindow.y2; y++) {
			if (gEffectHost->abort(instance)) break;

			OfxRGBAColourB *dstPix = pixelAddress(dst, dstRect, renderWindow.x1, y, dstRowBytes);
			for (int x = renderWindow.x1; x < renderWindow.x2; x++) {
				try
				{
					OfxRGBAColourB *srcPix = pixelAddress(src, srcRect, x, y, srcRowBytes);
					if (dst && dstPix && src && srcPix)
					{
						A = srcPix->a;
						R = srcPix->r;
						G = srcPix->g;
						B = srcPix->b;

						Y = (0.30*R + 0.59*G + 0.11*B);
						U = 0.493*(B - Y);
						V = 0.877*(R - Y);

						R = (unsigned char)(Y > 85 ? ((Y - 85) / 255 * 340) : 0);
						G = (unsigned char)Y;
						B = (unsigned char)(Y > 135 ? 255 : Y + 120);

						int offset_r = (int)((double)R*o_r);
						int offset_g = (int)((double)G*o_g);
						int offset_b = (int)((double)B*o_b);

						if (offset_r > 255) R = 255;
						else if (offset_r < 0) R = 0;
						else R = (unsigned char)offset_r;
						if (offset_g > 255) G = 255;
						else if (offset_g < 0) G = 0;
						else G = (unsigned char)offset_g;
						if (offset_b > 255) B = 255;
						else if (offset_b < 0) B = 0;
						else B = (unsigned char)offset_b;
					
						dstPix->a = (unsigned char)A;
						dstPix->b = (unsigned char)B;
						dstPix->g = (unsigned char)G;
						dstPix->r = (unsigned char)R;
					}
				}
				catch (const std::exception&)
				{

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
		OfxTime time_p;
		OfxRectI renderWindow;
		char* renderDepth;
		int colorDepth = 4;
		OfxStatus status = kOfxStatOK;

		time_p = ofxuGetTime(instance);
		time = ofxuGetTime(inArgs);

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
			//getFrameRange(instance, outputClip);
			// fetch image to render into from that clip
			int dstRowBytes;
			int dstBitDepth;
			bool dstIsAlpha;
			OfxRectI dstRect;
			void *dstPtr;
			outputImg = ofxuGetImage(outputClip, time, dstRowBytes, dstBitDepth, dstIsAlpha, dstRect, dstPtr);
			if (!outputImg) throw NoImageEx();


			TawawaInstanceData *myData = getMyInstanceData(instance);
			if (!myData) throw(new NoImageEx());
			double strength[3] = { myData->strength_r,myData->strength_g,myData->strength_b };

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

				blend_frame(instance, strength, renderWindow, srcPtr, srcRect, srcRowBytes, dstPtr, dstRect, dstRowBytes);
			}
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

	static OfxStatus renderSeqBegin(OfxImageEffectHandle instance,
		OfxPropertySetHandle inArgs,
		OfxPropertySetHandle /*outArgs*/)
	{
		//if (myData->ass != NULL) {
		//	myData->ass->~AssRender();
		//}
		return kOfxStatOK;
	}

	static OfxStatus renderSeqEnd(OfxImageEffectHandle instance,
		OfxPropertySetHandle inArgs,
		OfxPropertySetHandle /*outArgs*/)
	{
		//if (myData->ass != NULL) {
		//	myData->ass->~AssRender();
		//}
		return kOfxStatOK;
	}

	////////////////////////////////////////////////////////////////////////////////
	// The main entry point function
	static OfxStatus pluginMain(const char *action, const void *handle,
		OfxPropertySetHandle inArgs,
		OfxPropertySetHandle outArgs)
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
				return isIdentity(effect, inArgs, outArgs);
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

};

////////////////////////////////////////////////////////////////////////////////
// the plugin struct 
static OfxPlugin TawawaPlugin =
{
	kOfxImageEffectPluginApi,
	1,
	PluginIdentifier,
	Version_Majon,
	Version_Minor,
	Tawawa::setHostFunc,
	Tawawa::pluginMain
};

static OfxPlugin * GetTawawa(void)
{
	return &TawawaPlugin;
}

#endif