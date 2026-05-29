#pragma once

/**
 * @file NNVideoPlayer.h
 * @brief 视频播放器：封装 FFmpeg 解码 + SDL3 音频 + NNVideoTexture 纹理更新。
 */

#include <cstdint>
#include <memory>
#include "../NNRuntimeMediaExport.h"

namespace NN::Runtime::Media
{
	class NNVideoClipAsset;
	class NNVideoTexture;

	/// 视频播放器
	class NN_RUNTIME_MEDIA_API NNVideoPlayer
	{
	public:
		~NNVideoPlayer();

		bool Play(NNVideoClipAsset* clip);
		void Stop();
		void Pause();
		void Resume();
		void Seek(double seconds);

		void SetVolume(float volume);
		void SetLoop(bool loop);

		/// 每帧调用：解码 + 更新纹理
		void Update(float deltaTime);

		/// 获取当前视频纹理
		NNVideoTexture* GetVideoTexture() const;

		double GetPlaybackTime() const;
		bool IsPlaying() const;
		bool IsFinished() const;

	private:
		NNVideoClipAsset* m_CurrentClip = nullptr;
		std::unique_ptr<NNVideoTexture> m_VideoTexture;
		float m_Volume = 1.0f;
		bool m_Loop = false;
		bool m_Playing = false;
	};

} // namespace NN::Runtime::Media
