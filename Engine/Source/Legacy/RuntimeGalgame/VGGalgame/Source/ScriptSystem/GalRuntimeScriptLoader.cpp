/*
 * GalRuntimeScriptLoader 实现
 */

#include "ScriptSystem/GalRuntimeScriptLoader.h"

#include "VGAsset/Interface/Package.h"

namespace VisionGal::GalGame
{
	Ref<IStoryScriptExecutor> GalRuntimeScriptLoader::LoadExecutorForPath(const String& path)
	{
		if (m_Registry != nullptr)
		{
			if (IScriptRuntime* rt = m_Registry->FindRuntimeForPath(path))
			{
				if (Ref<IStoryScriptExecutor> storyScript = rt->CreateScriptExecutor(path))
					return storyScript;
			}
		}

		return GalGameScriptExecutorFactory::Get().LoadAssetExecutor(
			GetAssetTypeNameID(path),
			path
		);
	}
}
