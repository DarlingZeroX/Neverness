/*
* SSSequenceExecutionContext — Visual Sequence 运行时执行上下文（唯一数据源）
*
* 取代原先的 VisualSSExecutorRunningData：宿主填充本结构，SequenceExecutionInstance 与 IVGSSequenceRuntimeSystem
* 仅依赖 Context，不再经由中间 RunningData 间接取值。
*
* 生命周期：
* - SequenceData / ResourceManager 使用 Ref（shared_ptr）共享所有权；
* - SubsystemBus 总线指针（若使用）由宿主在 Run 时注入并保持有效。
*/

#pragma once
#include "../GSSExport.h"
#include "Sequence/DataContainer.h"
#include "ExecutorResourceManager.h"

namespace VisionGal
{
	/**
	* @brief Sequence 执行期数据。
	*/
	struct SSSequenceExecutionData
	{
		/// 当前播放的序列数据容器（组件剪辑序列）。
		Ref<VGSSequenceDataContainer> SequenceData;

		SSSequenceExecutionData()
		{
			SequenceData = MakeRef<VGSSequenceDataContainer>();
		}

		~SSSequenceExecutionData() = default;
	};
}
