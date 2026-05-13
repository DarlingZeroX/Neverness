/*
 * GalAssetTypeScriptRuntime 实现（Phase 8B）
 */

#include "ScriptSystem/GalAssetTypeScriptRuntime.h"

#include "VGAsset/Interface/Package.h"
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
}
