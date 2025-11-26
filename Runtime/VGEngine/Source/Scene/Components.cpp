/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#include "Scene/Components.h"
#include <HCore/Include/Math/GLM/ext/matrix_transform.hpp>
#include "Engine/Manager.h"
#include "HCore/Include/Math/GLM/gtc/quaternion.hpp"
#include "Resource/Audio.h"
#include "Resource/FVideo.h"

namespace VisionGal
{
	TransformComponent::TransformComponent()
	{
		location = float3(0.0f);
		rotation = float3(0.0f);
		scale = float3(1.0f);
		model = glm::mat4(1.0f);
		is_dirty = false;
		visible = true;
	}

	std::string TransformComponent::GetComponentType() const
	{
		return std::string("Transform");
	}

	void TransformComponent::Update()
	{
		if (is_dirty)
		{
			// 分别构建变换矩阵，然后按正确顺序相乘
			glm::mat4 S = glm::scale(glm::identity<glm::mat4>(), scale);     // 缩放矩阵
			glm::mat4 R = glm::mat4_cast(rotation);                          // 旋转矩阵  
			glm::mat4 T = glm::translate(glm::identity<glm::mat4>(), location); // 平移矩阵

			// 正确的组合：先缩放，再旋转，最后平移
			model = T * R * S;
		}

		is_dirty = false;
	}

	SpriteRendererComponent::SpriteRendererComponent()
	{
		color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		flip = int2(0, 0);
		material = CreateRef<Material>();
		sprite = nullptr;
		pipelineIndex = static_cast<uint>( RenderPipelineIndex::CoreRenderPipeline );
	}

	std::string SpriteRendererComponent::GetComponentType() const
	{
		return std::string("SpriteRenderer");;
	}

	FullScreenRendererComponent::FullScreenRendererComponent()
	{
		image = FullScreenQuad::Create();
		shader = ShaderManager::Get()->GetBuiltinProgram("FullScreenQuad");
	}

	std::string FullScreenRendererComponent::GetComponentType() const
	{
		return std::string("FullScreenRenderer");
	}

	ScriptComponent::ScriptComponent()
	{

	}

	std::string ScriptComponent::GetComponentType() const
	{
		return std::string("Script");
	}

	CameraComponent::CameraComponent()
	{
		camera = nullptr;
	}

	std::string CameraComponent::GetComponentType() const
	{
		return std::string("Camera");
	}

	VideoPlayerComponent::VideoPlayerComponent()
	{
		videoPlayer = CreateRef<FVideoPlayer>();
		pipelineIndex = static_cast<uint>(RenderPipelineIndex::CoreRenderPipeline);
	}

	std::string VideoPlayerComponent::GetComponentType() const
	{
		return "VideoPlayer";
	}

	bool VideoPlayerComponent::Play()
	{
		if (videoClip)
		{
			videoPlayer->Open(videoClip);
			videoPlayer->Play();
			videoPlayer->SetLoop(loop);

			auto tex2D = CreateRef<Texture2D>();
			tex2D->SetTexture(videoPlayer->GetVideoTextureRef());
			videoSprite = Sprite::Create(tex2D, { videoPlayer->GetVideoWidth(), videoPlayer->GetVideoHeight() });

			return true;
		}

		return false;
	}

	void VideoPlayerComponent::Stop()
	{
		if (videoClip)
		{
			videoPlayer->Stop();
		}
	}

	void VideoPlayerComponent::Update()
	{
		if (videoClip)
		{
			videoPlayer->Update();
		}
	}

	void VideoPlayerComponent::SetLoop(bool enable)
	{
		if (videoClip)
		{
			videoPlayer->SetLoop(enable);
		}
	}

	bool VideoPlayerComponent::IsPlaying() const
	{
		if (videoClip)
		{
			return videoPlayer->IsPlaying();
		}

		return false;
	}

	bool VideoPlayerComponent::IsLooping() const
	{
		if (videoClip)
		{
			return videoPlayer->IsLooping();
		}

		return false;
	}

	Sprite* VideoPlayerComponent::GetSprite() const
	{
		if (videoSprite)
		{
			return videoSprite.get();
		}

		return nullptr;
	}

	AudioSourceComponent::AudioSourceComponent()
	{
		audioPlayer = CreateRef<AudioPlayer>();
	}

	std::string AudioSourceComponent::GetComponentType() const
	{
		return "AudioSourceComponent";
	}

	bool AudioSourceComponent::Play()
	{
		if (audioClip)
		{
			audioPlayer->OpenAudioClip(audioClip);
			audioPlayer->SetLoop(loop);
			audioPlayer->SetVolume(volume);
			return audioPlayer->Play();
		}

		return false;
	}

	void AudioSourceComponent::Stop()
	{
		audioPlayer->Stop();
	}

	void AudioSourceComponent::Update()
	{
	}

	void AudioSourceComponent::SetVolume()
	{
		audioPlayer->SetVolume(volume);
	}

	void AudioSourceComponent::SetVolume(float volume)
	{
		this->volume = volume;
		SetVolume();
	}

	float AudioSourceComponent::GetVolume() const
	{
		return audioPlayer->GetVolume();
	}

	void AudioSourceComponent::SetLoop(bool enable)
	{
		audioPlayer->SetLoop(enable);
	}

	bool AudioSourceComponent::IsPlayingAudio() const
	{
		return audioPlayer->IsPlaying();
	}

	bool AudioSourceComponent::IsLooping() const
	{
		return audioPlayer->IsLooping();
	}

	RmlUIDocumentComponent::RmlUIDocumentComponent()
	{
	}

	std::string RmlUIDocumentComponent::GetComponentType() const
	{
		return "RmlUIDocumentComponent";
	}

	std::string AnimationScriptComponent::GetComponentType() const
	{
		return "TransformScript";
	}
}
