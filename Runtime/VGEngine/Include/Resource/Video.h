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
#include "../Core/Core.h"
#include "../EngineConfig.h"
#include "../Render/Sprite.h"
#include "Video/VideoDecoder.h"

namespace VisionGal {
    class VG_ENGINE_API VideoClip : public VGEngineResource
    {
    public:
        VideoClip() = default;
        VideoClip(const VideoClip&) = delete;
        VideoClip& operator=(const VideoClip&) = delete;
        VideoClip(VideoClip&&) noexcept = default;
        VideoClip& operator=(VideoClip&&) noexcept = default;
        ~VideoClip() override = default;

        float2 GetSize() const;

        Ref<VideoDecoder> decoder;
    };
	 
    // 视频播放器类 - 封装解码和渲染
    class VG_ENGINE_API VideoPlayer {
    public:
        VideoPlayer();
        ~VideoPlayer();

        bool Open(const Ref<VideoClip>& clip);
        bool Open(const std::string& filePath);
        void Play();
        void Stop();
        bool IsRunning() const;
        double GetDuration() const;
        //void PlayBackLoop();

		void SetLoop(bool enable);
		bool IsLoop();

        void Update();

        Sprite* GetSprite() const { return sprite.get(); }
    private:
		void ProcessVideoUpdate(uint8_t* frameData, int& linesize, int width, int height, double pts);

        Ref<VideoDecoder> decoder;
        //std::unique_ptr<OpenGLRenderer> renderer;
        std::thread playbackThread;
        std::mutex mutex;
        std::condition_variable cv;
        bool isPlaying;
        double totalDuration;

        Ref<Sprite> sprite;
        uint8_t* m_FrameData = nullptr;

		double m_LastPts = 0.0f;

		bool m_VideoClockInitialized = false;
		double m_VideoStartPTS = 0.f;
		std::chrono::time_point<std::chrono::steady_clock>  m_SystemStartTime;
    };


}
