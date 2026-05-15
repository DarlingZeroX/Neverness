/*
 * IAudioSubsystem — 音频播放与移除（Phase 8：Contract，前向声明 IGalAudio）
 *
 * 职责从 IGalGameEngine 门面迁移；与 Scene 中的语音/音效层命名约定保持一致。
 * 完整资源接口定义见 VGGalgameRuntimeCore `IGameObject.h`。
 */

#pragma once
#include <string>

namespace VisionGal::GalGame
{
	struct IGalAudio;

	struct IAudioSubsystem
	{
		virtual ~IAudioSubsystem() = default;

		virtual IGalAudio* PlayAudio(const std::string& layer, const std::string& path) = 0;
		virtual bool RemoveAudio(IGalAudio* audio) = 0;
	};
}
