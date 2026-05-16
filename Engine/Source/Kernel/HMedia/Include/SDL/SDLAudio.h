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

#pragma once
#include <HCore/Interface/HCore.h>
#include <SDL3/SDL_audio.h>

namespace Horizon {

	struct SDL3AudioDevice
	{
		SDL3AudioDevice();
		SDL3AudioDevice(const SDL_AudioSpec& spec);
		SDL3AudioDevice(SDL_AudioDeviceID devid, const SDL_AudioSpec& spec);
		SDL3AudioDevice(const SDL3AudioDevice&) = delete;
		SDL3AudioDevice& operator=(const SDL3AudioDevice&) = delete;
		SDL3AudioDevice(SDL3AudioDevice&&) = delete;
		SDL3AudioDevice& operator=(SDL3AudioDevice&&) = delete;

		const SDL_AudioSpec& GetAudioSpec() const;
		SDL_AudioDeviceID GetAudioDeviceID() const { return m_AudioDev; };

		bool OpenDevice(const SDL_AudioSpec& spec);
		bool OpenDevice(SDL_AudioDeviceID devid, const SDL_AudioSpec& spec);
	private:
		SDL_AudioDeviceID m_AudioDev;
		SDL_AudioSpec m_AudioSpec;
	};

	struct SDL3AudioStream
	{
		SDL3AudioStream();
		~SDL3AudioStream();
		SDL3AudioStream(const SDL3AudioStream&) = delete;
		SDL3AudioStream& operator=(const SDL3AudioStream&) = delete;
		SDL3AudioStream(SDL3AudioStream&&) = delete;
		SDL3AudioStream& operator=(SDL3AudioStream&&) = delete;

		bool OpenStream(SDL3AudioDevice& dev, SDL_AudioStreamCallback callback, void* userdata);
		bool OpenStream(SDL3AudioDevice& dev, const SDL_AudioSpec& spec, SDL_AudioStreamCallback callback, void* userdata);

		bool PauseStream() const;
		bool ResumeStream() const;

		void Clear();
		bool HasStream() const;
	private:
		SDL_AudioStream* m_AudioStream = nullptr;
	};
}
