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

namespace Horizon {

    class InMemoryRemuxer {
    public:
        InMemoryRemuxer();
        ~InMemoryRemuxer();
		 
        bool Remux(vfspp::VirtualFileSystemPtr& vfs, const char* input_filename);
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
