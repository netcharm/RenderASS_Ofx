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

#include <functional>  
#include <cstring>
#include <stdexcept>
#include <new>
#include <tchar.h>
#include <windows.h>
#include <math.h>
#include <Shlobj.h>
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


namespace OpenFX {
	using namespace std;

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

	// throws this if it can't fetch an image
	class NoImageEx {};

	typedef struct OfxPluginInfo {
	public:
		int Version_Majon;
		int Version_Minor;
		int Version_Revision;
		int Version_BuildNo;

		const char* PluginAuthor;
#ifdef _DEBUG
		const char* PluginLabel;
#else
		const char* PluginLabel;
#endif
		const char* PluginDescription;
#ifdef _DEBUG
		const char* PluginIdentifier;
#else
		const char* PluginIdentifier;
#endif
		//OfxPluginInfo();
		//~OfxPluginInfo();
	}OfxPluginInfo;

	// pointers to various bits of the host
	static OfxHost               *gHost;

	public interface IOfxPlugin {
		virtual OfxStatus pluginMain(const char *action, const void *handle, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs);
		virtual void setHostFunc(OfxHost *hostStruct);
		virtual OfxPlugin * GetPlugin(void);
	};

	public class RenderOfx {
	private:
		const int gHostSupportsMultipleBitDepths = false;

		OfxImageEffectSuiteV1 *gEffectHost = 0;
		OfxPropertySuiteV1    *gPropHost = 0;
		OfxParameterSuiteV1   *gParamHost = 0;
		OfxMemorySuiteV1      *gMemoryHost = 0;
		OfxMultiThreadSuiteV1 *gThreadHost = 0;
		OfxMessageSuiteV1     *gMessageSuite = 0;
		OfxInteractSuiteV1    *gInteractHost = 0;
		OfxTimeLineSuiteV1    *gTimeLineHost1 = 0;
		OfxTimeLineSuiteV2    *gTimeLineHost2 = 0;

		virtual OfxPluginInfo* getPluginInfo(void);
	public:
		static RenderOfx* ofx;
		RenderOfx();
		~RenderOfx();

		// actions callback
		virtual OfxStatus onLoad(void);
		virtual OfxStatus onUnLoad(void);

		virtual OfxStatus createInstance(OfxImageEffectHandle effect);
		virtual OfxStatus instanceChanged(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs);
		virtual OfxStatus destroyInstance(OfxImageEffectHandle effect);

		virtual OfxStatus describe(OfxImageEffectHandle effect);
		virtual OfxStatus describeInContext(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs);

		virtual OfxStatus isIdentity(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs);

		virtual OfxStatus getClipPreferences(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs);

		virtual OfxStatus getTimeDomain(OfxImageEffectHandle effect, OfxPropertySetHandle /*inArgs*/, OfxPropertySetHandle outArgs);

		virtual OfxStatus syncData(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs);

		virtual OfxStatus renderSeqBegin(OfxImageEffectHandle instance, OfxPropertySetHandle inArgs, OfxPropertySetHandle /*outArgs*/);
		virtual OfxStatus renderSeqEnd(OfxImageEffectHandle instance, OfxPropertySetHandle inArgs, OfxPropertySetHandle /*outArgs*/);
		virtual OfxStatus render(OfxImageEffectHandle instance, OfxPropertySetHandle inArgs, OfxPropertySetHandle /*outArgs*/);

		// main entry
		//virtual OfxStatus pluginMain(const char *action, const void *handle, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)=0;
		//virtual void setHostFunc(OfxHost *hostStruct)=0;
		//virtual OfxPlugin * GetPlugin(RenderOfx*)=0;
	};
	//void (RenderOfx::*shf)(OfxHost*) = &RenderOfx::setHostFunc;
	//OfxStatus(RenderOfx::*pm)(const char*, const void *, OfxPropertySetHandle, OfxPropertySetHandle) = &RenderOfx::pluginMain;
}

#endif
