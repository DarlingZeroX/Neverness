/*
 * DialogueTypingRuntime — 打字机效果子层（Phase 8）
 *
 * 中文：封装 TypingEffect 与 runtimeState.textDisplay.enableTyping 的协同；
 * 显示目标字符串由 DialogueRmlPresentation::MutableBoundDialogText 提供引用。
 */

#pragma once
#include "../VGGalgameConfig.h"
#include "VGGalgameCore/Include/GalGameContext.h"
#include "DialogueSystem/TypingEffect.h"
#include "sol/function.hpp"

namespace VisionGal::GalGame
{
	class VG_GALGAME_API DialogueTypingRuntime final
	{
	public:
		explicit DialogueTypingRuntime(std::string& boundDialogText);

		void Update(const Ref<GalGameContext>& ctx);

		void EnableTyping(const Ref<GalGameContext>& ctx, bool enable);
		void FinishTyping();
		[[nodiscard]] bool IsTypingText(const Ref<GalGameContext>& ctx) const;

		void StartLine(const Ref<GalGameContext>& ctx, const std::string& fullText);
		void SetFullTextImmediate(const std::string& text);

		float GetTypingDelay() { return m_TypingEffect.GetTypingDelay(); }
		void SetTypingDelay(float delay) { m_TypingEffect.SetTypingDelay(delay); }

		void AddTypingCallback(sol::function callback) { m_TypingEffect.AddTypingCallback(std::move(callback)); }
		void ClearAllTypingCallbacks() { m_TypingEffect.ClearAllTypingCallbacks(); }

	private:
		TypingEffect m_TypingEffect;
	};
}
