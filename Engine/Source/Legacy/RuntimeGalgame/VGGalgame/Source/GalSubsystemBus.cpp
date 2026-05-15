/*
 * GalSubsystemBus — 各 Adapter 对 GalGameEngine 子系统的直接转发（Phase 8）
 */

#include "GalSubsystemBus.h"

#include "GalGameEngine.h"
#include "Game.h"
#include "VGCore/Include/Core/Core.h"
#include <VGRHI/Interface/Texture.h>

namespace VisionGal::GalGame
{
	bool GalSceneSubsystemAdapter::PreLoadResource(const String& path)
	{
		return m_E && m_E->m_ResourceSystem ? m_E->m_ResourceSystem->PreLoadResource(path) : false;
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
		return m_E && m_E->m_ResourceSystem ? m_E->m_ResourceSystem->ShowSprite(layer, path) : nullptr;
	}

	IGalSprite* GalSceneSubsystemAdapter::ShowColor(const std::string& layer, const float4& color)
	{
		return m_E && m_E->m_ResourceSystem ? m_E->m_ResourceSystem->ShowColor(layer, color) : nullptr;
	}

	IGalVideo* GalSceneSubsystemAdapter::PlayVideo(const std::string& layer, const std::string& path)
	{
		return m_E && m_E->m_ResourceSystem ? m_E->m_ResourceSystem->PlayVideo(layer, path) : nullptr;
	}

	IGalCharacter* GalSceneSubsystemAdapter::CreateCharacter(const String& name)
	{
		if (!m_E)
			return nullptr;
		auto* character = new GalCharacter(m_E, name);
		m_E->m_LayeredSceneManager->AddCharacter(character);
		return character;
	}

	bool GalSceneSubsystemAdapter::RemoveSprite(IGalSprite* sprite)
	{
		return m_E && m_E->m_LayeredSceneManager ? m_E->m_LayeredSceneManager->GetSpriteManager()->RemoveSprite(sprite) : false;
	}

	void GalSceneSubsystemAdapter::HideAllCharacterSprite()
	{
		if (!m_E || !m_E->m_LayeredSceneManager)
			return;
		m_E->m_LayeredSceneManager->TraverseCharacter([](IGalCharacter* character)
			{
				if (GalCharacter* galChar = dynamic_cast<GalCharacter*>(character))
					galChar->HideFigure();
			});
	}

	void GalSceneSubsystemAdapter::CaptureSceneImage()
	{
		if (!m_E || !m_E->m_GalGameContext || !m_E->m_EngineContext)
			return;
		m_E->m_GalGameContext->runtimeState.screenshotPixels = MakeRef<VGFX::TexturePixels>();
		m_E->m_EngineContext->GetViewport()->GetViewportTexture()->ReadPixels(*m_E->m_GalGameContext->runtimeState.screenshotPixels);
	}

	ILayeredSceneManager* GalSceneSubsystemAdapter::GetLayeredSceneManager()
	{
		return m_E && m_E->m_LayeredSceneManager ? m_E->m_LayeredSceneManager.get() : nullptr;
	}

	IGalAudio* GalAudioSubsystemAdapter::PlayAudio(const std::string& layer, const std::string& path)
	{
		return m_E && m_E->m_ResourceSystem ? m_E->m_ResourceSystem->PlayAudio(layer, path) : nullptr;
	}

	bool GalAudioSubsystemAdapter::RemoveAudio(IGalAudio* audio)
	{
		return m_E && m_E->m_LayeredSceneManager ? m_E->m_LayeredSceneManager->GetAudioManager()->RemoveAudio(audio) : false;
	}

	IGalGameUISystem* GalUISubsystemAdapter::GetGalGameUISystem()
	{
		return m_E && m_E->m_GalGameUISystem ? m_E->m_GalGameUISystem.get() : nullptr;
	}

	bool GalScriptSubsystemAdapter::LoadStoryScript(const String& path)
	{
		if (!m_E || !m_E->m_StoryScriptSystem)
			return false;
		m_E->m_StoryScriptSystem->LoadStoryScript(path);
		return true;
	}

	void GalScriptSubsystemAdapter::LoadStoryScriptOnUpdate(const String& path)
	{
		if (m_E && m_E->m_StoryScriptSystem)
			m_E->m_StoryScriptSystem->LoadStoryScriptOnUpdate(path);
	}

	void GalScriptSubsystemAdapter::ReloadStoryScript()
	{
		if (m_E && m_E->m_StoryScriptSystem)
			m_E->m_StoryScriptSystem->ReloadStoryScript();
	}

	void GalScriptSubsystemAdapter::Wait(float duration)
	{
		if (m_E)
			m_E->WaitForStoryScript(duration);
	}

	IStoryScriptSystem* GalScriptSubsystemAdapter::GetStoryScriptSystem()
	{
		return m_E ? m_E->GetStoryScriptSystemPtr() : nullptr;
	}

	void GalPlaybackSubsystemAdapter::Wait(float durationSeconds)
	{
		if (m_E)
			m_E->WaitForStoryScript(durationSeconds);
	}

	bool GalArchiveSubsystemAdapter::LoadArchive(const SaveArchive& archive)
	{
		return m_E && m_E->m_StoryScriptSystem ? m_E->m_StoryScriptSystem->LoadArchive(archive) : false;
	}

	ArchiveDataContainer* GalArchiveSubsystemAdapter::GetArchiveDataContainer() const
	{
		return (m_E && m_E->m_GalGameContext) ? m_E->m_GalGameContext->archiveData.get() : nullptr;
	}

	IArchiveSystem* GalArchiveSubsystemAdapter::GetArchiveSystem()
	{
		return m_E && m_E->m_ArchiveSystem ? m_E->m_ArchiveSystem.get() : nullptr;
	}

	IDialogueSystem* GalDialogueSubsystemAdapter::GetDialogueSystem()
	{
		return m_E && m_E->m_DialogueSystem ? m_E->m_DialogueSystem.get() : nullptr;
	}

	GalSubsystemBus::GalSubsystemBus(GalGameEngine* owner) noexcept
		: m_Owner(owner)
		, m_Scene(owner)
		, m_Audio(owner)
		, m_UI(owner)
		, m_Script(owner)
		, m_Playback(owner)
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

	IPlaybackSubsystem* GalSubsystemBus::Playback()
	{
		return &m_Playback;
	}
}
