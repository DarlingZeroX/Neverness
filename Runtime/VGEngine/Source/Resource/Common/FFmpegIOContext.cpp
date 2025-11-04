#include "Resource/Common/FFmpegIOContext.h"

namespace VisionGal {
	VFSFFmpegIOContext::~VFSFFmpegIOContext()
	{
		//for (auto* buf: buffers)
		//{
		//	delete buf; // 释放缓冲区
		//}
	}

	int VFSFFmpegIOContext::read_packet(void* opaque, uint8_t* buf, int buf_size)
	{
		VFSFFmpegIOContext* ctx = static_cast<VFSFFmpegIOContext*>(opaque);
		// 调用 VFSpp 的 Read 接口
		uint64_t bytes_read = ctx->file->Read(buf, static_cast<uint64_t>(buf_size));
		if (bytes_read == 0) return AVERROR_EOF; // 文件结束

		return static_cast<int>(bytes_read);
	}

	int64_t VFSFFmpegIOContext::seek(void* opaque, int64_t offset, int whence)
	{
		VFSFFmpegIOContext* ctx = static_cast<VFSFFmpegIOContext*>(opaque);

		if (whence == AVSEEK_SIZE) {
			// FFmpeg在初始化时会用它来探测文件大小
			return ctx->file->Size();
		}

		vfspp::IFile::Origin origin;
		switch (whence) {
		case SEEK_SET:
			origin = vfspp::IFile::Origin::Begin;
			break;
		case SEEK_CUR:
			origin = vfspp::IFile::Origin::Set; //  这里注意：请确认Set是否代表 Current
			break;
		case SEEK_END:
			origin = vfspp::IFile::Origin::End;
			break;
		default:
			return -1; // 不支持
		}

		// 调用你的 Seek
		uint64_t new_pos = ctx->file->Seek(static_cast<uint64_t>(offset), origin);
		return static_cast<int64_t>(new_pos);
	}

	//static int64_t static_ffmpeg_seek(void* opaque, int64_t offset, int whence) {
    //    VFS_VIDEO_IO_Context* ctx = static_cast<VFS_VIDEO_IO_Context*>(opaque);
    //
    //    if (whence == AVSEEK_SIZE) {
    //        return ctx->file->Size();
    //    }
    //
    //    if (whence == SEEK_CUR) {
    //        uint64_t current = ctx->file->Tell();
    //        return ctx->file->Seek(current + offset, vfspp::IFile::Origin::Begin);
    //    }
    //
    //    vfspp::IFile::Origin origin;
    //    switch (whence) {
    //    case SEEK_SET:
    //        origin = vfspp::IFile::Origin::Begin;
    //        break;
    //    case SEEK_END:
    //        origin = vfspp::IFile::Origin::End;
    //        break;
    //    default:
    //        return -1;
    //    }
    //
    //    return ctx->file->Seek(static_cast<uint64_t>(offset), origin);
    //}
    int static_ffmpeg_read_packet(void* opaque, uint8_t* buf, int buf_size)
    {
		VFSFFmpegIOContext* ctx = static_cast<VFSFFmpegIOContext*>(opaque);
        // 调用 VFSpp 的 Read 接口
        uint64_t bytes_read = ctx->file->Read(buf, static_cast<uint64_t>(buf_size));
        if (bytes_read == 0) return AVERROR_EOF; // 文件结束

        return static_cast<int>(bytes_read);
    }

    int64_t static_ffmpeg_seek(void* opaque, int64_t offset, int whence)
    {
		VFSFFmpegIOContext* ctx = static_cast<VFSFFmpegIOContext*>(opaque);

        if (whence == AVSEEK_SIZE) {
            // FFmpeg在初始化时会用它来探测文件大小
            return ctx->file->Size();
        }

        vfspp::IFile::Origin origin;
        switch (whence) {
        case SEEK_SET:
            origin = vfspp::IFile::Origin::Begin;
            break;
        case SEEK_CUR:
            origin = vfspp::IFile::Origin::Set; //  这里注意：请确认Set是否代表 Current
            break;
        case SEEK_END:
            origin = vfspp::IFile::Origin::End;
            break;
        default:
            return -1; // 不支持
        }

        // 调用你的 Seek
        uint64_t new_pos = ctx->file->Seek(static_cast<uint64_t>(offset), origin);
        return static_cast<int64_t>(new_pos);
    }
}
