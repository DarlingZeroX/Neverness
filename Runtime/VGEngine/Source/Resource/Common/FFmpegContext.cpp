#include "Resource/Common/FFmpegContext.h"
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include "Core/VFS.h"

namespace VisionGal
{
	FFmpegContext::FFmpegContext()
		: formatContext(nullptr), codecContext(nullptr),
		videoStreamIndex(-1) {
	}

	FFmpegContext::~FFmpegContext()
	{
		Close();
	}

	Ref<FFmpegContext> FFmpegContext::Create(const std::string& filePath)
	{
		auto context = CreateRef<FFmpegContext>();

		if (bool result = context->Open(filePath))
			return context;
		else
			return nullptr;
	}

	AVCodecParameters* FFmpegContext::FindStream()
	{
		videoStreamIndex = -1;
		audioStreamIndex = -1;
		AVCodecParameters* codecParameters = nullptr;

		for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
			if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
				videoStreamIndex = i;
				codecParameters = formatContext->streams[i]->codecpar;
			}

			if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
				audioStreamIndex = i;
			}
		}

		return codecParameters;
	}

	int FFmpegContext::FindAudioStreamIndex()
	{
		int streamIndex = -1;

		for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
			if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
				streamIndex = i;
			}
		}

		return streamIndex;
	}

	int FFmpegContext::FindVideoStreamIndex()
	{
		int streamIndex = -1;

		for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
			if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
				streamIndex = i;
			}
		}

		return streamIndex;
	}

	bool FFmpegContext::Open(const std::string& filePath)
	{
		// 初始化 FFmpeg
		avformat_network_init();
		//av_register_all();

		auto& vfs = VFS::GetInstance();

		// 尝试打开文件
		auto file = vfs->OpenFile(vfspp::FileInfo(filePath), vfspp::IFile::FileMode::Read);
		if (!file)
			return false;

		// 文件是否打开
		if (file->IsOpened() == false)
			return false;

		// 用ffmpeg读取视频音频
		//AVFormatContext* fmt_ctx = avformat_alloc_context();
		formatContext = avformat_alloc_context();
		ioCtx = { file }; // file 是已打开的 vfspp::IFile 对象

		// 创建 AVIOContext（关键！）
		const int buffer_size = 4096; // 建议缓冲区大小
		ioBuffer = static_cast<unsigned char*>(av_malloc(buffer_size));
		avioCtx = avio_alloc_context(
			ioBuffer, buffer_size,				// 缓冲区及大小
			0,									// 只读模式
			&ioCtx,								// 传递给回调的 opaque 对象
			VFSFFmpegIOContext::read_packet,	// 绑定回调函数
			nullptr,							// 无需写回调
			VFSFFmpegIOContext::seek			// Seek 回调（可选）
		);

		// 关联到 FormatContext
		formatContext->pb = avioCtx;

		// 增加探测深度以处理复杂或损坏的文件
		{
			//ffmpeg -err_detect ignore_err -i yourfile.mp4 -c copy -movflags +faststart output.mp4
			AVDictionary* options = nullptr;
			av_dict_set(&options, "analyzeduration", "200000000", 0); // 最大int值
			av_dict_set(&options, "probesize", "200000000", 0);       // 最大int值
			av_dict_set(&options, "movflags", "faststart", 0);   // 修复MOOV原子位置
			av_dict_set(&options, "err_detect", "ignore_err", 0); //这样在读取数据时会少报错（但如果数据真损坏还是会丢帧）。

			// 打开输入文件
			if (avformat_open_input(&formatContext, nullptr, nullptr, &options) != 0) {
				std::cerr << "Failed to open input file" << std::endl;
				av_dict_free(&options);
				return false;
			}

			// 释放选项字典
			av_dict_free(&options);
		}

		// 读取文件信息,获取流信息
		if (avformat_find_stream_info(formatContext, nullptr) < 0) {
			std::cerr << "Failed to find stream information" << std::endl;
			return false;
		}

		// 打印文件信息（可选）
		//av_dump_format(formatContext, 0, filePath.c_str(), 0);

		// 查找视频流和音频流
		//AVCodecParameters* codecParameters = FindStream();

		return true;
	}

	void FFmpegContext::Close()
	{
		// 视频
		if (formatContext) {
			avformat_close_input(&formatContext);
			formatContext = nullptr;
		}

		if (codecContext) {
			avcodec_free_context(&codecContext);
			codecContext = nullptr;
		}

	}
}
