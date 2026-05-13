/*
 * DialoguePlaybackRuntime — 自动播放与快进节拍（Phase 8 Playback 子层）
 *
 * 中文：仅依赖 GalGameContext::runtimeState.playback 与外部回调（继续对白、是否打字中、是否语音中、是否转场）；
 * 不直接操作 Rml，也不维护对白历史列表。
 */

#pragma once
#include "../VGGalgameConfig.h"
#include "VGGalgameCore/Include/GalGameContext.h"
#include <chrono>
#include <functional>

namespace VisionGal::GalGame
{
	class VG_GALGAME_API DialoguePlaybackRuntime final
	{
	public:
		using ContinueDialogueFn = std::function<void()>;
		using BoolPredicate = std::function<bool()>;

		void ResetWaitState();

		void AutoDialogue(const Ref<GalGameContext>& ctx, bool enable);
		[[nodiscard]] bool IsAutoDialogue(const Ref<GalGameContext>& ctx) const;

		void FastForward(const Ref<GalGameContext>& ctx, bool enable, const std::function<void()>& finishTypingIfAny);
		[[nodiscard]] bool IsFastForward(const Ref<GalGameContext>& ctx) const;

		void SetFastForwardDelay(const Ref<GalGameContext>& ctx, float delay);
		float GetFastForwardDelay(const Ref<GalGameContext>& ctx) const;

		[[nodiscard]] bool IsVoicing(const Ref<GalGameContext>& ctx) const;

		/// 中文：每帧驱动；在 DialogueSystem::Update 中于打字更新之后调用。
		void Tick(const Ref<GalGameContext>& ctx, const ContinueDialogueFn& continueDialogue, const BoolPredicate& isTypingText,
			const BoolPredicate& isTransitioning);

	private:
		void ProcessFastForward(const Ref<GalGameContext>& ctx, const ContinueDialogueFn& continueDialogue);
		void ProcessAutoDialogue(const Ref<GalGameContext>& ctx, const ContinueDialogueFn& continueDialogue,
			const BoolPredicate& isTypingText, const BoolPredicate& isTransitioning);

		std::chrono::high_resolution_clock::time_point m_FastForwardTimerStart{};
		bool m_FastForwardWaitingForNextAuto = false;

		std::chrono::high_resolution_clock::time_point m_AutoTimerStart{};
		bool m_WaitingForNextAuto = false;
	};
}
