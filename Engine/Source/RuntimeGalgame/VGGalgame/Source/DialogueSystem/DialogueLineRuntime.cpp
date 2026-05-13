/*
 * DialogueLineRuntime — 对白行与历史实现
 */

#include "DialogueSystem/DialogueLineRuntime.h"

namespace VisionGal::GalGame
{
	namespace
	{
		const char* kEmpty = "";
	}

	void DialogueLineRuntime::RecordUtterance(const Ref<GalGameContext>& ctx, const String& character, const String& text)
	{
		if (!ctx)
			return;
		ctx->runtimeState.dialogue.currentDialogCharacter = character;
		ctx->runtimeState.dialogue.currentDialogText = text;
		ctx->runtimeState.dialogue.currentDialogLine += 1;
		m_DialogList.push_back({ character, text });
	}

	uint DialogueLineRuntime::GetCurrentDialogLine(const Ref<GalGameContext>& ctx) const
	{
		return ctx ? ctx->runtimeState.dialogue.currentDialogLine : 0;
	}

	String DialogueLineRuntime::GetDialogCharacter(uint index) const
	{
		if (index < m_DialogList.size())
			return m_DialogList[index].character.c_str();
		return kEmpty;
	}

	String DialogueLineRuntime::GetDialogText(uint index) const
	{
		if (index < m_DialogList.size())
			return m_DialogList[index].text.c_str();
		return kEmpty;
	}

	String DialogueLineRuntime::GetCurrentCharacter(const Ref<GalGameContext>& ctx) const
	{
		return ctx ? ctx->runtimeState.dialogue.currentDialogCharacter : String();
	}

	String DialogueLineRuntime::GetCurrentDialogText(const Ref<GalGameContext>& ctx) const
	{
		return ctx ? ctx->runtimeState.dialogue.currentDialogText : String();
	}

	void DialogueLineRuntime::ClearList(const Ref<GalGameContext>& ctx)
	{
		m_DialogList.clear();
		if (ctx)
		{
			ctx->runtimeState.playback.enableAutoDialogue = false;
			ctx->runtimeState.playback.enableFastForward = false;
		}
	}
}
