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
#include "PackageFile.h"
#include "PakReader.h"
#include <HCore/Include/VFS/IFileSystem.h>
//#include <HCore/Include/VFS/Global.h>
#include <HCore/Include/VFS/StringUtils.hpp>
//#include "ZipFile.hpp"
//#include "zip_file.hpp"

namespace fs = std::filesystem;

namespace vfspp
{

using VGPackageFileSystemPtr = std::shared_ptr<class VGPackageFileSystem>;
using VGPackageFileSystemWeakPtr = std::weak_ptr<class VGPackageFileSystem>;


class VGPackageFileSystem final : public IFileSystem
{
public:
	VGPackageFileSystem(const std::string& zipPath)
        : m_ZipPath(zipPath)
        //, m_ZipArchive(nullptr)
        , m_IsInitialized(false)
    {
    }

    ~VGPackageFileSystem() override
    {
        Shutdown();
    }
    
    /*
     * Initialize filesystem, call this method as soon as possible
     */
    virtual void Initialize() override
    {
        if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            InitializeST();
        } else {
            InitializeST();
        }
    }

    /*
     * Shutdown filesystem
     */
    virtual void Shutdown() override
    {
        if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            ShutdownST();
        } else {
            ShutdownST();
        }
    }
    
    /*
     * Check if filesystem is initialized
     */
    virtual bool IsInitialized() const override
    {
        return m_IsInitialized;
    }
    
    /*
     * Get base path
     */
    virtual const std::string& BasePath() const override
    {
        if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return BasePathST();
        } else {
            return BasePathST();
        }
    }
    
    /*
     * Retrieve file list according filter
     */
    virtual const TFileList& FileList() const override
    {
        if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return FileListST();
        } else {
            return FileListST();
        }
    }
    
    /*
     * Check is readonly filesystem
     */
    virtual bool IsReadOnly() const override
    {
        return true;
    }
    
    /*
     * Open existing file for reading, if not exists returns null for readonly filesystem. 
     * If file not exists and filesystem is writable then create new file
     */
    virtual IFilePtr OpenFile(const FileInfo& filePath, IFile::FileMode mode) override
    {
        if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return OpenFileST(filePath, mode);
        } else {
            return OpenFileST(filePath, mode);
        }
    }
    
    /*
     * Create file on writeable filesystem. Returns true if file created successfully
     */
    virtual bool CreateFile(const FileInfo& filePath) override
    {
        return false;
    }
    
    /*
     * Remove existing file on writable filesystem
     */
    virtual bool RemoveFile(const FileInfo& filePath) override
    {
        return false;
    }
    
    /*
     * Copy existing file on writable filesystem
     */
    virtual bool CopyFile(const FileInfo& src, const FileInfo& dest) override
    {
        return false;
    }
    
    /*
     * Rename existing file on writable filesystem
     */
    virtual bool RenameFile(const FileInfo& srcPath, const FileInfo& dstPath) override
    {
        return false;
    }
    /*
    * Close file 
    */
    virtual void CloseFile(IFilePtr file) override
    {
        //NO-OP
    }
    /*
     * Check if file exists on filesystem
     */
    virtual bool IsFileExists(const FileInfo& filePath) const override
    {
        if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return IsFileExistsST(filePath);
        } else {
            return IsFileExistsST(filePath);
        }
    }

    /*
     * Check is file
     */
    virtual bool IsFile(const FileInfo& filePath) const override
    {
        if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return IFileSystem::IsFile(filePath, m_FileList);
        } else {
            return IFileSystem::IsFile(filePath, m_FileList);
        }
    }
    
    /*
     * Check is dir
     */
    virtual bool IsDir(const FileInfo& dirPath) const override
    {
        if constexpr (VFSPP_MT_SUPPORT_ENABLED) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return IFileSystem::IsDir(dirPath, m_FileList);
        } else {
            return IFileSystem::IsDir(dirPath, m_FileList);
        }
    }

private:
    inline void InitializeST()
    {
        if (m_IsInitialized) {
            return;
        }

        if (!fs::is_regular_file(m_ZipPath)) {
            return;
        }

        //m_ZipArchive = std::make_shared<mz_zip_archive>();
		//
        //mz_bool status = mz_zip_reader_init_file(m_ZipArchive.get(), m_ZipPath.c_str(), 0);
        //if (!status) {
        //    return;
        //}
		//
        //BuildFilelist(m_ZipArchive, m_FileList);
        //m_IsInitialized = true;

		std::vector<VisionGal::PakEntry> entries;
		bool result = VisionGal::PakFileReader::ReadEntries(m_ZipPath, entries);

		if(result == false){
			return;
		}

		BuildFilelist(entries, m_FileList);
		m_IsInitialized = true;
    }

    inline void ShutdownST()
    {
        m_ZipPath = "";
        // close all files
        for (auto& file : m_FileList) {
            file.second->Close();
        }
        m_FileList.clear();

        // close zip archive
        //if (m_ZipArchive) {
        //    mz_zip_reader_end(m_ZipArchive.get());
        //    m_ZipArchive = nullptr;
        //}

        m_IsInitialized = false;
    }
    
    inline bool IsInitializedST() const
    {
        return m_IsInitialized;
    }

    inline const std::string& BasePathST() const
    {
        //static std::string rootPath = "/";
        //return rootPath;
		return m_ZipPath;
    }

    inline const TFileList& FileListST() const
    {
        return m_FileList;
    }
    
    inline IFilePtr OpenFileST(const FileInfo& filePath, IFile::FileMode mode)
    {
        // check if filesystem is readonly and mode is write then return null
        bool requestWrite = ((mode & IFile::FileMode::Write) == IFile::FileMode::Write);
        requestWrite |= ((mode & IFile::FileMode::Append) == IFile::FileMode::Append);
        requestWrite |= ((mode & IFile::FileMode::Truncate) == IFile::FileMode::Truncate);

        // Note 'IsReadOnly()' is safe to call on any thread
        if (IsReadOnly() && requestWrite) {
            return nullptr;
        }

        IFilePtr file = FindFile(filePath, m_FileList);
        if (file) {
			// 修复重复打开一个文件指针的错误
			file.reset(new VGPackageFile(*dynamic_cast<VGPackageFile*>(file.get())));

            file->Open(mode);
        }
        
        return file;
    }

    inline bool IsFileExistsST(const FileInfo& filePath) const
    {
        return FindFile(filePath, m_FileList) != nullptr;
    }

	void BuildFilelist(std::vector<VisionGal::PakEntry> entries, TFileList& outFileList)
	{
		//for (auto i = 0; i < entries.size(); i++) {
		for(auto& entry: entries)
		{
			//std::filesystem::path filePath(entry.path);
			FileInfo fileInfo(BasePathST(), entry.path, false);
			IFilePtr file(new VGPackageFile(fileInfo, entry, m_ZipPath));
			outFileList[fileInfo.AbsolutePath()] = file;
		}
	}

    //void BuildFilelist(std::shared_ptr<mz_zip_archive> zipArchive, TFileList& outFileList)
    //{
    //    for (mz_uint i = 0; i < mz_zip_reader_get_num_files(zipArchive.get()); i++) {
    //        mz_zip_archive_file_stat file_stat;
    //        if (!mz_zip_reader_file_stat(zipArchive.get(), i, &file_stat)) {
    //            // TODO: log error
    //            continue;
    //        }
    //        
    //        FileInfo fileInfo(BasePathST(), file_stat.m_filename, false);
    //        IFilePtr file(new ZipFile(fileInfo, file_stat.m_file_index, file_stat.m_uncomp_size, zipArchive));
    //        outFileList[fileInfo.AbsolutePath()] = file;
    //    }
    //}
    
private:
    std::string m_ZipPath;
    //std::shared_ptr<mz_zip_archive> m_ZipArchive;
    bool m_IsInitialized;
    TFileList m_FileList;
    mutable std::mutex m_Mutex;
};

} // namespace vfspp
