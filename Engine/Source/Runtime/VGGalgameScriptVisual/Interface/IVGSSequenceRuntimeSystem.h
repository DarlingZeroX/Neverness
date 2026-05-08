/*
 * IVGSSequenceRuntimeSystem — 数据驱动的 Sequence Runtime 系统接口
 *
 * 扩展方式：
 * - 新增 VGSSC_XXX 组件时：实现一个新的 RuntimeSystem，并在注册表中挂载；
 * - 禁止在 SSSequenceExecutor 内书写 switch(componentType) —— 违反开放封闭原则。
 *
 * Tick：
 * - 默认为空操作；Timeline / Tween / Transition 等可在 Tick 内推进本地状态机。
 */
#pragma once

#include "../VGGalScriptVisualConfig.h"

namespace VisionGal
{
	struct IVGSSequenceComponent;
	struct SSSequenceExecutionContext;

	/**
	 * @brief 单个「运行时域系统」——负责一类或多类语义相近的 Sequence 组件。
	 */
	class VG_GALGAME_VISUAL_SCRIPT_API IVGSSequenceRuntimeSystem
	{
	public:
		virtual ~IVGSSequenceRuntimeSystem() = default;

		/// 是否可由本系统驱动该组件（典型实现：dynamic_cast 探测具体数据类型）。
		[[nodiscard]] virtual bool CanExecute(IVGSSequenceComponent* component) const = 0;

		/// 执行组件对应的运行时副作用（展示对话、切换立绘等）。
		virtual void Execute(IVGSSequenceComponent* component, SSSequenceExecutionContext& context) = 0;

		/**
		 * @brief 每帧回调：用于动画、融合、并行轨道等时间相关逻辑。
		 *
		 * 默认空实现；需要时间的 System 应 override。
		 */
		virtual void Tick(IVGSSequenceComponent* component, SSSequenceExecutionContext& context, float deltaTime)
		{
			(void)component;
			(void)context;
			(void)deltaTime;
		}

		/**
		 * @brief Execute 之后是否阻塞序列索引前进（平均等待玩家输入 / 异步完成）。
		 *
		 * 典型：CommonDialogue::wait == true。
		 */
		[[nodiscard]] virtual bool ShouldHoldPlaybackAfterExecute(IVGSSequenceComponent* component) const = 0;
	};
}
