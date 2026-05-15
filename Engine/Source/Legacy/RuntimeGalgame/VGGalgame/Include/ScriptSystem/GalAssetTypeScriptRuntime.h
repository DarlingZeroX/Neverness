/*
 * GalAssetTypeScriptRuntime — 按资产类型 ID 委托工厂的 IScriptRuntime 实现（Phase 8B / VGGalgame）
 *
 * 中文：薄适配层，把 **GalGameLuaScript** / **GalGameSequenceScript** 等已注册进 **GalGameScriptExecutorFactory**
 * 的类型显式挂到 **GalScriptRuntimeRegistry**，完成「去硬编码分派」的第一步，而不复制加载逻辑。
 */

#pragma once

#include "../../VGGalgameConfig.h"
#include "VGGalgameContract/Interface/IScriptRuntime.h"
#include "VGGalgameContract/Interface/IStoryScriptSystem.h"

namespace VisionGal::GalGame
{
	class VG_GALGAME_API GalAssetTypeScriptRuntime final : public IScriptRuntime
	{
	public:
		~GalAssetTypeScriptRuntime() override = default;

		/// 中文：**assetTypeId** 与 **GalGameScriptExecutorFactory::LoadAssetExecutor** 的第一个参数一致；**debugName** 仅用于日志。
		GalAssetTypeScriptRuntime(String assetTypeId, String debugName);

		String GetRuntimeName() const override;
		bool CanLoad(const String& assetPath) const override;
		Ref<IStoryScriptExecutor> CreateScriptExecutor(const String& assetPath) override;
		Ref<IStoryExecutionInstance> TryCreateStoryExecution(const String& assetPath, IStoryScriptExecutor* executor) override;

	private:
		String m_AssetTypeId;
		String m_DebugName;
	};
}
