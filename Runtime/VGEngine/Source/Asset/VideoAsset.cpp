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

#include "Asset/VideoAsset.h"
#include "Resource/Video/FVideoDecoder.h"

namespace VisionGal
{
	// 自定义 opaque 结构（传递 VFS 文件对象）
	//struct VFSIOContext {
	//	vfspp::IFilePtr file; // VFSpp 文件句柄
	//};
	//
	//static int read_packet(void* opaque, uint8_t* buf, int buf_size) {
	//	VFSIOContext* ctx = static_cast<VFSIOContext*>(opaque);
	//	// 调用 VFSpp 的 Read 接口
	//	uint64_t bytes_read = ctx->file->Read(buf, static_cast<uint64_t>(buf_size));
	//	if (bytes_read == 0) return AVERROR_EOF; // 文件结束
	//	return static_cast<int>(bytes_read);
	//}
	 
	bool VideoAssetLoader::Read(const std::string path, Ref<VGAsset>& asset)
	{
		auto videoAsset = CreateRef<VideoAsset>();
		videoAsset->videoClip = CreateRef<FVideoClip>();
		//videoAsset->videoClip->decoder = CreateRef<FVideoDecoder>();

		if ( videoAsset->videoClip->Open(path))
		//if ( videoAsset->videoClip->decoder->Open(path))
		{
			asset = videoAsset;
			return true;
		}

		return false;
		/*
		auto& vfs = Core::GetVFS();

		// 尝试打开文件
		auto file = vfs->OpenFile(vfspp::FileInfo(path), vfspp::IFile::FileMode::Read);
		if (!file)
			return false;

		// 文件是否打开
		if (file->IsOpened() == false)
			return false;

		// 用ffmpeg读取视频
		//AVFormatContext* fmt_ctx = avformat_alloc_context();
		videoAsset->videoClip->pFormatCtx = avformat_alloc_context();
		VFSIOContext io_ctx{ file }; // file 是已打开的 vfspp::IFile 对象

		// 创建 AVIOContext（关键！）
		const int buffer_size = 4096; // 建议缓冲区大小
		unsigned char* io_buffer = static_cast<unsigned char*>(av_malloc(buffer_size));
		AVIOContext* avio_ctx = avio_alloc_context(
			io_buffer, buffer_size,       // 缓冲区及大小
			0,                            // 只读模式
			&io_ctx,                      // 传递给回调的 opaque 对象
			read_packet,                  // 绑定回调函数
			nullptr,                      // 无需写回调
			nullptr                        // Seek 回调（可选）
		);

		// 关联到 FormatContext
		videoAsset->videoClip->pFormatCtx->pb = avio_ctx;

		// 打开视频文件 注意：文件名传 nullptr，因数据源来自自定义 I/O
		int ret = avformat_open_input(&videoAsset->videoClip->pFormatCtx, nullptr, nullptr, nullptr);
		if (ret < 0) {	// Couldn't open file
			char err_buf[256];
			av_strerror(ret, err_buf, sizeof(err_buf));
			// 错误处理：输出或抛出异常
			return false;
		}

		// 获取流信息
		if (avformat_find_stream_info(videoAsset->videoClip->pFormatCtx, nullptr) < 0)
			return false; // Couldn't find stream information

		// 视频文件的多个流中查找视频流
		videoAsset->videoClip->videoStream = -1;

		for (int i = 0; i < videoAsset->videoClip->pFormatCtx->nb_streams; i++) {
			if (videoAsset->videoClip->pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
				videoAsset->videoClip->videoStream = i;
				break;
			}
		}

		if (videoAsset->videoClip->videoStream == -1)
			return false; // Didn't find a video stream

		// videoAsset->pFormatCtx->streams是所有流的数组，videoAsset->videoStream指定了要获取的视频流索引，codecpar就是该视频流的编解码参数。
		AVCodecParameters* pVideoParams = videoAsset->videoClip->pFormatCtx->streams[videoAsset->videoClip->videoStream]->codecpar;

		// 使用avcodec_find_decoder函数，根据获取的编解码参数中的codec_id（编解码器 ID），在系统中查找对应的解码器。返回值赋给codec。
		const AVCodec* codec = avcodec_find_decoder(pVideoParams->codec_id);

		if (codec == nullptr)
			return false; // Codec not found

		// 使用avcodec_alloc_context3函数，为找到的解码器分配一个编解码上下文，并将其赋给videoAsset的pCodecCtx成员。
		videoAsset->videoClip->pCodecCtx = avcodec_alloc_context3(codec);

		// 把第一步获取的编解码参数pVideoParams中的信息填充到刚刚分配的编解码上下文videoAsset->pCodecCtx中。这样编解码上下文就具备了正确解码视频流所需的参数。
		avcodec_parameters_to_context(videoAsset->videoClip->pCodecCtx, pVideoParams);

		if (avcodec_open2(videoAsset->videoClip->pCodecCtx, NULL, NULL) < 0)
			return false;

		// 分配内存用于存储图像数据
		videoAsset->videoClip->pFrame = av_frame_alloc();
		videoAsset->videoClip->pFrameRGB = av_frame_alloc();


		videoAsset->videoClip->buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, videoAsset->videoClip->pCodecCtx->width, videoAsset->videoClip->pCodecCtx->height, 0));
		av_image_fill_arrays(videoAsset->videoClip->pFrameRGB->data, &videoAsset->videoClip->pFrameRGB->linesize[0], videoAsset->videoClip->buffer, AV_PIX_FMT_RGB24, videoAsset->videoClip->pCodecCtx->width, videoAsset->videoClip->pCodecCtx->height, 1);
		*/
		return true;
	}
}
