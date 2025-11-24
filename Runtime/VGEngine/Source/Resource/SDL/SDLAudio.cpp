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

#include "Resource/SDL/SDLAudio.h"

namespace VisionGal {
	SDL3AudioDevice::SDL3AudioDevice()
	{
	}

	SDL3AudioDevice::SDL3AudioDevice(const SDL_AudioSpec& spec)
	{
		OpenDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, spec);
	}

	SDL3AudioDevice::SDL3AudioDevice(SDL_AudioDeviceID devid, const SDL_AudioSpec& spec)
	{
		OpenDevice(devid, spec);
	}

	const SDL_AudioSpec& SDL3AudioDevice::GetAudioSpec() const
	{
		return m_AudioSpec;
	}

	bool SDL3AudioDevice::OpenDevice(const SDL_AudioSpec& spec)
	{
		return OpenDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, spec);
	}

	bool SDL3AudioDevice::OpenDevice(SDL_AudioDeviceID devid, const SDL_AudioSpec& spec)
	{
		m_AudioSpec = spec;

		// 1. 打开默认输出设备
		m_AudioDev = SDL_OpenAudioDevice(devid, &spec);
		if (!m_AudioDev) {
			H_LOG_ERROR("Failed to open audio device: %s", SDL_GetError());
			return false;
		}

		return true;
	}

	SDL3AudioStream::SDL3AudioStream()
	{
	}

	SDL3AudioStream::~SDL3AudioStream()
	{
		Clear();
	}

	bool SDL3AudioStream::OpenStream(SDL3AudioDevice& dev, SDL_AudioStreamCallback callback, void* userdata)
	{
		return OpenStream(dev, dev.GetAudioSpec(), callback, userdata);
	}

	bool SDL3AudioStream::OpenStream(
		SDL3AudioDevice& dev, 
		const SDL_AudioSpec& spec,
		SDL_AudioStreamCallback callback, 
		void* userdata
	)
	{
		Clear();

		// 3. 打开音频流
		m_AudioStream = SDL_OpenAudioDeviceStream(dev.GetAudioDeviceID(), &spec, callback, userdata);
		if (!m_AudioStream) {
			H_LOG_ERROR("Failed to create audio stream: %s", SDL_GetError());
			return false;
		}

		return true;
	}

	bool SDL3AudioStream::PauseStream() const
	{
		//H_ASSERT_NOT_NULL(m_AudioStream);
		if (m_AudioStream == nullptr)
			return false;

		return SDL_PauseAudioStreamDevice(m_AudioStream);
	}

	bool SDL3AudioStream::ResumeStream() const
	{
		//H_ASSERT_NOT_NULL(m_AudioStream);
		if (m_AudioStream == nullptr)
			return false;

		return SDL_ResumeAudioStreamDevice(m_AudioStream);
	}

	void SDL3AudioStream::Clear()
	{
		if (m_AudioStream != nullptr)
		{
			SDL_DestroyAudioStream(m_AudioStream);
			m_AudioStream = nullptr;
		}
	}

	bool SDL3AudioStream::HasStream() const
	{
		return m_AudioStream != nullptr;
	}
}
