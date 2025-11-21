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

#include "Resource/FFmpeg/FContext.h"
#include "Core/VFS.h"
#include <HCore/Include/VFS/IFile.h>

namespace VisionGal {

	Ref<FfmpegContext> FfmpegContext::Create(const std::string& filePath)
	{
		auto context = CreateRef<FfmpegContext>();

		if (bool result = context->Open(filePath))
			return context;

		return nullptr;
	}

	int FfmpegContext::FindAudioStreamIndex() const
	{
		return m_AVFormatContext->FindAudioStreamIndex();
	}

	int FfmpegContext::FindVideoStreamIndex() const
	{
		return m_AVFormatContext->FindVideoStreamIndex();
	}

	FfmpegAVFormatContext* FfmpegContext::GetFormatContext() const
	{
		return m_AVFormatContext.get();
	}

	bool FfmpegContext::Open(const std::string& filePath)
	{
		// 初始化 FFmpeg
		avformat_network_init();

		auto& vfs = VFS::GetInstance();

		// 尝试打开文件
		auto file = vfs->OpenFile(vfspp::FileInfo(filePath), vfspp::IFile::FileMode::Read);
		if (!file)
			return false;

		// 文件是否打开
		if (file->IsOpened() == false)
			return false;

		m_AVFormatContext = FfmpegAVFormatContext::Create();
		m_VFSIoContext = { file };

		// 创建 IO 上下文
		m_AVIOContext = CreateRef<FfmpegAVIOContext>(
			&m_VFSIoContext, 
			VFSFFmpegIOContext::read_packet,
			VFSFFmpegIOContext::seek
		);

		// IO 上下文关联到 FormatContext
		m_AVFormatContext->SetIOContext(*m_AVIOContext);

		// 增加探测深度以处理复杂或损坏的文件
		{
			FfmpegAVDictionary options;
			options.Set("analyzeduration", "200000000", 0); // 最大int值
			options.Set("probesize", "200000000", 0); // 最大int值
			options.Set("movflags", "faststart", 0); // 修复MOOV原子位置
			options.Set("err_detect", "ignore_err", 0);//这样在读取数据时会少报错（但如果数据真损坏还是会丢帧）。

			// 打开输入文件
			if (m_AVFormatContext->OpenInput(options) != 0)
			{
				std::cerr << "Failed to open input file" << std::endl;
				return false;
			}
		}

		// 读取文件信息,获取流信息
		if (m_AVFormatContext->FindStreamInfo() < 0) {
			std::cerr << "Failed to find stream information" << std::endl;
			return false;
		}

		return true;
	}
}
