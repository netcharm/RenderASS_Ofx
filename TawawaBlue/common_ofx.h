#pragma once

#ifndef COMMON_OFX
#define COMMON_OFX

#if defined __APPLE__ || defined linux || defined __FreeBSD__
#  define EXPORT __attribute__((visibility("default")))
#elif defined _WIN32
#  define EXPORT OfxExport
#else
#  error Not building on your operating system quite yet
#endif

#include <math.h>

#include "ofxCore.h"
#include "ofxDialog.h"
#include "ofxImageEffect.h"
#include "ofxInteract.h"
#include "ofxKeySyms.h"
#include "ofxMemory.h"
#include "ofxMessage.h"
#include "ofxMultiThread.h"
#include "ofxOld.h"
#include "ofxOpenGLRender.h"
#include "ofxParam.h"
#include "ofxParametricParam.h"
#include "ofxPixels.h"
#include "ofxProgress.h"
#include "ofxProperty.h"
#include "ofxTimeLine.h"

#include "../include/ofxUtilities.H"
//#include "../include/ofxsImageBlender.H"
//#include "../include/ofxsProcessing.H"

typedef struct OfxTimeLineSuiteV2 {

	OfxStatus(*getProjectTime)(void *instance, double EffectTime, double *ProjectTime);
	// converts what host displays in it's user interface to local effect time, could be a large number if host project starts at 12:00:00:00)

	OfxStatus(*getEffectTrimPoints)(void *instance, double *InPoint, double *OutPoint);
	//  for example in an NLE this refers to In and out point handles of the video track on which the effect is applied, this is in effects local time. This is different then frame range and 0 to Duration.

	OfxStatus(*gotoEffectTime)(void *instance, double *time);  // this is in effects local time, if one asks to go to time -5000, it might not be defined
															   // because of this not being supported a lot, this is example of wanting to check if function pointer is NULL as means of seeing if supported

} OfxTimeLineSuiteV2;

#define _R(c)  ((c)>>24)
#define _G(c)  (((c)>>16)&0xFF)
#define _B(c)  (((c)>>8)&0xFF)
#define _A(c)  ((c)&0xFF)

enum ContextEnum {
	eIsGenerator,
	eIsFilter,
	eIsPaint,
	eIsGeneral
};

// pointers to various bits of the host
OfxHost               *gHost;
OfxImageEffectSuiteV1 *gEffectHost = 0;
OfxPropertySuiteV1    *gPropHost = 0;
OfxParameterSuiteV1   *gParamHost = 0;
OfxMemorySuiteV1      *gMemoryHost = 0;
OfxMultiThreadSuiteV1 *gThreadHost = 0;
OfxMessageSuiteV1     *gMessageSuite = 0;
OfxInteractSuiteV1    *gInteractHost = 0;
OfxTimeLineSuiteV1    *gTimeLineHost1 = 0;
OfxTimeLineSuiteV2    *gTimeLineHost2 = 0;


// throws this if it can't fetch an image
class NoImageEx {};

// look up a pixel in the image, does bounds checking to see if it is in the image rectangle
inline OfxRGBAColourB * pixelAddress(OfxRGBAColourB *img, OfxRectI rect, int x, int y, int bytesPerLine) {
	if (x < rect.x1 || x >= rect.x2 || y < rect.y1 || y > rect.y2)
		return 0;
	OfxRGBAColourB *pix = (OfxRGBAColourB *)(((char *)img) + (y - rect.y1) * bytesPerLine);
	pix += x - rect.x1;
	return pix;
}

inline errno_t copy_source(OfxImageEffectHandle instance,
	const OfxRectI rw,
	const void *sp, const OfxRectI sr, const int srb,
	const void *dp, const OfxRectI dr, const int drb) {

	if (sp && dp)
		return(memcpy_s(&dp, srb*(sr.y2 - sr.y1), sp, drb*(dr.y2 - dr.y1)));
	else if (dp) {
		memset(&dp, 0, drb*(dr.y2 - dr.y1));
		return 0;
	}
	return 1;
	/*
	// cast data pointers to 8 bit RGBA
	OfxRGBAColourB *src = (OfxRGBAColourB *)sp;
	OfxRGBAColourB *dst = (OfxRGBAColourB *)dp;

	// and do some inverting
	for (int y = rw.y1; y < rw.y2; y++) {
		if (gEffectHost->abort(instance)) break;

		OfxRGBAColourB *dstPix = pixelAddress(dst, dr, rw.x1, y, drb);

		for (int x = rw.x1; x < rw.x2; x++) {

			OfxRGBAColourB *srcPix = pixelAddress(src, sr, x, y, srb);

			if (src && srcPix) {
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
	*/
}

static OfxPlugin * GetTawawa(void);

EXPORT OfxPlugin * OfxGetPlugin(int nth)
{
	if (nth == 0)
		return GetTawawa();
	return 0;
}

EXPORT int OfxGetNumberOfPlugins(void)
{
	return 1;
}

#endif
