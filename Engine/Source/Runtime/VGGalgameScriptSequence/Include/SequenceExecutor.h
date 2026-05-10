/*
 * SSSequenceExecutor — Visual Galgame Sequence Runtime 调度核心（非 God Object）
 *
 * 职责边界：
 * - 仅负责：播放状态机、当前剪辑索引、Wait 闸门、按 Tick 调度 RuntimeSystem；
 * - 不负责：具体 AVG / UI / 动画业务 —— 全部下沉到 IVGSSequenceRuntimeSystem。
 *
 * 对标：Unreal MovieScene Player / Unity Timeline Director 的「调度层」，而非「指令解释器」。
 */
#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "../GSSExport.h"
#include "../Interface/IVGSSequenceRuntimeSystem.h"
#include "SequenceExecutionContext.h"
#include "SequenceRuntimeTypes.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief Visual Sequence 执行器 —— 数据驱动 + 可扩展 RuntimeSystem 注册表。
	 */
	class VG_GSS_API SSSequenceExecutor
	{
	public:
		SSSequenceExecutor();
		~SSSequenceExecutor();

		SSSequenceExecutor(const SSSequenceExecutor&) = delete;
		SSSequenceExecutor& operator=(const SSSequenceExecutor&) = delete;

		/// 绑定执行上下文（非拥有）；存活期由宿主保证，须在整个 Play/Tick 周期内保持有效。
		void SetExecutionContext(SSSequenceExecutionContext* executionContext);

		[[nodiscard]] SSSequenceExecutionContext* GetExecutionContext() const noexcept { return m_ExecutionContext; }

		/// 注册运行时域（查找时逆序遍历 CanExecute：后注册者可覆盖内置系统的匹配）。
		void RegisterRuntimeSystem(std::unique_ptr<IVGSSequenceRuntimeSystem> system);

		IRuntimeInterface* QueryInterface(InterfaceID id);

		void Play();
		void Pause();
		void Resume();
		void Stop();
		void Restart();

		/// 解除 Wait 阻塞（例如玩家点击继续对话）。
		void Continue();

		[[nodiscard]] ESSSequenceExecutorState GetState() const noexcept { return m_State; }
		[[nodiscard]] std::size_t GetCurrentSequenceIndex() const noexcept { return m_CurrentSequenceIndex; }
		[[nodiscard]] bool IsWaiting() const noexcept { return m_PlaybackWaiting; }

		/// 主驱动：每帧调用；内部完成 Dispatch / Wait / 索引前进（单帧最多前进一条非阻塞剪辑）。
		void Tick(float deltaTime);

		[[nodiscard]] SSSequenceRuntimeDebugInfo BuildRuntimeDebugInfo() const;

		/// 与 BuildRuntimeDebugInfo 等价，便于外部 tooling 命名统一。
		[[nodiscard]] SSSequenceRuntimeDebugInfo GetRuntimeDebugInfo() const { return BuildRuntimeDebugInfo(); }

	private:
		[[nodiscard]] SSSequenceExecutionContext BuildExecutionContext() const;

		[[nodiscard]] IVGSSequenceRuntimeSystem* FindRuntimeSystem(IVGSSequenceComponent* component) const;

		[[nodiscard]] IVGSSequenceComponent* GetCurrentComponentRaw() const;

	private:
		SSSequenceExecutionContext* m_ExecutionContext = nullptr;

		std::vector<std::unique_ptr<IVGSSequenceRuntimeSystem>> m_RuntimeSystems;

		ESSSequenceExecutorState m_State = ESSSequenceExecutorState::Stopped;

		std::size_t m_CurrentSequenceIndex = 0;

		/// 当前剪辑是否已在当前索引上执行过 Execute（Wait 期间保持 true）。
		bool m_HasDispatchedCurrentClip = false;

		/// 由 RuntimeSystem 报告的播放栅栏（对话 / 动画 / 异步等）。
		bool m_PlaybackWaiting = false;

		SSSequenceRuntimeDebugInfo m_RuntimeDebugInfo;
	};
}
