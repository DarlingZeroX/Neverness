#pragma once

#include "../../Core/Core.h"
#include "../../EngineConfig.h"

#include "FFmpegIOContext.h"

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

    class InMemoryRemuxer {
    public:
        InMemoryRemuxer();
        ~InMemoryRemuxer();
		 
        bool Remux(const char* input_filename);
        AVFormatContext* GetFormatContext() const { return fixed_fmt_ctx; }

    private:
        struct BufferData {
            uint8_t* ptr;
            size_t size;
        };

        static int ReadPacket(void* opaque, uint8_t* buf, int buf_size);

        AVFormatContext* fixed_fmt_ctx = nullptr;
        AVIOContext* mem_avio_ctx = nullptr;
        BufferData* buffer_data = nullptr;

        uint8_t* avio_internal_buf = nullptr; // avio_alloc_context的内部缓冲
        uint8_t* memory_buffer = nullptr;     // 复制后的完整mp4文件

		VFSFFmpegIOContext io_ctx;

        void Cleanup();
    };
}
