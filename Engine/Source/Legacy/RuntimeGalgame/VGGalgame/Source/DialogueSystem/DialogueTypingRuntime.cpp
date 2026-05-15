/*
 * DialogueTypingRuntime — 打字机实现
 */

#include "DialogueSystem/DialogueTypingRuntime.h"

namespace VisionGal::GalGame
{
	DialogueTypingRuntime::DialogueTypingRuntime(std::string& boundDialogText)
		: m_TypingEffect(boundDialogText)
	{
	}

	void DialogueTypingRuntime::Update(const Ref<GalGameContext>& /*ctx*/)
	{
		m_TypingEffect.Update();
	}

	void DialogueTypingRuntime::EnableTyping(const Ref<GalGameContext>& ctx, bool enable)
	{
		if (ctx)
			ctx->runtimeState.textDisplay.enableTyping = enable;
	}

	void DialogueTypingRuntime::FinishTyping()
	{
		m_TypingEffect.FinishTyping();
	}

	bool DialogueTypingRuntime::IsTypingText(const Ref<GalGameContext>& ctx) const
	{
		if (!ctx || !ctx->runtimeState.textDisplay.enableTyping)
			return false;
		return m_TypingEffect.IsTyping();
	}

	void DialogueTypingRuntime::StartLine(const Ref<GalGameContext>& ctx, const std::string& fullText)
	{
		if (ctx && ctx->runtimeState.textDisplay.enableTyping)
			m_TypingEffect.StartTyping(fullText);
		else
			SetFullTextImmediate(fullText);
	}

	void DialogueTypingRuntime::SetFullTextImmediate(const std::string& text)
	{
		m_TypingEffect.ApplyImmediateText(text);
	}

}
