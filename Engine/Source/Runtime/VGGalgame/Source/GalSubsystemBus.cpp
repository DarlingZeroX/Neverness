/*
 * GalSubsystemBus — 各 Adapter 对 GalGameEngine 的门面转发实现
 */

#include "GalSubsystemBus.h"

#include "GalGameEngine.h"

namespace VisionGal::GalGame
{
	bool GalSceneSubsystemAdapter::PreLoadResource(const String& path)
	{
		return m_E ? m_E->PreLoadResource(path) : false;
	}

	bool GalSceneSubsystemAdapter::TransitionCommand(const String& layer, const String& cmd)
	{
		return m_E ? m_E->TransitionCommand(layer, cmd) : false;
	}

	bool GalSceneSubsystemAdapter::TransitionCommandWithCustomImage(const String& layer, const String& imagePath, const String& cmd)
	{
		return m_E ? m_E->TransitionCommandWithCustomImage(layer, imagePath, cmd) : false;
	}

	IGalSprite* GalSceneSubsystemAdapter::ShowSprite(const std::string& layer, const std::string& path)
	{
		return m_E ? m_E->ShowSprite(layer, path) : nullptr;
	}

	IGalSprite* GalSceneSubsystemAdapter::ShowColor(const std::string& layer, const float4& color)
	{
		return m_E ? m_E->ShowColor(layer, color) : nullptr;
	}

	IGalVideo* GalSceneSubsystemAdapter::PlayVideo(const std::string& layer, const std::string& path)
	{
		return m_E ? m_E->PlayVideo(layer, path) : nullptr;
	}

	IGalCharacter* GalSceneSubsystemAdapter::CreateCharacter(const String& name)
	{
		return m_E ? m_E->CreateCharacter(name) : nullptr;
	}

	bool GalSceneSubsystemAdapter::RemoveSprite(IGalSprite* sprite)
	{
		return m_E ? m_E->RemoveSprite(sprite) : false;
	}

	void GalSceneSubsystemAdapter::HideAllCharacterSprite()
	{
		if (m_E)
			m_E->HideAllCharacterSprite();
	}

	void GalSceneSubsystemAdapter::CaptureSceneImage()
	{
		if (m_E)
			m_E->CaptureSceneImage();
	}

	ILayeredSceneManager* GalSceneSubsystemAdapter::GetLayeredSceneManager()
	{
		return m_E ? m_E->m_LayeredSceneManager.get() : nullptr;
	}

	IGalAudio* GalAudioSubsystemAdapter::PlayAudio(const std::string& layer, const std::string& path)
	{
		return m_E ? m_E->PlayAudio(layer, path) : nullptr;
	}

	bool GalAudioSubsystemAdapter::RemoveAudio(IGalAudio* audio)
	{
		return m_E ? m_E->RemoveAudio(audio) : false;
	}

	IGalGameUISystem* GalUISubsystemAdapter::GetGalGameUISystem()
	{
		return m_E ? m_E->m_GalGameUISystem.get() : nullptr;
	}

	bool GalScriptSubsystemAdapter::LoadStoryScript(const String& path)
	{
		return m_E ? m_E->LoadStoryScript(path) : false;
	}

	void GalScriptSubsystemAdapter::LoadStoryScriptOnUpdate(const String& path)
	{
		if (m_E)
			m_E->LoadStoryScriptOnUpdate(path);
	}

	void GalScriptSubsystemAdapter::ReloadStoryScript()
	{
		if (m_E)
			m_E->ReloadStoryScript();
	}

	void GalScriptSubsystemAdapter::Wait(float duration)
	{
		if (m_E)
			m_E->Wait(duration);
	}

	IStoryScriptSystem* GalScriptSubsystemAdapter::GetStoryScriptSystem()
	{
		return m_E ? m_E->m_StoryScriptSystem.get() : nullptr;
	}

	bool GalArchiveSubsystemAdapter::LoadArchive(const SaveArchive& archive)
	{
		return m_E ? m_E->LoadArchive(archive) : false;
	}

	ArchiveDataContainer* GalArchiveSubsystemAdapter::GetArchiveDataContainer() const
	{
		return (m_E && m_E->m_GalGameContext) ? m_E->m_GalGameContext->archiveData.get() : nullptr;
	}

	IArchiveSystem* GalArchiveSubsystemAdapter::GetArchiveSystem()
	{
		return m_E ? m_E->m_ArchiveSystem.get() : nullptr;
	}

	IDialogueSystem* GalDialogueSubsystemAdapter::GetDialogueSystem()
	{
		return m_E ? m_E->m_DialogueSystem.get() : nullptr;
	}

	GalSubsystemBus::GalSubsystemBus(GalGameEngine* owner) noexcept
		: m_Owner(owner)
		, m_Scene(owner)
		, m_Audio(owner)
		, m_UI(owner)
		, m_Script(owner)
		, m_Archive(owner)
		, m_Dialogue(owner)
	{
	}

	ISceneSubsystem* GalSubsystemBus::Scene()
	{
		return &m_Scene;
	}

	IUISubsystem* GalSubsystemBus::UI()
	{
		return &m_UI;
	}

	IAudioSubsystem* GalSubsystemBus::Audio()
	{
		return &m_Audio;
	}

	IScriptSubsystem* GalSubsystemBus::Script()
	{
		return &m_Script;
	}

	IArchiveSubsystem* GalSubsystemBus::Archive()
	{
		return &m_Archive;
	}

	IDialogueSubsystem* GalSubsystemBus::Dialogue()
	{
		return &m_Dialogue;
	}
}