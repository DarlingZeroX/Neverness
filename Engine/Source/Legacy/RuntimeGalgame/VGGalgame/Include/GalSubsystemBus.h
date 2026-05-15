/*
 * GalSubsystemBus — VGGalgame 侧 ISubsystemBus 实现
 *
 * 将宿主 GalGameEngine 既有子系统装配为总线；各 Adapter 仅转发，逻辑仍集中在引擎与原有 System。
 * Phase 8：Adapter 直接访问引擎私有子系统成员，避免经 IGalGameEngine 已删除的门面再转发。
 */

#pragma once
#include "../VGGalgameConfig.h"
#include "VGGalgameCore/Interface/IAudioSubsystem.h"
#include "VGGalgameCore/Interface/IArchiveSubsystem.h"
#include "VGGalgameCore/Interface/IDialogueSubsystem.h"
#include "VGGalgameCore/Interface/ISceneSubsystem.h"
#include "VGGalgameCore/Interface/IScriptSubsystem.h"
#include "VGGalgameCore/Interface/IPlaybackSubsystem.h"
#include "VGGalgameCore/Interface/ISubsystemBus.h"
#include "VGGalgameCore/Interface/IUISubsystem.h"

namespace VisionGal::GalGame
{
	class GalGameEngine;

	class GalSceneSubsystemAdapter final : public ISceneSubsystem
	{
		GalGameEngine* m_E = nullptr;

	public:
		explicit GalSceneSubsystemAdapter(GalGameEngine* e = nullptr) noexcept
			: m_E(e)
		{
		}

		void SetOwner(GalGameEngine* e) noexcept
		{
			m_E = e;
		}

		bool PreLoadResource(const String& path) override;
		bool TransitionCommand(const String& layer, const String& cmd) override;
		bool TransitionCommandWithCustomImage(const String& layer, const String& imagePath, const String& cmd) override;
		IGalSprite* ShowSprite(const std::string& layer, const std::string& path) override;
		IGalSprite* ShowColor(const std::string& layer, const float4& color) override;
		IGalVideo* PlayVideo(const std::string& layer, const std::string& path) override;
		IGalCharacter* CreateCharacter(const String& name) override;
		bool RemoveSprite(IGalSprite* sprite) override;
		void HideAllCharacterSprite() override;
		void CaptureSceneImage() override;
		ILayeredSceneManager* GetLayeredSceneManager() override;
	};

	class GalAudioSubsystemAdapter final : public IAudioSubsystem
	{
		GalGameEngine* m_E = nullptr;

	public:
		explicit GalAudioSubsystemAdapter(GalGameEngine* e = nullptr) noexcept
			: m_E(e)
		{
		}

		void SetOwner(GalGameEngine* e) noexcept
		{
			m_E = e;
		}

		IGalAudio* PlayAudio(const std::string& layer, const std::string& path) override;
		bool RemoveAudio(IGalAudio* audio) override;
	};

	class GalUISubsystemAdapter final : public IUISubsystem
	{
		GalGameEngine* m_E = nullptr;

	public:
		explicit GalUISubsystemAdapter(GalGameEngine* e = nullptr) noexcept
			: m_E(e)
		{
		}

		void SetOwner(GalGameEngine* e) noexcept
		{
			m_E = e;
		}

		IGalGameUISystem* GetGalGameUISystem() override;
	};

	class GalScriptSubsystemAdapter final : public IScriptSubsystem
	{
		GalGameEngine* m_E = nullptr;

	public:
		explicit GalScriptSubsystemAdapter(GalGameEngine* e = nullptr) noexcept
			: m_E(e)
		{
		}

		void SetOwner(GalGameEngine* e) noexcept
		{
			m_E = e;
		}

		bool LoadStoryScript(const String& path) override;
		void LoadStoryScriptOnUpdate(const String& path) override;
		void ReloadStoryScript() override;
		void Wait(float duration) override;
		IStoryScriptSystem* GetStoryScriptSystem() override;
	};

	class GalPlaybackSubsystemAdapter final : public IPlaybackSubsystem
	{
		GalGameEngine* m_E = nullptr;

	public:
		explicit GalPlaybackSubsystemAdapter(GalGameEngine* e = nullptr) noexcept
			: m_E(e)
		{
		}

		void SetOwner(GalGameEngine* e) noexcept { m_E = e; }

		void Wait(float durationSeconds) override;
	};

	class GalArchiveSubsystemAdapter final : public IArchiveSubsystem
	{
		GalGameEngine* m_E = nullptr;

	public:
		explicit GalArchiveSubsystemAdapter(GalGameEngine* e = nullptr) noexcept
			: m_E(e)
		{
		}

		void SetOwner(GalGameEngine* e) noexcept
		{
			m_E = e;
		}

		bool LoadArchive(const SaveArchive& archive) override;
		ArchiveDataContainer* GetArchiveDataContainer() const override;
		IArchiveSystem* GetArchiveSystem() override;
	};

	class GalDialogueSubsystemAdapter final : public IDialogueSubsystem
	{
		GalGameEngine* m_E = nullptr;

	public:
		explicit GalDialogueSubsystemAdapter(GalGameEngine* e = nullptr) noexcept
			: m_E(e)
		{
		}

		void SetOwner(GalGameEngine* e) noexcept
		{
			m_E = e;
		}

		IDialogueSystem* GetDialogueSystem() override;
	};

	class VG_GALGAME_API GalSubsystemBus final : public ISubsystemBus
	{
	public:
		explicit GalSubsystemBus(GalGameEngine* owner) noexcept;

		ISceneSubsystem* Scene() override;
		IUISubsystem* UI() override;
		IAudioSubsystem* Audio() override;
		IScriptSubsystem* Script() override;
		IArchiveSubsystem* Archive() override;
		IDialogueSubsystem* Dialogue() override;
		IPlaybackSubsystem* Playback() override;

	private:
		GalGameEngine* m_Owner = nullptr;
		GalSceneSubsystemAdapter m_Scene;
		GalAudioSubsystemAdapter m_Audio;
		GalUISubsystemAdapter m_UI;
		GalScriptSubsystemAdapter m_Script;
		GalPlaybackSubsystemAdapter m_Playback;
		GalArchiveSubsystemAdapter m_Archive;
		GalDialogueSubsystemAdapter m_Dialogue;
	};
}
