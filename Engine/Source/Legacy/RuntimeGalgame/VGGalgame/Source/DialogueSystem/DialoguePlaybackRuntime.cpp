/*
 * DialoguePlaybackRuntime — 自动 / 快进实现
 */

#include "DialogueSystem/DialoguePlaybackRuntime.h"
#include "VGEngine/Include/Render/TransitionManager.h"

namespace VisionGal::GalGame
{
	void DialoguePlaybackRuntime::ResetWaitState()
	{
		m_WaitingForNextAuto = false;
		m_FastForwardWaitingForNextAuto = false;
	}

	void DialoguePlaybackRuntime::AutoDialogue(const Ref<GalGameContext>& ctx, bool enable)
	{
		if (!ctx)
			return;
		ctx->runtimeState.playback.enableAutoDialogue = enable;
		m_WaitingForNextAuto = false;
	}

	bool DialoguePlaybackRuntime::IsAutoDialogue(const Ref<GalGameContext>& ctx) const
	{
		return ctx && ctx->runtimeState.playback.enableAutoDialogue;
	}

	void DialoguePlaybackRuntime::FastForward(const Ref<GalGameContext>& ctx, bool enable, const std::function<void()>& finishTypingIfAny)
	{
		if (!ctx)
			return;
		ctx->runtimeState.playback.enableFastForward = enable;
		if (ctx->runtimeState.playback.enableFastForward && finishTypingIfAny)
			finishTypingIfAny();
	}

	bool DialoguePlaybackRuntime::IsFastForward(const Ref<GalGameContext>& ctx) const
	{
		return ctx && ctx->runtimeState.playback.enableFastForward;
	}

	void DialoguePlaybackRuntime::SetFastForwardDelay(const Ref<GalGameContext>& ctx, float delay)
	{
		if (ctx)
			ctx->runtimeState.playback.fastForwardDelay = delay;
	}

	float DialoguePlaybackRuntime::GetFastForwardDelay(const Ref<GalGameContext>& ctx) const
	{
		return ctx ? ctx->runtimeState.playback.fastForwardDelay : 0.f;
	}

	bool DialoguePlaybackRuntime::IsVoicing(const Ref<GalGameContext>& ctx) const
	{
		return ctx && ctx->runtimeState.playback.IsVoicing;
	}

	void DialoguePlaybackRuntime::Tick(const Ref<GalGameContext>& ctx, const ContinueDialogueFn& continueDialogue,
		const BoolPredicate& isTypingText, const BoolPredicate& isTransitioning)
	{
		if (!ctx)
			return;
		ProcessFastForward(ctx, continueDialogue);
		ProcessAutoDialogue(ctx, continueDialogue, isTypingText, isTransitioning);
	}

	void DialoguePlaybackRuntime::ProcessFastForward(const Ref<GalGameContext>& ctx, const ContinueDialogueFn& continueDialogue)
	{
		if (!IsFastForward(ctx))
			return;

		if (!continueDialogue)
			return;

		if (!m_FastForwardWaitingForNextAuto)
		{
			m_FastForwardTimerStart = std::chrono::high_resolution_clock::now();
			m_FastForwardWaitingForNextAuto = true;
			return;
		}

		const auto current_time = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<float> elapsed = current_time - m_FastForwardTimerStart;
		if (elapsed.count() >= ctx->runtimeState.playback.fastForwardDelay)
		{
			continueDialogue();
			m_FastForwardWaitingForNextAuto = false;
		}
	}

	void DialoguePlaybackRuntime::ProcessAutoDialogue(const Ref<GalGameContext>& ctx, const ContinueDialogueFn& continueDialogue,
		const BoolPredicate& isTypingText, const BoolPredicate& isTransitioning)
	{
		if (!IsAutoDialogue(ctx) || !continueDialogue)
			return;

		if (isTypingText && isTypingText())
			return;

		if (IsVoicing(ctx))
			return;

		if (IsFastForward(ctx))
		{
			AutoDialogue(ctx, false);
			return;
		}

		if (isTransitioning && isTransitioning())
			return;

		if (!m_WaitingForNextAuto)
		{
			m_AutoTimerStart = std::chrono::high_resolution_clock::now();
			m_WaitingForNextAuto = true;
			return;
		}

		const auto current_time = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<float> elapsed = current_time - m_AutoTimerStart;
		if (elapsed.count() >= ctx->runtimeState.playback.autoDialogueDelay)
		{
			continueDialogue();
			m_WaitingForNextAuto = false;
		}
	}
}
