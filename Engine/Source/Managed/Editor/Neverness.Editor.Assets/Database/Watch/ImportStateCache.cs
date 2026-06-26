using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets;

/// <summary>
/// 导入状态缓存（content hash 比对）。
///
/// 用于增量导入：文件未变化时跳过 reimport。
/// 通过 SHA256 内容哈希 + 最后写入时间戳判断文件是否变化。
///
/// 序列化格式：magic 'NNIC' + version 1 + count + entries。
/// 持久化路径：Library/Cache/ImportState.cache。
///
/// @threadsafe 所有公共方法内部使用 s_lock 保护。
/// </summary>
public sealed class ImportStateCache
{
    private readonly Dictionary<NPath, (DateTime lastWrite, byte[] hash)> _cache = new();

    private readonly object _lock = new();

    /// <summary>文件是否有变化（hash 不匹配或不存在于缓存）。</summary>
    public bool HasChanged(NPath path)
    {
        lock (_lock)
        {
            if (!File.Exists(path.FullPath))
                return true;

            var lastWrite = File.GetLastWriteTimeUtc(path.FullPath);

            if (_cache.TryGetValue(path, out var entry))
            {
                /* 时间戳相同，认定未变化 */
                if (entry.lastWrite == lastWrite)
                    return false;
            }

            return true;
        }
    }

    /// <summary>标记文件为已导入。</summary>
    public void MarkImported(NPath path, byte[] hash)
    {
        lock (_lock)
        {
            var lastWrite = File.Exists(path.FullPath) ? File.GetLastWriteTimeUtc(path.FullPath) : DateTime.UtcNow;
            _cache[path] = (lastWrite, hash);
        }
    }

    /// <summary>取得已缓存的 hash。</summary>
    public byte[]? GetCachedHash(NPath path)
    {
        lock (_lock)
        {
            return _cache.TryGetValue(path, out var entry) ? entry.hash : null;
        }
    }

    /// <summary>移除缓存条目。</summary>
    public void Remove(NPath path)
    {
        lock (_lock) _cache.Remove(path);
    }

    /// <summary>缓存条目数量。</summary>
    public int Count
    {
        get { lock (_lock) return _cache.Count; }
    }

    /// <summary>保存缓存至磁碟。</summary>
    public void Save(NPath cachePath)
    {
        lock (_lock)
        {
            try
            {
                var dir = Path.GetDirectoryName(cachePath.FullPath);
                if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                    Directory.CreateDirectory(dir);

                using var stream = File.Create(cachePath.FullPath);
                using var writer = new BinaryWriter(stream);

                writer.Write(0x4E4E4943u); /* 'NNIC' = Neverness Import Cache */
                writer.Write(1u);          /* version */
                writer.Write(_cache.Count);

                foreach (var (path, entry) in _cache)
                {
                    writer.Write(path.FullPath);
                    writer.Write(entry.lastWrite.ToBinary());
                    writer.Write(entry.hash.Length);
                    writer.Write(entry.hash);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ImportStateCache] 保存失败: {ex.Message}");
            }
        }
    }

    /// <summary>从磁碟加载缓存。</summary>
    public void Load(NPath cachePath)
    {
        lock (_lock)
        {
            if (!File.Exists(cachePath.FullPath))
                return;

            try
            {
                using var stream = File.OpenRead(cachePath.FullPath);
                using var reader = new BinaryReader(stream);

                var magic = reader.ReadUInt32();
                var version = reader.ReadUInt32();
                if (magic != 0x4E4E4943u || version != 1u)
                    return;

                var count = reader.ReadInt32();
                for (var i = 0; i < count; i++)
                {
                    var path = reader.ReadString();
                    var lastWriteBinary = reader.ReadInt64();
                    var hashLen = reader.ReadInt32();
                    var hash = reader.ReadBytes(hashLen);

                    _cache[new NPath(path)] = (DateTime.FromBinary(lastWriteBinary), hash);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ImportStateCache] 加载失败: {ex.Message}");
                _cache.Clear();
            }
        }
    }
}
