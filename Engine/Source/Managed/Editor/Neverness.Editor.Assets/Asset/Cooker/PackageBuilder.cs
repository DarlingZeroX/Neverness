using System.Text;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// .nnpack 包构建器（纯 C# 实现）。
///
/// 不依赖 C++ NNAssetCooker，直接在 C# 侧生成 .nnpack 文件。
/// 适用于简单场景或 C++ 模块不可用时的降级方案。
/// </summary>
public static class PackageBuilder
{
    /// <summary>
    /// 将指定目录下的所有 .nnasset 打包为一个 .nnpack。
    /// </summary>
    public static bool BuildFromDirectory(string inputDir, string outputPath, string packageName = "default")
    {
        if (!Directory.Exists(inputDir))
            return false;

        var files = Directory.EnumerateFiles(inputDir, "*.nnasset", SearchOption.AllDirectories).ToList();
        if (files.Count == 0)
            return false;

        var assets = new List<(GUID guid, ulong typeId, byte[] data)>();

        foreach (var file in files)
        {
            var data = File.ReadAllBytes(file);
            if (data.Length < 64) continue;

            /* 从 Header 读取 GUID 和 typeId */
            var guidHigh = BitConverter.ToUInt64(data, 8);
            var guidLow = BitConverter.ToUInt64(data, 16);
            var typeId = BitConverter.ToUInt64(data, 24);
            var guid = new GUID(guidHigh, guidLow);

            assets.Add((guid, typeId, data));
        }

        return Build(assets, outputPath, packageName);
    }

    /// <summary>
    /// 从 AssetGroup 构建 .nnpack。
    /// </summary>
    public static bool BuildFromGroup(AssetGroup group, string libraryRoot, string outputPath)
    {
        var assets = new List<(GUID guid, ulong typeId, byte[] data)>();

        foreach (var guid in group.Assets)
        {
            var hex = guid.ToHexString();
            var prefix = hex[..2];
            var nnassetPath = Path.Combine(libraryRoot, "Imported", prefix, hex + ".nnasset");

            if (!File.Exists(nnassetPath))
                continue;

            var data = File.ReadAllBytes(nnassetPath);
            if (data.Length < 64) continue;

            var typeId = BitConverter.ToUInt64(data, 24);
            assets.Add((guid, typeId, data));
        }

        return Build(assets, outputPath, group.Name);
    }

    /// <summary>
    /// 核心打包逻辑（与 C++ NNPackBuilder::Build 对齐的纯 C# 实现）。
    /// </summary>
    public static bool Build(List<(GUID guid, ulong typeId, byte[] data)> assets,
                               string outputPath, string packageName)
    {
        if (assets.Count == 0 || string.IsNullOrEmpty(outputPath))
            return false;

        try
        {
            var dir = Path.GetDirectoryName(outputPath);
            if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                Directory.CreateDirectory(dir);

            /* 计算偏移 */
            const int headerSize = 64;
            const int entrySize = 56; /* NNPackAssetEntry: 6*uint64 + 2*uint32 = 56 bytes */
            var tableSize = assets.Count * entrySize;
            var tableOffset = headerSize;

            var manifestBytes = Encoding.UTF8.GetBytes(packageName);
            var manifestOffset = Align64(tableOffset + tableSize);
            var manifestSize = manifestBytes.Length;
            var dataOffset = Align64(manifestOffset + manifestSize);

            /* 计算数据总大小 */
            long totalDataSize = 0;
            foreach (var (_, _, data) in assets)
                totalDataSize += Align64(data.Length);

            /* 构建 */
            using var ms = new MemoryStream();
            using var w = new BinaryWriter(ms, Encoding.UTF8, leaveOpen: true);

            /* Header (64 bytes) */
            w.Write(0x4E4E504Bu);          /* 'NNPK' */
            w.Write(1u);                   /* version */
            w.Write(assets.Count);         /* assetCount */
            w.Write(0u);                   /* flags */
            w.Write((long)tableOffset);    /* tableOffset */
            w.Write((long)tableSize);      /* tableSize */
            w.Write((long)manifestOffset); /* manifestOffset */
            w.Write((long)manifestSize);   /* manifestSize */
            w.Write((long)dataOffset);     /* dataOffset */
            w.Write(totalDataSize);        /* totalDataSize */
            /* reserved */
            for (int i = 0; i < 8; i++) w.Write((byte)0);

            /* 对齐到 table */
            PadTo(w, tableOffset);

            /* AssetTable */
            long currentOffset = 0;
            foreach (var (guid, typeId, data) in assets)
            {
                w.Write(guid.High);
                w.Write(guid.Low);
                w.Write(typeId);
                w.Write(currentOffset);       /* offset */
                w.Write((long)data.Length);   /* size */
                w.Write(0L);                  /* compressedSize (未压缩) */
                w.Write(0u);                  /* flags */
                w.Write(0u);                  /* _pad */
                currentOffset += Align64(data.Length);
            }

            /* 对齐到 Manifest */
            PadTo(w, manifestOffset);

            /* Manifest */
            w.Write(manifestBytes);

            /* 对齐到 Data */
            PadTo(w, dataOffset);

            /* Asset Data */
            foreach (var (_, _, data) in assets)
            {
                w.Write(data);
                var aligned = Align64(data.Length);
                var pad = aligned - data.Length;
                for (int p = 0; p < pad; p++) w.Write((byte)0);
            }

            File.WriteAllBytes(outputPath, ms.ToArray());
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[PackageBuilder] 构建失败: {outputPath} → {ex.Message}");
            return false;
        }
    }

    private static long Align64(long offset) => (offset + 63) & ~63L;
    private static int Align64(int size) => (size + 63) & ~63;

    private static void PadTo(BinaryWriter w, long target)
    {
        while (w.BaseStream.Position < target)
            w.Write((byte)0);
    }
}
