#pragma once

#ifndef COMMON_OFX
#define COMMON_OFX

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

#endif
