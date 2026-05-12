/*
 * GalGameSequenceScriptModule::MountEngineRuntime 宿主侧实现（Phase 7）
 *
 * 资产工厂与 GalGameScriptExecutorFactory 注册依赖 VGGalgameRuntime，故放在 VGGalgame，
 * 使 VGGalgameScriptSequence 库不再链接 VGGalgameRuntime。
 */

#include "VGGalgameScriptSequence/Module.h"
#include "VGGalgameScriptSequence/Include/Asset/Asset.h"
#include "VGGalgameScriptSequence/Include/Asset/AssetFactory.h"
#include "VGGalgameScriptSequence/Include/ExecutorCreator.h"
#include "VGAsset/Interface/AssetFactory.h"
#include "VGGalgameRuntime/Interface/IStoryScript.h"

namespace VisionGal::GalGame
{
	void GalGameSequenceScriptModule::MountEngineRuntime()
	{
		/// 注册 Visual Script 资产工厂（序列化 / 编辑器侧加载）。
		EngineAssetFactory::Get().RegisterFactory(MakeScope<GalGameSequenceScriptAssetFactory>());

		/// 向全局执行器工厂注册 Sequence 资产类型对应的创建器（需链接 Runtime 单例实现）。
		GalGameScriptExecutorFactory::Get().RegisterAssetExecutor(
			SequenceScriptAssetType{}.GetNameID(),
			MakeRef<SSExecutorCreatorSequence>());
	}
}
