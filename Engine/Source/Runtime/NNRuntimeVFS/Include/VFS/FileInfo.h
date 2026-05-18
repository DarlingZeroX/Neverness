#ifndef FILEINFO_H
#define FILEINFO_H

#include "../../RuntimeVFSExport.h"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace NN::Runtime::VFS
{

class NN_RUNTIME_VFS_API FileInfo final
{
public:
    FileInfo(const std::string& filePath);
    FileInfo(const std::string& basePath, const std::string& fileName, bool isDir);
    FileInfo(const fs::path& path, bool isDir);

    FileInfo() = delete;
    ~FileInfo() = default;

    std::string Name() const;
    std::string BaseName() const;
    std::string Extension() const;
    std::string AbsolutePath() const;
    bool IsDir() const;
    const fs::path& Path() const;
    bool IsValid() const;

private:
    void Configure(const std::string& basePath, const std::string& fileName, bool isDir);

    fs::path m_Path;
    bool m_IsDir;
};

bool operator==(const FileInfo& fi1, const FileInfo& fi2);
bool operator<(const FileInfo& fi1, const FileInfo& fi2);

} // namespace NN::Runtime::VFS

#endif // FILEINFO_H
