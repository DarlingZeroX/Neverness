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
//#include "Interface/VideoInterface.h"
#include "VGCore/Include/Core/Core.h"
#include "../EngineConfig.h"
#include <NNMediaCore/Include/FVideo.h>
#include <VGAsset/Interface/VideoClip.h>

namespace VisionGal {

	// 音频解码器接口
	struct IVideoPlayer: public Horizon::IVideoPlayer{
		~IVideoPlayer() override = default;

		virtual bool Open(const Ref<IVideoClip>& clip) = 0;					// 打开音频片段
		//
		//virtual void Update() = 0;											// 更新
		virtual VGFX::ITexture* GetVideoTexture() const = 0;				// 获取视频纹理
		virtual Ref<VGFX::ITexture>GetVideoTextureRef() const = 0;				// 获取视频纹理
	};

	// 视频播放器类 - 封装解码和渲染
	class VG_ENGINE_API FVideoPlayer : public IVideoPlayer
	{
	public:
		FVideoPlayer();
		~FVideoPlayer() override;

		static Ref<FVideoPlayer> CreatePlayer(const Ref<IVideoClip>& clip);

		bool Open(const Ref<IVideoClip>& clip) override;				// 打开音频片段
		void Play() override;											// 播放视频
		void Stop() override;											// 暂停播放
		bool IsPlaying() const override;								// 是否正在播放视频
		double GetDuration() const override;							// 获取视频时长
		bool Pause() override;											// 暂停播放
		bool Restore() override;										// 恢复播放
		bool IsLooping() const override;								// 是否循环播放
		void SetLoop(bool loop) override;								// 设置循环播放
		double GetPlaybackTime() const override;						// 获取当前播放时间（秒）
		bool Seek(double seconds) override;								// 视频跳转
		bool RestartPlay() override;
		bool SetVolume(float v) override;									// 设置音量
		float GetVolume() const override;									// 获取音量
		float GetVideoWidth() const override;
		float GetVideoHeight() const override;

		void Update() override;											// 更新
		VGFX::ITexture* GetVideoTexture() const override;				// 获取视频纹理
		Ref<VGFX::ITexture> GetVideoTextureRef() const override;				// 获取视频纹理
		uint8_t* GetFrameData() override;

	private:
		Ref<Horizon::IVideoPlayer> m_VideoPlayer;
		Ref<VGFX::ITexture> m_VideoTexture;
	};

}
