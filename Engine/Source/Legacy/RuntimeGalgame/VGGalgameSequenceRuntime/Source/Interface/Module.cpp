/*
 * GalGameSequenceScriptModule::MountEngineRuntime 宿主侧实现（Phase 7）
 *
 * 资产工厂 GalGameScriptExecutorFactory 位于 VGGalgameCore（IStoryScript.h）；注册调用放在 VGGalgame，
 * 使 VGGalgameSequenceRuntime 库不依赖 VGGalgame 宿主以外的脚本装配细节。
 */

#include "VGGalgameSequenceRuntime/Module.h"
#include "VGGalgameSequenceRuntime/Include/Asset/Asset.h"
#include "VGGalgameSequenceRuntime/Include/Asset/AssetFactory.h"
#include "VGGalgameSequenceRuntime/Include/ExecutorCreator.h"
#include "NNRuntimeAssetLegacy/Interface/AssetFactory.h"
#include "VGGalgameCore/Interface/IStoryScript.h"

namespace VisionGal::GalGame
{
	void GalGameSequenceScriptModule::MountEngineRuntime()
	{
		/// 注册 Visual Script 资产工厂（序列化 / 编辑器侧加载）。
		EngineAssetFactory::Get().RegisterFactory(MakeScope<GalGameSequenceScriptAssetFactory>());

		/// 向全局执行器工厂注册 Sequence 资产类型对应的创建器（工厂单例实现位于 VGGalgameCore）。
		GalGameScriptExecutorFactory::Get().RegisterAssetExecutor(
			SequenceScriptAssetType{}.GetNameID(),
			MakeRef<SSExecutorCreatorSequence>());
	}
}
