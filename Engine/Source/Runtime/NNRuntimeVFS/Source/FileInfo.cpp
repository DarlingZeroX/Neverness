#include "VFS/FileInfo.h"

namespace NN::Runtime::VFS
{

FileInfo::FileInfo(const std::string& filePath)
{
    Configure("", filePath, false);
}

FileInfo::FileInfo(const std::string& basePath, const std::string& fileName, bool isDir)
{
    Configure(basePath, fileName, isDir);
}

FileInfo::FileInfo(const fs::path& path, bool isDir)
    : m_Path(path)
    , m_IsDir(isDir)
{
}

std::string FileInfo::Name() const
{
    return m_Path.filename().string();
}

std::string FileInfo::BaseName() const
{
    return m_Path.stem().string();
}

std::string FileInfo::Extension() const
{
    return m_Path.extension().string();
}

std::string FileInfo::AbsolutePath() const
{
    return m_Path.string();
}

bool FileInfo::IsDir() const
{
    return m_IsDir;
}

const fs::path& FileInfo::Path() const
{
    return m_Path;
}

bool FileInfo::IsValid() const
{
    return !m_Path.string().empty();
}

void FileInfo::Configure(const std::string& basePath, const std::string& fileName, bool isDir)
{
    m_Path = (fs::path(basePath) / fs::path(fileName)).generic_string();
    m_IsDir = isDir;
}

bool operator==(const FileInfo& fi1, const FileInfo& fi2)
{
    return fi1.AbsolutePath() == fi2.AbsolutePath();
}

bool operator<(const FileInfo& fi1, const FileInfo& fi2)
{
    return fi1.AbsolutePath() < fi2.AbsolutePath();
}

} // namespace NN::Runtime::VFS
