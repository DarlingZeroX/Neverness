/*
 * DialogueLineRuntime — 对白「行」与历史（Phase 8 Runtime 数据子层）
 *
 * 中文：维护对话列表与 GalGameContext::runtimeState.dialogue 中的「当前行」语义；
 * 不负责 UI 字符串绑定与打字机效果。
 */

#pragma once
#include "../VGGalgameConfig.h"
#include "VGGalgameCore/Include/GalGameContext.h"
#include <vector>

namespace VisionGal::GalGame
{
	class VG_GALGAME_API DialogueLineRuntime final
	{
	public:
		struct DialogEntry
		{
			String character;
			String text;
		};

		void RecordUtterance(const Ref<GalGameContext>& ctx, const String& character, const String& text);

		[[nodiscard]] uint GetCurrentDialogLine(const Ref<GalGameContext>& ctx) const;
		[[nodiscard]] uint GetDialogNumber() const { return static_cast<uint>(m_DialogList.size()); }
		String GetDialogCharacter(uint index) const;
		String GetDialogText(uint index) const;
		String GetCurrentCharacter(const Ref<GalGameContext>& ctx) const;
		String GetCurrentDialogText(const Ref<GalGameContext>& ctx) const;

		void ClearList(const Ref<GalGameContext>& ctx);

	private:
		std::vector<DialogEntry> m_DialogList;
	};
}
