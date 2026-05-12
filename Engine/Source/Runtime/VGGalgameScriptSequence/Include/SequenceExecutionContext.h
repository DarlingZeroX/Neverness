/*
 * SSSequenceExecutionContext — Visual Sequence 运行时执行上下文（唯一数据源）
 *
 * 取代原先的 VisualSSExecutorRunningData：宿主填充本结构，SequenceExecutionInstance 与 IVGSSequenceRuntimeSystem
 * 仅依赖 Context，不再经由中间 RunningData 间接取值。
 *
 * 生命周期：
 * - SequenceData / ResourceManager 使用 Ref（shared_ptr）共享所有权；
 * - Engine 为非拥有指针，由宿主保证 Tick/Execute 调用栈内有效。
 */
#pragma once

#include "../GSSExport.h"
#include "VGCore/Include/Core/Core.h"
#include "VGGalgameCore/Interface/IGameEngine.h"
#include "Sequence/DataContainer.h"
#include "ExecutorResourceManager.h"

namespace VisionGal
{
	/**
	 * @brief Sequence 执行期上下文（可扩展白盒）。
	 */
	struct VG_GSS_API SSSequenceExecutionContext
	{
		/// 当前播放的序列数据容器（组件剪辑序列）。
		Ref<VGSSequenceDataContainer> SequenceData;

		/// 运行时对象注册表（角色 / 精灵 / 音视频绑定）。
		Ref<SSExecutorResourceManager> ResourceManager;

		/// GalGame 引擎实例（ShowSprite、对话管线等）。
		GalGame::IGalGameEngine* Engine = nullptr;

		[[nodiscard]] bool HasSequenceBinding() const noexcept { return SequenceData != nullptr; }

		[[nodiscard]] SSExecutorResourceManager* GetResourceManagerRaw() const noexcept
		{
			return ResourceManager ? ResourceManager.get() : nullptr;
		}
	};
}
