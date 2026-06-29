using System.Text;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets;

/// <summary>
/// EditorAssetDatabase — 快取序列化（Library/Cache/AssetDatabase.cache）。
/// </summary>
public static partial class EditorAssetDatabase
{
    /* ======================== 快取序列化 ======================== */

    /// <summary>若髒則儲存快取至磁碟（Tick 批量保存用）。</summary>
    public static void SaveIfDirty()
    {
        lock (s_lock)
        {
            if (!s_cacheDirty) return;
            s_cacheDirty = false;
            SaveCacheInternal();
        }
    }

    /// <summary>儲存快取至磁碟。</summary>
    public static void SaveCache()
    {
        lock (s_lock)
        {
            if (!s_initialized) return;
            SaveCacheInternal();
        }
    }

    /// <summary>載入快取。</summary>
    public static void LoadCache()
    {
        lock (s_lock)
        {
            if (!s_initialized) return;
            TryLoadCache();
        }
    }

    /// <summary>使快取失效。</summary>
    public static void InvalidateCache()
    {
        lock (s_lock)
        {
            if (!s_initialized) return;
            var cachePath = GetCachePath();
            if (File.Exists(cachePath.FullPath))
                File.Delete(cachePath.FullPath);
        }
    }

    /* ======================== 內部實作 ======================== */

    private static NPath GetCachePath()
    {
        return s_libraryRoot.Combine("Cache/AssetDatabase.cache");
    }

    private static void SaveCacheInternal()
    {
        try
        {
            var cachePath = GetCachePath();
            var dir = Path.GetDirectoryName(cachePath.FullPath);
            if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                Directory.CreateDirectory(dir);

            using var stream = File.Create(cachePath.FullPath);
            using var writer = new BinaryWriter(stream, Encoding.UTF8, leaveOpen: true);

            /* Header: magic + version */
            writer.Write(0x4E4E4442u); /* 'NNDB' */
            writer.Write(1u);          /* version */

            /* Path → Guid 表 */
            writer.Write(s_pathToGuid.Count);
            foreach (var (path, g) in s_pathToGuid)
            {
                writer.Write(path.FullPath);
                writer.Write(g.High);
                writer.Write(g.Low);
            }

            /* Guid → TypeId 表 */
            writer.Write(s_guidToTypeId.Count);
            foreach (var (guidKey, typeId) in s_guidToTypeId)
            {
                writer.Write(guidKey);
                writer.Write(typeId);
            }

            /* Label 索引 */
            writer.Write(s_labelToGuids.Count);
            foreach (var (label, guids) in s_labelToGuids)
            {
                writer.Write(label);
                writer.Write(guids.Count);
                foreach (var g in guids)
                    writer.Write(g);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[EditorAssetDatabase] 儲存快取失敗: {ex.Message}");
        }
    }

    private static void TryLoadCache()
    {
        var cachePath = GetCachePath();
        if (!File.Exists(cachePath.FullPath))
            return;

        try
        {
            using var stream = File.OpenRead(cachePath.FullPath);
            using var reader = new BinaryReader(stream, Encoding.UTF8, leaveOpen: true);

            var magic = reader.ReadUInt32();
            var version = reader.ReadUInt32();
            if (magic != 0x4E4E4442u || version != 1u)
                return; /* 格式不匹配，忽略 */

            /* Path → Guid 表 */
            var count = reader.ReadInt32();
            for (var i = 0; i < count; i++)
            {
                var pathStr = reader.ReadString();
                var high = reader.ReadUInt64();
                var low = reader.ReadUInt64();
                var guid = new GUID(high, low);
                var vp = new NVirtualPath(pathStr);
                s_pathToGuid[vp] = guid;
                s_guidToPath[guid.ToHexString()] = vp;
            }

            /* Guid → TypeId 表 */
            count = reader.ReadInt32();
            for (var i = 0; i < count; i++)
            {
                var guidKey = reader.ReadString();
                var typeId = reader.ReadUInt64();
                s_guidToTypeId[guidKey] = typeId;
            }

            /* Label 索引 */
            count = reader.ReadInt32();
            for (var i = 0; i < count; i++)
            {
                var label = reader.ReadString();
                var guidCount = reader.ReadInt32();
                var guids = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                for (var j = 0; j < guidCount; j++)
                {
                    var g = reader.ReadString();
                    guids.Add(g);

                    /* 同時更新反向索引 */
                    if (!s_guidToLabels.TryGetValue(g, out var labels))
                    {
                        labels = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                        s_guidToLabels[g] = labels;
                    }
                    labels.Add(label);
                }
                s_labelToGuids[label] = guids;
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[EditorAssetDatabase] 載入快取失敗: {ex.Message}");
            /* 快取損壞，清除 */
            s_pathToGuid.Clear();
            s_guidToPath.Clear();
            s_guidToTypeId.Clear();
            s_labelToGuids.Clear();
            s_guidToLabels.Clear();
        }
    }
}
