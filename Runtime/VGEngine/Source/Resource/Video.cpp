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
    void VideoPlayer::PlayBackLoop()
    {
        if (!isPlaying)
            return;
        
        int linesize = 0;
        int width = 0, height = 0;
        double videoPts = 0.0, audioPts = 0.0;
        double lastVideoPts = 0.0;
        auto lastFrameTime = std::chrono::high_resolution_clock::now();
        
        //while (isPlaying) {
        //
		//	{
        //        //std::lock_guard<std::mutex> lock(mutex);
        //        if (!decoder->GetVideoFrame(frameData, linesize, width, height, videoPts))
        //        {
        //            isPlaying = false; // 播放结束
        //            break;
        //        }
		//	}
        //
        //
        //    double audioClock = decoder->GetAudioClock();
        //    while (videoPts > audioClock) {
        //        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        //        audioClock = decoder->GetAudioClock();
        //    }
        //    H_LOG_INFO("videoPts: %f, audioClock: %f", videoPts, audioClock);
        //}

        double pts = 0.0;
        double lastPts = 0.0;
        while (isPlaying) {
            // 读取一帧（可能是视频或音频）
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (!decoder->GetVideoFrame(frameData, linesize, width, height, pts))
                {
                    isPlaying = false; // 播放结束
                    break;
                }
            }

        	// 控制播放速度
            if (lastPts > 0.0) {
                double timeDiff = pts - lastPts;
                auto currentTime = std::chrono::high_resolution_clock::now();
                auto elapsedTime = std::chrono::duration<double>(currentTime - lastFrameTime).count();

                if (elapsedTime < timeDiff) {
                    std::this_thread::sleep_for(std::chrono::duration<double>(timeDiff - elapsedTime));
                }
            }

            lastPts = pts;
            lastFrameTime = std::chrono::high_resolution_clock::now();

        }
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

        isPlaying = true;
        playbackThread = std::thread(&VideoPlayer::PlayBackLoop, this);
    }

	void VideoPlayer::Stop()
    {
        if (!isPlaying) return;

		if (decoder)
			decoder->Stop();
        isPlaying = false;

		if (playbackThread.joinable()) {
			playbackThread.join();
		}

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->GetDesc().Width, tex->GetDesc().Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frameData);

	}
}
