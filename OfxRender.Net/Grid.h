// OfxRender.Net.h

#pragma once
#include "common_ofx.h"

using namespace System;

namespace OpenFX {

	public class Grid:public RenderOfx {
	private:

	public:
		static OfxStatus pluginMain(const char *action, const void *handle, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs);
		static void setHostFunc(OfxHost *hostStruct);
		OfxPlugin * GetPlugin(void);
	};

}
