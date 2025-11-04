#pragma once
#include "../../Core/Core.h"
#include "../../EngineConfig.h"
#include "../Common/FFmpegIOContext.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

namespace VisionGal {

	// 视频解码器类
	struct VG_ENGINE_API FFmpegContext {

		static Ref<FFmpegContext> Create(const std::string& filePath);

		FFmpegContext();
		~FFmpegContext();

		void Close();
		AVCodecParameters* FindStream();
		int FindAudioStreamIndex();
		int FindVideoStreamIndex();
	public:
		VFSFFmpegIOContext ioCtx;
		AVIOContext* avioCtx = nullptr;
		unsigned char* ioBuffer = nullptr;
		AVFormatContext* formatContext = nullptr;
		AVCodecContext* codecContext = nullptr;
		int videoStreamIndex;
		int audioStreamIndex;
		//AVFrame* frame = nullptr;
		//AVFrame* frameRGB = nullptr;
		//struct SwsContext* swsContext = nullptr;

	private:
		bool Open(const std::string& filePath);
	};

}
