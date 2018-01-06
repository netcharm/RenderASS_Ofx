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
#include "ofxImageEffect.h"
#include "ofxMemory.h"
#include "ofxMultiThread.h"
#include "ofxPixels.h"
#include "../include/ofxUtilities.H"


//extern "C" {
#  include <ass.h>
//}

#include "libass_helper.h"

enum ContextEnum {
	eIsGenerator,
	eIsFilter,
	eIsGeneral
};

// private instance data type
struct MyInstanceData {
	ContextEnum context;
	AssRender * ass;

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

inline void blend_frame(OfxImageEffectHandle instance,
	const ASS_Image* img,
	const OfxRectI renderWindow,
	const void *dstPtr, const OfxRectI dstRect, const int dstRowBytes) {

	if (!img || img->w <= 0 || img->h <= 0) return;

	// cast data pointers to 8 bit RGBA
	OfxRGBAColourB *dst = (OfxRGBAColourB *)dstPtr;

	OfxRectI dstRectAss;
	dstRectAss.x1 = img->dst_x;
	dstRectAss.x2 = img->dst_x + img->w;
	dstRectAss.y1 = dstRect.y2 - (img->dst_y + img->h);
	dstRectAss.y2 = dstRect.y2 - img->dst_y;

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

		//if (!img || img->w <= 0 || img->h <= 0) break;

		OfxRGBAColourB *dstPix = pixelAddress(dst, dstRect, renderWindow.x1, y, dstRowBytes);
		for (int x = renderWindow.x1; x < renderWindow.x2; x++) {
			try
			{
				//if (!img || img->w <= 0 || img->h <= 0) break;

				long idx_x = x - dstRectAss.x1;
				long idx_y = img->h - (y - dstRectAss.y1) - 1;

				if (dst && dstPix && x >= dstRectAss.x1 && x < dstRectAss.x2 && y >= dstRectAss.y1 && y < dstRectAss.y2)
				{
					unsigned long idx = idx_x + idx_y*stride;
					if (idx >= imglen) break;
					ok = (unsigned)src_map[idx];
					ak = ok*a / 255;
					sk = 255 - ak;
					dstPix->a = (unsigned char)ak;
					dstPix->b = (unsigned char)((ak*b + sk*dstPix->b) / 255);
					dstPix->g = (unsigned char)((ak*g + sk*dstPix->g) / 255);
					dstPix->r = (unsigned char)((ak*r + sk*dstPix->r) / 255);
				}
			}
			catch (const std::exception&)
			{

			}
			dstPix++;
		}
	}
}


inline void copy_source(OfxImageEffectHandle instance,
	const OfxRectI renderWindow,
	const void *srcPtr, const OfxRectI srcRect, const int srcRowBytes,
	const void *dstPtr, const OfxRectI dstRect, const int dstRowBytes) {

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

static OfxPlugin * GetRenderASS(void);

// the two mandated functions
EXPORT OfxPlugin * OfxGetPlugin(int nth)
{
	if (nth == 0)
		//return &RenderAssPlugin;
		return GetRenderASS();
	return 0;
}

EXPORT int OfxGetNumberOfPlugins(void)
{
	return 1;
}

#endif
