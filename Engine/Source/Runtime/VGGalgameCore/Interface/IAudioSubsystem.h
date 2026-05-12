/*
 * IAudioSubsystem — 音频播放与移除（由 ISubsystemBus::Audio() 访问）
 *
 * 职责从 IGalGameEngine 门面迁移；与 Scene 中的语音/音效层命名约定保持一致。
 */

#pragma once
#include "IGameObject.h"
#include "../VGGalCoreConfig.h"
#include <string>

namespace VisionGal::GalGame
{
	struct IAudioSubsystem
	{
		virtual ~IAudioSubsystem() = default;

		virtual IGalAudio* PlayAudio(const std::string& layer, const std::string& path) = 0;
		virtual bool RemoveAudio(IGalAudio* audio) = 0;
	};
}
