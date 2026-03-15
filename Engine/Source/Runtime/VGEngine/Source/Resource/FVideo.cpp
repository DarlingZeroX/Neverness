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

#include "Resource/FVideo.h"
#include <SDL3/SDL_audio.h>

#include "VGCore/Include/Core/VFS.h"
//#include "Graphics/OpenGL/Core.h"
//#include "Graphics/Interface/Device.h"
//#include "Graphics/OpenGL/ThrowMarco.h"
#include <VGRHI/Include/OpenGL/Core.h>
#include <VGRHI/Include/OpenGL/ThrowMarco.h>
#include <VGRHI/Interface/Device.h>

namespace VisionGal
{
	////////////////////	FVideoClip
	uint2 FVideoClip::GetSize() const
	{
		//return { m_VideoDecoder->GetVideoWidth(),m_VideoDecoder->GetVideoHeight() };
		return { m_VideoClip.GetSize().x,m_VideoClip.GetSize().y };
	}

	Horizon::IVideoDecoder* FVideoClip::GetDecoder()
	{
		//return m_VideoDecoder.get();
		return m_VideoClip.GetDecoder();
	}

	bool FVideoClip::Open(const std::string& filePath)
	{
		return m_VideoClip.Open(VFS::GetInstance(), filePath);
	}

	bool FVideoPlayer::IsLooping() const
	{
		if (m_VideoPlayer == nullptr)
			return false;

		return m_VideoPlayer->IsLooping();
	}

	void FVideoPlayer::SetLoop(bool loop)
	{
		if (m_VideoPlayer == nullptr)
			return;

		return m_VideoPlayer->SetLoop(loop);
	}

	FVideoPlayer::FVideoPlayer()
	{
	}

	FVideoPlayer::~FVideoPlayer()
	{
	}

	Ref<FVideoPlayer> FVideoPlayer::CreatePlayer(const Ref<IVideoClip>& clip)
	{
		auto player = CreateRef<FVideoPlayer>();
		player->Open(clip);
		return player;
	}

	bool FVideoPlayer::Open(const Ref<IVideoClip>& clip)
	{
		m_VideoPlayer = Horizon::FVideoPlayer::CreatePlayer(clip);

		if (m_VideoPlayer == nullptr)
			return false;

		VGFX::TextureDesc desc;
		desc.Width = m_VideoPlayer->GetVideoWidth();
		desc.Height = m_VideoPlayer->GetVideoHeight();
		desc.Type = GL_UNSIGNED_BYTE;
		desc.Format = GL_RGBA;
		desc.InternalFormat = GL_RGBA;
		desc.Data = nullptr;
		m_VideoTexture = VGFX::CreateTextureFromMemory(desc);

		return true;
	}

	void FVideoPlayer::Play()
	{
		if (m_VideoPlayer == nullptr)
			return;

		return m_VideoPlayer->Play();
	}

	void FVideoPlayer::Stop()
	{
		if (m_VideoPlayer == nullptr)
			return;

		return m_VideoPlayer->Stop();
	}

	bool FVideoPlayer::IsPlaying() const
	{
		if (m_VideoPlayer == nullptr)
			return false;

		return m_VideoPlayer->IsPlaying();
	}

	double FVideoPlayer::GetDuration() const
	{
		if (m_VideoPlayer == nullptr)
			return false;

		return m_VideoPlayer->GetDuration();
	}

	bool FVideoPlayer::Pause()
	{
		if (m_VideoPlayer == nullptr)
			return false;

		return m_VideoPlayer->Pause();
	}

	bool FVideoPlayer::Restore()
	{
		if (m_VideoPlayer == nullptr)
			return false;

		return m_VideoPlayer->Restore();
	}

	void FVideoPlayer::Update()
	{
		if (m_VideoPlayer == nullptr)
			return;

		m_VideoPlayer->Update();

		int textureID = reinterpret_cast<int>(m_VideoTexture->GetShaderResourceView());

		// 更新纹理
		GL_THROW_INFO(glBindTexture(GL_TEXTURE_2D, textureID));
		GL_THROW_INFO(glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA,
			m_VideoTexture->GetDesc().Width,
			m_VideoTexture->GetDesc().Height,
			0, GL_RGBA, GL_UNSIGNED_BYTE,
			GetFrameData()
		));

		GL_THROW_INFO(glBindTexture(GL_TEXTURE_2D, 0));
	}

	VGFX::ITexture* FVideoPlayer::GetVideoTexture() const
	{
		return m_VideoTexture.get();
	}

	Ref<VGFX::ITexture> FVideoPlayer::GetVideoTextureRef() const
	{
		return m_VideoTexture;
	}

	uint8_t* FVideoPlayer::GetFrameData()
	{
		if (m_VideoPlayer == nullptr)
			return nullptr;

		return m_VideoPlayer->GetFrameData();
	}

	double FVideoPlayer::GetPlaybackTime() const
	{
		if (m_VideoPlayer == nullptr)
			return false;

		return m_VideoPlayer->GetPlaybackTime();
	}

	bool FVideoPlayer::Seek(double seconds)
	{
		if (m_VideoPlayer == nullptr)
			return false;

		return m_VideoPlayer->Seek(seconds);
	}

	bool FVideoPlayer::RestartPlay()
	{
		if (m_VideoPlayer == nullptr)
			return false;

		return m_VideoPlayer->RestartPlay();
	}

	bool FVideoPlayer::SetVolume(float v)
	{
		if (m_VideoPlayer == nullptr)
			return false;

		return m_VideoPlayer->SetVolume(v);
	}

	float FVideoPlayer::GetVolume() const
	{
		if (m_VideoPlayer == nullptr)
			return 0.f;

		return m_VideoPlayer->GetVolume();
	}

	float FVideoPlayer::GetVideoWidth() const
	{
		if (m_VideoPlayer == nullptr)
			return 0.f;

		return m_VideoPlayer->GetVideoWidth();
	}

	float FVideoPlayer::GetVideoHeight() const
	{
		if (m_VideoPlayer == nullptr)
			return 0.f;

		return m_VideoPlayer->GetVideoHeight();
	}
}
