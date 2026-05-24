/*
 * GalAssetTypeScriptRuntime 实现（Phase 8B）
 */

#include "ScriptSystem/GalAssetTypeScriptRuntime.h"

#include "NNRuntimeAssetLegacy/Interface/Package.h"
#include "VGGalgameContract/Interface/IStoryScriptSystem.h"
#include "VGGalgameRuntimeCore/Interface/IStoryScript.h"

namespace VisionGal::GalGame
{
	GalAssetTypeScriptRuntime::GalAssetTypeScriptRuntime(String assetTypeId, String debugName)
		: m_AssetTypeId(std::move(assetTypeId))
		, m_DebugName(std::move(debugName))
	{
	}

	String GalAssetTypeScriptRuntime::GetRuntimeName() const
	{
		return m_DebugName;
	}

	bool GalAssetTypeScriptRuntime::CanLoad(const String& assetPath) const
	{
		return GetAssetTypeNameID(assetPath) == m_AssetTypeId;
	}

	Ref<IStoryScriptExecutor> GalAssetTypeScriptRuntime::CreateScriptExecutor(const String& assetPath)
	{
		return GalGameScriptExecutorFactory::Get().LoadAssetExecutor(m_AssetTypeId, assetPath);
	}

	Ref<IStoryExecutionInstance> GalAssetTypeScriptRuntime::TryCreateStoryExecution(const String& /*assetPath*/, IStoryScriptExecutor* /*executor*/)
	{
		/// 中文：薄工厂适配层不构造自定义 **IStoryExecutionInstance**；由宿主 **StoryExecutionInstance** 包装。
		return nullptr;
	}
}
