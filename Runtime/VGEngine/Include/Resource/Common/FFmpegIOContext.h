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

extern "C" {
#include <libavformat/avio.h>
#include <libavutil/error.h>
}

#include "../../Core/VFS.h"

namespace VisionGal {

    // 自定义 opaque 结构（传递 VFS 文件对象）
    struct VFSFFmpegIOContext {
        vfspp::IFilePtr file; // VFSpp 文件句柄
		std::vector<uint8_t*> buffers;

		~VFSFFmpegIOContext();

		static int read_packet(void* opaque, uint8_t* buf, int buf_size);
		static int64_t seek(void* opaque, int64_t offset, int whence);
    };
	 
    int static_ffmpeg_read_packet(void* opaque, uint8_t* buf, int buf_size);

    int64_t static_ffmpeg_seek(void* opaque, int64_t offset, int whence);
}
