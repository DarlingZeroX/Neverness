/*
 * SequenceExecutionInstance — Sequence Runtime Kernel（执行内核）
 *
 * 承接原 SSSequenceExecutor 的全部调度职责：状态机、帧栈（Phase 2A 线性单帧）、Wait、
 * RuntimeSystem 注册与 Tick 管线。对外通过 IStoryExecutionInstance 表达「可替换执行后端」，
 * 对宿主仍可通过具体类型调用 RegisterRuntimeSystem / SetExecutionContext 等。
 */
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../GSSExport.h"
#include "../../Interface/IVGSSequenceRuntimeSystem.h"
#include "../SequenceExecutionContext.h"
#include "../SequenceRuntimeTypes.h"
#include "IStoryExecutionInstance.h"
#include "SequenceExecutionFrame.h"
#include "SequenceRuntimeExecutionContext.h"
#include "SequenceRuntimeCommandAPI.h"
#include "SequenceVariableTable.h"
#include "AsyncWaitRegistry.h"
#include "SequenceSignalBus.h"
#include "ResumeToken.h"

namespace VisionGal
{
	struct IVGSSequenceComponent;
}

namespace VisionGal::GalGame
{
	/**
	 * @brief Sequence 脚本运行时内核实现。
	 *
	 * 线程：与旧版相同，假定在单线程游戏线程上 Tick；未做内部同步。
	 */
	class VG_GSS_API SequenceExecutionInstance final : public IStorySequenceExecutionInstance
	{
	public:
		SequenceExecutionInstance();
		~SequenceExecutionInstance() override;

		SequenceExecutionInstance(const SequenceExecutionInstance&) = delete;
		SequenceExecutionInstance& operator=(const SequenceExecutionInstance&) = delete;

		void SetExecutionContext(SSSequenceExecutionContext* executionContext);

		[[nodiscard]] SSSequenceExecutionContext* GetExecutionContext() const noexcept { return m_ExecutionContext; }

		/// 注册运行时域（逆序 CanExecute / SupportsType 匹配：后注册者优先）。
		void RegisterRuntimeSystem(std::unique_ptr<IVGSSequenceRuntimeSystem> system);

		[[nodiscard]] IRuntimeInterface* QueryInterface(InterfaceID id) override;

		void Play();
		void Pause();
		void Resume();
		void Stop();
		void Restart();

		void Continue() override;

		[[nodiscard]] ESSSequenceExecutorState GetState() const noexcept override { return m_State; }

		/// Phase 2C：仅当 ResumeToken 对应活动帧上的挂起 id 时 Resolve（非虚扩展）。
		void ContinueWithResumeToken(const ResumeToken& token);

		[[nodiscard]] std::uint64_t GetGlobalTickCounter() const noexcept { return m_GlobalTickCounter; }

		[[nodiscard]] std::size_t GetCurrentSequenceIndex() const noexcept;
		[[nodiscard]] bool IsWaiting() const noexcept;

		void Tick(float deltaTime) override;

		[[nodiscard]] SSSequenceRuntimeDebugInfo BuildRuntimeDebugInfo() const;
		[[nodiscard]] SSSequenceRuntimeDebugInfo GetRuntimeDebugInfo() const { return BuildRuntimeDebugInfo(); }

	private:
		friend class SequenceRuntimeCommandAPI;
		friend class SequenceStateSerializer;

		static constexpr std::size_t kFrameTraceCapacity = 32;

		struct FrameTraceEntry
		{
			std::uint64_t GlobalTick = 0;
			std::size_t SequenceIndex = 0;
			bool Waiting = false;
			char Phase[48]{};
		};

		void PushFrameTrace(const char* phase) noexcept;

		void BeginFrame(float deltaTime);
		void TickActiveFrames();
		void ProcessSignals();
		void CleanupFinishedFrames();
		void EndFrame();

		[[nodiscard]] SSSequenceExecutionContext BuildSharedContextCopy() const;

		[[nodiscard]] IVGSSequenceRuntimeSystem* FindRuntimeSystem(IVGSSequenceComponent* component) const;

		[[nodiscard]] IVGSSequenceComponent* GetCurrentComponentRaw() const;

		[[nodiscard]] SequenceRuntimeExecutionContext MakeRuntimeContext(float deltaTime);

		/// 由 CommandAPI::Continue 转调，避免 Continue() 虚调用重入歧义。
		void ClearActiveFrameWaiting();

		/// Phase 2C：按 token 精确 Resolve（若 id 不属于活动帧则忽略）。
		void TryResolveWaitToken(std::uint64_t tokenId) noexcept;

		[[nodiscard]] bool IsLinearWaitBlocking(const SequenceExecutionFrame& frame) const noexcept;

		[[nodiscard]] bool IsParallelWaitBlocking(const SequenceExecutionFrame& frame) const noexcept;

		void TickActiveFramesLinear(SequenceExecutionFrame& frame);
		void TickActiveFramesParallel(SequenceExecutionFrame& frame);

		/// 由 CommandAPI::JumpToSequenceIndex 转调。
		void JumpActiveFrameToIndex(std::size_t index);

		SSSequenceExecutionContext* m_ExecutionContext = nullptr;

		std::vector<std::unique_ptr<IVGSSequenceRuntimeSystem>> m_RuntimeSystems;

		ESSSequenceExecutorState m_State = ESSSequenceExecutorState::Stopped;

		/// 执行帧栈；Phase 2A 线性播放时栈深度恒为 1。
		std::vector<SequenceExecutionFrame> m_Frames;

		SequenceRuntimeCommandAPI m_CommandApi;

		/// Phase 2E：脚本运行时变量（存档由 SequenceStateSerializer 序列化）。
		SequenceVariableTable m_VariableTable;

		/// 本帧 Tick 的 deltaTime，供 MakeRuntimeContext 使用。
		float m_LastDeltaTime = 0.f;

		SSSequenceRuntimeDebugInfo m_RuntimeDebugInfo;
		SSSequenceRuntimeInspectorInfo m_RuntimeInspectorInfo;

		std::uint64_t m_GlobalTickCounter = 0;

		AsyncWaitRegistry m_WaitRegistry;
		SequenceSignalBus m_SignalBus;

		std::array<FrameTraceEntry, kFrameTraceCapacity> m_FrameTrace{};
		/// 环形缓冲区写指针：当 m_FrameTraceCount == kFrameTraceCapacity 时，下一笔覆盖 m_FrameTrace[m_FrameTraceHead]。
		std::size_t m_FrameTraceHead = 0;
		std::size_t m_FrameTraceCount = 0;

		/// Phase 2B：扩展用 UTF-8 键值 blob，不参与调度；由 SequenceStateSerializer 持久化。
		std::unordered_map<std::string, std::string> m_UserStateBlob;
	};
}
