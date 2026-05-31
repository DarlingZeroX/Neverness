using System.Collections.Generic;
using System.Threading.Tasks;

namespace NevernessLauncher.Contracts
{
    /// <summary>
    /// 文件系统接口
    /// </summary>
    public interface IFileSystem
    {
        /// <summary>检查文件是否存在</summary>
        bool FileExists(string path);

        /// <summary>检查目录是否存在</summary>
        bool DirectoryExists(string path);

        /// <summary>创建目录</summary>
        void CreateDirectory(string path);

        /// <summary>读取文件内容</summary>
        Task<string> ReadAllTextAsync(string path);

        /// <summary>写入文件内容</summary>
        Task WriteAllTextAsync(string path, string content);

        /// <summary>读取文件字节</summary>
        Task<byte[]> ReadAllBytesAsync(string path);

        /// <summary>写入文件字节</summary>
        Task WriteAllBytesAsync(string path, byte[] bytes);

        /// <summary>删除文件</summary>
        void DeleteFile(string path);

        /// <summary>获取目录下的文件列表</summary>
        IEnumerable<string> GetFiles(string directory, string searchPattern = "*");

        /// <summary>获取目录下的子目录列表</summary>
        IEnumerable<string> GetDirectories(string directory);

        /// <summary>递归获取目录下的文件列表</summary>
        IEnumerable<string> GetFilesRecursive(string directory, string searchPattern = "*");
    }
}
