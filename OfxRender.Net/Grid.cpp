// 这是主 DLL 文件。

#include "stdafx.h"

//#include "common_ofx.h"
#include "Grid.h"

namespace OpenFX {

	OfxStatus Grid::pluginMain(const char *action, const void *handle, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs)
	{
		if (!action) return kOfxStatReplyDefault;

		try {
			// cast to appropriate type
			OfxImageEffectHandle effect = (OfxImageEffectHandle)handle;
			if (!effect) return kOfxStatReplyDefault;

			if (strcmp(action, kOfxActionLoad) == 0) {
				//return onLoad();
			}
			else if (strcmp(action, kOfxActionUnload) == 0) {
				//return onUnLoad();
			}
			else if (strcmp(action, kOfxActionDescribe) == 0) {
				//return describe(effect);
			}
			else if (strcmp(action, kOfxImageEffectActionDescribeInContext) == 0) {
				//return describeInContext(effect, inArgs);
			}
			else if (strcmp(action, kOfxActionCreateInstance) == 0) {
				//return createInstance(effect);
			}
			else if (strcmp(action, kOfxActionDestroyInstance) == 0) {
				//return destroyInstance(effect);
			}
			else if (strcmp(action, kOfxActionInstanceChanged) == 0) {
				//return instanceChanged(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionIsIdentity) == 0) {
				//return isIdentity(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionGetClipPreferences) == 0) {
				//return getClipPreferences(effect, inArgs, outArgs);
			}
			else if (strcmp(action, kOfxImageEffectActionRender) == 0) {
				//return render(effect, inArgs, outArgs);
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
				//return getTimeDomain(effect, inArgs, outArgs);
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

	void Grid::setHostFunc(OfxHost *hostStruct) 
	{
		gHost = hostStruct;
	}

	OfxPlugin * Grid::GetPlugin(void) 
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

		OfxPlugin* plugin = new OfxPlugin;
		plugin->pluginApi = kOfxImageEffectPluginApi;
		plugin->apiVersion = 1;
		plugin->pluginIdentifier = plugInfo->PluginIdentifier;
		plugin->pluginVersionMajor = plugInfo->Version_Majon;
		plugin->pluginVersionMinor = plugInfo->Version_Minor;
		plugin->setHost = setHostFunc;
		plugin->mainEntry = pluginMain;
		return(plugin);
	}
}