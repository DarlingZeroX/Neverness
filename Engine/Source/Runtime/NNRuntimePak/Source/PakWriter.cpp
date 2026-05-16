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

#include "PakWriter.h"
#include <filesystem>
#include "PakCrc32.h"
#include "NNCore/Interface/HLog.h"

namespace NN::Runtime {

	void PakFileWriter::WriteHeader()
	{
		m_OutFile.write(reinterpret_cast<char*>(&m_Header), sizeof(m_Header)); // 先写占位头
	}

	void PakFileWriter::WriteFiles(const std::vector<std::filesystem::path>& files)
	{
		// 遍历所有文件，读取内容，压缩和加密后写入包文件
		for (const auto& path : files) {
			std::ifstream in(path, std::ios::binary | std::ios::ate);
			if (in.is_open() == false)
			{
				H_LOG_ERROR("[PakFileWriter]File path error: %s", path.c_str());
				return;
			}
			size_t size = in.tellg();
			in.seekg(0);

			std::vector<uint8_t> buffer(size);
			in.read(reinterpret_cast<char*>(buffer.data()), size);

			// 压缩
			//auto compressed = CompressZstd(buffer);

			// 加密
			//auto encrypted = EncryptAES(compressed, aesKey);

			auto& data = buffer; // 这里假设没有压缩和加密，直接使用原始数据

			std::filesystem::path absPath = path;
			std::filesystem::path relativePath;
			if (m_BasePath.empty())
			{
				relativePath = absPath; // 如果没有设置基础路径，使用绝对路径
			}
			else
			{
				relativePath = absPath.lexically_relative(m_BasePath);
			}

			PakEntry entry;
			entry.offset = m_OutFile.tellp();
			entry.compressedSize = static_cast<uint32_t>(data.size());
			entry.originalSize = static_cast<uint32_t>(buffer.size());
			entry.crc32 = PakCrc32::CalcCRC32(buffer);
			entry.path = relativePath.string();

			m_OutFile.write(reinterpret_cast<const char*>(data.data()), data.size());
			m_Entries.push_back(entry);
		}
	}

	void PakFileWriter::WriteEntries()
	{
		uint32_t count = static_cast<uint32_t>(m_Entries.size());
		m_OutFile.write(reinterpret_cast<char*>(&count), sizeof(count));
		for (auto& e : m_Entries) {
			e.Write(m_OutFile);
		}
	}

	void PakFileWriter::Finalize(uint64_t indexOffset)
	{
		// 更新 header
		m_OutFile.seekp(0);
		m_Header.indexOffset = indexOffset;
		m_OutFile.write(reinterpret_cast<char*>(&m_Header), sizeof(m_Header));
	}

	bool PakFileWriter::WritePakFile(const std::vector<std::filesystem::path>& files, const std::filesystem::path& outPakPath, const std::string& aesKeyHex)
	{
		m_Entries.clear();

		// 尝试打开输出文件
		m_OutFile.open(outPakPath, std::ios::binary);
		if (!m_OutFile) 
			return false;

		// 初始化包头
		WriteHeader();	// 先写占位头

		//std::vector<uint8_t> aesKey = HexToBytes(aesKeyHex);

		// 写文件内容
		WriteFiles(files);

		// 写索引表
		uint64_t indexOffset = m_OutFile.tellp();
		WriteEntries();

		// 更新 header
		Finalize(indexOffset);
		m_OutFile.close();

		return true;
	}

	bool PakFileWriter::WriteDirectoryToPakFile(const std::filesystem::path& InDirectoryPath, const std::filesystem::path& outPakPath,
		const std::string& aesKeyHex)
	{
		m_BasePath = InDirectoryPath;

		// 获取目录下所有文件
		std::vector<std::filesystem::path> files;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(InDirectoryPath))
		{
			if (entry.is_regular_file())
			{
				//auto relativePath = entry.path().lexically_relative(dirPath);
				files.push_back(entry.path());
			}
		}

		if (files.empty())
		{
			H_LOG_ERROR("[PakFileWriter]No files found in directory: %s", InDirectoryPath.c_str());
			return false;
		}

		// 调用 WritePakFile 写入文件
		return WritePakFile(files, outPakPath, aesKeyHex);
	}
}
