/*
 * ISceneSubsystem — 场景 / 分层展示 / 转场 / 角色放置（Phase 8：Contract）
 *
 * 职责从 IGalGameEngine 门面迁移而来；宿主在 VGGalgame 中委托 ResourceSystem / LayeredSceneSystem。
 * IGalSprite 等资源接口定义见 VGGalgameRuntimeCore `IGameObject.h`。
 */

#pragma once
#include "NNRuntimeCore/Include/Core/Core.h"
#include <string>

namespace VisionGal::GalGame
{
	struct IGalSprite;
	struct IGalVideo;
	struct IGalCharacter;
	struct ILayeredSceneManager;

	struct ISceneSubsystem
	{
		virtual ~ISceneSubsystem() = default;

		virtual bool PreLoadResource(const String& path) = 0;
		virtual bool TransitionCommand(const String& layer, const String& cmd) = 0;
		virtual bool TransitionCommandWithCustomImage(const String& layer, const String& imagePath, const String& cmd) = 0;
		virtual IGalSprite* ShowSprite(const std::string& layer, const std::string& path) = 0;
		virtual IGalSprite* ShowColor(const std::string& layer, const float4& color) = 0;
		virtual IGalVideo* PlayVideo(const std::string& layer, const std::string& path) = 0;
		virtual IGalCharacter* CreateCharacter(const String& name) = 0;
		virtual bool RemoveSprite(IGalSprite* sprite) = 0;
		virtual void HideAllCharacterSprite() = 0;
		virtual void CaptureSceneImage() = 0;
		virtual ILayeredSceneManager* GetLayeredSceneManager() = 0;
	};
}
