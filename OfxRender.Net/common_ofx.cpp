#pragma once

#include "stdafx.h"
#include "common_ofx.h"

namespace OpenFX {
	using namespace std;
	using namespace System;

	//RenderOfx::RenderOfx()
	//{
	//}

	//RenderOfx::~RenderOfx()
	//{
	//}

	// look up a pixel in the image, does bounds checking to see if it is in the image rectangle
	inline OfxRGBAColourB * pixelAddress(OfxRGBAColourB *img, OfxRectI rect, int x, int y, int bytesPerLine) {
		if (x < rect.x1 || x >= rect.x2 || y < rect.y1 || y > rect.y2)
			return 0;
		OfxRGBAColourB *pix = (OfxRGBAColourB *)(((char *)img) + (y - rect.y1) * bytesPerLine);
		pix += x - rect.x1;
		return pix;
	}

	inline void copy_source(OfxImageEffectHandle instance,
		const OfxRectI rw,
		const void *sp, const OfxRectI sr, const int srb,
		const void *dp, const OfxRectI dr, const int drb) {

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
	}

	/*
	void RenderOfx::setHostFunc(OfxHost *hostStruct)
	{
		gHost = hostStruct;
	}

	
	OfxStatus RenderOfx::pluginMain(const char *action, const void *handle,
		OfxPropertySetHandle inArgs,
		OfxPropertySetHandle outArgs)
	{
		if (!action) return kOfxStatReplyDefault;

		if (!this) return kOfxStatReplyDefault;

		try {
			// cast to appropriate type
			OfxImageEffectHandle effect = (OfxImageEffectHandle)handle;
			if (!effect) return kOfxStatReplyDefault;

			if (strcmp(action, kOfxActionLoad) == 0) {
				return this->onLoad();
			}
			else if (strcmp(action, kOfxActionUnload) == 0) {
				return this->onUnLoad();
			}
			else if (strcmp(action, kOfxActionDescribe) == 0) {
				return this->describe(effect);
			}
			else if (strcmp(action, kOfxImageEffectActionDescribeInContext) == 0) {
				return this->describeInContext(effect, inArgs);
			}
			else if (strcmp(action, kOfxActionCreateInstance) == 0) {
				return this->createInstance(effect);
			}
			else if (strcmp(action, kOfxActionDestroyInstance) == 0) {
				return this->destroyInstance(effect);
			}
			else if (strcmp(action, kOfxActionInstanceChanged) == 0) {
				return this->instanceChanged(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionIsIdentity) == 0) {
				return this->isIdentity(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionGetClipPreferences) == 0) {
				return this->getClipPreferences(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionRender) == 0) {
				return this->render(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxActionSyncPrivateData) == 0) {
				return this->syncData(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionBeginSequenceRender) == 0) {
				return this->renderSeqBegin(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionEndSequenceRender) == 0) {
				return this->renderSeqEnd(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionGetRegionOfDefinition) == 0) {
				return this->renderSeqEnd(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionGetRegionsOfInterest) == 0) {
				return this->renderSeqEnd(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionGetTimeDomain) == 0) {
				return this->getTimeDomain(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionGetFramesNeeded) == 0) {
				return this->renderSeqEnd(effect, inArgs, outArgs);
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
	
	OfxPlugin * RenderOfx::GetPlugin(RenderOfx* ofx)
	{
		OfxPluginInfo* plugInfo = getPluginInfo();

		//function<void(OfxHost*&)> shf = bind(&RenderOfx::setHostFunc, ofx);
		//function<OfxStatus(char*&, void*&, OfxPropertySetHandle&, OfxPropertySetHandle&)> pm = bind(&RenderOfx::pluginMain, ofx, placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4);

		OfxPlugin* plugin = new OfxPlugin();
		plugin->pluginApi = kOfxImageEffectPluginApi;
		plugin->apiVersion = 1;
		plugin->pluginIdentifier = plugInfo->PluginIdentifier;
		plugin->pluginVersionMajor = plugInfo->Version_Majon;
		plugin->pluginVersionMinor = plugInfo->Version_Minor;
		plugin->setHost = &(ofx->setHostFunc);
		plugin->mainEntry = &(ofx->pluginMain);
		//plugin->setHost = &shf;
		//plugin->mainEntry = pm;
		return(plugin);
	}
	*/
	OfxPluginInfo * RenderOfx::getPluginInfo(void)
	{
		OfxPluginInfo* plugInfo = new OfxPluginInfo();
		plugInfo->PluginAuthor = "NetCharm";
#ifdef _DEBUG
		plugInfo->PluginLabel = "Ofx Render.Net Debug";
#else
		plugInfo->PluginLabel = "Ofx Render.Net";
#endif
		plugInfo->PluginDescription = "Ofx.Net Render Filter";
#ifdef _DEBUG
		plugInfo->PluginIdentifier = "cn.netcharm.Ofx.RenderOfx.Net-d";
#else
		plugInfo->PluginIdentifier = "cn.netcharm.Ofx.RenderOfx.Net";
#endif
		plugInfo->Version_Majon = 1;
		plugInfo->Version_Minor = 0;
		plugInfo->Version_Revision = 3;

		return nullptr;
	}

	RenderOfx::RenderOfx()
	{
		//ofx = this;
	}

	RenderOfx::~RenderOfx()
	{
		return;
	}

	OfxStatus RenderOfx::onLoad(void)
	{
		return kOfxStatOK;
	}

	OfxStatus RenderOfx::onUnLoad(void)
	{
		return kOfxStatOK;
	}

	OfxStatus RenderOfx::createInstance(OfxImageEffectHandle effect)
	{
		return kOfxStatOK;
	}

	OfxStatus RenderOfx::instanceChanged(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
	{
		return kOfxStatOK;
	}

	OfxStatus RenderOfx::destroyInstance(OfxImageEffectHandle effect)
	{
		return kOfxStatOK;
	}

	OfxStatus RenderOfx::describe(OfxImageEffectHandle effect)
	{
		return kOfxStatOK;
	}

	OfxStatus RenderOfx::describeInContext(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs)
	{
		return kOfxStatOK;
	}

	OfxStatus RenderOfx::isIdentity(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
	{
		return kOfxStatOK;
	}
	
	OfxStatus RenderOfx::getClipPreferences(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
	{
		return kOfxStatOK;
	}

	OfxStatus RenderOfx::getTimeDomain(OfxImageEffectHandle effect, OfxPropertySetHandle, OfxPropertySetHandle outArgs)
	{
		return kOfxStatOK;
	}

	OfxStatus RenderOfx::syncData(OfxImageEffectHandle effect, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
	{
		return kOfxStatOK;
	}
	
	OfxStatus RenderOfx::renderSeqBegin(OfxImageEffectHandle instance, OfxPropertySetHandle inArgs, OfxPropertySetHandle)
	{
		return kOfxStatOK;
	}

	OfxStatus RenderOfx::renderSeqEnd(OfxImageEffectHandle instance, OfxPropertySetHandle inArgs, OfxPropertySetHandle)
	{
		return kOfxStatOK;
	}

	OfxStatus RenderOfx::render(OfxImageEffectHandle instance, OfxPropertySetHandle inArgs, OfxPropertySetHandle)
	{
		return kOfxStatOK;
	}
}