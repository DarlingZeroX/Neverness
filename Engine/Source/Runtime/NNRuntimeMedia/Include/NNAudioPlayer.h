#pragma once

/**
 * @file NNAudioPlayer.h
 * @brief 音频播放器：封装 SDL3 音频输出。
 */

#include <cstdint>
#include "../NNRuntimeMediaExport.h"

namespace NN::Runtime::Media
{
	class NNAudioClipAsset;

	/// 音频播放器
	class NN_RUNTIME_MEDIA_API NNAudioPlayer
	{
	public:
		bool Play(NNAudioClipAsset* clip);
		void Stop();
		void Pause();
		void Resume();
		void Seek(double seconds);

		void SetVolume(float volume);
		void SetPitch(float pitch);
		void SetLoop(bool loop);

		double GetPlaybackTime() const;
		bool IsPlaying() const;

	private:
		NNAudioClipAsset* m_CurrentClip = nullptr;
		float m_Volume = 1.0f;
		float m_Pitch = 1.0f;
		bool m_Loop = false;
		bool m_Playing = false;
	};

} // namespace NN::Runtime::Media
