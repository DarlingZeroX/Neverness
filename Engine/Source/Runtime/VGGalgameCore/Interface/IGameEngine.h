/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 *
 * CORE ABI STABLE
 * DO NOT MODIFY WITHOUT VERSION BUMP
 * 中文：变更须同步升版本、SaveArchive / Lua 绑定与文档。
 */

#pragma once
#include "IGameSystem.h"
#include "IStoryScriptSystem.h"
#include "../Include/SaveArchive.h"
#include "IGalGameContext.h"
#include "ISubsystemBus.h"
#include "VGCore/Interface/GameEngineInterface.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief GalGame 引擎接口
	 *
	 * 【SubsystemBus 架构】新逻辑请优先通过 GetSubsystemBus() 访问各子系统；
	 * 下列门面方法保留用于 Lua 绑定与存量 C++ 调用，实现上委托同一套子系统能力。
	 */
	class IGalGameEngine : public ISubGameEngine
	{
	public:
		~IGalGameEngine() override = default;

		virtual bool PreLoadResource(const String& path) = 0;

		virtual bool TransitionCommand(const String& layer, const String& cmd) = 0;
		virtual bool TransitionCommandWithCustomImage(const String& layer, const String& imagePath, const String& cmd) = 0;

		virtual IGalSprite* ShowSprite(const std::string& layer, const std::string& path) = 0;
		virtual IGalSprite* ShowColor(const std::string& layer, const float4& color) = 0;
		virtual IGalAudio* PlayAudio(const std::string& layer, const std::string& path) = 0;
		virtual IGalVideo* PlayVideo(const std::string& layer, const std::string& path) = 0;
		virtual IGalCharacter* CreateCharacter(const String& name) = 0;

		virtual bool LoadArchive(const SaveArchive& archive) = 0;

		virtual bool RemoveSprite(IGalSprite* sprite) = 0;
		virtual bool RemoveAudio(IGalAudio* audio) = 0;
		virtual void HideAllCharacterSprite() = 0;

		virtual IArchiveSystem* GetArchiveSystem() = 0;
		virtual IDialogueSystem* GetDialogueSystem() = 0;
		virtual ILayeredSceneManager* GetLayeredSceneManager() = 0;
		virtual IStoryScriptSystem* GetStoryScriptSystem() = 0;
		virtual IGalGameUISystem* GetGalGameUISystem() = 0;

		virtual bool LoadStoryScript(const String& path) = 0;
		virtual void LoadStoryScriptOnUpdate(const String& path) = 0;
		virtual void ReloadStoryScript() = 0;

		virtual void Reset() = 0;
		virtual void Wait(float duration) = 0;

		virtual void CaptureSceneImage() = 0;

		virtual ArchiveDataContainer* GetArchiveDataContainer() const = 0;

		virtual ISubsystemBus* GetSubsystemBus() = 0;
		virtual IGalGameContext* GetContext() = 0;
	};
}
