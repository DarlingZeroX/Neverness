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

#include "Resource/Video.h"
#include <SDL3/SDL_audio.h>
#include "Graphics/OpenGL/Core.h"

namespace VisionGal
{
	/*
	void VideoPlayer::PlayBackLoop()
	{
		if (!isPlaying) return;

		int linesize = 0;
		int width = 0, height = 0;
		double pts = 0.0;
		double lastPts = 0.0;
		auto lastFrameTime = std::chrono::high_resolution_clock::now();

		// 默认帧率（兜底）
		double defaultFrameInterval = 1.0 / 30.0; // 30 FPS -> interval seconds
		double avgFrameInterval = defaultFrameInterval;

		while (isPlaying)
		{
			// 取一帧（内部会阻塞等待）
			{
				//std::lock_guard<std::mutex> lock(mutex);
				if (!decoder->GetVideoFrame(frameData, linesize, width, height, pts))
				{
					// 文件结束或解码结束
					isPlaying = false;
					break;
				}
			}

			// 🔹 确保 pts 单位是秒（假设 GetVideoFrame 已返回秒）
			if (lastPts >= 0.0)
			{
				// 理论帧间隔（秒）
				double timeDiff = pts - lastPts;
				// 过滤异常时间戳
				if (timeDiff <= 0.0 || timeDiff > 1.0) {
					timeDiff = avgFrameInterval;
				}

				// 平滑估计帧间隔
				avgFrameInterval = 0.9 * avgFrameInterval + 0.1 * timeDiff;

				auto currentTime = std::chrono::high_resolution_clock::now();
				double elapsed = std::chrono::duration<double>(currentTime - lastFrameTime).count();

				std::cout << "[Play] pts " << pts << "s lastPts=" << lastPts << "s (elapsed=" << elapsed << ", target=" << timeDiff << ")\n";

				// 只在需要时睡眠，且保证 sleep 时间非负
				if (elapsed < timeDiff) {
					double sleepSec = timeDiff - elapsed;
					if (sleepSec > 0.0) {
						// 可在这里打开调试打印
						std::cout << "[Play] sleep " << sleepSec << "s (elapsed=" << elapsed << ", target=" << timeDiff << ")\n";
						std::this_thread::sleep_for(std::chrono::duration<double>(sleepSec));
					}
				}
			}

			lastPts = pts;
			lastFrameTime = std::chrono::high_resolution_clock::now();

			// 渲染帧（把这行替换成你的渲染调用）
			// RenderFrame(frameData, linesize, width, height);
		}

		std::cout << "[VideoPlayer] Playback finished.\n";
	}*/


	void VideoPlayer::ProcessVideoUpdate(uint8_t* frameData, int& linesize, int width, int height, double pts)
	{
		//if (!isPlaying)
		//	return;
		//
		//// 暂且以 60 fps来算
		////pts = pts * 2;
		//
		//m_FrameData = frameData;
		//auto lastFrameTime = std::chrono::high_resolution_clock::now();
		//
		//double audioClock = decoder->GetAudioClock();
    	////double lastPts = 0.0;
		//
		//// 控制播放速度
		////if (audioClock == 0.f)
		//{
		//	if (m_LastPts >= 0.0) {
		//		double timeDiff = pts - m_LastPts;
		//		auto currentTime = std::chrono::high_resolution_clock::now();
		//		auto elapsedTime = std::chrono::duration<double>(currentTime - lastFrameTime).count();
		//
		//		//std::cout << "[Play] pts " << pts << "s lastPts=" << m_LastPts << ", target=" << timeDiff << ")\n";
		//		std::cout << "[Play] videoPts " << pts << "s AudioClock=" << decoder->GetAudioClock() << ")\n";
		//		//if (elapsedTime < timeDiff || pts > decoder->GetAudioClock()) {
		//		if (elapsedTime < timeDiff) {
		//			std::this_thread::sleep_for(std::chrono::duration<double>(timeDiff - elapsedTime));
		//		}
		//	}
		//}
		//
		//m_LastPts = pts;
		//lastFrameTime = std::chrono::high_resolution_clock::now();


		if (!isPlaying)
			return;

		// 视频开始时初始化基准时间
		if (!m_VideoClockInitialized) {
			m_VideoClockInitialized = true;
			 m_SystemStartTime = std::chrono::steady_clock::now();
			m_VideoStartPTS = pts;          // 通常是 0
		}

		// 计算从视频开头起，应该显示这帧的时间（秒）
		double targetTime = pts - m_VideoStartPTS;

		// 当前系统已经过去的时间（秒）
		auto now = std::chrono::steady_clock::now();
		double elapsed = std::chrono::duration<double>(now - m_SystemStartTime).count();

		//std::cout << "[Play] videoPts " << pts << "s AudioClock=" << decoder->GetAudioClock() << ")\n";

		// 如果视频太快，就睡眠
		if (elapsed < targetTime) {
			std::this_thread::sleep_for(std::chrono::duration<double>(targetTime - elapsed));
		}

		// 在这里更新纹理 / 显示图像
		m_FrameData = frameData;

		m_LastPts = pts;
	}


    void VideoPlayer::SetLoop(bool enable)
    {
		if (decoder)
		{
			decoder->SetDecodeLoop(enable);
		}
    }

    bool VideoPlayer::IsLoop()
    {
		if (decoder)
		{
			decoder->IsDecodeLoop();
		}

		return false;
    }

    float2 VideoClip::GetSize() const
    {
        return { decoder->GetWidth(),decoder->GetHeight() };
    }

	VideoPlayer::VideoPlayer()
        :
        isPlaying(false), totalDuration(0.0) {
    }

	VideoPlayer::~VideoPlayer()
    {
        Stop();

		if (decoder != nullptr)
		{
			decoder->Close();
			decoder = nullptr;
		}
    }

	bool VideoPlayer::Open(const Ref<VideoClip>& clip)
	{
        Stop();

        decoder = clip->decoder;
        totalDuration = decoder->GetDuration();

        VGFX::TextureDesc desc;
        desc.Width = decoder->GetWidth();
        desc.Height = decoder->GetHeight();
        desc.Type = GL_UNSIGNED_BYTE;
        desc.Format = GL_RGBA;
        desc.InternalFormat = GL_RGBA;
        desc.Data = nullptr;
        auto tex = VGFX::CreateTextureFromMemory(desc);
        auto tex2D = CreateRef<Texture2D>();
        tex2D->SetTexture(tex);

        sprite = Sprite::Create(tex2D, { decoder->GetWidth(), decoder->GetHeight() });

        return true;
	}

	bool VideoPlayer::Open(const std::string& filePath)
    {
        if (!decoder->Open(filePath)) {
            std::cerr << "Failed to open video file" << std::endl;
            return false;
        }

        totalDuration = decoder->GetDuration();

        //// 初始化渲染器
        //if (!renderer->initialize(decoder->GetWidth(), decoder->GetHeight(), "Video Player")) {
        //    std::cerr << "Failed to initialize renderer" << std::endl;
        //    return false;
        //}

        return true;
    }

	void VideoPlayer::Play()
    {
        if (isPlaying) return;

        decoder->Start();
        decoder->PlayAudio();

		decoder->OnVideoDataUpdate = [this](uint8_t* frameData, int& linesize, int width, int height, double pts)
			{
				ProcessVideoUpdate(frameData, linesize, width, height, pts);
			};

        isPlaying = true;
       // playbackThread = std::thread(&VideoPlayer::PlayBackLoop, this);
    }

	void VideoPlayer::Stop()
    {
		if (decoder)
			decoder->Stop();
		isPlaying = false;

		// 修复视频播放完成， 再加载线程中止崩溃Bug
		//if (playbackThread.joinable()) {
		//	playbackThread.join();
		//}

        //if (!isPlaying) return;
    }

	bool VideoPlayer::IsRunning() const
    {
        return isPlaying;// && !renderer->shouldClose();
    }

	double VideoPlayer::GetDuration() const
    {
        return totalDuration;
    }

	void VideoPlayer::Update()
	{
        if (!isPlaying)
            return;

        std::lock_guard<std::mutex> lock(mutex);

        VGFX::ITexture* tex = sprite->GetITexture();
        int textureID = reinterpret_cast<int>(tex->GetShaderResourceView());

        // 更新纹理
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->GetDesc().Width, tex->GetDesc().Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_FrameData);

	}


}
