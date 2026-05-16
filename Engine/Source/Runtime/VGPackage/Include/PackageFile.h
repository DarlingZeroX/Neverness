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
#include "PakReader.h"
#include <NNFileSystem/Include/VFS/IFile.h>
#include <filesystem>
#include <sstream>

namespace vfspp
{
	using VGPackageFilePtr = std::shared_ptr<class VGPackageFile>;
	using VGPackageFileWeakPtr = std::weak_ptr<class VGPackageFile>;

	class PackageVectorStreamBuf : public std::streambuf {
	public:
		PackageVectorStreamBuf(const std::vector<uint8_t>& data) {
			char* begin = const_cast<char*>(reinterpret_cast<const char*>(data.data()));
			setg(begin, begin, begin + data.size());
		}
	protected:
		// 允许 seekg() 工作
		std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way,
			std::ios_base::openmode /*which*/) override {
			char* newPos = nullptr;
			if (way == std::ios_base::beg)
				newPos = eback() + off;
			else if (way == std::ios_base::cur)
				newPos = gptr() + off;
			else if (way == std::ios_base::end)
				newPos = egptr() + off;
			else
				return -1;

			if (newPos < eback() || newPos > egptr())
				return -1;

			setg(eback(), newPos, egptr());
			return gptr() - eback();
		}

		std::streampos seekpos(std::streampos pos,
			std::ios_base::openmode /*which*/) override {
			return seekoff(pos, std::ios_base::beg, std::ios_base::in);
		}
	};

	class PackageVectorIStream : public std::istream {
	public:
		PackageVectorIStream(const std::vector<uint8_t>& data)
			: std::istream(nullptr), buf(data) {
			rdbuf(&buf);
		}

	private:
		PackageVectorStreamBuf buf;
	};

	class VGPackageFile final : public IFile
	{
	public:
		VGPackageFile(const FileInfo& fileInfo, const VisionGal::PakEntry& entry, std::filesystem::path pakAbsPath)
			: m_FileInfo(fileInfo)
			, m_Entry(entry)
			, m_PakAbsPath(pakAbsPath)
			, m_IsOpened(false)
			, m_SeekPos(0)
		{
		}

		VGPackageFile(const VGPackageFile& other)
			: m_FileInfo(other.m_FileInfo)
			, m_Entry(other.m_Entry)
			, m_PakAbsPath(other.m_PakAbsPath)
			, m_IsOpened(other.m_IsOpened)
			, m_SeekPos(other.m_SeekPos)
		{

		}

		~VGPackageFile() override
		{
			Close();
		}

		/*
		 * Get file information
		 */
		virtual const FileInfo& GetFileInfo() const override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				return GetFileInfoST();
			}
			else {
				return GetFileInfoST();
			}
		}

		/*
		 * Returns file size
		 */
		virtual uint64_t Size() override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				return SizeST();
			}
			else {
				return SizeST();
			}
		}

		/*
		 * Check is readonly filesystem
		 */
		virtual bool IsReadOnly() const override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				return IsReadOnlyST();
			}
			else {
				return IsReadOnlyST();
			}
		}

		/*
		 * Open file for reading/writting
		 */
		virtual void Open(FileMode mode) override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				OpenST(mode);
			}
			else {
				OpenST(mode);
			}
		}

		/*
		 * Close file
		 */
		virtual void Close() override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				CloseST();
			}
			else {
				CloseST();
			}
		}

		/*
		 * Check is file ready for reading/writing
		 */
		virtual bool IsOpened() const override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				return IsOpenedST();
			}
			else {
				return IsOpenedST();
			}
		}

		/*
		 * Seek on a file
		 */
		virtual uint64_t Seek(uint64_t offset, Origin origin) override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				return SeekST(offset, origin);
			}
			else {
				return SeekST(offset, origin);
			}
		}
		/*
		 * Returns offset in file
		 */
		virtual uint64_t Tell() override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				return TellST();
			}
			else {
				return TellST();
			}
		}

		/*
		 * Read data from file to buffer
		 */
		virtual uint64_t Read(uint8_t* buffer, uint64_t size) override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				return ReadST(buffer, size);
			}
			else {
				return ReadST(buffer, size);
			}
		}
		/*
		 * Write buffer data to file
		 */
		virtual uint64_t Write(const uint8_t* buffer, uint64_t size) override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				return WriteST(buffer, size);
			}
			else {
				return WriteST(buffer, size);
			}
		}

		/*
		 * Read data from file to vector
		 */
		virtual uint64_t Read(std::vector<uint8_t>& buffer, uint64_t size) override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				return ReadST(buffer, size);
			}
			else {
				return ReadST(buffer, size);
			}
		}

		/*
		 * Write data from vector to file
		 */
		virtual uint64_t Write(const std::vector<uint8_t>& buffer) override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				return WriteST(buffer);
			}
			else {
				return WriteST(buffer);
			}
		}

		/*
		 * Read data from file to stream
		 */
		virtual uint64_t Read(std::ostream& stream, uint64_t size, uint64_t bufferSize = 1024) override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				return ReadST(stream, size, bufferSize);
			}
			else {
				return ReadST(stream, size, bufferSize);
			}
		}

		/*
		 * Write data from stream to file
		 */
		virtual uint64_t Write(std::istream& stream, uint64_t size, uint64_t bufferSize = 1024) override
		{
			if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
				std::lock_guard<std::mutex> lock(m_Mutex);
				return WriteST(stream, size, bufferSize);
			}
			else {
				return WriteST(stream, size, bufferSize);
			}
		}

	private:
		inline const FileInfo& GetFileInfoST() const
		{
			return m_FileInfo;
		}

		inline uint64_t SizeST()
		{
			//if (IsOpenedST()) {
			//	return m_Entry.originalSize;
			//}

			if (IsOpenedST()) {
				uint64_t curPos = TellST();
				uint64_t size = SeekST(0, Origin::End);
				SeekST(curPos, Origin::Begin);

				return size;
			}

			return 0;
		}

		inline bool IsReadOnlyST() const
		{
			return true;
		}

		inline void OpenST(FileMode mode)
		{
			bool requestWrite = ((mode & IFile::FileMode::Write) == IFile::FileMode::Write);
			requestWrite |= ((mode & IFile::FileMode::Append) == IFile::FileMode::Append);
			requestWrite |= ((mode & IFile::FileMode::Truncate) == IFile::FileMode::Truncate);

			if (IsReadOnlyST() && requestWrite) {
				return;
			}

			if (IsOpenedST()) {
				SeekST(0, IFile::Origin::Begin);
				return;
			}

			// 这里是原本的 zip 文件读取逻辑
			//std::shared_ptr<mz_zip_archive> zipArchive = m_ZipArchive.lock();
			//if (!zipArchive) {
			//	return;
			//}
			//
			//m_SeekPos = 0;
			//
			//m_Data.resize(m_Size);
			//m_IsOpened = mz_zip_reader_extract_to_mem_no_alloc(zipArchive.get(), m_Offset, m_Data.data(), static_cast<size_t>(m_Size), 0, 0, 0);
			
			// 这里是 VGPackage 文件读取逻辑
			if( !std::filesystem::exists(m_PakAbsPath)) {
				return;
			}
			m_SeekPos = 0;

            if (VisionGal::PakFileReader::ReadEntryFile(m_PakAbsPath, m_Entry, m_Data)) {
				m_IsOpened = true;
				m_Stream = std::make_unique<PackageVectorIStream>(m_Data);
            } else {
				m_IsOpened = false;
				m_Data.clear();
            }
		}

		inline void CloseST()
		{
			m_IsOpened = false;
			m_SeekPos = 0;

			m_Data.clear();
		}

		inline bool IsOpenedST() const
		{
			return m_IsOpened;
		}

		inline uint64_t SeekST(uint64_t offset, Origin origin)
		{
			//if (!IsOpenedST()) {
			//	return 0;
			//}
			//
			//if (origin == IFile::Origin::Begin) {
			//	m_SeekPos = offset;
			//}
			//else if (origin == IFile::Origin::End) {
			//	//m_SeekPos = SizeST() - offset;
			//	m_SeekPos = (offset <= SizeST()) ? SizeST() - offset : 0;
			//}
			//else if (origin == IFile::Origin::Set) {
			//	m_SeekPos += offset;
			//}
			//m_SeekPos = std::min(m_SeekPos, SizeST());
			//
			//return TellST();

			if (!IsOpenedST()) {
				return 0;
			}

			std::ios_base::seekdir dir = std::ios_base::beg;
			if (origin == IFile::Origin::End) {
				dir = std::ios_base::end;
			}
			else if (origin == IFile::Origin::Set) {
				dir = std::ios_base::cur;
			}

			// Seek both read and write pointers to keep them synchronized
			m_Stream->seekg(offset, dir);

			if (m_Stream->fail()) {
				m_Stream->clear();
				return 0;
			}

			return TellST();
		}

		inline uint64_t TellST()
		{
			//return m_SeekPos;

			if (!IsOpenedST()) {
				return 0;
			}

			auto pos = m_Stream->tellg();
			return (pos != std::streampos(-1)) ? static_cast<uint64_t>(pos) : 0;
			return 0;
		}

		inline uint64_t ReadST(uint8_t* buffer, uint64_t size)
		{
			//if (!IsOpenedST()) {
			//	return 0;
			//}
			//
			//uint64_t leftSize = SizeST() - TellST();
			//uint64_t maxSize = std::min(size, leftSize);
			//if (maxSize > 0) {
			//	memcpy(buffer, m_Data.data() + m_SeekPos, static_cast<size_t>(maxSize));
			//	m_SeekPos += maxSize;
			//	return maxSize;
			//}
			//
			//return 0;

			if (!IsOpenedST()) {
				return 0;
			}

			uint64_t leftSize = SizeST() - TellST();
			uint64_t maxSize = std::min(size, leftSize);
			if (maxSize > 0) { 
				m_Stream->read(reinterpret_cast<char*>(buffer), maxSize);
				if (m_Stream->good() || m_Stream->eof()) {
					return static_cast<uint64_t>(m_Stream->gcount());
				}
				// Clear error flags for failed read
				m_Stream->clear();
				return 0;
			}

			return 0;
		}

		inline uint64_t WriteST(const uint8_t* buffer, uint64_t size)
		{
			return 0;
		}

		inline uint64_t ReadST(std::vector<uint8_t>& buffer, uint64_t size)
		{
			buffer.resize(size);
			return ReadST(buffer.data(), size);
		}

		inline uint64_t WriteST(const std::vector<uint8_t>& buffer)
		{
			return WriteST(buffer.data(), buffer.size());
		}

		inline uint64_t ReadST(std::ostream& stream, uint64_t size, uint64_t bufferSize = 1024)
		{
			// read chunk of data from file and write it to stream untill all data is read
			uint64_t totalSize = size;
			std::vector<uint8_t> buffer(bufferSize);
			while (size > 0) {
				uint64_t bytesRead = ReadST(buffer.data(), std::min(size, static_cast<uint64_t>(buffer.size())));
				if (bytesRead == 0) {
					break;
				}

				if (size < bytesRead) {
					bytesRead = size;
				}

				stream.write(reinterpret_cast<char*>(buffer.data()), bytesRead);

				size -= bytesRead;
			}

			return totalSize - size;
		}

		inline uint64_t WriteST(std::istream& stream, uint64_t size, uint64_t bufferSize = 1024)
		{
			// write chunk of data from stream to file untill all data is written
			uint64_t totalSize = size;
			std::vector<uint8_t> buffer(bufferSize);
			while (size > 0) {
				stream.read(reinterpret_cast<char*>(buffer.data()), std::min(size, static_cast<uint64_t>(buffer.size())));
				uint64_t bytesRead = stream.gcount();
				if (bytesRead == 0) {
					break;
				}

				if (size < bytesRead) {
					bytesRead = size;
				}

				WriteST(buffer.data(), bytesRead);

				size -= bytesRead;
			}

			return totalSize - size;
		}

	private:
		FileInfo m_FileInfo;
		//uint32_t m_Offset;
		//uint64_t m_Size;
		VisionGal::PakEntry m_Entry;
		std::filesystem::path m_PakAbsPath;
		std::vector<uint8_t> m_Data;
		//std::istringstream m_Stream;
		std::unique_ptr<PackageVectorIStream> m_Stream;
		bool m_IsOpened;
		uint64_t m_SeekPos;
		mutable std::mutex m_Mutex;
	};

} // namespace vfspp

