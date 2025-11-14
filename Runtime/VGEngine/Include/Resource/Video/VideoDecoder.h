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
#include <SDL3/SDL_audio.h>

#include "../../Core/Core.h"
#include "../../EngineConfig.h"

#include "../Common/FFmpegIOContext.h"
#include "../Audio/AudioRingBuffer.h"
//#include "InMemoryRemuxer.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
}

namespace VisionGal {
	 
    // 视频解码器类
    class VG_ENGINE_API VideoDecoder {
    public:
        VideoDecoder();
        ~VideoDecoder();

        bool Open(const std::string& filePath);

        //bool ReadFrame(uint8_t*& frameData, int& linesize, int& width, int& height, double& videoPts, double& audioPts);
        void Close();
        int GetWidth() const { return width; }
        int GetHeight() const { return height; }
        double GetDuration() const;
        void PlayAudio();

        void Start();
        void Stop();

        void DemuxLoop();
        void AudioDecodeLoop();
		bool VideoDecodeLoop();

        bool GetVideoFrame(uint8_t*& frameData, int& linesize, int& width, int& height, double& pts);

        AudioRingBuffer* GetAudioBuffer() { return audioRingBuffer.get(); }
        double GetAudioClock() const { return audioClock; }

		void SetDecodeLoop(bool enable);
		bool IsDecodeLoop();

    	std::function<void(uint8_t* frameData, int& linesize, int width, int height, double pts)> OnVideoDataUpdate = nullptr;
    protected:
        AVCodecParameters* FindStream();
        bool OpenVideoDecoder(const AVCodecParameters* codecParameters);
    private:
		bool m_LoopPlay = false;

		AVIOContext* avio_ctx = nullptr;
		unsigned char* io_buffer = nullptr;
        AVFormatContext* formatContext = nullptr;
        AVCodecContext* codecContext = nullptr;
        int videoStreamIndex;
        int audioStreamIndex;
        AVFrame* frame = nullptr;
        AVFrame* frameRGB = nullptr;
        struct SwsContext* swsContext = nullptr;

        //视频信息
        uint8_t* buffer = nullptr;
        int numBytes;
        int width, height;
        AVRational timeBase;

		VFSFFmpegIOContext io_ctx;

        //音频
        AVCodecContext* actx = nullptr;
        AVFrame* aframe = nullptr;
        SwrContext* swr = nullptr;
        SDL_AudioDeviceID audioDev;
        SDL_AudioStream* audioStream = nullptr;
        uint8_t* audioBuf[2];
        int audioMaxSamples = 1024;
        Ref<AudioRingBuffer> audioRingBuffer;
        bool audioThreadRunning = true;
        //std::thread audioThread;

        //
        std::queue<AVPacket*> vPackets;
        std::queue<AVPacket*> aPackets;
        mutable std::mutex mutex;
        std::condition_variable cond;

        std::thread demuxThread;
        std::thread audioThread;
		std::thread videoThread;
        std::atomic<bool> running = false;
        bool demuxFinished = false;

        double audioClock = 0.0;
    };

}
