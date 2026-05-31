using NevernessLauncher.Contracts;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

namespace NevernessLauncher.Infrastructure
{
    /// <summary>
    /// 文件系统实现
    /// </summary>
    public class FileSystem : IFileSystem
    {
        /// <summary>检查文件是否存在</summary>
        public bool FileExists(string path) => File.Exists(path);

        /// <summary>检查目录是否存在</summary>
        public bool DirectoryExists(string path) => Directory.Exists(path);

        /// <summary>创建目录</summary>
        public void CreateDirectory(string path) => Directory.CreateDirectory(path);

        /// <summary>读取文件内容</summary>
        public async Task<string> ReadAllTextAsync(string path) => await File.ReadAllTextAsync(path);

        /// <summary>写入文件内容</summary>
        public async Task WriteAllTextAsync(string path, string content)
        {
            var directory = Path.GetDirectoryName(path);
            if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
            }
            await File.WriteAllTextAsync(path, content);
        }

        /// <summary>读取文件字节</summary>
        public async Task<byte[]> ReadAllBytesAsync(string path) => await File.ReadAllBytesAsync(path);

        /// <summary>写入文件字节</summary>
        public async Task WriteAllBytesAsync(string path, byte[] bytes)
        {
            var directory = Path.GetDirectoryName(path);
            if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
            }
            await File.WriteAllBytesAsync(path, bytes);
        }

        /// <summary>删除文件</summary>
        public void DeleteFile(string path)
        {
            if (File.Exists(path))
            {
                File.Delete(path);
            }
        }

        /// <summary>获取目录下的文件列表</summary>
        public IEnumerable<string> GetFiles(string directory, string searchPattern = "*")
        {
            if (!Directory.Exists(directory))
            {
                return Enumerable.Empty<string>();
            }
            return Directory.GetFiles(directory, searchPattern);
        }

        /// <summary>获取目录下的子目录列表</summary>
        public IEnumerable<string> GetDirectories(string directory)
        {
            if (!Directory.Exists(directory))
            {
                return Enumerable.Empty<string>();
            }
            return Directory.GetDirectories(directory);
        }

        /// <summary>递归获取目录下的文件列表</summary>
        public IEnumerable<string> GetFilesRecursive(string directory, string searchPattern = "*")
        {
            if (!Directory.Exists(directory))
            {
                return Enumerable.Empty<string>();
            }
            return Directory.GetFiles(directory, searchPattern, SearchOption.AllDirectories);
        }
    }
}
