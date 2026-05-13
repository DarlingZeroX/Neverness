/*
 * GalGameRuntimeStateSerializable 实现
 */

#include "GalGameRuntimeStateSerializable.h"

namespace VisionGal::GalGame
{
	namespace
	{
		constexpr int kRuntimeStateJsonSchema = 1;
	}

	void GalGameRuntimeStateSerializable::SaveToJson(nlohmann::json& out) const
	{
		out = nlohmann::json::object();
		out["schema"] = kRuntimeStateJsonSchema;

		if (m_State == nullptr)
			return;

		const GalGameRuntimeState& s = *m_State;
		out["currentScriptPath"] = s.currentScriptPath;

		nlohmann::json dialogue = nlohmann::json::object();
		dialogue["currentDialogCharacter"] = s.dialogue.currentDialogCharacter;
		dialogue["currentDialogText"] = s.dialogue.currentDialogText;
		dialogue["currentDialogLine"] = s.dialogue.currentDialogLine;
		out["dialogue"] = std::move(dialogue);

		nlohmann::json textDisplay = nlohmann::json::object();
		textDisplay["textShownProgress"] = s.textDisplay.textShownProgress;
		textDisplay["isTextFullyShown"] = s.textDisplay.isTextFullyShown;
		textDisplay["enableTyping"] = s.textDisplay.enableTyping;
		out["textDisplay"] = std::move(textDisplay);

		nlohmann::json playback = nlohmann::json::object();
		playback["enableFastForward"] = s.playback.enableFastForward;
		playback["fastForwardDelay"] = s.playback.fastForwardDelay;
		playback["isCurrentLoadingArchive"] = s.playback.isCurrentLoadingArchive;
		playback["enableAutoDialogue"] = s.playback.enableAutoDialogue;
		playback["autoDialogueDelay"] = s.playback.autoDialogueDelay;
		playback["IsVoicing"] = s.playback.IsVoicing;
		out["playback"] = std::move(playback);
	}

	bool GalGameRuntimeStateSerializable::LoadFromJson(const nlohmann::json& in)
	{
		if (m_State == nullptr)
			return false;

		GalGameRuntimeState& s = *m_State;
		s.screenshotPixels = nullptr;

		if (!in.is_object())
			return false;

		(void)in.value("schema", kRuntimeStateJsonSchema);

		if (in.contains("currentScriptPath") && in["currentScriptPath"].is_string())
			s.currentScriptPath = in["currentScriptPath"].get<std::string>();

		if (in.contains("dialogue") && in["dialogue"].is_object())
		{
			const auto& d = in["dialogue"];
			s.dialogue.currentDialogCharacter = d.value("currentDialogCharacter", std::string{});
			s.dialogue.currentDialogText = d.value("currentDialogText", std::string{});
			s.dialogue.currentDialogLine = d.value("currentDialogLine", static_cast<std::uint32_t>(0));
		}

		if (in.contains("textDisplay") && in["textDisplay"].is_object())
		{
			const auto& t = in["textDisplay"];
			s.textDisplay.textShownProgress = t.value("textShownProgress", 0.f);
			s.textDisplay.isTextFullyShown = t.value("isTextFullyShown", false);
			s.textDisplay.enableTyping = t.value("enableTyping", true);
		}

		if (in.contains("playback") && in["playback"].is_object())
		{
			const auto& p = in["playback"];
			s.playback.enableFastForward = p.value("enableFastForward", false);
			s.playback.fastForwardDelay = p.value("fastForwardDelay", 0.f);
			s.playback.isCurrentLoadingArchive = p.value("isCurrentLoadingArchive", false);
			s.playback.enableAutoDialogue = p.value("enableAutoDialogue", false);
			s.playback.autoDialogueDelay = p.value("autoDialogueDelay", 2.0f);
			s.playback.IsVoicing = p.value("IsVoicing", false);
		}

		return true;
	}
}
