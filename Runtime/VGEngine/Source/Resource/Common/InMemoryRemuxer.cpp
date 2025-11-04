#include "Resource/Common/InMemoryRemuxer.h"
#include "Core/VFS.h"

namespace VisionGal
{
	InMemoryRemuxer::InMemoryRemuxer() {}

	InMemoryRemuxer::~InMemoryRemuxer() {
		Cleanup();
	}

	void InMemoryRemuxer::Cleanup() {
		if (fixed_fmt_ctx) {
			avformat_close_input(&fixed_fmt_ctx);
		}
		if (mem_avio_ctx) {
			av_free(mem_avio_ctx->buffer);
			av_free(mem_avio_ctx);
		}
		if (buffer_data) {
			delete buffer_data;
		}
		if (memory_buffer) {
			av_free(memory_buffer);
		}
	}

	int InMemoryRemuxer::ReadPacket(void* opaque, uint8_t* buf, int buf_size) {
		BufferData* bd = (BufferData*)opaque;
		int to_copy = FFMIN(buf_size, (int)bd->size);
		if (to_copy == 0) {
			return AVERROR_EOF; // 必须用 AVERROR_EOF，不是 0
		}
		memcpy(buf, bd->ptr, to_copy);
		bd->ptr += to_copy;
		bd->size -= to_copy;
		return to_copy;
	}

	bool InMemoryRemuxer::Remux(const char* input_filename) {
		Cleanup();

		int ret = 0;
		//AVFormatContext* in_fmt_ctx = nullptr;
		AVFormatContext* out_fmt_ctx = nullptr;

		// ------------------------
		// 1. 打开输入文件
		// ------------------------
		auto& vfs = VFS::GetInstance();

		// 尝试打开文件
		auto file = vfs->OpenFile(vfspp::FileInfo(input_filename), vfspp::IFile::FileMode::Read);
		if (!file)
			return false;

		// 文件是否打开
		if (file->IsOpened() == false)
			return false;

		// 用ffmpeg读取视频
		//AVFormatContext* fmt_ctx = avformat_alloc_context();
		auto in_fmt_ctx = avformat_alloc_context();
		io_ctx = { file }; // file 是已打开的 vfspp::IFile 对象

		// 创建 AVIOContext（关键！）
		const int buffer_size = 4096; // 建议缓冲区大小
		unsigned char* io_buffer = static_cast<unsigned char*>(av_malloc(buffer_size));
		AVIOContext* avio_ctx = avio_alloc_context(
			io_buffer, buffer_size,       // 缓冲区及大小
			0,                            // 只读模式
			&io_ctx,                      // 传递给回调的 opaque 对象
			static_ffmpeg_read_packet,    // 绑定回调函数
			nullptr,                      // 无需写回调
			static_ffmpeg_seek                        // Seek 回调（可选）
		);

		// 关联到 FormatContext
		in_fmt_ctx->pb = avio_ctx;
		//Core::GetVFS()->AbsolutePath(input_filename).c_str()

		AVDictionary* in_opts = nullptr;
		av_dict_set(&in_opts, "err_detect", "ignore_err", 0);
		if ((ret = avformat_open_input(&in_fmt_ctx, nullptr, nullptr, &in_opts)) < 0) {
			std::cerr << "Could not open input file\n";
			av_dict_free(&in_opts);
			return false;
		}
		av_dict_free(&in_opts);

		if ((ret = avformat_find_stream_info(in_fmt_ctx, nullptr)) < 0) {
			std::cerr << "Failed to retrieve input stream info\n";
			avformat_close_input(&in_fmt_ctx);
			return false;
		}

		// ------------------------
		// 2. 创建输出上下文到内存 buffer
		// ------------------------
		avformat_alloc_output_context2(&out_fmt_ctx, nullptr, "mp4", nullptr);
		if (!out_fmt_ctx) {
			std::cerr << "Could not create output context\n";
			avformat_close_input(&in_fmt_ctx);
			return false;
		}

		for (unsigned int i = 0; i < in_fmt_ctx->nb_streams; i++) {
			AVStream* in_stream = in_fmt_ctx->streams[i];
			AVStream* out_stream = avformat_new_stream(out_fmt_ctx, nullptr);
			avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
			out_stream->codecpar->codec_tag = 0;
		}

		// ------------------------
		// 3. 设置写到内存 buffer
		// ------------------------
		uint8_t* out_buffer = nullptr;
		avio_open_dyn_buf(&out_fmt_ctx->pb);

		AVDictionary* mux_opts = nullptr;
		//av_dict_set(&mux_opts, "movflags", "faststart", 0);
		av_dict_set(&mux_opts, "movflags", "frag_keyframe+empty_moov", 0);

		avformat_write_header(out_fmt_ctx, &mux_opts);
		av_dict_free(&mux_opts);

		// 写数据包
		AVPacket pkt;
		while (av_read_frame(in_fmt_ctx, &pkt) >= 0) {
			AVStream* in_stream = in_fmt_ctx->streams[pkt.stream_index];
			AVStream* out_stream = out_fmt_ctx->streams[pkt.stream_index];
			pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
				(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
				(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
			pkt.pos = -1;

			av_interleaved_write_frame(out_fmt_ctx, &pkt);
			av_packet_unref(&pkt);
		}

		av_write_trailer(out_fmt_ctx);

		// ------------------------
		// 4. 从内存 buffer 中取出数据
		// ------------------------
		int out_buffer_size = avio_close_dyn_buf(out_fmt_ctx->pb, &out_buffer);

		std::cout << "Remuxed data size: " << out_buffer_size << " bytes" << std::endl;

		// ------------------------
		// 5. 用 avio_alloc_context 从该 buffer 创建新的 AVFormatContext
		// ------------------------
		memory_buffer = (uint8_t*)av_malloc(out_buffer_size);
		memcpy(memory_buffer, out_buffer, out_buffer_size);
		av_free(out_buffer);

		buffer_data = new BufferData{
			.ptr = memory_buffer,
			.size = (size_t)out_buffer_size
		};

		avio_internal_buf = (uint8_t*)av_malloc(4096);
		mem_avio_ctx = avio_alloc_context(
			avio_internal_buf, 4096, 0,
			buffer_data,
			&InMemoryRemuxer::ReadPacket, nullptr, nullptr);

		// 创建并打开新的格式上下文
		fixed_fmt_ctx = avformat_alloc_context();
		fixed_fmt_ctx->pb = mem_avio_ctx;
		fixed_fmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

		if ((ret = avformat_open_input(&fixed_fmt_ctx, nullptr, nullptr, nullptr)) < 0) {
			std::cerr << "Could not open input from memory\n";
			avformat_close_input(&in_fmt_ctx);
			avformat_free_context(out_fmt_ctx);
			return false;
		}
		if ((ret = avformat_find_stream_info(fixed_fmt_ctx, nullptr)) < 0) {
			std::cerr << "Failed to read stream info from memory\n";
			avformat_close_input(&in_fmt_ctx);
			avformat_free_context(out_fmt_ctx);
			return false;
		}

		// ------------------------
		// 清理原始输入 & 输出上下文
		// ------------------------
		avformat_close_input(&in_fmt_ctx);
		avformat_free_context(out_fmt_ctx);

		return true;
	}

}
